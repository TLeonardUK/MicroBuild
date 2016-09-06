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

#include "App/Workspace/WorkspaceFile.h"
#include "App/Config/BaseConfigFile.h"

namespace MicroBuild {

#define SCHEMA_FILE "App/Database/DatabaseSchema.inc"
#define SCHEMA_CLASS DatabaseFile
#include "App/Config/SchemaGlobalDecl.h"
#undef SCHEMA_FILE
#undef SCHEMA_CLASS

class App;

// Stores intermediate data about generated project files, used to implement
// cleaning and building.
class DatabaseFile : public BaseConfigFile
{
public:
	DatabaseFile(
		const Platform::Path& outputPathh,
		const std::string& targetIde);
	~DatabaseFile();

	virtual void Resolve() override;

	// Reads or writes this database file from its output path designated on
	// construction.
	bool Write();
	bool Read();

	// Erases all files and directories that have previously been created by
	// previous project file generation.
	bool Clean(
		App* app,
		WorkspaceFile& workspaceFile, 
		bool bDeleteProjectFiles = false
	);

	// Stores the given data in the given file and creates the directory
	// structure if required. This will also create a cleanup log for the file
	// and directory structure so they are removed when the workspace is
	// cleaned.
	bool StoreFile(
		WorkspaceFile& workspaceFile,
		Platform::Path& location,
		const char* data
	);

protected:

private:
	Platform::Path m_outputPath;

#define SCHEMA_FILE "App/Database/DatabaseSchema.inc"
#define SCHEMA_CLASS DatabaseFile
#define SCHEMA_IS_DERIVED
#include "App/Config/SchemaDecl.h"
#undef SCHEMA_FILE
#undef SCHEMA_CLASS
#undef SCHEMA_IS_DERIVED

};

}; // namespace MicroBuild