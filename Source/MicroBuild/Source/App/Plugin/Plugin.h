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
#include "Core/Platform/Module.h"

#include "Schemas/Plugin/PluginInterface.h"

namespace MicroBuild {

class IPluginInterface;
class PluginManager;

// Base class for all plugins.
class Plugin
{
public:

	Plugin(PluginManager* manager);
	virtual ~Plugin();

	// Gets the name of this plugin.
	std::string GetName();

	// Loads the plugin.
	bool Load(Platform::Path& path);

	// Fires any registered events of the given type.
	bool OnEvent(EPluginEvent Event, PluginEventData* Data);

	// Registers an event callback for the given plugin.
	void RegisterCallback(EPluginEvent Event, PluginCallbackSignature FuncPtr);

private:
	struct PluginCallback
	{
		EPluginEvent Event;
		PluginCallbackSignature Callback;
	};

	typedef bool  (*InitializePlugin_t)(IPluginInterface* pluginInterface);

	InitializePlugin_t InitializePlugin;

	std::vector<PluginCallback> m_callbacks;
	
	std::string m_name;
	Platform::Module m_module;
	IPluginInterface* m_pluginInterface;
	PluginManager* m_manager;

};

}; // namespace MicroBuild