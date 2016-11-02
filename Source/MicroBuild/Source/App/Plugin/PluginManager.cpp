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

PluginManager::PluginManager(App* app)
	: m_app(app)
{
}

PluginManager::~PluginManager()
{
	for (Plugin* plugin : m_plugins)
	{
		delete plugin;
	}
	m_plugins.clear();
}

bool PluginManager::FindAndLoadAll()
{
	std::string extension = "";
	std::string architecture = "";
	std::string configuration = "";

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

#if defined(MB_DEBUG_BUILD)
	configuration = "Debug";
#elif defined(MB_RELEASE_BUILD)
	configuration = "Release";
#elif defined(MB_SHIPPING_BUILD)
	configuration = "Shipping";
#endif

	std::string recursiveFilter = 
		Strings::Format(
			"**.plugin.%s.%s.%s",
			architecture.c_str(),
			configuration.c_str(),
			extension.c_str()
		);
	std::string filter = 
		Strings::Format(
			"*.plugin.%s.%s.%s",
			architecture.c_str(),
			configuration.c_str(),
			extension.c_str()
		);

	Platform::Path recursivePathFilter = 
		Platform::Path::GetExecutablePath().GetDirectory().AppendFragment(recursiveFilter, true);

	Platform::Path pluginDirectory = 
		Platform::Path::GetExecutablePath().GetDirectory().AppendFragment("Plugins", true);

	Platform::Path pluginPathFilter = 
		pluginDirectory.AppendFragment(recursiveFilter, true);

	if (!pluginDirectory.Exists())
	{
		pluginDirectory.CreateAsDirectory();
	}

	std::vector<Platform::Path> matchingPaths =
		Platform::Path::MatchFilter(recursivePathFilter);

	for (auto path : matchingPaths)
	{
		Platform::Path loadPath = path;

		// If plugin is in plugin directory, load as normal. Otherwise
		// check timestamp if its more recent than the one in the plugin directory copy it
		// in and load that one. This allows us to rebuild plugins using microbuild itself without
		// having the issue of trying to write to files that are in use.
		if (path.GetDirectory() != pluginDirectory)
		{
			loadPath = pluginDirectory.AppendFragment(path.GetFilename(), true);
			
			Platform::Path pdbPath = path.ChangeExtension("pdb");

			bool bCopy = false;
			if (!loadPath.Exists())
			{
				bCopy = true;			
			}
			else
			{
				if (path.GetModifiedTime() >  loadPath.GetModifiedTime())
				{
					bCopy = true;
				}
			}

			if (bCopy)
			{
				Log(LogSeverity::Info, "Plugin is new or updated, copying to plugin directory: %s\n", path.GetFilename().c_str());				
				path.Copy(loadPath);

				if (pdbPath.Exists())
				{
					pdbPath.Copy(loadPath.ChangeExtension("pdb"));
				}
			}

/*			path.Delete();
			if (pdbPath.Exists())
			{
				pdbPath.Delete();
			} */
		}
	}
	
	matchingPaths =
		Platform::Path::MatchFilter(pluginPathFilter);
	
	for (auto path : matchingPaths)
	{
		Platform::Path loadPath = path;
	
		Log(LogSeverity::Info, "Loading plugin: %s\n", path.GetFilename().c_str());	
			
		Plugin* plugin = new Plugin(this);
		if (!plugin->Load(loadPath))
		{
			Log(LogSeverity::Info, "\tPlugin load failed.\n");
		}
		else
		{
			m_plugins.push_back(plugin);
		}
	}

	if (matchingPaths.size() > 0)
	{
		Log(LogSeverity::Info, "\n");
	}

	return true;
}

std::vector<Plugin*> PluginManager::GetPlugins()
{
	return m_plugins;
}

App* PluginManager::GetApp()
{
	return m_app;
}

bool PluginManager::OnEvent(EPluginEvent Event, PluginEventData* Data)
{
	for (Plugin* plugin : m_plugins)
	{
		if (!plugin->OnEvent(Event, Data))
		{
			return false;
		}
	}
	return true;
}

} // namespace MicroBuild