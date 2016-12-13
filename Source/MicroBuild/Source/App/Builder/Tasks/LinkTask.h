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
	
// Links all the files given into a final output binary.
class LinkTask
	: public BuildTask
{
private:
	Toolchain* m_toolchain;
	std::vector<BuilderFileInfo>& m_sourceFiles;
	BuilderFileInfo m_outputFile;

public:
	LinkTask(std::vector<BuilderFileInfo>& sourceFiles, Toolchain* toolchain, ProjectFile& project, BuilderFileInfo& outputFile);

	virtual bool Execute() override;

}; 

}; // namespace MicroBuild
