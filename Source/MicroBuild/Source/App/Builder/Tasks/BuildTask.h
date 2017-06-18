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
#include "App/Builder/BuilderFileInfo.h"
#include "Core/Parallel/Jobs/JobScheduler.h"

namespace MicroBuild {

// Stage of the build process where a given task is executed. Each stage
// is executed sequentially, tasks within each stage can run in parallel.
enum class BuildStage
{
	// Run before all else, executes any toolchain commands that need
	// to be executed before the build gets started.
	PreBuild,

	// Same as PreBuild, but runs commands the user has defined in the
	// project configuration.
	PreBuildUser,

	// Precompiled headers are generated during this stage.
	PchCompile,

	// Massively parallel stage, during this stage all source files
	// are converted to object files.
	Compile,

	// Run before the link, executes any toolchain commands that need
	// to be executed before the link gets started.
	PreLink,
	
	// Same as PreLink, but runs commands the user has defined in the
	// project configuration.
	PreLinkUser,
	
	// During this stage object files generated during the compile phase 
	// are linked together into a final output file.
	Link,
	
	// Same as PostBuild, but runs commands the user has defined in the
	// project configuration.
	PostBuildUser,
	
	// Run before the link, executes any toolchain commands that need
	// to be executed after the build finishes.
	PostBuild,

	COUNT
};	

//MicrosoftGenerateManifestTask

// Base task for all internal builder tasks.
class BuildTask
{
private:

	BuildStage m_stage;
	bool m_bCanRunInParallel;
	bool m_bCanDistribute;

	int m_threadId;
	int m_jobIndex;
	int m_totalJobs;
	bool m_bGiveJobIndex;

protected:
	int m_subTaskCount;

public:
	BuildTask(BuildStage stage, bool bCanRunInParallel, bool bGiveJobIndex, bool bCanDistribute);

	// Gets the state during which this task is executed.
	BuildStage GetBuildState();

	// If true this task can be run in parallel during its build state. If false, 
	// it will get queued up with all other synchronous tasks and executed one
	// at a time at the end of the phase.
	bool CanRunInParallel();

	// If true the task can be distributed to other computers using a build accelerator.
	bool CanDistribute();

	// If we should assing this task a job index for progression tracking, 
	// only false for tasks we explicitly do not want to log to the user,
	// such as shell commands.
	bool ShouldGiveJobIndex();

	// Entrey point for the task, returns true on success, false on failure.
	virtual bool Execute();

	// Gets or sets the index of the thread we are executing on.
	int GetTaskThreadId();
	void GetTaskThreadId(int id);

	// Gets or sets the current task progress state to use when printing.
	void SetTaskProgress(int jobIndex, int totalJobs);
	void GetTaskProgress(int& jobIndex, int& totalJobs);

	// Logs a progress messages prefixed with some progress information.
	void TaskLog(LogSeverity Severity, int subTaskIndex, const char* format, ...);

	// Gets total number of tasks this toolchain is made out of. This means the progress index
	// of the task is given + this count above it for printing sub task details.
	int GetSubTaskCount();

	// Gets the action that this build task performs.
	virtual BuildAction GetAction() = 0;

}; 

}; // namespace MicroBuild
