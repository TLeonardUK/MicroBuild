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

#include "Schemas/Project/ProjectFile.h"
#include "App/Builder/Toolchains/Toolchain.h"
#include "App/Builder/Tasks/BuildTask.h"

namespace MicroBuild {
	
// Executes a command line string through the default platform shell.
class ShellCommandTask
	: public BuildTask
{
private:
	std::string m_command;

public:
	ShellCommandTask(BuildStage stage, const std::string& command);

	virtual bool Execute() override;

}; 

}; // namespace MicroBuild