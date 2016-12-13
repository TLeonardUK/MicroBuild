/*
MicroBuild
Copyright (C) 2016 TwinDrills

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "PCH.h"

#include "App/Builder/Builder.h"
#include "App/Builder/BuilderFileInfo.h"

#include "App/Builder/Toolchains/Toolchain.h"
#include "App/Builder/Toolchains/Cpp/Clang/Toolchain_Clang.h"
#include "App/Builder/Toolchains/Cpp/Gcc/Toolchain_Gcc.h"
#include "App/Builder/Toolchains/Cpp/Microsoft/Toolchain_Microsoft.h"
#include "App/Builder/Toolchains/Cpp/XCode/Toolchain_XCode.h"
#include "App/Builder/Toolchains/Cpp/Emscripten/Toolchain_Emscripten.h"
#include "App/Builder/Toolchains/Cpp/AndroidNdk/Toolchain_AndroidNdk.h"
#include "App/Builder/Toolchains/Cpp/Playstation3/Toolchain_Playstation3.h"
#include "App/Builder/Toolchains/Cpp/Playstation4/Toolchain_Playstation4.h"
#include "App/Builder/Toolchains/Cpp/PlaystationVita/Toolchain_PlaystationVita.h"
#include "App/Builder/Toolchains/Cpp/Xbox360/Toolchain_Xbox360.h"
#include "App/Builder/Toolchains/Cpp/XboxOne/Toolchain_XboxOne.h"
#include "App/Builder/Toolchains/Cpp/Nintendo3ds/Toolchain_Nintendo3ds.h"
#include "App/Builder/Toolchains/Cpp/NintendoWiiU/Toolchain_NintendoWiiU.h"

#include "App/Builder/Toolchains/CSharp/DotNet/Toolchain_DotNet.h"
#include "App/Builder/Toolchains/CSharp/Mono/Toolchain_Mono.h"

#include "App/Builder/Tasks/ArchiveTask.h"
#include "App/Builder/Tasks/CompileTask.h"
#include "App/Builder/Tasks/CompilePchTask.h"
#include "App/Builder/Tasks/LinkTask.h"
#include "App/Builder/Tasks/ShellCommandTask.h"

#include "Schemas/BuildManifest/BuildManifestFile.h"
#include "Schemas/Plugin/PluginInterface.h"

#include "Core/Parallel/Jobs/JobScheduler.h"

namespace MicroBuild {

Builder::Builder(App* app)
	: m_app(app)
{
}

Builder::~Builder()
{
}

bool Builder::Clean(WorkspaceFile& workspaceFile, ProjectFile& project)
{
	MB_UNUSED_PARAMETER(workspaceFile);
	MB_UNUSED_PARAMETER(project);

	Log(LogSeverity::SilentInfo, "Cleaning: %s (%s_%s)\n", 
		project.Get_Project_Name().c_str(), 
		project.Get_Target_Configuration().c_str(), 
		CastToString(project.Get_Target_Platform()).c_str()
	);

	Platform::Path intermediateDirectory = project.Get_Project_IntermediateDirectory();
	if (intermediateDirectory.Exists() && !intermediateDirectory.Delete())
	{
		Log(LogSeverity::Fatal, "Failed to delete intermediate directory '%s'.\n", intermediateDirectory.ToString().c_str());
		return false;
	} 

	Platform::Path outPath = project.Get_Project_OutputDirectory()
		.AppendFragment(Strings::Format("%s%s", project.Get_Project_OutputName().c_str(), project.Get_Project_OutputExtension().c_str()), true);

	if (outPath.Exists())
	{
		outPath.Delete();
	}

	// todo: abstract into ToolChain::Clean method
	Platform::Path pdbPath = project.Get_Project_OutputDirectory()
		.AppendFragment(Strings::Format("%s.pdb", project.Get_Project_OutputName().c_str()), true);

	if (pdbPath.Exists())
	{
		pdbPath.Delete();
	}

	return true;
}

JobHandle Builder::QueueTask(
	JobScheduler& scheduler, 
	JobHandle& groupJob, 
	JobHandle* startAfterJob, 
	std::shared_ptr<BuildTask> task,
	bool& bFailureFlag,
	int* totalJobs,
	std::atomic<int>* currentJobIndex)
{
	if (totalJobs != nullptr)
	{
		(*totalJobs)++;
	}

	JobHandle handle = scheduler.CreateJob([&scheduler, &bFailureFlag, task, totalJobs, currentJobIndex]() {
		if (bFailureFlag)
		{
			return;
		}
		if (task->ShouldGiveJobIndex())
		{
			int jobIndex = -2;
			int totalJobCount = -1;
			if (currentJobIndex != nullptr)
			{
				jobIndex = ((*currentJobIndex)++);
				totalJobCount = *totalJobs;
			}
			task->SetTaskProgress(jobIndex + 1, totalJobCount);
		}
		task->GetTaskThreadId(scheduler.GetThreadId());
		if (!task->Execute())
		{
			bFailureFlag = true;
		}
	});

	if (startAfterJob)
	{
		scheduler.AddDependency(handle, *startAfterJob);
	}
	scheduler.AddDependency(groupJob, handle);

	return handle;
}

bool Builder::Build(WorkspaceFile& workspaceFile, ProjectFile& project, bool bRebuild)
{	
	if (bRebuild)
	{
		if (!Clean(workspaceFile, project))
		{
			return false;
		}
	}
	
	auto startTime = std::chrono::high_resolution_clock::now();

	Log(LogSeverity::SilentInfo, "Building: %s (%s_%s), on %i threads\n", 
		project.Get_Project_Name().c_str(), 
		project.Get_Target_Configuration().c_str(), 
		CastToString(project.Get_Target_Platform()).c_str(),
		Platform::GetConcurrencyFactor()
	);

	// The configuration hash is used to figure out if configuration changes should
	// require file rebuilds.
	uint64_t configurationHash = 0;
	configurationHash = Strings::Hash64(project.Get_Target_Configuration(), configurationHash);
	configurationHash = Strings::Hash64(CastToString(project.Get_Target_Platform()), configurationHash);
	configurationHash = Strings::Hash64(project.Get_Project_Location().ToString(), configurationHash);
	configurationHash = Strings::Hash64(Strings::Format("%llu", project.Get_Project_File().GetModifiedTime()), configurationHash);
	configurationHash = Strings::Hash64(Strings::Format("%llu", workspaceFile.Get_Workspace_File().GetModifiedTime()), configurationHash);

	// Make sure output directories exists.
	Platform::Path outDir = project.Get_Project_OutputDirectory();
	Platform::Path intDir = project.Get_Project_IntermediateDirectory();
	if (!outDir.Exists() && !outDir.CreateAsDirectory())
	{
		Log(LogSeverity::Fatal, "Failed to create output directory '%s'.\n", outDir.ToString().c_str());
		return false;
	}
	if (!intDir.Exists() && !intDir.CreateAsDirectory())
	{
		Log(LogSeverity::Fatal, "Failed to create intermediate directory '%s'.\n", intDir.ToString().c_str());
		return false;
	}

	// Load manifest.
	Platform::Path manifestPath = 
		project.Get_Project_IntermediateDirectory()
			.AppendFragment(project.Get_Project_Name() + ".manifest", true);

	BuildManifestFile manifest(manifestPath);
	if (manifestPath.Exists())
	{
		if (!manifest.Read())
		{
			Log(LogSeverity::Fatal, "Failed to load manifest file '%s', possibly corrupted? Try rebuilding.\n", manifestPath.ToString().c_str());
			return false;
		}
	}

	std::vector<Platform::Path> projectFiles =
		project.Get_Files_File();

	std::vector<Platform::Path> sourceFiles;

	for (Platform::Path& path : projectFiles)
	{
		if (path.IsSourceFile())
		{
			sourceFiles.push_back(path);
		}
	}

	// Find the toolchain we need to build.
	Toolchain* toolchain = GetToolchain(project, configurationHash);
	if (!toolchain)
	{
		Log(LogSeverity::Fatal, "No toolchain available to compile '%s'.\n", project.Get_Project_Name().c_str());
		return false;
	}

	if (!toolchain->Init())
	{
		Log(LogSeverity::Fatal, "Toolchain not available to compile '%s', are you sure its installed?\nIf it is installed and in a non-default dictionary, please make sure its findable through the PATH environment variable.", project.Get_Project_Name().c_str());
		return false;
	}

	Log(LogSeverity::SilentInfo, "Toolchain: %s\n", toolchain->GetDescription().c_str());
	Log(LogSeverity::SilentInfo, "\n");

	// Setup scheduler and create main task to parent all build tasks to.
	JobScheduler scheduler(Platform::GetConcurrencyFactor());
	bool bBuildFailed = false;
	JobHandle previousCommandHandle;

	// Run the pre-build commands syncronously in case they update plugin source state.
	for (auto& command : project.Get_PreBuildCommands_Command())
	{
		std::shared_ptr<ShellCommandTask> task = std::make_shared<ShellCommandTask>(BuildStage::PreBuildUser, command);
		if (!task->Execute())
		{
			bBuildFailed = true;
			break;
		}
	}	
					
	// Grab any source files that plugins need to add.
	{
		PluginIbtPopulateCompileFilesData eventData;
		eventData.File = &project;
		eventData.SourceFiles = &sourceFiles;
		m_app->GetPluginManager()->OnEvent(EPluginEvent::IbtPopulateCompileFiles, &eventData);
	}

	// Determine what files are out of date.
	Platform::Path outputDir = project.Get_Project_IntermediateDirectory();
	Platform::Path rootDir;
	
	Platform::Path::GetCommonPath(sourceFiles, rootDir);

	std::vector<BuilderFileInfo> fileInfos = BuilderFileInfo::GetMultipleFileInfos(
		sourceFiles,	
		rootDir, 
		outputDir,
		configurationHash,
		!toolchain->RequiresCompileStep()
	);
	
	bool bUpToDate = true;
	
	for (auto iter = fileInfos.begin(); iter != fileInfos.end(); iter++)
	{
		BuilderFileInfo& file = *iter;
		if (file.bOutOfDate)
		{
			bUpToDate = false;
			break;
		}
	}

	// Check the output manifest in case linked libraries etc have changed.
	BuilderFileInfo outputFile;
	outputFile.SourcePath			= "";
	outputFile.OutputPath			= project.Get_Project_OutputDirectory().AppendFragment(Strings::Format("%s%s", project.Get_Project_OutputName().c_str(), project.Get_Project_OutputExtension().c_str()), true);
	outputFile.ManifestPath			= project.Get_Project_IntermediateDirectory().AppendFragment(Strings::Format("%s.target.build.manifest", project.Get_Project_Name().c_str()), true);
	outputFile.Hash					= 0;
	outputFile.bOutOfDate			= BuilderFileInfo::CheckOutOfDate(outputFile, configurationHash, false);

	if (outputFile.bOutOfDate)
	{
		bUpToDate = false;
	}

	// If its a container, just 
	if (project.Get_Project_OutputType() == EOutputType::Container)
	{
		Log(LogSeverity::SilentInfo, "Project is container, skipping.\n");
		return true;
	}
	else if (bUpToDate)
	{
		Log(LogSeverity::SilentInfo, "Project up to date.\n");
		return true;
	}
	else
	{
		std::atomic<int> currentJobIndex(0);
		int totalJobs = 0;

		// We have one global job that each build stage is a child of.		
		JobHandle hostJob = scheduler.CreateJob();

		// Create host jobs for each build stage.
		std::vector<JobHandle> buildStageHostJobs;
		for (int i = 0; i < (int)BuildStage::COUNT; i++)
		{
			JobHandle job = scheduler.CreateJob();

			// Host job is always dependent on the previous one executing.
			if (i > 0)
			{
				scheduler.AddDependency(job, buildStageHostJobs[i - 1]);
			}

			scheduler.AddDependency(hostJob, job);

			buildStageHostJobs.push_back(job);
		}

		// Gets all the tasks required to build the project.
		std::vector<std::shared_ptr<BuildTask>> tasks = toolchain->GetTasks(fileInfos, configurationHash, outputFile);

		// Register individual tasks for each build stage.
		for (int i = 0; i < (int)BuildStage::COUNT; i++)
		{
			JobHandle stageJob = buildStageHostJobs[i];

			std::vector<std::shared_ptr<BuildTask>> parallelTasks;
			std::vector<std::shared_ptr<BuildTask>> sequentialTasks;

			for (auto& task : tasks)
			{
				if (task->GetBuildState() == (BuildStage)i)
				{
					if (task->CanRunInParallel())
					{
						parallelTasks.push_back(task);
					}
					else
					{
						sequentialTasks.push_back(task);
					}
				}
			}

			// Register parallel tasks first.
			JobHandle parallelGorupJob = scheduler.CreateJob();
			if (i > 0)
			{
				scheduler.AddDependency(parallelGorupJob, buildStageHostJobs[i - 1]);
			}
			scheduler.AddDependency(stageJob, parallelGorupJob);

			for (auto& task : parallelTasks)
			{
				JobHandle* parentJob = nullptr;
				if (i > 0)
				{
					parentJob = &buildStageHostJobs[i - 1];
				}

				QueueTask(
					scheduler, 
					parallelGorupJob, 
					parentJob, 
					task, 
					bBuildFailed, 
					&totalJobs, 
					&currentJobIndex
				);
			}

			// Register sequential tasks after.
			JobHandle previousSequentialTask;

			for (auto& task : sequentialTasks)
			{
				JobHandle* parentJob = nullptr;
				if (i > 0)
				{
					parentJob = &buildStageHostJobs[i - 1];
				}

				JobHandle taskJobHandle = QueueTask(
					scheduler, 
					stageJob, 
					parentJob, 
					task, 
					bBuildFailed, 
					&totalJobs, 
					&currentJobIndex
				);

				// Dependent on all parallel tasks finishing.
				scheduler.AddDependency(taskJobHandle, parallelGorupJob);

				// Also dependent on the previous task finishing.
				if (previousSequentialTask.IsValid())
				{
					scheduler.AddDependency(taskJobHandle, previousSequentialTask);
				}

				previousSequentialTask = taskJobHandle;
			}
		}

		//scheduler.PrintJobTree();
		scheduler.Enqueue(hostJob);
		scheduler.Wait(hostJob);

		if (bBuildFailed)
		{
			Log(LogSeverity::Fatal, "Build of '%s' failed.\n", project.Get_Project_Name().c_str());
			return false;
		}

		// Save out the new manifest state.
		if (!manifest.Write())
		{
			Log(LogSeverity::Fatal, "Failed to write manifest file '%s'.\n", manifestPath.ToString().c_str());
			return false;
		}
			
		auto elapsedTime = std::chrono::high_resolution_clock::now() - startTime;
		auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count();

		Log(LogSeverity::SilentInfo, "\n");
		Log(LogSeverity::SilentInfo, "Completed in %.1f seconds\n", elapsedMs / 1000.0f);
	}
	
	return true;
}

Toolchain* Builder::GetToolchain(ProjectFile& project, uint64_t configurationHash)
{
	switch (project.Get_Project_Language())
	{
	case ELanguage::Cpp:
		{
			switch (project.Get_Target_Platform())
			{

			// Desktop Platforms
			case EPlatform::x86:
			case EPlatform::x64:
			case EPlatform::ARM:
			case EPlatform::ARM64:
				{								
					if (project.Get_Build_PlatformToolset() == EPlatformToolset::Clang)
					{
						return new Toolchain_Clang(project, configurationHash);
					}
					else if (project.Get_Build_PlatformToolset() == EPlatformToolset::GCC)
					{
						return new Toolchain_Gcc(project, configurationHash);
					}
#if defined(MB_PLATFORM_WINDOWS)
					else if (project.Get_Build_PlatformToolset() == EPlatformToolset::MSBuild_v140)
					{
						return new Toolchain_Microsoft(project, configurationHash);
					}
#endif
#if defined(MB_PLATFORM_MACOS)
					else if (project.Get_Build_PlatformToolset() == EPlatformToolset::XCode)
					{
						return new Toolchain_XCode(project, configurationHash);
					}
#endif
					else if (project.Get_Build_PlatformToolset() == EPlatformToolset::Default)
					{							
#if defined(MB_PLATFORM_WINDOWS) 
						return new Toolchain_Microsoft(project, configurationHash);
#elif defined(MB_PLATFORM_LINUX) 
						return new Toolchain_Clang(project, configurationHash);
#elif defined(MB_PLATFORM_MACOS)
						return new Toolchain_XCode(project, configurationHash);
#endif
					}
					break;
				}
			
			case EPlatform::WinRT_x86:
			case EPlatform::WinRT_x64:
			case EPlatform::WinRT_ARM:
			case EPlatform::WinRT_ARM64:
				{						
#if defined(MB_PLATFORM_WINDOWS)
					return new Toolchain_Microsoft(project, configurationHash);
#else
					break;
#endif
				}

			// Web Platforms			
			case EPlatform::HTML5:
				{
					return new Toolchain_Emscripten(project, configurationHash);
				}

			// Mobile Platforms
			case EPlatform::iOS:
				{					
#if defined(MB_PLATFORM_WINDOWS)
					return new Toolchain_XCode(project, configurationHash);
#else
					break;
#endif
				}

			case EPlatform::Android_ARM:
			case EPlatform::Android_ARM64:
			case EPlatform::Android_x86:
			case EPlatform::Android_x64:
			case EPlatform::Android_MIPS:
			case EPlatform::Android_MIPS64:
				{
					return new Toolchain_AndroidNdk(project, configurationHash);
				}

			// Console Platforms
			case EPlatform::PS3:
				{		
#if defined(MB_PLATFORM_WINDOWS)
					return new Toolchain_Playstation3(project, configurationHash);
#else
					break;
#endif
				}
			case EPlatform::PS4:
				{
#if defined(MB_PLATFORM_WINDOWS)
					return new Toolchain_Playstation4(project, configurationHash);
#else
					break;
#endif
				}
			case EPlatform::PSVita:
				{
#if defined(MB_PLATFORM_WINDOWS)
					return new Toolchain_PlaystationVita(project, configurationHash);
#else
					break;
#endif
				}
			case EPlatform::Xbox360:
				{
#if defined(MB_PLATFORM_WINDOWS)
					return new Toolchain_Xbox360(project, configurationHash);
#else
					break;
#endif
				}
			case EPlatform::XboxOne:
				{
#if defined(MB_PLATFORM_WINDOWS)
					return new Toolchain_XboxOne(project, configurationHash);
#else
					break;
#endif
				}
			case EPlatform::NintendoWiiU:
				{
#if defined(MB_PLATFORM_WINDOWS)
					return new Toolchain_NintendoWiiU(project, configurationHash);
#else
					break;
#endif
				}
			case EPlatform::Nintendo3DS:
				{
#if defined(MB_PLATFORM_WINDOWS)
					return new Toolchain_Nintendo3ds(project, configurationHash);
#else
					break;
#endif
				}

			// MacOS Bundles
			case EPlatform::Native:
			case EPlatform::Universal86:
			case EPlatform::Universal64:
			case EPlatform::Universal:
				{
#if defined(MB_PLATFORM_MACOS)
					return new Toolchain_XCode(project, configurationHash);
#endif
					break;
				}

			default:
				{
					return nullptr;
				}
			}		
			break;
		}
	case ELanguage::CSharp:
		{
			switch (project.Get_Target_Platform())
			{
			// General platforms
			case EPlatform::x86:
			case EPlatform::x64:
			case EPlatform::ARM:
			case EPlatform::ARM64:
			case EPlatform::AnyCPU:
				{		
					if (project.Get_Build_PlatformToolset() == EPlatformToolset::DotNet_2_0 ||
						project.Get_Build_PlatformToolset() == EPlatformToolset::DotNet_3_0 ||
						project.Get_Build_PlatformToolset() == EPlatformToolset::DotNet_3_5 ||
						project.Get_Build_PlatformToolset() == EPlatformToolset::DotNet_4_0 ||
						project.Get_Build_PlatformToolset() == EPlatformToolset::DotNet_4_5 ||
						project.Get_Build_PlatformToolset() == EPlatformToolset::DotNet_4_5_1 ||
						project.Get_Build_PlatformToolset() == EPlatformToolset::DotNet_4_5_2 ||
						project.Get_Build_PlatformToolset() == EPlatformToolset::DotNet_4_6)
					{
						return new Toolchain_DotNet(project, configurationHash);
					}
					else if (project.Get_Build_PlatformToolset() == EPlatformToolset::Mono)
					{
						return new Toolchain_Mono(project, configurationHash);
					}
					else if (project.Get_Build_PlatformToolset() == EPlatformToolset::Default)
					{				
#if defined(MB_PLATFORM_WINDOWS)
						return new Toolchain_DotNet(project, configurationHash);
#elif defined(MB_PLATFORM_LINUX) || defined(MB_PLATFORM_MACOS)
						return new Toolchain_Mono(project, configurationHash);
#endif
					}
					break;
				}
			default:
				{
					return nullptr;
				}
			}		
			break;
		default:
			{
				return nullptr;
			}
		}
	}

	return nullptr;
}

}; // namespace MicroBuild
