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
#include "App/App.h"
#include "App/Plugin/Plugin.h"
#include "App/Plugin/PluginManager.h"
#include "Schemas/Workspace/WorkspaceFile.h"
#include "Schemas/Project/ProjectFile.h"
#include "App/Ides/IdeHelper.h"
#include "Schemas/Plugin/PluginInterface.h"

namespace MicroBuild {

// Project wrapper for plugin states.
class ProjectState_Impl : public IProjectState
{
public:
	bool Read(
		ProjectFile& baseFile,
		const std::string& config,
		EPlatform platform
	)
	{
		m_file = baseFile;
		m_file.Set_Target_Configuration(config);
		m_file.Set_Target_Platform(platform);
		m_file.Set_Target_PlatformName(IdeHelper::ResolvePlatformName(platform));
		m_file.Set_Target_MicroBuildExecutable(Platform::Path::GetExecutablePath());
		m_file.Set_Target_MicroBuildDirectory(Platform::Path::GetExecutablePath().GetDirectory());

		m_file.Resolve();
		if (!m_file.Validate())
		{
			return false;
		}

		return true;
	}

	virtual ProjectFile& GetConfigFile() override
	{
		return m_file;
	}

	virtual std::string GetConfiguration() override
	{
		return m_file.Get_Target_Configuration();
	}

	virtual std::string GetPlatform() override
	{
		return CastToString(m_file.Get_Target_Platform());
	}

private:
	ProjectFile m_file;

};

// Project wrapper for plugins.
class Project_Impl : public IProject
{
public:
	virtual std::shared_ptr<IProjectState> GetState(
		const std::string& config, 
		const std::string& platform
	) override
	{
		for (auto state : m_states)
		{
			if (state->GetConfiguration() == config &&
				state->GetPlatform() == platform)
			{
				return state;
			}
		}

		return nullptr;
	}

	virtual std::string GetName() override
	{
		return m_file.Get_Project_Name();
	}

	bool Read(
		const Platform::Path& path,
		WorkspaceFile& workspaceFile,
		std::vector<Platform::Path> includePaths,
		const std::string& targetIde
	)
	{
		std::vector<std::string> configurations =
			workspaceFile.Get_Configurations_Configuration();

		std::vector<EPlatform> platforms =
			workspaceFile.Get_Platforms_Platform();

		if (m_file.Parse(path, includePaths))
		{
			m_file.Merge(workspaceFile);
			m_file.Set_Target_IDE(targetIde);
			m_file.Set_Target_MicroBuildExecutable(Platform::Path::GetExecutablePath());
			m_file.Set_Target_MicroBuildDirectory(Platform::Path::GetExecutablePath().GetDirectory());

			m_file.Resolve();
			if (!m_file.Validate())
			{
				return false;
			}

			// Resolve each configuration/platform combination and extract the 
			// information that we require.
			for (std::string& config : configurations)
			{
				for (EPlatform& platform : platforms)
				{
					std::shared_ptr<IProjectState> statePtr = std::make_shared<ProjectState_Impl>();
					if (static_cast<ProjectState_Impl*>(statePtr.get())->Read(m_file, config, platform))
					{
						m_states.push_back(statePtr);
					}
					else
					{
						return false;
					}
				}
			}

			return true;
		}

		return false;
	}

private:
	ProjectFile m_file;
	std::vector<std::shared_ptr<IProjectState>> m_states;

};

// Workspace wrapper for plugins.
class Workspace_Impl : public IWorkspace
{
public:
	virtual std::shared_ptr<IProject> GetProject(
		const std::string& projectName
	) override
	{
		for (auto project : m_projects)
		{
			if (project->GetName() == projectName)
			{
				return project;
			}
		}

		return nullptr;
	}

	virtual std::vector<std::shared_ptr<IProject>> GetProjects() override
	{
		return m_projects;
	}

	virtual WorkspaceFile& GetConfigFile() override
	{
		return m_file;
	}

	bool Read(const Platform::Path& path)
	{
		std::vector<Platform::Path> includePaths;
		includePaths.push_back(path.GetDirectory());

		// Read in workspace.
		if (m_file.Parse(path, includePaths))
		{
			m_file.Resolve();
			if (!m_file.Validate())
			{
				return false;
			}

			// Read in database to extract target-ide etc settings.
			Platform::Path databaseFileLocation =
				m_file.Get_Workspace_Location()
				.AppendFragment("workspace.mb", true);

			std::string targetIde = "";
			if (databaseFileLocation.Exists())
			{
				DatabaseFile databaseFile(databaseFileLocation, "");

				if (databaseFile.Read())
				{
					targetIde = databaseFile.Get_Target_IDE();
				}
				else
				{
					Log(LogSeverity::Fatal,
						"Failed to read workspace database '%s'.\n",
						databaseFileLocation.ToString().c_str());

					return false;
				}
			}
			else
			{
				Log(LogSeverity::Fatal, "Database file does not exist, have project files been generated?\n");
				return false;
			}

			// Re-resolve with new target-ide.
			m_file.Set_Target_IDE(targetIde);
			m_file.Resolve();
			if (!m_file.Validate())
			{
				return false;
			}

			// Load all projects.
			std::vector<Platform::Path> projectPaths =
				m_file.Get_Projects_Project();

			for (unsigned int i = 0; i < projectPaths.size(); i++)
			{
				std::vector<Platform::Path> subIncludePaths;
				subIncludePaths.push_back(projectPaths[i].GetDirectory());
				subIncludePaths.insert(
					subIncludePaths.end(),
					includePaths.begin(), includePaths.end()
				);

				std::shared_ptr<IProject> ptr = std::make_shared<Project_Impl>();
				if (static_cast<Project_Impl*>(ptr.get())->Read(projectPaths[i], m_file, subIncludePaths, targetIde))
				{
					m_projects.push_back(ptr);
				}
				else
				{
					return false;
				}
			}

			return true;
		}

		return false;
	}

private:
	WorkspaceFile m_file;
	std::vector<std::shared_ptr<IProject>> m_projects;

};

// Base class for all plugin interfaces.
class PluginInterface_Impl : public IPluginInterface
{
public:
	PluginInterface_Impl(PluginManager* manager, Plugin* plugin)
		: m_manager(manager)
		, m_plugin(plugin)
	{
	}

	virtual void SetName(const std::string& value) override
	{
		m_name = value;
	}

	virtual std::string GetName() const override
	{
		return m_name;
	}

	virtual void SetDescription(const std::string& value) override
	{
		m_description = value;
	}

	virtual std::string GetDescription() const override
	{
		return m_description;
	}

	virtual void RegisterCommand(Command* cmd) override
	{
		m_manager->GetApp()->RegisterCommand(cmd);
	}

	virtual void UnregisterCommand(Command* cmd) override
	{
		m_manager->GetApp()->UnregisterCommand(cmd);
	}

	virtual void RegisterCallback(EPluginEvent Event, PluginCallbackSignature FuncPtr) override
	{
		m_plugin->RegisterCallback(Event, FuncPtr);
	}

	virtual std::shared_ptr<IWorkspace> ReadWorkspace(const Platform::Path& path) override
	{	
		std::shared_ptr<IWorkspace> ptr = std::make_shared<Workspace_Impl>();
		if (static_cast<Workspace_Impl*>(ptr.get())->Read(path))
		{
			return ptr;
		}
		return nullptr;
	}

private:
	std::string m_name;
	std::string m_description;
	PluginManager* m_manager;
	Plugin* m_plugin;

};

IPluginInterface* CreatePluginInterface(PluginManager* manager, Plugin* plugin)
{
	return new PluginInterface_Impl(manager, plugin);
}

} // namespace MicroBuild