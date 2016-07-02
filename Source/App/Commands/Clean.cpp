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
#include "App/Commands/Clean.h"
#include "Core/Commands/CommandLineParser.h"
#include "Core/Commands/CommandComboArgument.h"
#include "Core/Commands/CommandPathArgument.h"
#include "Core/Commands/CommandFlagArgument.h"
#include "Core/Commands/CommandStringArgument.h"

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
	workspaceFile->SetOutput(&m_workspaceFile);
	RegisterArgument(workspaceFile);
}

bool CleanCommand::Invoke(CommandLineParser* parser)
{
	// todo

	return true;
}

}; // namespace MicroBuild