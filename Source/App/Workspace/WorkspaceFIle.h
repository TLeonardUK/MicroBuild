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

#include "Core/Platform/Path.h"
#include "Core/Helpers/Strings.h"
#include "App/Config/BaseConfigFile.h"

namespace MicroBuild {

#define SCHEMA_FILE "App/Workspace/WorkspaceSchema.inc"
#define SCHEMA_CLASS WorkspaceFile
#include "App/Config/SchemaGlobalDecl.h"
#undef SCHEMA_FILE
#undef SCHEMA_CLASS

// Stores and manages a configuration file for a workspace config file.
class WorkspaceFile : public BaseConfigFile
{
public:
	WorkspaceFile();
	~WorkspaceFile();

	virtual void Resolve() override;

	// Returns true if the configuration and platform both
	// exist in the workspace file.
	bool IsConfigurationValid(
		const std::string& configuration, 
		const std::string& platform);

protected:

private:

#define SCHEMA_FILE "App/Workspace/WorkspaceSchema.inc"
#define SCHEMA_CLASS WorkspaceFile
#include "App/Config/SchemaDecl.h"
#undef SCHEMA_FILE
#undef SCHEMA_CLASS

};

}; // namespace MicroBuild