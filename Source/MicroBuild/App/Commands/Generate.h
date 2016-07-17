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

#pragma once

#include "Core/Commands/Command.h"
#include "Core/Platform/Path.h"
#include "App/Workspace/WorkspaceFile.h"
#include "App/Project/ProjectFile.h"

namespace MicroBuild {

class App;

// Generates a set of project files from the given workspace file.
class GenerateCommand : public Command
{
public:
	GenerateCommand(App* app);

protected:
	virtual bool Invoke(CommandLineParser* parser) override;

private:
	App* m_app;

	std::string m_targetIde;

	WorkspaceFile m_workspaceFile;
	std::vector<ProjectFile> m_projectFiles;

	Platform::Path m_workspaceFilePath;

};

}; // namespace MicroBuild