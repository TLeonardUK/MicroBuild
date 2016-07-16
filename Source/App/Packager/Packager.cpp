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
#include "App/Packager/Packager.h"
#include "Core/Platform/Process.h"
#include "Core/Helpers/ZipFile.h"

namespace MicroBuild {

Packager::Packager()
{
	RegisterCommand("copy",		2, &Packager::Copy);
	RegisterCommand("delete",	1, &Packager::Delete);
	RegisterCommand("archive",	2, &Packager::Archive);
}

Packager::~Packager()
{
}

void Packager::RegisterCommand(std::string name, int argCount, CommandCallbackType callback)
{
	Command cmd;
	cmd.name = name;
	cmd.callback = callback;
	cmd.argCount = argCount;
	m_commands[Strings::ToLowercase(name)] = cmd;
}

bool Packager::Package(
	const Platform::Path& workingDirectory,
	const std::vector<std::string>& commands
)
{
	UNUSED_PARAMETER(commands);

	m_workingDirectory = workingDirectory;

	for (std::string cmd : commands)
	{
		Log(LogSeverity::Info, "Command: %s\n", cmd.c_str());

		std::vector<std::string> fragments = Strings::Crack(cmd);
		if (fragments.size() > 0)
		{
			std::string commandName = Strings::ToLowercase(fragments[0]);

			auto result = m_commands.find(commandName);
			if (result != m_commands.end())
			{
				auto command = result->second;

				fragments.erase(fragments.begin());

				if (static_cast<int>(fragments.size()) != command.argCount)
				{
					Log(LogSeverity::Info,
						"Command '%s' takes %i arguments but recieved %i.\n", 
						commandName.c_str(),
						command.argCount,
						fragments.size());

					return false;
				}

				if (!command.callback(m_workingDirectory, fragments))
				{
					Log(LogSeverity::Info, 
						"Command '%s' failed.\n", commandName.c_str());
					return false;
				}
			}
			else
			{
				if (!NativeCommand(m_workingDirectory, fragments))
				{
					Log(LogSeverity::Info, 
						"Command '%s' failed.\n", commandName.c_str());
					return false;
				}
			}
		}
	}

	return true;
}

bool Packager::NativeCommand(const Platform::Path& workingDir, const std::vector<std::string>& args)
{
	Platform::Process process;

	Platform::Path command = args[0];
	std::vector<std::string> arguments(args.begin() + 1, args.end());

	if (process.Open(command, workingDir, arguments, false))
	{
		process.Wait();
		if (process.GetExitCode() == 0)
		{
			return true;
		}
	}

	return false;
}

bool Packager::Copy(const Platform::Path& workingDir, const std::vector<std::string>& args)
{
	UNUSED_PARAMETER(workingDir);

	Platform::Path src = args[0];
	Platform::Path dest = args[1];

	size_t wildcardOffset = src.ToString().find('*');
	if (wildcardOffset != std::string::npos)
	{
		std::vector<Platform::Path> srcMatches = Platform::Path::MatchFilter(src);

		for (Platform::Path path : srcMatches)
		{
			std::string uncommonPath = path.ToString().substr(wildcardOffset);

			Platform::Path destPath = dest.AppendFragment(uncommonPath, true);

			if (!path.Copy(destPath))
			{
				Log(LogSeverity::Fatal, "Failed to copy '%s' to '%s'.\n", 
					path.ToString().c_str(),
					destPath.ToString().c_str());

				return false;
			}
			else
			{
				Log(LogSeverity::Fatal, "Copied '%s' to '%s'.\n",
					path.ToString().c_str(),
					destPath.ToString().c_str());
			}
		}
	}
	else
	{
		if (!src.Copy(dest))
		{
			Log(LogSeverity::Fatal, "Failed to copy '%s' to '%s'.\n",
				src.ToString().c_str(),
				dest.ToString().c_str());

			return false;
		}
		else
		{
			Log(LogSeverity::Fatal, "Copied '%s' to '%s'.\n",
				src.ToString().c_str(),
				dest.ToString().c_str());
		}	
	}

	return true;
}

bool Packager::Delete(const Platform::Path& workingDir, const std::vector<std::string>& args)
{
	UNUSED_PARAMETER(workingDir);

	Platform::Path src = args[0];

	if (!src.Delete())
	{
		Log(LogSeverity::Fatal, "Failed to delete '%s'.\n",
			src.ToString().c_str());

		return false;
	}
	else
	{
		Log(LogSeverity::Fatal, "Deleted '%s'.\n",
			src.ToString().c_str());
	}
	return true;
}

bool Packager::Archive(const Platform::Path& workingDir, const std::vector<std::string>& args)
{
	UNUSED_PARAMETER(workingDir);

	Platform::Path src = args[0];
	Platform::Path dest = args[1];

	ZipFile file;
	if (!file.Open(dest))
	{
		Log(LogSeverity::Fatal, "Failed to write zip file '%s'.\n",
			dest.ToString().c_str());

		return false;
	}

	size_t wildcardOffset = src.ToString().find('*');
	if (wildcardOffset != std::string::npos)
	{
		std::vector<Platform::Path> srcMatches = Platform::Path::MatchFilter(src);

		for (Platform::Path path : srcMatches)
		{
			if (path.IsDirectory())
			{
				continue;
			}

			Log(LogSeverity::Fatal, "Adding file '%s' to archive.\n",
				path.ToString().c_str());

			std::string uncommonPath = path.ToString().substr(wildcardOffset);
			if (!file.AddFile(path, uncommonPath))
			{
				Log(LogSeverity::Fatal, "Failed to write zip file '%s'.\n",
					dest.ToString().c_str());

				return false;
			}
		}
	}
	else
	{
		Log(LogSeverity::Fatal, "Adding directory '%s' to archive.\n",
			src.ToString().c_str());

		if (!file.AddDirectory(src, ""))
		{
			Log(LogSeverity::Fatal, "Failed to write zip file '%s'.\n",
				dest.ToString().c_str());

			return false;
		}
	}

	file.Close();
		
	Log(LogSeverity::Fatal, "Wrote zip file to '%s'.\n",
			dest.ToString().c_str());

	return true;
}

}; // namespace MicroBuild