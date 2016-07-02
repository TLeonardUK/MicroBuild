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

#include "App/Workspace/WorkspaceFile.h"
#include "Core/Platform/Path.h"
#include "App/Config/BaseConfigFile.h"

namespace MicroBuild {

// Stores and manages a configuration file for a workspace config file.
class WorkspaceFile : public BaseConfigFile
{
public:
	WorkspaceFile();
	~WorkspaceFile();

	// Gets all build configurations defined by this workspace file.
	std::vector<std::string> GetConfigurations();

	// Gets all build platforms defined by this workspace file.
	std::vector<std::string> GetPlatform();

	// Gets all the project files that were referenced by this workspace file.
	std::vector<Platform::Path> GetProjectPaths();

	virtual void Resolve() override;

protected:
	
	// If path is relative it is made absolute based on the workspace file 
	// path, otherwise it is returned as-is.
	Platform::Path ResolvePath(Platform::Path& value);

private:


};

}; // namespace MicroBuild