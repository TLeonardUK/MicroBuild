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

class PlistNode;

// Contains the code required to generate a XCode file.
class XCode_CppProjectFile
{
public:

	XCode_CppProjectFile();
	~XCode_CppProjectFile();

	// Generates a basic msbuild solution file that links to the given
	// project files.
	bool Generate(
		DatabaseFile& databaseFile,
		WorkspaceFile& workspaceFile,
		ProjectFile& projectFile,
		IdeHelper::BuildProjectMatrix& buildMatrix
	);

private:
	/*
	void Write_PBXBuildFile(
		PlistNode& root,
		const std::string& rootUuid,
		const std::vector<Platform::Path>& files
	);

	void Write_PBXFileReference(
		PlistNode& root,
		const std::string& rootUuid,
		const std::vector<Platform::Path>& files,
		const Platform::Path& projectLocation
	);

	std::string Write_PBXGroup(
		PlistNode& root,
		const std::string& rootUuid,
		const std::vector<std::string>& filters,
		const std::string& rootGroupId
	);

	void Write_PBXProject(
		PlistNode& root,
		const std::string& rootUuid,
		const std::string& id,
		const std::string& configListId,
		const std::string& mainGroupId
	);

	void Write_PBXNativeTarget(
		PlistNode& root,
		const std::string& rootUuid,
		const std::string& id,
		const std::string& configListId
	);

	void Write_XCBuildConfigurationList_Project(
		PlistNode& root,
		const std::string& rootUuid, 
		const std::string& id,
		IdeHelper::BuildProjectMatrix& buildMatrix
	);

	void Write_XCBuildConfigurationList_Target(
		PlistNode& root,
		const std::string& rootUuid, 
		const std::string& id,
		IdeHelper::BuildProjectMatrix& buildMatrix
	);

	void Write_XCBuildConfiguration(
		PlistNode& root,
		const std::string& rootUuid, 
		IdeHelper::BuildProjectMatrix& buildMatrix
	);

	void Write_PBXHeadersBuildPhase(
		PlistNode& root,
		const std::string& rootUuid, 
		const std::string& id,
		std::vector<Platform::Path>& files,
	);

	void Write_PBXSourcesBuildPhase(
		PlistNode& root,
		const std::string& rootUuid, 
		const std::string& id,
		std::vector<Platform::Path>& files,
	);*/

};

}; // namespace MicroBuild