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
#include "App/Commands/Package.h"
#include "App/Commands/Build.h"
#include "App/Database/DatabaseFile.h"
#include "App/Packager/Packager.h"

#include "Core/Commands/CommandLineParser.h"
#include "Core/Commands/CommandComboArgument.h"
#include "Core/Commands/CommandPathArgument.h"
#include "Core/Commands/CommandFlagArgument.h"
#include "Core/Commands/CommandStringArgument.h"
#include "Core/Helpers/Time.h"

namespace MicroBuild {

PackageCommand::PackageCommand(App* app)
	: m_app(app)
	, m_rebuild(false)
{
	SetName("package");
	SetShortName("p");
	SetDescription("Packages the project for the given configuration and "
				   "platform");

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

	CommandPathArgument* packageDirectory = new CommandPathArgument();
	packageDirectory->SetName("PackageDirectory");
	packageDirectory->SetShortName("o");
	packageDirectory->SetDescription("The output directory that the package "
									 "project will be stored in.");
	packageDirectory->SetExpectsDirectory(true);
	packageDirectory->SetExpectsExisting(false);
	packageDirectory->SetRequired(true);
	packageDirectory->SetPositional(true);
	packageDirectory->SetOutput(&m_packageDirectoryPath);
	RegisterArgument(packageDirectory);

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
}

bool PackageCommand::Invoke(CommandLineParser* parser)
{
	UNUSED_PARAMETER(parser);

	Time::TimedScope timingScope;

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

		// Database file to do all file manipulation through.
		Platform::Path databaseFileLocation =
			m_workspaceFile.Get_Workspace_Location()
			.AppendFragment("workspace.mb", true);

		// If database already exists then clean the workspace.
		if (databaseFileLocation.Exists())
		{
			DatabaseFile databaseFile(databaseFileLocation, "");

			if (databaseFile.Read())
			{
				EPlatform platformId = CastFromString<EPlatform>(m_platform);

				// Ask IDE we originally tided up to clean up any build artifacts.
				IdeType* type = m_app->GetIdeByShortName(databaseFile.Get_Target_IDE());
				if (type == nullptr)
				{
					Log(LogSeverity::Fatal,
						"Could not find original target ide for workspace '%s'.\n",
						databaseFile.Get_Target_IDE().c_str());

					return false;
				}
				else
				{
					if (!type->Build(
						m_workspaceFile,
						m_rebuild,
						m_configuration,
						m_platform,
                        databaseFile
					))
					{
						Log(LogSeverity::Warning,
							"Failed to build workspace.\n",
							databaseFileLocation.ToString().c_str());

						return false;
					}
				}

				// Base configuration.
				m_workspaceFile.Set_Target_IDE(databaseFile.Get_Target_IDE());
				m_workspaceFile.Set_Target_Configuration(m_configuration);
				m_workspaceFile.Set_Target_Platform(platformId);
				m_workspaceFile.Set_Target_PlatformName(IdeHelper::ResolvePlatformName(platformId));
				m_workspaceFile.Set_Target_PackageDirectory(m_packageDirectoryPath);

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
						
						projectFiles[i].Set_Target_IDE(databaseFile.Get_Target_IDE());
						projectFiles[i].Set_Target_Configuration(m_configuration);
						projectFiles[i].Set_Target_Platform(platformId);
						m_workspaceFile.Set_Target_PlatformName(IdeHelper::ResolvePlatformName(platformId));
						m_workspaceFile.Set_Target_PackageDirectory(m_packageDirectoryPath);
						
						projectFiles[i].Resolve();

						if (!projectFiles[i].Validate())
						{
							return false;
						}

					}
					else
					{
						return false;
					}
				}

				// Generate the package command list.
				std::vector<std::string> packageCommands = 
					m_workspaceFile.Get_Package_Command();

				for (unsigned int i = 0; i < projectPaths.size(); i++)
				{
					std::vector<std::string> projectCommands = 
						projectFiles[i].Get_Package_Command();

					packageCommands.insert(
						packageCommands.begin(), 
						projectCommands.begin(),
						projectCommands.end()
					);
				}

				// Delete the folder we are packaging if it exists.
				if (m_packageDirectoryPath.Exists())
				{
					if (!m_packageDirectoryPath.Delete())
					{
						Log(LogSeverity::Warning,
							"Failed to delete package folder '%s'.\n", 
							m_packageDirectoryPath.ToString().c_str());

						return false;
					}
				}

				// Now do packaging commands.
				Packager packager;
				if (!packager.Package(m_packageDirectoryPath, packageCommands))
				{
					Log(LogSeverity::Warning,
						"Failed to build package.\n");

					return false;
				}

				return true;
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
				"Workspace database does not exist, nothing to package.\n",
				databaseFileLocation.ToString().c_str());

			return true;
		}
	}

	return false;
}

}; // namespace MicroBuild