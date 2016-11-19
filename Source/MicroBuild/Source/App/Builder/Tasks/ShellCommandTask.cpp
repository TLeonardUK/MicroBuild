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

ShellCommandTask::ShellCommandTask(BuildStage stage, const std::string& command)
	: BuildTask(stage, false)
	, m_command(command)
{
}

bool ShellCommandTask::Execute()
{
	std::vector<std::string> arguments = Strings::Crack(m_command, ' ', true);
	std::string executable = Strings::StripQuotes(arguments[0]);
	arguments.erase(arguments.begin());
	
	TaskLog(LogSeverity::SilentInfo, "Executing: %s\n", m_command.c_str());

	Platform::Process process;
	if (!process.Open(executable, Platform::Path::GetExecutablePath().GetDirectory(), arguments, false))
	{
		return false;
	}

	process.Wait();
	return (process.GetExitCode() == 0);
}

}; // namespace MicroBuild