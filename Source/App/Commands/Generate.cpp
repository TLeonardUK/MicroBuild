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
	Time::TimedScope timingScope;

	// Load the workspace.
	if (m_workspaceFile.Parse(m_workspaceFilePath))
	{
		m_workspaceFile.Resolve();

		if (!m_workspaceFile.Validate())
		{
			return false;
		}

		// Load all projects.
		std::vector<Platform::Path> projectPaths =
			m_workspaceFile.Get_Projects_Project();

		m_projectFiles.resize(projectPaths.size());

		for (unsigned int i = 0; i < projectPaths.size(); i++)
		{
			if (m_projectFiles[i].Parse(projectPaths[i]))
			{
				m_projectFiles[i].Merge(m_workspaceFile);
				m_projectFiles[i].Resolve();
			}
			else
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}

	// If we are regenerating, delete any existing files.
	if (m_regenerate)
	{
		Log(LogSeverity::Info, 
			"Performing full rebuilding, deleting old files.\n",
			m_targetIde.c_str());
		
		// todo
	}

	// Find and generate project files for our chosen ide.
	IdeType* type = m_app->GetIdeByShortName(m_targetIde);
	assert(type != nullptr);

	Log(LogSeverity::Info, "Generating project files for '%s'.\n", 
		m_targetIde.c_str());

	if (type->Generate(m_workspaceFile, m_projectFiles))
	{
		Log(LogSeverity::Info, "Finished generation in %.2f ms.\n",
			timingScope.GetElapsed());

		return true;
	}

	return true;
}

}; // namespace MicroBuild