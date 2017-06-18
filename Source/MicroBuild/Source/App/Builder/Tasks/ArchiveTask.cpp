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
	: BuildTask(BuildStage::Link, true, true, false)
	, m_toolchain(toolchain)
	, m_sourceFiles(sourceFiles)
	, m_outputFile(outputFile)
{
	MB_UNUSED_PARAMETER(project);
}

BuildAction ArchiveTask::GetAction()
{	
	BuildAction action;
	action.StatusMessage = Strings::Format("Archiving: %s\n", m_outputFile.OutputPath.GetFilename().c_str());
	
	m_toolchain->GetArchiveAction(action, m_sourceFiles, m_outputFile);

	return action;
}

}; // namespace MicroBuild
