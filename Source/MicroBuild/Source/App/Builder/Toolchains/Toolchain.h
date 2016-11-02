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
#include "App/Builder/BuilderFileInfo.h"

#include <string>

namespace MicroBuild {
	
// Base class for all internal builder toolchains.
class Toolchain
{
protected:
	bool m_bAvailable;
	bool m_bRequiresCompileStep;
	std::string m_description;
	ProjectFile& m_projectFile;

public:

	// Constructor.
	Toolchain(ProjectFile& file);

	// Returns a description that describes this toolchain and its version.
	std::string GetDescription();

	// Returns true if this toolchain is available for use.
	bool IsAvailable();

	// Returns true if we require an individual compile-call for each
	// source file, if not we jump straight to the Archive/Link step.
	bool RequiresCompileStep();

	// Compiles the PCH described in the project file.	
	virtual bool CompilePch(BuilderFileInfo& fileInfo);

	// Compiles the given file contained in the project.
	virtual bool Compile(BuilderFileInfo& fileInfo, BuilderFileInfo& pchFileInfo);

	// Archives all the source files provided into a single lib.
	virtual bool Archive(std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile) = 0;

	// Links all the source files provided into a sinmgle executable.
	virtual bool Link(std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile) = 0;

}; 

}; // namespace MicroBuild