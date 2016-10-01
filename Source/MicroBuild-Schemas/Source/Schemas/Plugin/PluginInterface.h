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
#include <memory>

#include "Core/Config/ConfigFile.h"
#include "Core/Commands/Command.h"
#include "Schemas/Project/ProjectFile.h"
#include "Schemas/Workspace/WorkspaceFile.h"

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
	WorkspaceFile* File;
};

// Plugin data for the event PostProcessProjectFile
struct PluginPostProcessProjectFileData : public PluginEventData
{
	ProjectFile* File;
};

// Function type for event callbacks.
// Returns true on success, false on failure (which will abort further processing).
typedef std::function<bool(PluginEventData* Data)> PluginCallbackSignature;

// Project wrapper for plugin states.
class IProjectState
{
public:
	virtual ~IProjectState() { };

	virtual ProjectFile& GetConfigFile() = 0;
	virtual std::string GetConfiguration() = 0;
	virtual std::string GetPlatform() = 0;

};

// Project wrapper for plugins.
class IProject
{
public:
	virtual ~IProject() { };

	virtual std::shared_ptr<IProjectState> GetState(const std::string& config, const std::string& platform) = 0;
	virtual std::string GetName() = 0;

};

// Workspace wrapper for plugins.
class IWorkspace
{
public:
	virtual ~IWorkspace() { };

	virtual std::shared_ptr<IProject> GetProject(const std::string& projectName) = 0;
	virtual WorkspaceFile& GetConfigFile() = 0;

};

// Base class for all plugin interfaces.
class IPluginInterface
{
public:
	virtual ~IPluginInterface() { };

	// Gets or sets the name of the plugin.
	virtual void SetName(const std::string& value) = 0;
	virtual std::string GetName() const = 0;

	// Gets or sets the description of the plugin.
	virtual void SetDescription(const std::string& value) = 0;
	virtual std::string GetDescription() const = 0;

	// Registers a new command that can be called from the command line.
	virtual void RegisterCommand(Command* command) = 0;

	// Registers all callback for a specific event.
	virtual void RegisterCallback(EPluginEvent Event, PluginCallbackSignature FuncPtr) = 0;

	// Reads in the given workspace configuration.
	virtual std::shared_ptr<IWorkspace> ReadWorkspace(const Platform::Path& path) = 0;

};

}; // namespace MicroBuild

// Currently active plugin interface that all plugins that use
// this header should use.

#include "Schemas/Plugin/PluginInterface.h"

// Implements the interface required for a microbuild plugin to work.

#ifdef MB_PLATFORM_WINDOWS
#define DLL_EXPORT extern "C" __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

#define MicroBuildPlugin() \
	namespace MicroBuild { \
		bool PluginMain(IPluginInterface* pluginInterface); \
	} \
	DLL_EXPORT bool InitializePlugin(MicroBuild::IPluginInterface* pluginInterface) \
	{ \
		return MicroBuild::PluginMain(pluginInterface); \
	} \

