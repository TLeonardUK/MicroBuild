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

#include "App/Builder/Tasks/ArchiveTask.h"
#include "Core/Platform/Process.h"

namespace MicroBuild {

ArchiveTask::ArchiveTask(std::vector<BuilderFileInfo>& sourceFiles, Toolchain* toolchain, ProjectFile& project, BuilderFileInfo& outputFile)
	: m_projectFile(project)
	, m_toolchain(toolchain)
	, m_sourceFiles(sourceFiles)
	, m_outputFile(outputFile)
{
}

bool ArchiveTask::Execute()
{	
	int jobIndex = 0, totalJobs = 0;
	GetTaskProgress(jobIndex, totalJobs);

	TaskLog(LogSeverity::SilentInfo, "Archiving: %s\n", m_outputFile.OutputPath.GetFilename().c_str());

	return m_toolchain->Archive(m_sourceFiles, m_outputFile);
}

}; // namespace MicroBuild