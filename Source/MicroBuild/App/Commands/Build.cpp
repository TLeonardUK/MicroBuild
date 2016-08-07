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
#include "App/Commands/Clean.h"
#include "App/Commands/Build.h"
#include "App/Database/DatabaseFile.h"

#include "Core/Commands/CommandLineParser.h"
#include "Core/Commands/CommandComboArgument.h"
#include "Core/Commands/CommandPathArgument.h"
#include "Core/Commands/CommandFlagArgument.h"
#include "Core/Commands/CommandStringArgument.h"
#include "Core/Helpers/Time.h"

namespace MicroBuild {

BuildCommand::BuildCommand(App* app)
	: m_app(app)
	, m_rebuild(false)
{
	SetName("build");
	SetShortName("b");
	SetDescription("Builds the project files that have previously been "
				   "generated.");

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

bool BuildCommand::Invoke(CommandLineParser* parser)
{
	UNUSED_PARAMETER(parser);

	Time::TimedScope timingScope;

	// Load the workspace.
	if (m_workspaceFile.Parse(m_workspaceFilePath))
	{
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
						m_platform
					))
					{
						Log(LogSeverity::Warning,
							"Failed to build workspace.\n",
							databaseFileLocation.ToString().c_str());

						return false;
					}
					else
					{
						Log(LogSeverity::Info, "Finished building in %.2f ms.\n",
							timingScope.GetElapsed());

						return true;
					}
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
				"Workspace database does not exist, nothing to build.\n",
				databaseFileLocation.ToString().c_str());

			return true;
		}
	}

	return false;
}

}; // namespace MicroBuild