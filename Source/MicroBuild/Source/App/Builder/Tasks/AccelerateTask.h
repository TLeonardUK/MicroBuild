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
#include "App/Builder/Accelerators/Accelerator.h"
#include "App/Builder/Tasks/BuildTask.h"

namespace MicroBuild {
	
// Runs a list of sub-tasks through a build accelerator.
class AccelerateTask
	: public BuildTask
{
private:
	Toolchain* m_toolchain;
	Accelerator* m_accelerator;
	std::vector<std::shared_ptr<BuildTask>> m_tasks;
	ProjectFile& m_projectFile;

public:
	AccelerateTask(ProjectFile& projectFile, std::vector<std::shared_ptr<BuildTask>>& tasks, Toolchain* toolchain, Accelerator* accelerator);

	virtual bool Execute() override;
	virtual BuildAction GetAction() override;

}; 

}; // namespace MicroBuild
