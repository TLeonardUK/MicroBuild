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

#include "Schemas/Workspace/WorkspaceFile.h"
#include "Schemas/Config/BaseConfigFile.h"

namespace MicroBuild {

#define SCHEMA_FILE "Schemas/BuildManifest/BuildManifestSchema.inc"
#define SCHEMA_CLASS BuildManifestFile
#include "Schemas/Config/SchemaGlobalDecl.h"
#undef SCHEMA_FILE
#undef SCHEMA_CLASS

class App;

// Stores intermediate data about the state of the last internal build pass.
class BuildManifestFile : public BaseConfigFile
{
public:
	BuildManifestFile(
		const Platform::Path& outputPath);
	~BuildManifestFile();

	virtual void Resolve() override;

	// Reads or writes this database file from its output path designated on
	// construction.
	bool Write();
	bool Read();

protected:

private:
	Platform::Path m_outputPath;

#define SCHEMA_FILE "Schemas/BuildManifest/BuildManifestSchema.inc"
#define SCHEMA_CLASS BuildManifestFile
#define SCHEMA_IS_DERIVED
#include "Schemas/Config/SchemaDecl.h"
#undef SCHEMA_FILE
#undef SCHEMA_CLASS
#undef SCHEMA_IS_DERIVED

};

}; // namespace MicroBuild