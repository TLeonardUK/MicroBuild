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
#include "App/Builder/Toolchains/Toolchain_Clang.h"
#include "App/Builder/Toolchains/Toolchain_DotNet.h"
#include "App/Builder/Toolchains/Toolchain_Gcc.h"
#include "App/Builder/Toolchains/Toolchain_Mono.h"
#include "App/Builder/Toolchains/Toolchain_Microsoft.h"
#include "App/Builder/Toolchains/Toolchain_XCode.h"
#include "App/Builder/Toolchains/Toolchain_Emscripten.h"

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
	: m_project(nullptr)
	, m_bRebuild(false)
	, m_app(app)
{
}

Builder::~Builder()
{
}

bool Builder::Clean(ProjectFile& project)
{
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
		int jobIndex = -2;
		int totalJobCount = -1;
		if (currentJobIndex != nullptr)
		{
			jobIndex = ((*currentJobIndex)++);
			totalJobCount = *totalJobs;
		}
		task->SetTaskProgress(jobIndex + 1, totalJobCount);
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

bool Builder::Build(ProjectFile& project, bool bRebuild)
{	
	if (bRebuild)
	{
		if (!Clean(project))
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
	Toolchain* toolchain = GetToolchain(project);
	if (!toolchain)
	{
		Log(LogSeverity::Fatal, "No toolchain available to compile '%s'.\n", project.Get_Project_Name().c_str());
		return false;
	}
	if (!toolchain->IsAvailable())
	{
		Log(LogSeverity::Fatal, "Toolchain not available to compile '%s', are you sure its installed?\n", project.Get_Project_Name().c_str());
		return false;
	}

	Log(LogSeverity::SilentInfo, "Toolchain: %s\n", toolchain->GetDescription().c_str());
	Log(LogSeverity::SilentInfo, "\n");

	// Setup scheduler and create main task to parent all build tasks to.
	JobScheduler scheduler(Platform::GetConcurrencyFactor());
	JobHandle preCompileJob = scheduler.CreateJob();
	JobHandle pchCompileJob = scheduler.CreateJob();
	JobHandle compileJob = scheduler.CreateJob();
	JobHandle preLinkJob = scheduler.CreateJob();
	JobHandle linkJob = scheduler.CreateJob();
	JobHandle postBuildJob = scheduler.CreateJob();
	bool bBuildFailed = false;
	JobHandle previousCommandHandle;

	// Run the pre-build commands syncronously in case they update plugin source state.
	for (auto& command : project.Get_PreBuildCommands_Command())
	{
		std::shared_ptr<ShellCommandTask> task = std::make_shared<ShellCommandTask>(command);
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
		outputDir
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
	outputFile.bOutOfDate			= BuilderFileInfo::CheckOutOfDate(outputFile);

	if (outputFile.bOutOfDate)
	{
		bUpToDate = false;
	}

	// If its a container, just 
	if (project.Get_Project_OutputType() == EOutputType::Container)
	{
		Log(LogSeverity::SilentInfo, "Project is container, skipping");
		return true;
	}
	else if (bUpToDate)
	{
		Log(LogSeverity::SilentInfo, "Project up to date.");
		return true;
	}
	else
	{
		std::atomic<int> currentJobIndex = 0;
		int totalJobs = 0;

		// Skip all individual builds if we are not multi-pass.
		if (toolchain->RequiresCompileStep())
		{
			// Do we have a precompiled header? If so compile that first.
			Platform::Path precompiledSourcePath = project.Get_Build_PrecompiledSource();
			BuilderFileInfo precompiledSourceFile;
			bool bPrecompiledSourceFileFound = false;
			for (auto iter = fileInfos.begin(); iter != fileInfos.end(); iter++)
			{
				BuilderFileInfo& file = *iter;
				if (file.SourcePath == precompiledSourcePath)
				{
					// Remove precompiled source file from the list as we want to compile it seperately.
					bPrecompiledSourceFileFound = true;
					precompiledSourceFile = file;
					fileInfos.erase(iter);
					break;
				}
			}

			if (bPrecompiledSourceFileFound)
			{
				if (precompiledSourceFile.bOutOfDate)
				{
					std::shared_ptr<CompilePchTask> task = std::make_shared<CompilePchTask>(toolchain, project, precompiledSourceFile);
					QueueTask(scheduler, pchCompileJob, &preCompileJob, task, bBuildFailed, &totalJobs, &currentJobIndex);
				}
			}

			// General build tasks for each translation unit.
			for (auto& file : fileInfos)
			{
				if (file.bOutOfDate)
				{
					std::shared_ptr<CompileTask> task = std::make_shared<CompileTask>(toolchain, project, file, precompiledSourceFile);
					QueueTask(scheduler, compileJob, &pchCompileJob, task, bBuildFailed, &totalJobs, &currentJobIndex);
				}
			}
		}
	
		previousCommandHandle = compileJob;
		for (auto& command : project.Get_PreLinkCommands_Command())
		{
			std::shared_ptr<ShellCommandTask> task = std::make_shared<ShellCommandTask>(command);
			previousCommandHandle = QueueTask(scheduler, preLinkJob, &previousCommandHandle, task, bBuildFailed, nullptr, nullptr);
		}

		// Generate a final link task for all the object files.
		if (project.Get_Project_OutputType() == EOutputType::StaticLib)
		{
			std::shared_ptr<ArchiveTask> archiveTask = std::make_shared<ArchiveTask>(fileInfos, toolchain, project, outputFile);
			QueueTask(scheduler, linkJob, &preLinkJob, archiveTask, bBuildFailed, &totalJobs, &currentJobIndex);
		}
		else
		{
			std::shared_ptr<LinkTask> linkTask = std::make_shared<LinkTask>(fileInfos, toolchain, project, outputFile);
			QueueTask(scheduler, linkJob, &preLinkJob, linkTask, bBuildFailed, &totalJobs, &currentJobIndex);
		}

		// Queue any postbuild commands.
		previousCommandHandle = linkJob;
		for (auto& command : project.Get_PostBuildCommands_Command())
		{
			std::shared_ptr<ShellCommandTask> task = std::make_shared<ShellCommandTask>(command);
			previousCommandHandle = QueueTask(scheduler, postBuildJob, &previousCommandHandle, task, bBuildFailed, nullptr, nullptr);
		}

		// Chain tasks together, enqueue and wait for result!
		JobHandle hostJob = scheduler.CreateJob();
		scheduler.AddDependency(hostJob, preCompileJob);
		scheduler.AddDependency(hostJob, pchCompileJob);
		scheduler.AddDependency(hostJob, compileJob);
		scheduler.AddDependency(hostJob, preLinkJob);
		scheduler.AddDependency(hostJob, linkJob);
		scheduler.AddDependency(hostJob, postBuildJob);

		scheduler.AddDependency(pchCompileJob, preCompileJob);
		scheduler.AddDependency(compileJob, pchCompileJob);
		scheduler.AddDependency(preLinkJob, compileJob);
		scheduler.AddDependency(linkJob, preLinkJob);
		scheduler.AddDependency(postBuildJob, linkJob);

		scheduler.Enqueue(hostJob);
		scheduler.Wait(hostJob);

		if (bBuildFailed)
		{
			Log(LogSeverity::Fatal, "Build of '%s' failed.", project.Get_Project_Name().c_str());
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

Toolchain* Builder::GetToolchain(ProjectFile& project)
{
	switch (project.Get_Project_Language())
	{
	case ELanguage::Cpp:
		{
			switch (project.Get_Target_Platform())
			{
			case EPlatform::x86:
			case EPlatform::x64:
			case EPlatform::ARM:
			case EPlatform::ARM64:
				{				
#if defined(MB_PLATFORM_WINDOWS)
					return new Toolchain_Microsoft(project);
#elif defined(MB_PLATFORM_LINUX) 
					if (project.Get_Build_PlatformToolset() == EPlatformToolset::Clang)
					{
						return new Toolchain_Clang(project);
					}
					else
					{
						return new Toolchain_Gcc(project);
					}
#elif defined(MB_PLATFORM_MACOS)
					return new Toolchain_XCode(project);
#endif
					break;
				}

			case EPlatform::Native:
			case EPlatform::Universal86:
			case EPlatform::Universal64:
			case EPlatform::Universal:
				{
#if defined(MB_PLATFORM_MACOS)
					return new Toolchain_XCode(project);
#endif
					break;
				}

			case EPlatform::HTML5:
				{
					return new Toolchain_Emscripten(project);
					break;
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
#if defined(MB_PLATFORM_WINDOWS)
					return new Toolchain_DotNet(project);
#elif defined(MB_PLATFORM_LINUX) || defined(MB_PLATFORM_MACOS)
					return new Toolchain_Mono(project);
#endif
					break;
				}
			}		
			break;
		}
	}

	return nullptr;
}

}; // namespace MicroBuild