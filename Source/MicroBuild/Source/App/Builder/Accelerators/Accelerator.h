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

#include "App/Builder/BuilderFileInfo.h"
#include "Schemas/Project/ProjectFile.h"

namespace MicroBuild {

class BuildTask;
class Toolchain;

// Base class for all accelerator providers.
class Accelerator
{
protected:
	bool m_bAvailable;
	std::string m_description;

public:

	// Constructor.
	Accelerator();

	// Returns a description that describes this toolchain and its version.
	std::string GetDescription();

	// Returns true if this accelerator is available for use.
	bool IsAvailable();

	// Attempts to find the accelerator install, returns true if everything is available,	
	// otherwise false.
	virtual bool Init() = 0;

	// Runs all the actions in distribution.
	virtual bool RunActions(Toolchain* toolchain, BuildTask* baseTask, ProjectFile& projectFile, std::vector<BuildAction>& actions) = 0;

}; 

}; // namespace MicroBuild
