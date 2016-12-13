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
	bool Build(WorkspaceFile& workspaceFile, ProjectFile& project, bool bRebuild);

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

private:
	App* m_app;

}; 

}; // namespace MicroBuild
