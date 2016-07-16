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
	ProjectFile projectFile;
};
typedef std::vector<BuildProjectPair> BuildProjectMatrix;
typedef std::vector<BuildProjectMatrix> BuildWorkspaceMatrix;

// Base class for all IDE targets.
class IdeType
{
public:

	IdeType();

	// Gets the short-named use on the command line and in config files
	// to refer to this IDE.
	std::string GetShortName();

	// Takes a fully resolved workspace file and generates the project
	// files it defines.
	virtual bool Generate(
		DatabaseFile& databaseFile,
		WorkspaceFile& workspaceFile, 
		std::vector<ProjectFile>& projectFiles) = 0;

	// Requests that the ide cleans up any intermediate files. This is
	// an optional implementation as some targets do not support cleaning. 
	virtual bool Clean(
		WorkspaceFile& workspaceFile);

	// Rebuilds a workspace file that has previously been generated. This is
	// an optional implementation as some targets do not support command
	// line building.
	virtual bool Build(
		WorkspaceFile& workspaceFile,
		bool bRebuild,
		const std::string& configuration,
		const std::string& platform
	);

protected:

	typedef std::pair<Platform::Path, std::string> VPathPair;

	ProjectFile* GetProjectByName(
		std::vector<ProjectFile>& projectFiles,
		const std::string& name
	);

	bool GetProjectDependency(
		WorkspaceFile& workspaceFile,
		std::vector<ProjectFile>& projectFiles,
		ProjectFile* activeFile,
		ProjectFile*& output,
		std::string name
	);

	std::vector<ProjectGroupFolder> GetGroupFolders(
		WorkspaceFile& workspaceFile,
		std::vector<ProjectFile>& projectFiles,
		std::vector<VPathPair>& vpaths
	);

	void GetGroupFoldersInternal(
		Platform::Path group,
		std::vector<ProjectGroupFolder>& result
	);

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
		std::map<std::string, std::string>& sourceFilterMap,
		std::map<std::string, std::string>& includeFilterMap,
		std::map<std::string, std::string>& noneFilterMap
	);

	void SetShortName(const std::string& value);

private:

	std::string m_shortName;

};

}; // namespace MicroBuild