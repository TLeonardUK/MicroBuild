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

#include "PCH.h"
#include "App/Workspace/WorkspaceFile.h"

namespace MicroBuild {

WorkspaceFile::WorkspaceFile()
{
}

WorkspaceFile::~WorkspaceFile()
{
}

void WorkspaceFile::Resolve()
{
	Set_Workspace_Directory(GetPath().GetDirectory());
	Set_Workspace_File(GetPath());

	BaseConfigFile::Resolve();
}

#define SCHEMA_FILE "App/Workspace/WorkspaceSchema.inc"
#define SCHEMA_CLASS WorkspaceFile
#include "App/Config/SchemaImpl.h"
#undef SCHEMA_FILE
#undef SCHEMA_CLASS

}; // namespace MicroBuild