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

namespace MicroBuild {

class IPluginInterface;

// Base class for all plugins.
class Plugin
{
public:

	Plugin();
	virtual ~Plugin();

	// Gets the name of this plugin.
	std::string GetName();

	// Loads the plugin.
	bool Load(Platform::Path& path);

protected:

private:
	typedef int   (*GetPluginInterfaceVersion_t)();
	typedef void  (*GetPluginVersion_t)(float* minimumVersion, float* maximumVersion);
	typedef bool  (*InitializePlugin_t)(IPluginInterface* pluginInterface);

	GetPluginInterfaceVersion_t GetPluginInterfaceVersion;
	GetPluginVersion_t GetPluginVersion;
	InitializePlugin_t InitializePlugin;

	std::string m_name;
	Platform::Module m_module;
	IPluginInterface* m_pluginInterface;

};

}; // namespace MicroBuild