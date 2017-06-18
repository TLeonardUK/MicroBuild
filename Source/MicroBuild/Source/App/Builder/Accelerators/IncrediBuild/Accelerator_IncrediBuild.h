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

#include "App/Builder/Accelerators/Accelerator.h"

#include "Core/Platform/Path.h"

namespace MicroBuild {

// Provides incredibuild distributed compilation.
class Accelerator_IncrediBuild
	: public Accelerator
{
protected:
	Platform::Path m_xgConsolePath;

public:

	// Constructor.
	Accelerator_IncrediBuild();

	// Attempts to find the accelerator install, returns true if everything is available,	
	// otherwise false.
	virtual bool Init() override;

	// Runs all the actions in distribution.
	virtual bool RunActions(Toolchain* toolchain, BuildTask* baseTask, ProjectFile& projectFile, std::vector<BuildAction>& actions) override;

}; 

}; // namespace MicroBuild
