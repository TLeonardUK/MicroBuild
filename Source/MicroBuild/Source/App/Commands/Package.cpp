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
#include "Schemas/Database/DatabaseFile.h"
#include "App/Packager/PackagerType.h"

#include "Core/Commands/CommandLineParser.h"
#include "Core/Commands/CommandComboArgument.h"
#include "Core/Commands/CommandPathArgument.h"
#include "Core/Commands/CommandFlagArgument.h"
#include "Core/Commands/CommandStringArgument.h"
#include "Core/Commands/CommandMapArgument.h"
#include "Core/Helpers/Time.h"
#include "Core/Platform/Process.h"

namespace MicroBuild {

PackageCommand::PackageCommand(App* app)
	: m_rebuild(false)
	, m_app(app)
{
	MB_UNUSED_PARAMETER(app);

	SetName("package");
	SetShortName("p");
	SetDescription("Packages the project for the given configuration and "
				   "platform");

	std::vector<std::string> packagerOptions;
	std::vector<PackagerType*> ideTypes = m_app->GetPackagers();
	for (auto ide : ideTypes)
	{
		packagerOptions.push_back(ide->GetShortName());
	}

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

	CommandComboArgument* format = new CommandComboArgument();
	format->SetName("Target");
	format->SetShortName("t");
	format->SetDescription("Defines what target packager / format to use when packaging.");
	format->SetRequired(true);
	format->SetPositional(true);
	format->SetOptions(packagerOptions);
	format->SetOutput(&m_targetPackager);
	RegisterArgument(format);

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

bool PackageCommand::ExecuteCommand(const std::string& command, ProjectFile* projectFile)
{
	MB_UNUSED_PARAMETER(projectFile);

	std::vector<std::string> arguments = Strings::Crack(command, ' ', true);
	Platform::Path executable = Strings::StripQuotes(arguments[0]);
	arguments.erase(arguments.begin());

	Platform::Path baseDir = executable.GetDirectory();
	if (baseDir.IsRelative())
	{
		baseDir = Platform::Path::GetExecutablePath().GetDirectory();
	}

	Platform::Process process;
	if (!process.Open(executable, baseDir, arguments, false))
	{
		Log(LogSeverity::Fatal, "Failed to run command: %s\n", executable.ToString().c_str());
		return false;
	}

	process.Wait();
	int exitCode = process.GetExitCode();
	if (exitCode == 0)
	{
		return true;
	}
	else
	{
		Log(LogSeverity::Fatal, "Failed to run command (exited with code %i): %s\n", exitCode, executable.ToString().c_str());
	}

	return false;
}

bool PackageCommand::Invoke(CommandLineParser* parser)
{
	MB_UNUSED_PARAMETER(parser);

	Time::TimedScope timingScope;

	PackagerType* packager = m_app->GetPackagerByShortName(m_targetPackager);

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

				// Base configuration.
				m_workspaceFile.Set_Target_IDE(databaseFile.Get_Target_IDE());
				m_workspaceFile.Set_Target_Configuration(m_configuration);
				m_workspaceFile.Set_Target_Platform(platformId);
				m_workspaceFile.Set_Target_PlatformName(IdeHelper::ResolvePlatformName(platformId));
				m_workspaceFile.Set_Target_PackageDirectory(packager->GetContentDirectory(m_packageDirectoryPath));
				m_workspaceFile.Set_Target_Packager(m_targetPackager);

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
						
						projectFiles[i].Set_Target_IDE(databaseFile.Get_Target_IDE());
						projectFiles[i].Set_Target_Configuration(m_configuration);
						projectFiles[i].Set_Target_Platform(platformId);
						projectFiles[i].Set_Target_PlatformName(IdeHelper::ResolvePlatformName(platformId));
						projectFiles[i].Set_Target_PackageDirectory(packager->GetContentDirectory(m_packageDirectoryPath));
						projectFiles[i].Set_Target_Packager(m_targetPackager);

						for (auto& pair : m_setArguments)
						{
							m_workspaceFile.SetOrAddValue("", pair.first, pair.second, true);
						}

						projectFiles[i].Resolve();

						if (!projectFiles[i].Validate())
						{
							return false;
						}

						if (projectFiles[i].Get_Project_Name() == m_projectName)
						{
							buildProjectFile = &projectFiles[i];
						}
					}
					else
					{
						return false;
					}
				}

				if (buildProjectFile != nullptr)
				{
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
					if (buildProjectFile->Get_Packager_RequiresBuild())
					{
						Log(LogSeverity::SilentInfo, "Compiling project before packaging, using configuration %s_%s ...\n\n", m_configuration.c_str(), m_platform.c_str());

						std::vector<std::string> buildProjects = buildProjectFile->Get_Packager_BuildProject();
						if (buildProjects.size() == 0)
						{
							buildProjects.push_back(m_projectName);
						}

						for (auto& proj : buildProjects)
						{
							BuildCommand buildCommand(m_app);
							if (!buildCommand.IndirectInvoke(parser, m_workspaceFilePath, proj, m_rebuild, true, m_configuration, m_platform, true))
							{
								Log(LogSeverity::Warning,
									"Failed to compile workspace.\n");

								return false;
							}
						}
					}

					// Run pre-package commands.
					std::vector<std::string> commands = buildProjectFile->Get_PrePackageCommands_Command();
					for (auto& command : commands)
					{
						if (!ExecuteCommand(command, buildProjectFile))
						{
							return false;
						}
					}

					Log(LogSeverity::SilentInfo, "\nPackaging project for %s ...\n\n", packager->GetShortName().c_str());
					if (!packager->Package(*buildProjectFile, m_packageDirectoryPath))
					{
						Log(LogSeverity::Warning,
							"Failed to package project.\n");

						return false;
					}

					// Run post-package commands.
					commands = buildProjectFile->Get_PostPackageCommands_Command();
					for (auto& command : commands)
					{
						if (!ExecuteCommand(command, buildProjectFile))
						{
							return false;
						}
					}
					
					Log(LogSeverity::SilentInfo,
						"Successfully packaged project.\n");

					return true;
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
				"Workspace database does not exist, nothing to package.\n",
				databaseFileLocation.ToString().c_str());

			return true;
		}
	}

	return false;
}

}; // namespace MicroBuild
