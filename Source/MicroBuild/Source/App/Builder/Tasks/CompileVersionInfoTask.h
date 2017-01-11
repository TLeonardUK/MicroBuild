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
	
// Compiles an object file that contains the projects versioning information.
class CompileVersionInfoTask
	: public BuildTask
{
private:
	Toolchain* m_toolchain;
	BuilderFileInfo m_file;
	VersionNumberInfo m_versionInfo;

public:
	CompileVersionInfoTask(Toolchain* toolchain, ProjectFile& project, BuilderFileInfo file, VersionNumberInfo versionInfo);

	virtual bool Execute() override;

}; 

}; // namespace MicroBuild
