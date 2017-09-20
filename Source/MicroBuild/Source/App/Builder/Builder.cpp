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

#include "App/Builder/Accelerators/Accelerator.h"
#include "App/Builder/Accelerators/Sndbs/Accelerator_Sndbs.h"
#include "App/Builder/Accelerators/IncrediBuild/Accelerator_IncrediBuild.h"

#include "App/Builder/Tasks/AccelerateTask.h"
#include "App/Builder/Tasks/ArchiveTask.h"
#include "App/Builder/Tasks/CompileTask.h"
#include "App/Builder/Tasks/CompilePchTask.h"
#include "App/Builder/Tasks/LinkTask.h"
#include "App/Builder/Tasks/ShellCommandTask.h"

#include "App/Builder/SourceControl/Providers/GitSourceControlProvider.h"

#include "App/Plugin/PluginManager.h"
#include "App/Plugin/Plugin.h"

#include "Schemas/BuildManifest/BuildManifestFile.h"
#include "Schemas/Plugin/PluginInterface.h"

#include "Core/Parallel/Jobs/JobScheduler.h"
#include "Core/Helpers/TextStream.h"

#include <algorithm>

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
	std::vector<std::string> cleanExtensions = { "exp", "lib", "pdb", "dll", "exe", "ilk" };	
	for (std::string& ext : cleanExtensions)
	{
		Platform::Path pdbPath = project.Get_Project_OutputDirectory()
			.AppendFragment(Strings::Format("%s.%s", project.Get_Project_OutputName().c_str(), ext.c_str()), true);

		if (pdbPath.Exists())
		{
			pdbPath.Delete();
		}
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
		(*totalJobs) += task->GetSubTaskCount();
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
				jobIndex = ((*currentJobIndex) += task->GetSubTaskCount());
				totalJobCount = *totalJobs;
			}
			task->SetTaskProgress(jobIndex - task->GetSubTaskCount() + 1, totalJobCount);
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

void Builder::BuildDependencyList(WorkspaceFile& workspaceFile, std::vector<ProjectFile*> projectFiles, ProjectFile& project, std::vector<ProjectFile*>& dependencyList, std::vector<ProjectFile*>& processedList)
{
	if (std::find(processedList.begin(), processedList.end(), &project) != processedList.end())
	{
		return;
	}
	processedList.push_back(&project);

	std::vector<std::string> deps = project.Get_Dependencies_Dependency();

	for (auto& depName : deps)
	{
		for (auto& depProject : projectFiles)
		{
			if (depProject->Get_Project_Name() == depName)
			{
				BuildDependencyList(workspaceFile, projectFiles, *depProject, dependencyList, processedList);
			}
		}
	}

	dependencyList.push_back(&project);
}

void Builder::BuildDependencyTree(bool& bFailed, JobScheduler& scheduler, JobHandle& rootHandle, WorkspaceFile& workspaceFile, std::vector<ProjectFile*> projectFiles, ProjectFile& project, bool bRebuild, bool bBuildPackageFiles, std::vector<ProjectFile*>& processedList)
{
	if (std::find(processedList.begin(), processedList.end(), &project) != processedList.end())
	{
		return;
	}
	processedList.push_back(&project);

	std::vector<std::string> deps = project.Get_Dependencies_Dependency();

	JobHandle job = scheduler.CreateJob([=, &bFailed]() {
		if (bFailed)
		{
			return;
		}
		WorkspaceFile wks = workspaceFile;
		ProjectFile prj = project;
		if (!Build(wks, projectFiles, prj, bRebuild, false, bBuildPackageFiles))
		{
			bFailed = true;
		}
	});
	scheduler.AddDependency(rootHandle, job);

	for (auto& depName : deps)
	{
		for (auto& depProject : projectFiles)
		{
			if (depProject->Get_Project_Name() == depName)
			{
				BuildDependencyTree(bFailed, scheduler, job, workspaceFile, projectFiles, *depProject, bRebuild, bBuildPackageFiles, processedList);
			}
		}
	}
}


bool Builder::GetSourceControlProvider(ProjectFile& project, std::shared_ptr<ISourceControlProvider>& provider)
{
	switch (project.Get_SourceControl_Type())
	{
	case ESourceControlType::Git:
		{
			provider = std::make_shared<GitSourceControlProvider>();
			break;
		}
	case ESourceControlType::None:
		{
			Log(LogSeverity::Fatal, "Attempt to use source control, but no source control settings configured.\n");
			return false;
		}
	}

	Platform::Path rootDirectory = project.Get_SourceControl_Root();
	if (!rootDirectory.Exists())
	{
		Log(LogSeverity::Fatal, "Source control root does not exist: %s\n", rootDirectory.ToString().c_str());
		return false;
	}

	if (!provider->Connect(rootDirectory))
	{
		Log(LogSeverity::Fatal, "Failed to connect to source control.\n");
		return false;
	}

	return true;
}

bool Builder::WriteVersionNumberSource(ProjectFile& project, Platform::Path& hppPath, Platform::Path cppPath, VersionNumberInfo& info)
{
	MB_UNUSED_PARAMETER(project);
	
	struct VersionEntry
	{
		std::string Name;
		std::string Type;
		std::string Value;
	};

	struct tm* utcTime = gmtime(&info.LastChangeTime);

	std::vector<VersionEntry> entries;
	entries.push_back({ "DAY",					"char*",	CastToString(utcTime->tm_mday)			});
	entries.push_back({ "MONTH",				"char*",	CastToString(utcTime->tm_mon + 1)		});
	entries.push_back({ "YEAR",					"char*",	CastToString(utcTime->tm_year + 1900)	});
	entries.push_back({ "HOUR",					"char*",	CastToString(utcTime->tm_hour)			});
	entries.push_back({ "MINUTE",				"char*",	CastToString(utcTime->tm_min)			});
	entries.push_back({ "SECOND",				"char*",	CastToString(utcTime->tm_sec)			});
	entries.push_back({ "CHANGELIST",			"char*",	info.Changelist							});
	entries.push_back({ "FULLVERSION_STRING",	"char*",	info.ShortString						});
	entries.push_back({ "TOTAL_CHANGELISTS",	"long",		CastToString(info.TotalChangelists)		});

	TextStream header;
	header.WriteLine("// Automatically generated, do not modify directly.");
	header.WriteLine("// Generated by MicroBuild.");
	header.WriteLine("");
	header.WriteLine("#pragma once");
	header.WriteLine("");
	header.WriteLine("class VersionInfo");
	header.WriteLine("{");
	header.WriteLine("public:");
	header.Indent();
	for (auto& entry : entries)
	{
		header.WriteLine("static const %s %s;", entry.Type.c_str(), entry.Name.c_str());
	}
	header.Undent();
	header.WriteLine("};");

	TextStream source;
	source.WriteLine("// Automatically generated, do not modify directly.");
	source.WriteLine("// Generated by MicroBuild.");
	source.WriteLine("");
	source.WriteLine("#include \"%s\"", hppPath.GetFilename().c_str());
	source.WriteLine("");
	for (auto& entry : entries)
	{
		if (entry.Type == "char*")
		{
			source.WriteLine("const %s VersionInfo::%s = \"%s\";", entry.Type.c_str(), entry.Name.c_str(), entry.Value.c_str());
		}
		else
		{
			source.WriteLine("const %s VersionInfo::%s = %s;", entry.Type.c_str(), entry.Name.c_str(), entry.Value.c_str());
		}
	}

	if (!header.WriteToFile(hppPath, true))
	{
		Log(LogSeverity::Warning, "Failed to write to file '%s'.\n", hppPath.ToString().c_str());
		return false;
	}

	if (!source.WriteToFile(cppPath, true))
	{
		Log(LogSeverity::Warning, "Failed to write to file '%s'.\n", cppPath.ToString().c_str());
		return false;
	}

	return true;
}

bool Builder::CalculateVersionNumber(ProjectFile& project, VersionNumberInfo& info, std::shared_ptr<ISourceControlProvider> provider)
{
	switch (project.Get_ProductInfo_VersionSource())
	{
	case EVersionNumberSource::None:
		{
			info.Changelist = project.Get_ProductInfo_Version();
			info.LastChangeTime = std::time(nullptr);
			info.TotalChangelists = 0;
			break;
		}
	case EVersionNumberSource::SourceControl:
		{
			SourceControlChangelist changelist;

			if (!provider->GetChangelist(project.Get_SourceControl_Root(), changelist))
			{
				Log(LogSeverity::Fatal, "Failed to query source control for current changelist.\n");
				return false;
			}

			info.Changelist = changelist.Id;
			info.LastChangeTime = changelist.Date;

			if (!provider->GetTotalChangelists(info.TotalChangelists))
			{
				Log(LogSeverity::Fatal, "Failed to query source control for total changelists.\n");
				return false;
			}

			break;
		}
	}

	switch (project.Get_ProductInfo_VersionFormat())
	{
	case EVersionFormat::Changelist:
		{
			info.ShortString = info.Changelist;
			break;
		}
	default:
		{
			assert(false);
			break;
		}
	}

	// Dump version info into files if required.
	Platform::Path hppPath = project.Get_ProductInfo_VersionHpp();
	Platform::Path cppPath = project.Get_ProductInfo_VersionCpp();

	if (hppPath.Exists() && cppPath.Exists())
	{
		if (!WriteVersionNumberSource(project, hppPath, cppPath, info))
		{
			return false;		
		}
	}

	return true;
}

bool Builder::BuildChangelog(
	ProjectFile& project,
	VersionNumberInfo& info,
	std::shared_ptr<ISourceControlProvider> provider
)
{
	struct ChangelogGroup
	{
	std::string Version;
	std::vector<std::string> Commits;
	};


	int requestOffset = 0;
	int requestChunkSize = 100;

	std::string changelistTag = "[changelog]";
	std::string changelistLegacyTag = "[changelog-legacy]";
	std::string changelistVersionMessage = Strings::Format("[ChangeLog] Updated '%s' changelog to commit #", project.Get_Project_Name().c_str());

	std::string legacyText = "";

	std::vector<ChangelogGroup> groups;

	ChangelogGroup rootGroup;
	rootGroup.Version = info.Changelist;

	groups.push_back(rootGroup);

	while (true)
	{
		std::vector<SourceControlChangelist> history;

		if (!provider->GetHistory(project.Get_SourceControl_Root(), history, requestChunkSize, requestOffset))
		{
			Log(LogSeverity::Fatal, "Failed to query source control for changelist history.\n");
			return false;
		}
		requestOffset += requestChunkSize;

		for (auto& changelist : history)
		{
			// Changelog update.
			if (changelist.Description.substr(0, changelistVersionMessage.size()) == changelistVersionMessage)
			{
				ChangelogGroup group;
				group.Version = changelist.Description.substr(changelistVersionMessage.size());
				groups.push_back(group);
			}

			// General changelist commit.
			else
			{
				size_t offset = Strings::ToLowercase(changelist.Description).find(changelistTag);
				if (offset != std::string::npos)
				{
					std::string text = changelist.Description.substr(offset + changelistTag.size());
					text = Strings::Trim(text);

					std::vector<std::string> lines = Strings::Split('\n', text);

					ChangelogGroup& group = groups[groups.size() - 1];

					for (auto& line : lines)
					{
						group.Commits.push_back(line);
					}
				}
				else
				{
					offset = Strings::ToLowercase(changelist.Description).find(changelistLegacyTag);
					if (offset != std::string::npos)
					{
						std::string text = changelist.Description.substr(offset + changelistLegacyTag.size());
						legacyText += text;
					}
				}
			}
		}

		if (static_cast<int>(history.size()) < requestChunkSize)
		{
			break;
		}
	}

	for (auto& group : groups)
	{
		std::sort(group.Commits.begin(), group.Commits.end());
	}

	TextStream stream;
	stream.WriteLine("=================================================================================");
	stream.WriteLine(" Version History");
	stream.WriteLine("=================================================================================");
	stream.WriteLine(" This file is automatically generated from commit messages in the");
	stream.WriteLine(" source control server. Incorrect commit message formats may result ");
	stream.WriteLine(" in incorrect formatting or irrelevant entries.");
	stream.WriteLine("=================================================================================");

	for (auto& group : groups)
	{
		if (group.Commits.size() > 0)
		{
			stream.WriteNewLine();
			stream.WriteLine("---------------------------------------------------------------------------------");
			stream.WriteLine("  Version %s", group.Version.c_str());
			stream.WriteLine("---------------------------------------------------------------------------------");

			for (auto& commit : group.Commits)
			{
				stream.WriteLine("%s", commit.c_str());
			}
		}
	}

	if (legacyText.size() > 0)
	{
		stream.WriteNewLine();
		stream.WriteNewLine();
		stream.WriteLine("===== LEGACY VERSION INFO BELOW =================================================");
		stream.WriteNewLine();
		stream.WriteLine("%s", legacyText.c_str());
	}

	Platform::Path outputPath = project.Get_Packager_ChangelogFile();

	if (!stream.IsDifferentFromFile(outputPath))
	{
		Log(LogSeverity::Verbose, "Changelog file is up to date, ignoring.\n");
		return true;
	}

	if (project.Get_SourceControl_Type() != ESourceControlType::None)
	{
		bool bIsNewFile = false;

		if (!provider->Exists(outputPath, bIsNewFile))
		{
			Log(LogSeverity::Fatal, "Failed to check if file exists in source control: %s\n", outputPath.ToString().c_str());
			return false;
		}

		if (!bIsNewFile)
		{
			if (!provider->Checkout(outputPath))
			{
				Log(LogSeverity::Fatal, "Failed to checkout file from source control: %s\n", outputPath.ToString().c_str());
				return false;
			}
		}

		if (!stream.WriteToFile(outputPath, true))
		{
			Log(LogSeverity::Verbose, "Failed to write changelog file.");
			return true;
		}

		if (bIsNewFile)
		{
			if (!provider->Add(outputPath))
			{
				Log(LogSeverity::Fatal, "Failed to add file to source control: %s\n", outputPath.ToString().c_str());
				return false;
			}
		}

		if (!provider->Commit({ outputPath }, changelistVersionMessage + info.Changelist))
		{
			Log(LogSeverity::Fatal, "Failed to commit file to source control: %s\n", outputPath.ToString().c_str());
			return false;
		}

		if (!provider->Sync())
		{
			Log(LogSeverity::Fatal, "Failed to sync source control\n");
			return false;
		}
	}
	else
	{
		if (!stream.WriteToFile(outputPath, true))
		{
			Log(LogSeverity::Verbose, "Failed to write changelog file.\n");
			return true;
		}
	}

	return true;
}

bool Builder::Build(WorkspaceFile& workspaceFile, std::vector<ProjectFile*> projectFileInstances, ProjectFile& project, bool bRebuild, bool bBuildDependencies, bool bBuildPackageFiles)
{
	if (bBuildDependencies)
	{		
		if (workspaceFile.Get_Workspace_BuildProjectsInParallel())
		{
			// Run each dependency in parallel.
			bool bFailed = false;

			JobScheduler scheduler((int)projectFileInstances.size());

			JobHandle hostJob = scheduler.CreateJob();

			std::vector<ProjectFile*> processedList;
			BuildDependencyTree(bFailed, scheduler, hostJob, workspaceFile, projectFileInstances, project, bRebuild, bBuildPackageFiles, processedList);

			scheduler.Enqueue(hostJob);
			scheduler.Wait(hostJob);

			return !bFailed;
		}
		else
		{
			std::vector<ProjectFile*> dependencyList;
			std::vector<ProjectFile*> processedList;
			BuildDependencyList(workspaceFile, projectFileInstances, project, dependencyList, processedList);

			for (auto& depProject : dependencyList)
			{
				if (!Build(workspaceFile, projectFileInstances, *depProject, bRebuild, false, bBuildPackageFiles))
				{
					return false;
				}
			}

			return true;
		}
	}

	// Check we have required plugins.
	PluginManager* pluginManager = m_app->GetPluginManager();

	std::vector<std::string> requiredPlugins = project.Get_MicroBuild_RequiredPlugin();
	for (std::string pluginName : requiredPlugins)
	{
		if (!pluginManager->IsPluginLoaded(pluginName))
		{
			if (!pluginManager->LoadPluginByName(pluginName))
			{
				Log(LogSeverity::Fatal, "Required plugin '%s' is not loaded, are you sure it is built for the current configuration?\n", pluginName.c_str());
				return false;
			}
		}
	}

	// Unload any excluded plugins.
	std::vector<std::string> excludedPlugins = project.Get_MicroBuild_ExcludedPlugin();
	for (std::string pluginName : excludedPlugins)
	{
		pluginManager->UnloadPluginByName(pluginName);
	}

	if (bRebuild)
	{
		if (!Clean(workspaceFile, project))
		{
			return false;
		}
	}
	
	auto startTime = std::chrono::high_resolution_clock::now();

	Log(LogSeverity::Info, "%s: %s (%s_%s), on %i threads\n", 
		bRebuild ? "Rebuilding" : "Building",
		project.Get_Project_Name().c_str(), 
		project.Get_Target_Configuration().c_str(), 
		CastToString(project.Get_Target_Platform()).c_str(),
		Platform::GetConcurrencyFactor()
	);

	// Try and retrieve the build version if possible.
	VersionNumberInfo versionInfo;
	bool bVersionInfoRequired = false;

	if (bBuildPackageFiles && project.Get_Packager_BuildChangelog())
	{
		bVersionInfoRequired = true;
	}

	if (!project.Get_ProductInfo_VersionHpp().IsEmpty() ||
		!project.Get_ProductInfo_VersionCpp().IsEmpty())
	{
		bVersionInfoRequired = true;
	}

	if (!project.Get_ProductInfo_Name().empty())
	{
		if (project.Get_Project_OutputType() == EOutputType::ConsoleApp ||
			project.Get_Project_OutputType() == EOutputType::Executable)
		{
			bVersionInfoRequired = true;
		}
	}

	// Grab a source control provider if we are using one.
	std::shared_ptr<ISourceControlProvider> provider;

	if (project.Get_SourceControl_Type() != ESourceControlType::None)
	{
		if (bVersionInfoRequired || (bBuildPackageFiles && project.Get_Packager_BuildChangelog()))
		{
			if (!GetSourceControlProvider(project, provider))
			{
				return false;
			}
		}
	}

	if (bVersionInfoRequired)
	{
		if (!CalculateVersionNumber(project, versionInfo, provider))
		{
			return false;
		}
	}

	// Try and generate a changelog if requested.
	if (bBuildPackageFiles)
	{
		if (project.Get_Packager_BuildChangelog())
		{
			if (!BuildChangelog(project, versionInfo, provider))
			{
				return false;
			}
		}
	}

	// The configuration hash is used to figure out if configuration changes should
	// require file rebuilds.
	uint64_t configurationHash = 0;
	configurationHash = Strings::Hash64(project.Get_Target_Configuration(), configurationHash);
	configurationHash = Strings::Hash64(CastToString(project.Get_Target_Platform()), configurationHash);
	configurationHash = Strings::Hash64(project.Get_Project_Location().ToString(), configurationHash);
	configurationHash = Strings::Hash64(Strings::Format("%llu", project.Get_Project_File().GetModifiedTime()), configurationHash);
	configurationHash = Strings::Hash64(Strings::Format("%llu", workspaceFile.Get_Workspace_File().GetModifiedTime()), configurationHash);

	Log(LogSeverity::Verbose, "Configuration Hash: %llu\n", configurationHash);

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
		Log(LogSeverity::Fatal, "Toolchain '%s' not available to compile '%s', are you sure its installed?\nIf it is installed and in a non-default dictionary, please make sure its findable through the PATH environment variable.", toolchain->GetDescription().c_str(), project.Get_Project_Name().c_str());
		return false;
	}

	// Find build accelerator.
	Accelerator* accelerator = GetAccelerator(project);	
	if (!toolchain->CanDistribute())
	{
		accelerator = nullptr;
	}

	Log(LogSeverity::Info, "Toolchain: %s\n", toolchain->GetDescription().c_str());
	if (accelerator)
	{
		Log(LogSeverity::Info, "Accelerator: %s\n", accelerator->GetDescription().c_str());
	}

	Log(LogSeverity::Info, "\n");

	// Setup scheduler and create main task to parent all build tasks to.
	JobScheduler scheduler(Platform::GetConcurrencyFactor());
	bool bBuildFailed = false;
	JobHandle previousCommandHandle;

	// Run the pre-build commands syncronously in case they update plugin source state.
	for (auto& command : project.Get_PreBuildCommands_Command())
	{
		std::shared_ptr<ShellCommandTask> task = std::make_shared<ShellCommandTask>(BuildStage::PreBuildUser, command, toolchain);
		if (!task->Execute())
		{
			bBuildFailed = true;
			break;
		}
	}	

	if (bBuildFailed)
	{
		Log(LogSeverity::Fatal, "Build of '%s' failed.\n", project.Get_Project_Name().c_str());
		return false;
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
		Log(LogSeverity::SilentInfo, "%s is container, up to date.\n", project.Get_Project_Name().c_str());
		return true;
	}
	else if (bUpToDate)
	{
		Log(LogSeverity::SilentInfo, "%s is up to date.\n", project.Get_Project_Name().c_str());
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
		std::vector<std::shared_ptr<BuildTask>> tasks = toolchain->GetTasks(fileInfos, configurationHash, outputFile, versionInfo);

		// Register individual tasks for each build stage.
		for (int i = 0; i < (int)BuildStage::COUNT; i++)
		{
			JobHandle stageJob = buildStageHostJobs[i];

			std::vector<std::shared_ptr<BuildTask>> parallelTasks;
			std::vector<std::shared_ptr<BuildTask>> distributableTasks;
			std::vector<std::shared_ptr<BuildTask>> sequentialTasks;

			for (auto& task : tasks)
			{
				if (task->GetBuildState() == (BuildStage)i)
				{
					if (task->CanDistribute() && accelerator)
					{
						assert(task->CanRunInParallel()); // Distributed tasks are by nature parallel.
						distributableTasks.push_back(task);
					}
					else if (task->CanRunInParallel())
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

			// Queue accelerator tasks.
			if (accelerator && distributableTasks.size() > 0)
			{
				std::shared_ptr<AccelerateTask> task = std::make_shared<AccelerateTask>(project, distributableTasks, toolchain, accelerator);

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

		Log(LogSeverity::Info, "\n");
		Log(LogSeverity::Info, "Completed in %.1f seconds\n", elapsedMs / 1000.0f);
	}
	
	return true;
}

template <typename AcceleratorType>
AcceleratorType* GetCachedAccelerator(ProjectFile& project)
{
	MB_UNUSED_PARAMETER(project);

	/*
	static AcceleratorType* s_CachedAccelerator = nullptr;
	if (s_CachedAccelerator == nullptr)
	{
		s_CachedAccelerator = new AcceleratorType();
		s_CachedAccelerator->Init();
	}

	return s_CachedAccelerator;*/

	AcceleratorType* acc = new AcceleratorType();
	acc->Init();
	return acc;
}

Accelerator* Builder::GetAccelerator(ProjectFile& project)
{
	if (!project.Get_Acceleration_UseAcceleration())
	{
		return nullptr;
	}

	Accelerator_Sndbs* sndbsAccelerator = GetCachedAccelerator<Accelerator_Sndbs>(project);
	Accelerator_IncrediBuild* incrediBuildAccelerator = GetCachedAccelerator<Accelerator_IncrediBuild>(project);

	switch (project.Get_Acceleration_Accelerator())
	{
	case EAccelerator::Sndbs:
		{
			if (sndbsAccelerator->IsAvailable())
			{
				return sndbsAccelerator;
			}
			else
			{
				return nullptr;
			}
			break;
		}
	case EAccelerator::IncrediBuild:
		{
			if (incrediBuildAccelerator->IsAvailable())
			{
				return incrediBuildAccelerator;
			}
			else
			{
				return nullptr;
			}
			break;
		}
	case EAccelerator::Default:
		{
			if (sndbsAccelerator->IsAvailable())
			{
				return sndbsAccelerator;
			}
			else if (incrediBuildAccelerator->IsAvailable())
			{
				return incrediBuildAccelerator;
			}
			break;
		}
	}

	return nullptr;
}

template <typename ToolchainType>
Toolchain* GetCachedToolchain(ProjectFile& project, uint64_t configurationHash)
{
	/*
	static ToolchainType* s_CachedToolchain = nullptr;
	if (s_CachedToolchain == nullptr)
	{
		s_CachedToolchain = new ToolchainType(project, configurationHash);
	}
	s_CachedToolchain->SetProjectInfo(project, configurationHash);
	return s_CachedToolchain;
	*/
	ToolchainType* acc = nullptr;
	acc = new ToolchainType(project, configurationHash);
	acc->SetProjectInfo(project, configurationHash);
	return acc;
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
						return GetCachedToolchain<Toolchain_Clang>(project, configurationHash);
					}
					else if (project.Get_Build_PlatformToolset() == EPlatformToolset::GCC)
					{
						return GetCachedToolchain<Toolchain_Gcc>(project, configurationHash);
					}
#if defined(MB_PLATFORM_WINDOWS)
					else if (project.Get_Build_PlatformToolset() == EPlatformToolset::MSBuild_v140 ||
							 project.Get_Build_PlatformToolset() == EPlatformToolset::MSBuild_v141)
					{
						return GetCachedToolchain<Toolchain_Microsoft>(project, configurationHash);
					}
#endif
#if defined(MB_PLATFORM_MACOS)
					else if (project.Get_Build_PlatformToolset() == EPlatformToolset::XCode)
					{
						return GetCachedToolchain<Toolchain_XCode>(project, configurationHash);
					}
#endif
					else if (project.Get_Build_PlatformToolset() == EPlatformToolset::Default)
					{							
#if defined(MB_PLATFORM_WINDOWS) 
						return GetCachedToolchain<Toolchain_Microsoft>(project, configurationHash);
#elif defined(MB_PLATFORM_LINUX) 
						return GetCachedToolchain<Toolchain_Clang>(project, configurationHash);
#elif defined(MB_PLATFORM_MACOS)
						return GetCachedToolchain<Toolchain_XCode>(project, configurationHash);
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
					return GetCachedToolchain<Toolchain_Microsoft>(project, configurationHash);
#else
					break;
#endif
				}

			// Web Platforms			
			case EPlatform::HTML5:
				{
					return GetCachedToolchain<Toolchain_Emscripten>(project, configurationHash);
				}

			// Mobile Platforms
			case EPlatform::iOS:
				{					
#if defined(MB_PLATFORM_WINDOWS)
					return GetCachedToolchain<Toolchain_XCode>(project, configurationHash);
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
					return GetCachedToolchain<Toolchain_AndroidNdk>(project, configurationHash);
				}

			// Console Platforms
			case EPlatform::PS3:
				{		
#if defined(MB_PLATFORM_WINDOWS)
					return GetCachedToolchain<Toolchain_Playstation3>(project, configurationHash);
#else
					break;
#endif
				}
			case EPlatform::PS4:
				{
#if defined(MB_PLATFORM_WINDOWS)
					return GetCachedToolchain<Toolchain_Playstation4>(project, configurationHash);
#else
					break;
#endif
				}
			case EPlatform::PSVita:
				{
#if defined(MB_PLATFORM_WINDOWS)
					return GetCachedToolchain<Toolchain_PlaystationVita>(project, configurationHash);
#else
					break;
#endif
				}
			case EPlatform::Xbox360:
				{
#if defined(MB_PLATFORM_WINDOWS)
					return GetCachedToolchain<Toolchain_Xbox360>(project, configurationHash);
#else
					break;
#endif
				}
			case EPlatform::XboxOne:
				{
#if defined(MB_PLATFORM_WINDOWS)
					return GetCachedToolchain<Toolchain_XboxOne>(project, configurationHash);
#else
					break;
#endif
				}
			case EPlatform::NintendoWiiU:
				{
#if defined(MB_PLATFORM_WINDOWS)
					return GetCachedToolchain<Toolchain_NintendoWiiU>(project, configurationHash);
#else
					break;
#endif
				}
			case EPlatform::Nintendo3DS:
				{
#if defined(MB_PLATFORM_WINDOWS)
					return GetCachedToolchain<Toolchain_Nintendo3ds>(project, configurationHash);
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
					return GetCachedToolchain<Toolchain_XCode>(project, configurationHash);
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
						return GetCachedToolchain<Toolchain_DotNet>(project, configurationHash);
					}
					else if (project.Get_Build_PlatformToolset() == EPlatformToolset::Mono)
					{
						return GetCachedToolchain<Toolchain_Mono>(project, configurationHash);
					}
					else if (project.Get_Build_PlatformToolset() == EPlatformToolset::Default)
					{				
#if defined(MB_PLATFORM_WINDOWS)
						return GetCachedToolchain<Toolchain_DotNet>(project, configurationHash);
#elif defined(MB_PLATFORM_LINUX) || defined(MB_PLATFORM_MACOS)
						return GetCachedToolchain<Toolchain_Mono>(project, configurationHash);
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
