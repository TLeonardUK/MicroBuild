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

#include "Core/Platform/Path.h"
#include "Core/Helpers/Strings.h"
#include "App/Config/BaseConfigFile.h"

#include <functional>

namespace MicroBuild {

// Responsible for parsing and executing packaging commands.
class Packager
{
public:
	Packager();
	~Packager();

	bool Package(
		const Platform::Path& workingDirectory, 
		const std::vector<std::string>& commands
	);

private:
	typedef std::function<
		bool(const Platform::Path& workingDir, const std::vector<std::string>&)
	> CommandCallbackType;

	struct Command
	{
		std::string name;
		int argCount;
		CommandCallbackType callback;
	};

	Platform::Path m_workingDirectory;
	std::map<std::string, Command> m_commands;

protected:
	static bool Copy(const Platform::Path& workingDir, const std::vector<std::string>& args);
	static bool Delete(const Platform::Path& workingDir, const std::vector<std::string>& args);
	static bool Archive(const Platform::Path& workingDir, const std::vector<std::string>& args);
	static bool NativeCommand(const Platform::Path& workingDir, const std::vector<std::string>& args);

	void RegisterCommand(std::string name, int argCount, CommandCallbackType callback);

};

}; // namespace MicroBuild