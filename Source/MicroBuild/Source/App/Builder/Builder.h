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

#pragma once

#include "Schemas/Project/ProjectFile.h"
#include "App/Builder/Toolchains/Toolchain.h"
#include "App/Builder/Tasks/BuildTask.h"
#include "App/Builder/SourceControl/SourceControlProvider.h"
#include "App/App.h"

namespace MicroBuild {

// Internal builder. Takes a project configuration and builds the file as it specifies.
class Builder
{
public:
	Builder(App* app);
	~Builder();

	// Cleans all intermediate files generate by previous builds of the project.
	bool Clean(WorkspaceFile& workspaceFile, ProjectFile& project);

	// Builds the project in the configuration the project file defines.
	bool Build(WorkspaceFile& workspaceFile, std::vector<ProjectFile*> projectFiles, ProjectFile& project, bool bRebuild, bool bBuildDependencies, bool bBuildPackageFiles);

protected:

	// Gets the toolchain required to build the project.
	Toolchain* GetToolchain(ProjectFile& project, uint64_t configurationHash);

	// Queues the task with the given scheduler and parent job and sets
	// a given flag on failure.
	JobHandle QueueTask(
		JobScheduler& scheduler, 
		JobHandle& groupJob, 
		JobHandle* startAfterJob, 
		std::shared_ptr<BuildTask> task,
		bool& bFailureFlag,
		int* totalJobs,
		std::atomic<int>* currentJobIndex);

	// Generates a dependency list which needs to be satisifed in order to build the output. Build order and dependencies
	// are usually handled by the IDE, but there are a handful of cases where we need to do it ourselves.
	void BuildDependencyList(
		WorkspaceFile& workspaceFile, 
		std::vector<ProjectFile*> projectFiles,
		ProjectFile& project, 
		std::vector<ProjectFile*>& dependencyList, 
		std::vector<ProjectFile*>& processedList);

	// Attempts to generate a changelog file from source control commit messages.
	bool BuildChangelog(
		ProjectFile& project,
		VersionNumberInfo& info,
		std::shared_ptr<ISourceControlProvider> provider
	);

	// Attempts to extract version number information for use in the build. This usually results in a source-control call.
	bool CalculateVersionNumber(
		ProjectFile& project,
		VersionNumberInfo& info,
		std::shared_ptr<ISourceControlProvider> provider
	);

	// Gets a source control provider and connects to it, the returned pointer can be used to query and manipulate
	// the source control server.
	bool GetSourceControlProvider(
		ProjectFile& project,
		std::shared_ptr<ISourceControlProvider>& provider
	);

	// Writes versioning information to a source file which is (assumed to be) used in the builder process.
	bool WriteVersionNumberSource(
		ProjectFile& project,
		Platform::Path& hppPath,
		Platform::Path cppPath,
		VersionNumberInfo& info
	);

private:
	App* m_app;

}; 

}; // namespace MicroBuild
