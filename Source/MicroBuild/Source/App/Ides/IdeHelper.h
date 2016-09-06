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

#include "App/Database/DatabaseFile.h"
#include "App/Workspace/WorkspaceFile.h"
#include "App/Project/ProjectFile.h"
#include "Core/Helpers/TextStream.h"

namespace MicroBuild {
namespace IdeHelper {

// Used to build a hierarchy of group folders in a workspace.
struct ProjectGroupFolder
{
	std::string baseName;
	std::string name;
	std::string parentName;
};

// Used to define the platform/configuration matrix that defines what
// projects we build in what configurations.
struct BuildProjectPair
{
	std::string config;
	EPlatform platform;
	bool shouldBuild;
	bool shouldDeploy;
	ProjectFile projectFile;
};
typedef std::vector<BuildProjectPair> BuildProjectMatrix;
typedef std::vector<BuildProjectMatrix> BuildWorkspaceMatrix;

// Expanded version of a vpath, the first value is the expanded path,
// the second value is a concatination of all matched values.
typedef std::pair<Platform::Path, std::string> VPathPair;

// Given a list of project it finds the one with the given name.
ProjectFile* GetProjectByName(
	std::vector<ProjectFile>& projectFiles,
	const std::string& name
);
ProjectFile* GetProjectByName(
	std::vector<ProjectFile*>& projectFiles,
	const std::string& name
);

// Given a name it finds and returns the project file associated 
// with it. Checks for errors such as cyclic dependencies.
// Returns true on success, otherwise emits errors to stdout.
bool GetProjectDependency(
	WorkspaceFile& workspaceFile,
	std::vector<ProjectFile>& projectFiles,
	ProjectFile* activeFile,
	ProjectFile*& output,
	std::string name
);
bool GetProjectDependency(
	WorkspaceFile& workspaceFile,
	std::vector<ProjectFile*>& projectFiles,
	ProjectFile* activeFile,
	ProjectFile*& output,
	std::string name
);

// Works out the filter folders that should be created for the project
// based on the virtual path patterns provided.  REturns true on success, 
// otherwise emits errors to stdout.
std::vector<ProjectGroupFolder> GetGroupFolders(
	WorkspaceFile& workspaceFile,
	std::vector<ProjectFile>& projectFiles,
	std::vector<VPathPair>& vpaths
);

// Creates a build matrix that stores the configuration state of each
// project file in each configuration/platform combination defined in
// the workspace file.
bool CreateBuildMatrix(
	WorkspaceFile& workspaceFile,
	std::vector<ProjectFile>& projectFiles,
	BuildWorkspaceMatrix& output);

// Takes a list of virtual paths and expands them to the relative project
// path names that should be used for project filters.
std::vector<VPathPair> ExpandVPaths(
	std::vector<ConfigFile::KeyValuePair>& vpaths,
	std::vector<Platform::Path>& files);

// Checks if the platforms provided are in the valid list, if not
// an error is given and it returns false.
bool ArePlatformsValid(
	ProjectFile& file,
	std::vector<EPlatform> toCheck,
	std::vector<EPlatform> validOptions);

// Extracts a unique list of filters from the given vpath and file list.
std::vector<std::string> SortFiltersByType(
	std::vector<VPathPair>& vpaths,
	Platform::Path& rootPath,
	std::map<std::string, std::string>& filterMap
);

// Resolves a platform-id to a platform name, this is the same as the platform-id
// in all cases apart from desktop platforms (x86/x64/etc), in which case the 
// platform name resolves to the host platform.
std::string ResolvePlatformName(EPlatform platform);

}; // namespace IdeHelper
}; // namespace MicroBuild