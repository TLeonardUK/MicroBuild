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

#include "App/Builder/Tasks/CompileTask.h"
#include "Core/Platform/Process.h"

namespace MicroBuild {

CompileTask::CompileTask(Toolchain* toolchain, ProjectFile& project, BuilderFileInfo& file, BuilderFileInfo pchFile)
	: BuildTask(BuildStage::Compile, true, true)
	, m_projectFile(project)
	, m_file(file)
	, m_pchFile(pchFile)
	, m_toolchain(toolchain)
{
}

bool CompileTask::Execute()
{
	int jobIndex = 0, totalJobs = 0;
	GetTaskProgress(jobIndex, totalJobs);
	
	TaskLog(LogSeverity::SilentInfo, "Compiling: %s\n", m_file.SourcePath.GetFilename().c_str());

	return m_toolchain->Compile(m_file, m_pchFile);
}

}; // namespace MicroBuild