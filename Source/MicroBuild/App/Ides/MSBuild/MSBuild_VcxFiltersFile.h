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

#include "App/Ides/IdeType.h"
#include "App/Ides/MSBuild/MSBuild.h"

namespace MicroBuild {

// Contains the code required to generate a .vcxproj.filters file.
class MSBuild_VcxFiltersFile
{
public:

	MSBuild_VcxFiltersFile(std::string defaultToolsetString);
	~MSBuild_VcxFiltersFile();

	// Generates a basic msbuild solution file that links to the given
	// project files.
	bool Generate(
		DatabaseFile& databaseFile,
		WorkspaceFile& workspaceFile,
		ProjectFile& projectFile,
		IdeHelper::BuildProjectMatrix& buildMatrix,
		std::vector<MSBuildFileGroup>& groups
	);
	
private:
	std::string m_defaultToolsetString;

};

}; // namespace MicroBuild