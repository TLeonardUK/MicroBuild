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
        std::vector<ProjectFile>& projectFiles,
		IdeHelper::BuildProjectMatrix& buildMatrix
	);

private:
    
    std::string FileTypeFromPath(
    	const Platform::Path& path
    );

    std::string FileTypeFromOutput(
        const EOutputType& type
    );
    
    std::string ProductTypeFromOutput(
        const EOutputType& type
    );
    
	void Write_PBXBuildFile(
		PlistNode& root,
		const std::string& rootUuid,
        const std::map<std::string, std::string>& filterMap
	);
    
    void Write_PBXFileReference(
        PlistNode& root,
        const std::string& rootUuid,
        const std::map<std::string, std::string>& filterMap,
        const Platform::Path& projectLocation,
        const std::string& projectName,
        const EOutputType& outputType,
        std::vector<ProjectFile*>& dependencies
    );
    
	void Write_PBXGroup(
		PlistNode& root,
		const std::string& rootUuid,
		const std::vector<std::string>& filters,
        const std::map<std::string, std::string>& filterMap,
		const std::string& rootGroupId,
        const std::string& rootName,
        const std::string& projectName,
        std::vector<ProjectFile*>& dependencies
	);

	void Write_PBXProject(
		PlistNode& root,
		const std::string& rootUuid,
		const std::string& id,
		const std::string& configListId,
		const std::string& mainGroupId,
        std::vector<ProjectFile*>& dependencies
	);
    
	void Write_PBXReferenceProxy(
		PlistNode& root,
		const std::string& rootUuid,
        std::vector<ProjectFile*>& dependencies
	);
    
	void Write_PBXNativeTarget(
		PlistNode& root,
		const std::string& rootUuid,
		const std::string& id,
		const std::string& configListId,
        const std::string& projectName,
        const EOutputType& outputType,
        const std::string& sourcePhaseId,
        const std::string& frameworksPhaseId,
        const std::string& resourcesPhaseId,
        std::vector<ProjectFile*>& dependencies
	);

	bool Write_XCBuildConfiguration(
		PlistNode& root,
		const std::string& rootUuid, 
		IdeHelper::BuildProjectMatrix& buildMatrix,
        const Platform::Path& projectBaseDirectory
	);

	void Write_XCBuildConfigurationList_Project(
		PlistNode& root,
		const std::string& rootUuid, 
		const std::string& id,
		IdeHelper::BuildProjectMatrix& buildMatrix,
        const std::vector<EPlatform>& platforms,
        const std::vector<std::string>& configurations
	);

	void Write_XCBuildConfigurationList_Target(
		PlistNode& root,
		const std::string& rootUuid, 
		const std::string& id,
		IdeHelper::BuildProjectMatrix& buildMatrix,
        const std::vector<EPlatform>& platforms,
        const std::vector<std::string>& configurations
	);

	void Write_PBXSourcesBuildPhase(
		PlistNode& root,
		const std::string& rootUuid, 
		const std::string& id,
        const std::map<std::string, std::string>& filterMap
	);

	void Write_PBXFrameworksBuildPhase(
		PlistNode& root,
		const std::string& rootUuid, 
		const std::string& id,
        const std::map<std::string, std::string>& filterMap
	);

	void Write_PBXResourcesBuildPhase(
		PlistNode& root,
		const std::string& rootUuid, 
		const std::string& id,
        const std::map<std::string, std::string>& filterMap
	);
    
};

}; // namespace MicroBuild