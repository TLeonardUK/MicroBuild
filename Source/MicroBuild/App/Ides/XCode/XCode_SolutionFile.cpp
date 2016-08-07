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

// todo: project dependencies

#include "PCH.h"
#include "App/Ides/XCode/XCode_SolutionFile.h"
#include "Core/Helpers/TextStream.h"
#include "Core/Helpers/XmlNode.h"

namespace MicroBuild {

XCode_SolutionFile::XCode_SolutionFile()
{
}

XCode_SolutionFile::~XCode_SolutionFile()
{
}

void XCode_SolutionFile::BuildProjectTree(
		XmlNode& rootNode,
		const std::string& rootPath, 
		std::vector<IdeHelper::ProjectGroupFolder>& folders,
		std::vector<IdeHelper::VPathPair>& files,
		Platform::Path& solutionDirectory,
        std::vector<ProjectFile>& projectFiles)
{
	// Add all files in this tree.
	for (auto pair : files)
	{
		if (pair.second == rootPath)
		{
			std::string relativePath =
				solutionDirectory.RelativeTo(pair.first).ToString();

            rootNode.Node("FileRef")
				.Attribute("location", "group:%s", relativePath.c_str());
		}
	}

	// Add all projects in this tree.
	for (ProjectFile& file : projectFiles)
	{
		std::string name = file.Get_Project_Name();
		std::string group = file.Get_Project_Group().ToString();
		if (group == rootPath)
		{
			Platform::Path projectLocation = file.Get_Project_Location().
				AppendFragment(file.Get_Project_Name() + ".xcodeproj", true);

			std::string relativeLocation =
				solutionDirectory.RelativeTo(projectLocation).ToString();

			rootNode.Node("FileRef")
				.Attribute("location", "group:%s", relativeLocation.c_str());
		}
	}

	// Add all sub-trees.	
	for (auto pair : folders)
	{
		if (pair.parentName == rootPath)
		{
			XmlNode& groupNode =
				rootNode.Node("Group")
				.Attribute("location", "container:")
				.Attribute("name", "%s", pair.baseName.c_str());

			BuildProjectTree(
				groupNode, 
				pair.name, 
				folders,
				files,
				solutionDirectory,
                projectFiles
			);
		}
	}
}

bool XCode_SolutionFile::Generate(
	DatabaseFile& databaseFile,
	WorkspaceFile& workspaceFile,
	std::vector<ProjectFile>& projectFiles,
	IdeHelper::BuildWorkspaceMatrix& buildMatrix
)
{
	Platform::Path solutionDirectory =
		workspaceFile.Get_Workspace_Location();

	Platform::Path solutionLocation =
		solutionDirectory
		.AppendFragment(workspaceFile.Get_Workspace_Name() + ".xcworkspace", true)
		.AppendFragment("contents.xcworkspace", true);

	std::vector<Platform::Path> files = workspaceFile.Get_WorkspaceFiles_File();

	// Resolve VPath's
	std::vector<ConfigFile::KeyValuePair> virtualPaths =
		workspaceFile.Get_WorkspaceVirtualPaths();

	std::vector<IdeHelper::VPathPair> vpathFilters =
		IdeHelper::ExpandVPaths(virtualPaths, files);

	// Get a list of folders.
	std::vector<IdeHelper::ProjectGroupFolder> folders =
		IdeHelper::GetGroupFolders(workspaceFile, projectFiles, vpathFilters);

	XmlNode root;

	// Header
	root.Node("?xml")
		.Attribute("version", "1.0")
		.Attribute("encoding", "UTF-8");

	XmlNode& workspace =
		root.Node("Workspace")
		.Attribute("version", "1.0");

	// Build the full project / file tree.
	BuildProjectTree(
		workspace, 
		"", 
		folders,
		vpathFilters, 
		solutionDirectory,
        projectFiles
	);
    
	// Generate result.
	if (!databaseFile.StoreFile(
		workspaceFile,
		solutionDirectory,
		solutionLocation,
		root.ToString().c_str()))
	{
		return false;
	}

	return true;
}

}; // namespace MicroBuild
