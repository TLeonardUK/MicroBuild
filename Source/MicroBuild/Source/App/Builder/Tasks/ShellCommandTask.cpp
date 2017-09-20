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

#include "App/Builder/Tasks/ShellCommandTask.h"
#include "Core/Platform/Process.h"

namespace MicroBuild {

ShellCommandTask::ShellCommandTask(BuildStage stage, const std::string& command, Toolchain* toolchain)
	: BuildTask(stage, false, false, false)
	, m_command(command)
    , m_toolchain(toolchain)
{
}

BuildAction ShellCommandTask::GetAction()
{
	std::vector<std::string> arguments = Strings::Crack(m_command, ' ', true);
	std::string executable = Strings::StripQuotes(arguments[0]);
	arguments.erase(arguments.begin());

	Platform::Path rootPath = Platform::Path(executable).GetDirectory();
	if (rootPath.IsRelative())
	{
		rootPath = Platform::Path::GetExecutablePath().GetDirectory();
	}

	BuildAction action;
	action.StatusMessage = "";
	action.Tool = executable;
	action.WorkingDirectory = rootPath;
	action.Arguments = arguments;

	action.PostProcessDelegate = [=](BuildAction& action) -> bool
	{
        if (!m_toolchain->ParseOutput(action.FileInfo, action.Output))
        {
            return false;
        } 
        if (LogGetVerbose())
        {
            printf("%s", action.Output.c_str());
        }        
        m_toolchain->PrintMessages(action.FileInfo);
		return (action.ExitCode == 0);
	};

	return action;
}

}; // namespace MicroBuild