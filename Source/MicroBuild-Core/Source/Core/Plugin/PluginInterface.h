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

#include <functional>

#include "Core/Config/ConfigFile.h"

namespace MicroBuild {

// The different kind of events that can be fired and passed to 
// individual plugins.
enum EPluginEvent
{
	// Invoked when the workspace file is loaded in, allows the plugin
	// to make any manipulations required before its used.
	PostProcessWorkspaceFile,

	// Invoked when a project file is loaded in, allows the plugin
	// to make any manipulations required before its used.
	PostProcessProjectFile,
};

// Contains plugin data specific to individual events. Get the event specific
// data by calling Get<PluginDataType>().
struct PluginEventData
{
public:
	template<typename DataType>
	DataType* Get()
	{
		return reinterpret_cast<DataType*>(this);
	}
};

// Plugin data for the event PostProcessWorkspaceFile
struct PluginPostProcessWorkspaceFileData : public PluginEventData
{
	ConfigFile* File;
};

// Plugin data for the event PostProcessProjectFile
struct PluginPostProcessProjectFileData : public PluginEventData
{
	ConfigFile* File;
};

// Function type for event callbacks.
// Returns true on success, false on failure (which will abort further processing).
typedef std::function<bool(PluginEventData* Data)> PluginCallbackSignature;

// Base class for all plugin interfaces.
class IPluginInterface
{
public:
};

}; // namespace MicroBuild


// Currently active plugin interface that all plugins that use
// this header should use.

#include "Core/Plugin/Interfaces/PluginInterface1.h"

#define CurrentPluginInterface			MicroBuild::PluginInterface1
#define CurrentPluginInterfaceVersion	1

// Implements the interface required for a microbuild plugin to work.

#ifdef MB_PLATFORM_WINDOWS
#define DLL_EXPORT extern "C" __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

#define MicroBuildPlugin() \
	namespace MicroBuild { \
		bool PluginMain(CurrentPluginInterface* pluginInterface); \
	} \
	DLL_EXPORT int GetPluginInterfaceVersion() \
	{ \
		return CurrentPluginInterfaceVersion; \
	} \
	DLL_EXPORT bool InitializePlugin(MicroBuild::IPluginInterface* pluginInterface) \
	{ \
		return MicroBuild::PluginMain(static_cast<CurrentPluginInterface*>(pluginInterface)); \
	} \

