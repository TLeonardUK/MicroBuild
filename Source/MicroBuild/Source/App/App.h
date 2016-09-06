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

#include "Core/Commands/CommandLineParser.h"
#include "App/Plugin/PluginManager.h"

namespace MicroBuild {

class IdeType;
class PlatformType;

// Base class that reads command line input and dispatching the commands to
// the correct places.
class App
{
public:
	App(int argc, char* argv[]);
	~App();

	int Run();

	// Gets a list of all supported ides we can generated files for.
	std::vector<IdeType*> GetIdes() const;

	// Gets an ide given its short name.
	IdeType* GetIdeByShortName(const std::string& shortName) const;

	// Gets the plugin manager the app is currently being used.
	PluginManager* GetPluginManager();

protected:
	void PrintLicense();

private:
	char** m_argv;
	int m_argc;
	CommandLineParser m_commandLineParser;

	PluginManager m_pluginManager;

	std::vector<IdeType*> m_ides;

}; 

}; // namespace MicroBuild