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
#include "App/Plugin/Plugin.h"
#include "App/Plugin/PluginManager.h"
#include "Core/Platform/Path.h"
#include "Core/Helpers/Strings.h"

namespace MicroBuild {

PluginManager::PluginManager()
{
}

PluginManager::~PluginManager()
{
}

bool PluginManager::FindAndLoadAll()
{
	std::string extension = "";
	std::string architecture = "";

#if defined(MB_PLATFORM_WINDOWS)
	extension = "dll";
#elif defined(MB_PLATFORM_MACOS)
	extension = "dylib";
#elif defined(MB_PLATFORM_LINUX)
	extension = "so";
#endif
#if defined(MB_ARCHITECTURE_X64)
	architecture = "x64";
#elif defined(MB_ARCHITECTURE_X86)
	architecture = "x86";
#endif

	std::string filter = 
		Strings::Format(
			"**.plugin.%s.%s",
			architecture.c_str(),
			extension.c_str()
		);

	Platform::Path pathFilter = 
		Platform::Path::GetWorkingDirectory().AppendFragment(filter, true);

	std::vector<Platform::Path> matchingPaths =
		Platform::Path::MatchFilter(pathFilter);

	for (auto path : matchingPaths)
	{
		Log(LogSeverity::Info, "Loading plugin: %s", path.ToString().c_str());
		
		Plugin* plugin = new Plugin();
		if (!plugin->Load(path))
		{
			Log(LogSeverity::Info, "\tPlugin load failed.");
		}
	}

	return true;
}

std::vector<Plugin*> PluginManager::GetPlugins()
{
	return m_plugins;
}

} // namespace MicroBuild