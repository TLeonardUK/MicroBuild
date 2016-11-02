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
	
// Base task for all internal builder tasks.
class BuildTask
{
private:
	int m_threadId;
	int m_jobIndex;
	int m_totalJobs;

public:

	// Entrey point for the task, returns true on success, false on failure.
	virtual bool Execute() = 0;

	// Gets or sets the index of the thread we are executing on.
	int GetTaskThreadId();
	void GetTaskThreadId(int id);

	// Gets or sets the current task progress state to use when printing.
	void SetTaskProgress(int jobIndex, int totalJobs);
	void GetTaskProgress(int& jobIndex, int& totalJobs);

	// Logs a progress messages prefixed with some progress information.
	void TaskLog(LogSeverity Severity, const char* format, ...);

}; 

}; // namespace MicroBuild