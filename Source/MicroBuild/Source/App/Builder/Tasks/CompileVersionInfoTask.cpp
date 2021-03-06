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

#include "App/Builder/Tasks/CompileVersionInfoTask.h"
#include "Core/Platform/Process.h"

namespace MicroBuild {

CompileVersionInfoTask::CompileVersionInfoTask(Toolchain* toolchain, ProjectFile& project, BuilderFileInfo file, VersionNumberInfo versionInfo)
	: BuildTask(BuildStage::PreBuild, true, true, false)
	, m_toolchain(toolchain)
	, m_file(file)
	, m_versionInfo(versionInfo)
{
	MB_UNUSED_PARAMETER(project);
}

BuildAction CompileVersionInfoTask::GetAction()
{
	BuildAction action;
	action.StatusMessage = "Compiling Version Info\n";

	m_toolchain->GetCompileVersionInfoAction(action, m_file, m_versionInfo);

	return action;
}

}; // namespace MicroBuild
