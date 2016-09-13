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
#include "App/Commands/Generate.h"
#include "App/Database/DatabaseFile.h"

#include "Core/Commands/CommandLineParser.h"
#include "Core/Commands/CommandComboArgument.h"
#include "Core/Commands/CommandPathArgument.h"
#include "Core/Commands/CommandFlagArgument.h"
#include "Core/Helpers/Time.h"

namespace MicroBuild {

GenerateCommand::GenerateCommand(App* app)
	: m_app(app)
{
	SetName("generate");
	SetShortName("g");
	SetDescription("Generates project files for the given platform from "
				   "the given workspace file.");

	std::vector<std::string> ideOptions;
	std::vector<IdeType*> ideTypes = m_app->GetIdes();
	for (auto ide : ideTypes)
	{
		ideOptions.push_back(ide->GetShortName());
	}

	CommandComboArgument* targetIde = new CommandComboArgument();
	targetIde->SetName("TargetIDE");
	targetIde->SetShortName("t");
	targetIde->SetDescription("The target IDE that project files should be "
							  "generated for.");
	targetIde->SetOptions(ideOptions);
	targetIde->SetRequired(true);
	targetIde->SetPositional(true);
	targetIde->SetOutput(&m_targetIde);
	RegisterArgument(targetIde);

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
}

bool GenerateCommand::Invoke(CommandLineParser* parser)
{
	UNUSED_PARAMETER(parser);

	Time::TimedScope timingScope;

	// Load the workspace.
	std::vector<Platform::Path> includePaths;
	includePaths.push_back(m_workspaceFilePath.GetDirectory());

	if (m_workspaceFile.Parse(m_workspaceFilePath, includePaths))
	{
		// Base configuration.
		m_workspaceFile.Set_Target_IDE(m_targetIde);

		m_workspaceFile.Resolve();
		if (!m_workspaceFile.Validate())
		{
			return false;
		}

		// Fire plugin events!
		{
			PluginPostProcessProjectFileData eventData;
			eventData.File = &m_workspaceFile;
			m_app->GetPluginManager()->OnEvent(EPluginEvent::PostProcessWorkspaceFile, &eventData);
		}

		// Database file to do all file manipulation through.
		Platform::Path databaseFileLocation =
			m_workspaceFile.Get_Workspace_Location()
			.AppendFragment("workspace.mb", true);

		// If database already exists then clean the workspace.
		if (databaseFileLocation.Exists())
		{
			DatabaseFile databaseFile(databaseFileLocation, "");

			Log(LogSeverity::Info,
				"Workspace already exists, cleaning old workspace.\n");

			if (databaseFile.Read())
			{
				if (!databaseFile.Clean(m_app, m_workspaceFile))
				{
					Log(LogSeverity::Warning,
						"Failed to clean workspace, generated project may not be correctly cleanable in future.\n",
						databaseFileLocation.ToString().c_str());
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

		// Load all projects.
		std::vector<Platform::Path> projectPaths =
			m_workspaceFile.Get_Projects_Project();

		m_projectFiles.resize(projectPaths.size());

		for (unsigned int i = 0; i < projectPaths.size(); i++)
		{
			std::vector<Platform::Path> subIncludePaths;
			subIncludePaths.push_back(projectPaths[i].GetDirectory());
			subIncludePaths.insert(
				subIncludePaths.end(),
				includePaths.begin(), includePaths.end()
			);

			if (m_projectFiles[i].Parse(projectPaths[i], subIncludePaths))
			{
				m_projectFiles[i].Merge(m_workspaceFile);

				// Base configuration.
				m_projectFiles[i].Set_Target_IDE(m_targetIde);

				m_projectFiles[i].Resolve();
				if (!m_projectFiles[i].Validate())
				{
					return false;
				}

				// Fire plugin events!
				{
					PluginPostProcessProjectFileData eventData;
					eventData.File = &m_projectFiles[i];
					m_app->GetPluginManager()->OnEvent(EPluginEvent::PostProcessProjectFile, &eventData);
				}
			}
			else
			{
				return false;
			}
		}

		// Find and generate project files for our chosen ide.
		IdeType* type = m_app->GetIdeByShortName(m_targetIde);
		assert(type != nullptr);

		Log(LogSeverity::Info, "Generating project files for '%s'.\n",
			m_targetIde.c_str());

		DatabaseFile outputDatabaseFile(databaseFileLocation, m_targetIde);
        
        // Store some generic database properties.
        std::vector<std::string> projectNames;
        std::vector<std::string> configNames;
        std::vector<EPlatform> platformNames;
        for (ProjectFile& file : m_projectFiles)
        {
            projectNames.push_back(file.Get_Project_Name());
        }
        for (std::string value : m_workspaceFile.Get_Configurations_Configuration())
        {
            configNames.push_back(value);
        }
        for (EPlatform value : m_workspaceFile.Get_Platforms_Platform())
        {
            platformNames.push_back(value);
        }
        outputDatabaseFile.Set_Workspace_Project(projectNames);
        outputDatabaseFile.Set_Workspace_Configuration(configNames);
        outputDatabaseFile.Set_Workspace_Platform(platformNames);

		if (type->Generate(outputDatabaseFile, m_workspaceFile, m_projectFiles))
		{
			// Write database file.
			if (!outputDatabaseFile.Write())
			{
				Log(LogSeverity::Fatal, "Failed to write workspace database to '%s'.\n",
					databaseFileLocation.ToString().c_str());

				return false;
			}

			Log(LogSeverity::Info, "Finished generation in %.2f ms.\n",
				timingScope.GetElapsed());

			return true;
		}
	}

	return false;
}

}; // namespace MicroBuild
