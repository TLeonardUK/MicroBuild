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
#include "App/Ides/IdeType.h"
#include "App/Ides/IdeHelper.h"
#include "App/Commands/Clean.h"
#include "App/Commands/Build.h"
#include "Schemas/Database/DatabaseFile.h"

#include "App/Builder/Builder.h"

#include "Core/Commands/CommandLineParser.h"
#include "Core/Commands/CommandComboArgument.h"
#include "Core/Commands/CommandPathArgument.h"
#include "Core/Commands/CommandFlagArgument.h"
#include "Core/Commands/CommandStringArgument.h"
#include "Core/Commands/CommandMapArgument.h"
#include "Core/Helpers/Time.h"

namespace MicroBuild {

BuildCommand::BuildCommand(App* app)
	: m_app(app)
	, m_rebuild(false)
	, m_buildPackageFiles(false)
{
	SetName("build");
	SetShortName("b");
	SetDescription("Builds the given project that has been generated "
				   "using the internal build tool.");

	CommandPathArgument* workspaceFile = new CommandPathArgument();
	workspaceFile->SetName("WorkspaceFile");
	workspaceFile->SetShortName("w");
	workspaceFile->SetDescription("The workspace file that defines how the"
								  "project files should be generated.");
	workspaceFile->SetExpectsDirectory(false);
	workspaceFile->SetExpectsExisting(true);
	workspaceFile->SetRequired(true);
	workspaceFile->SetPositional(true);
	workspaceFile->SetOutput(&m_workspaceFilePath);
	RegisterArgument(workspaceFile);

	CommandStringArgument* projectFile = new CommandStringArgument();
	projectFile->SetName("ProjectFile");
	projectFile->SetShortName("j");
	projectFile->SetDescription("The name of the project to build.");
	projectFile->SetRequired(true);
	projectFile->SetPositional(true);
	projectFile->SetOutput(&m_projectName);
	RegisterArgument(projectFile);

	CommandFlagArgument* rebuild = new CommandFlagArgument();
	rebuild->SetName("Rebuild");
	rebuild->SetShortName("r");
	rebuild->SetDescription("Operation will not be performed incrementally "
							"even if it appears nothing has changed.");
	rebuild->SetRequired(false);
	rebuild->SetPositional(false);
	rebuild->SetDefault(false);
	rebuild->SetOutput(&m_rebuild);
	RegisterArgument(rebuild);

	CommandFlagArgument* builddeps = new CommandFlagArgument();
	builddeps->SetName("BuildDependencies");
	builddeps->SetShortName("d");
	builddeps->SetDescription("All project dependencies will be built as well as well.");
	builddeps->SetRequired(false);
	builddeps->SetPositional(false);
	builddeps->SetDefault(true);
	builddeps->SetOutput(&m_buildDependencies);
	RegisterArgument(builddeps);

	CommandStringArgument* configuration = new CommandStringArgument();
	configuration->SetName("Configuration");
	configuration->SetShortName("c");
	configuration->SetDescription("Defines the configuration of the project "
								  "that should be built.");
	configuration->SetRequired(false);
	configuration->SetPositional(false);
	configuration->SetDefault("");
	configuration->SetOutput(&m_configuration);
	RegisterArgument(configuration);

	CommandStringArgument* platform = new CommandStringArgument();
	platform->SetName("Platform");
	platform->SetShortName("p");
	platform->SetDescription("Defines the platform this project should "
							 "be built for.");
	platform->SetRequired(false);
	platform->SetPositional(false);
	platform->SetDefault("");
	platform->SetOutput(&m_platform);
	RegisterArgument(platform);

	CommandMapArgument* setArguments = new CommandMapArgument();
	setArguments->SetName("Defines");
	setArguments->SetShortName("set");
	setArguments->SetDescription("Defines a key-value pair that is defined as a variable "
		"when workspace and project files are parsed.");
	setArguments->SetRequired(false);
	setArguments->SetPositional(false);
	setArguments->SetOutput(&m_setArguments);
	RegisterArgument(setArguments);
}

bool BuildCommand::IndirectInvoke(
	CommandLineParser* parser,
	const Platform::Path& workspacePath,
	const std::string& projectName,
	bool bRebuild,
	bool bBuildDependencies,
	const std::string& configuration,
	const std::string& platform,
	bool bBuildPackageFiles)
{
	m_workspaceFilePath = workspacePath;
	m_projectName = projectName;
	m_rebuild = bRebuild;
	m_buildDependencies = bBuildDependencies;
	m_configuration = configuration;
	m_platform = platform;
	m_buildPackageFiles = bBuildPackageFiles;

	return Invoke(parser);
}

bool BuildCommand::Invoke(CommandLineParser* parser)
{
	MB_UNUSED_PARAMETER(parser);

	Time::TimedScope timingScope;

	EPlatform platformId = CastFromString<EPlatform>(m_platform);

	// Load the workspace.
	std::vector<Platform::Path> includePaths;
	includePaths.push_back(m_workspaceFilePath.GetDirectory());

	if (m_workspaceFile.Parse(m_workspaceFilePath, includePaths))
	{
		m_workspaceFile.Resolve();
		if (!m_workspaceFile.Validate())
		{
			return false;
		}
		
		// Fire plugin events!
		{
			PluginPostProcessWorkspaceFileData eventData;
			eventData.File = &m_workspaceFile;
			m_app->GetPluginManager()->OnEvent(EPluginEvent::PostProcessWorkspaceFile, &eventData);

			// Reresolve in case it was changed.
			m_workspaceFile.Resolve();
			if (!m_workspaceFile.Validate())
			{
				return false;
			}
		}
	
		if (!m_workspaceFile.IsConfigurationValid(m_configuration, m_platform))
		{
			Log(LogSeverity::Fatal,
				"Configuration %s|%s is not valid.\n",
				m_configuration.c_str(),
				m_platform.c_str());

			return false;
		}

		// Database file to do all file manipulation through.
		Platform::Path databaseFileLocation =
			m_workspaceFile.Get_Workspace_Location()
			.AppendFragment("workspace.mb", true);

		// If database already exists then build the workspace.
		if (databaseFileLocation.Exists())
		{
			DatabaseFile databaseFile(databaseFileLocation, "");

			if (databaseFile.Read())
			{
				// Base configuration.
				m_workspaceFile.Set_Target_IDE(databaseFile.Get_Target_IDE());
				m_workspaceFile.Set_Target_Configuration(m_configuration);
				m_workspaceFile.Set_Target_Platform(platformId);
				m_workspaceFile.Set_Target_PlatformName(IdeHelper::ResolvePlatformName(platformId));

				for (auto& pair : m_setArguments)
				{
					m_workspaceFile.SetOrAddValue("", pair.first, pair.second, true);
				}

				m_workspaceFile.Resolve();

				if (!m_workspaceFile.Validate())
				{
					return false;
				}

				if (!m_workspaceFile.IsConfigurationValid(m_configuration, m_platform))
				{
					Log(LogSeverity::Fatal,
						"Configuration %s|%s is not valid.\n",
						m_configuration.c_str(),
						m_platform.c_str());

					return false;
				}

				// Load all projects.
				std::vector<Platform::Path> projectPaths =
					m_workspaceFile.Get_Projects_Project();

				std::vector<ProjectFile> projectFiles;
				projectFiles.resize(projectPaths.size());

				ProjectFile* buildProjectFile = nullptr;

				for (unsigned int i = 0; i < projectPaths.size(); i++)
				{
					std::vector<Platform::Path> subIncludePaths;
					subIncludePaths.push_back(projectPaths[i].GetDirectory());
					subIncludePaths.insert(
						subIncludePaths.end(),
						includePaths.begin(), includePaths.end()
					);

					if (projectFiles[i].Parse(projectPaths[i], subIncludePaths))
					{
						projectFiles[i].Merge(m_workspaceFile);
						projectFiles[i].Set_Target_Configuration(m_configuration);
						projectFiles[i].Set_Target_Platform(platformId);
						projectFiles[i].Set_Target_PlatformName(IdeHelper::ResolvePlatformName(platformId));
						projectFiles[i].Set_Target_MicroBuildExecutable(Platform::Path::GetExecutablePath());
						projectFiles[i].Set_Target_IDE(databaseFile.Get_Target_IDE());

						projectFiles[i].Resolve();

						if (!projectFiles[i].Validate())
						{
							return false;
						}
					
						// Fire plugin events!
						{
							PluginPostProcessProjectFileData eventData;
							eventData.File = &projectFiles[i];
							m_app->GetPluginManager()->OnEvent(EPluginEvent::PostProcessProjectFile, &eventData);

							// Reresolve in case it was changed.
							projectFiles[i].Resolve();
							if (!projectFiles[i].Validate())
							{
								return false;
							}
						}

						if (projectFiles[i].Get_Project_Name() == m_projectName)
						{
							buildProjectFile = &projectFiles[i];
						}
					}
				}

				if (buildProjectFile != nullptr)
				{					
					std::vector<ProjectFile*> configProjectFiles;	
					for (unsigned int i = 0; i < projectFiles.size(); i++)
					{
						configProjectFiles.push_back(&projectFiles[i]);
					}

					if (!IdeHelper::UpdateAutoLinkDependencies(m_workspaceFile, configProjectFiles))
					{
						return false;
					}

					Builder builder(m_app);
					if (builder.Build(m_workspaceFile, configProjectFiles, *buildProjectFile, m_rebuild, m_buildDependencies, m_buildPackageFiles))
					{
						return true;
					}
				}
				else
				{
					Log(LogSeverity::Fatal,
						"Failed to find project '%s' in workspace.",
						m_projectName.c_str());
					return false;
				}
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
			Log(LogSeverity::Info,
				"Workspace database does not exist, nothing to clean.\n",
				databaseFileLocation.ToString().c_str());

			return true;
		}
	}

	return false;
}

}; // namespace MicroBuild