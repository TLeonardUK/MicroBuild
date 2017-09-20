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

#include "Schemas/Plugin/PluginInterface.h"

namespace MicroBuild {

class Plugin;
class App;

// Manager for all plugins.
class PluginManager
{
public:
	PluginManager(App* app);
	virtual ~PluginManager();

	// Loads all the plugins it can find, returns false on 
	// catastophic failure.
	bool FindAndLoadAll();

	// Unloads all plugins that are currently loaded.
	void UnloadAll();

	// Gets a list of all plugins that are currently loaded.
	std::vector<Plugin*> GetPlugins();

	// Fires any registered events of the given type.
	bool OnEvent(EPluginEvent Event, PluginEventData* Data);

	// Gets the app instance this plugin manager was created for.
	App* GetApp();

	// Returns true if a plugin with the given name is loaded.
	bool IsPluginLoaded(const std::string& fileName);

	// Gets a plugin based on its filename.
	Plugin* GetPluginByName(const std::string& fileName);

	// Attempts to unload the plugin with the given name, if the plugin
	// is not currently loaded this returns false.
	bool UnloadPluginByName(const std::string& fileName);

	// Attempts to load the plugin with the given name, if the plugin
	// could not be found, it returns false.
	bool LoadPluginByName(const std::string& fileName);

private:
	std::vector<Plugin*> m_plugins;
	App* m_app;

	Platform::Path m_recursivePathFilter;
	Platform::Path m_pluginDirectory;

	std::string m_recursiveFilter;
	std::string m_filter;
	std::string m_fileFormat;

};

}; // namespace MicroBuild