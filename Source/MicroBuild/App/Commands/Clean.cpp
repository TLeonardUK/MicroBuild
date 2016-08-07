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
#include "App/Database/DatabaseFile.h"

#include "Core/Commands/CommandLineParser.h"
#include "Core/Commands/CommandComboArgument.h"
#include "Core/Commands/CommandPathArgument.h"
#include "Core/Commands/CommandFlagArgument.h"
#include "Core/Helpers/Time.h"

namespace MicroBuild {

CleanCommand::CleanCommand(App* app)
	: m_app(app)
{
	SetName("clean");
	SetShortName("c");
	SetDescription("Cleans all temporary intermediate files generated "
				   "for the given workspace.");

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

bool CleanCommand::Invoke(CommandLineParser* parser)
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

		// Database file to do all file manipulation through.
		Platform::Path databaseFileLocation =
			m_workspaceFile.Get_Workspace_Location()
			.AppendFragment("workspace.mb", true);

		// If database already exists then clean the workspace.
		if (databaseFileLocation.Exists())
		{
			DatabaseFile databaseFile(databaseFileLocation, "");

			Log(LogSeverity::Info,
				"Found workspace, cleaning up.\n");

			if (databaseFile.Read())
			{
				if (!databaseFile.Clean(m_app, m_workspaceFile))
				{
					Log(LogSeverity::Warning,
						"Failed to clean workspace, workspace files may be in an indeterminate state.\n",
						databaseFileLocation.ToString().c_str());

					return false;
				}
				else
				{
					Log(LogSeverity::Info, "Finished cleaning in %.2f ms.\n",
						timingScope.GetElapsed());

					return true;
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