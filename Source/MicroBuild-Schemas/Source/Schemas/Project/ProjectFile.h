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
#include "Core/Helpers/Strings.h"
#include "Schemas/Config/BaseConfigFile.h"

namespace MicroBuild {

#define SCHEMA_FILE "Schemas/Project/ProjectSchema.inc"
#define SCHEMA_CLASS ProjectFile
#include "Schemas/Config/SchemaGlobalDecl.h"
#undef SCHEMA_FILE
#undef SCHEMA_CLASS

// Stores and manages a configuration file for a project config file.
class ProjectFile : public BaseConfigFile
{
public:
	ProjectFile();
	~ProjectFile();

	virtual void Resolve() override;

protected:

private:

#define SCHEMA_FILE "Schemas/Project/ProjectSchema.inc"
#define SCHEMA_CLASS ProjectFile
#define SCHEMA_IS_DERIVED
#include "Schemas/Config/SchemaDecl.h"
#undef SCHEMA_IS_DERIVED
#undef SCHEMA_FILE
#undef SCHEMA_CLASS

};

}; // namespace MicroBuild