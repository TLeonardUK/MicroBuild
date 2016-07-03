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

namespace MicroBuild {

// MSBuild file format version to use.
enum class MSBuildVersion
{
	Version12, // Visual Studio 2015

	COUNT
};

// Base class for all Microsoft Build types (visual studio).
class Ide_MSBuild 
	: public IdeType
{
public:
	Ide_MSBuild();
	~Ide_MSBuild();

	// Sets the msbuild version to use, this determines what format to 
	// generate for solution and project files.
	void SetMSBuildVersion(MSBuildVersion version);

	// Generates a basic msbuild solution file that links to the given
	// project files.
	bool GenerateMSBuildSolutionFile(
		WorkspaceFile& workspaceFile,
		std::vector<ProjectFile>& projectFiles
	);
	
protected:

	// Converts a platform enum into an msbuild platform id.
	std::string GetPlatformID(EPlatform platform);

	// Converts a project name into a GUID.
	std::string GetProjectGUID(
		const std::string& workspaceName, 
		const std::string& projectName);

private:
	MSBuildVersion m_msBuildVersion;

};

}; // namespace MicroBuild