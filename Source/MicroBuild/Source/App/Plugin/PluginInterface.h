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

namespace MicroBuild {

// Base class for all plugin interfaces.
class IPluginInterface
{
public:
};

}; // namespace MicroBuild


// Currently active plugin interface that all plugins that use
// this header should use.

#include "App/Plugin/Interfaces/PluginInterface1.h"

#define CurrentPluginInterface			MicroBuild::PluginInterface1
#define CurrentPluginInterfaceVersion	1

// Implements the interface required for a microbuild plugin to work.

#ifdef MB_PLATFORM_WINDOWS
#define DLL_EXPORT extern "C" __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

#define ImplmentMicroBuildPlugin() \
	bool PluginMain(CurrentPluginInterface* pluginInterface); \
	\
	DLL_EXPORT int GetPluginInterfaceVersion() \
	{ \
		return CurrentPluginInterfaceVersion; \
	} \
	DLL_EXPORT void GetPluginVersion(float* minimumVersion, float* maximumVersion) \
	{ \
		*minimumVersion = static_cast<int>(MB_VERSION); \
		*maximumVersion = static_cast<int>(MB_VERSION); \
	} \
	DLL_EXPORT bool InitializePlugin(MicroBuild::IPluginInterface* pluginInterface) \
	{ \
		return PluginMain(static_cast<CurrentPluginInterface*>(pluginInterface)); \
	} \

