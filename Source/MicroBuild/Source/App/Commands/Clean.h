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
#include "Schemas/Workspace/WorkspaceFile.h"

namespace MicroBuild {

class App;

// Cleans all temporary intermediate files using the internal build tool.
class CleanCommand : public Command
{
public:
	CleanCommand(App* app);

protected:
	virtual bool Invoke(CommandLineParser* parser) override;

private:
	App* m_app;

	WorkspaceFile m_workspaceFile;
	Platform::Path m_workspaceFilePath;

	std::string m_projectName;

	std::string m_configuration;
	std::string m_platform;
};

}; // namespace MicroBuild