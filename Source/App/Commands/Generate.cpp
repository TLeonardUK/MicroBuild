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
#include "App/Commands/Generate.h"
#include "Core/Commands/CommandLineParser.h"
#include "Core/Commands/CommandComboArgument.h"
#include "Core/Commands/CommandPathArgument.h"
#include "Core/Commands/CommandFlagArgument.h"

namespace MicroBuild {

GenerateCommand::GenerateCommand()
{
	SetName("generate");
	SetShortName("g");
	SetDescription("Generates project files for the given platform from "
				   "the given workspace file.");

	CommandComboArgument* targetIde = new CommandComboArgument();
	targetIde->SetName("TargetIDE");
	targetIde->SetShortName("t");
	targetIde->SetDescription("The target IDE that project files should be "
							  "generated for.");
	targetIde->SetOptions({ "vs2015", "xcode4", "makefile" });
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
	workspaceFile->SetOutput(&m_workspaceFile);
	RegisterArgument(workspaceFile);

	CommandFlagArgument* regenerate = new CommandFlagArgument();
	regenerate->SetName("Rebuild");
	regenerate->SetShortName("r");
	regenerate->SetDescription("Operation will not be performed incrementally "
								"even if it appears nothing has changed.");
	regenerate->SetRequired(false);
	regenerate->SetPositional(false);
	regenerate->SetDefault(false);
	regenerate->SetOutput(&m_regenerate);
	RegisterArgument(regenerate);
}

bool GenerateCommand::Invoke(CommandLineParser* parser)
{
	// todo

	return true;
}

}; // namespace MicroBuild