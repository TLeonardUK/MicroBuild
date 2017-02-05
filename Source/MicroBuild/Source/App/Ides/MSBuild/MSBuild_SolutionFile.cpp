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

#include "PCH.h"
#include "App/Ides/MSBuild/MSBuild_SolutionFile.h"
#include "App/Ides/MSBuild/MSBuild_Ids.h"
#include "Core/Helpers/VbNode.h"

namespace MicroBuild {

MSBuild_SolutionFile::MSBuild_SolutionFile(
	std::string version,
	std::string versionName
)
	: m_version(version)
	, m_versionName(versionName)
{
}

MSBuild_SolutionFile::~MSBuild_SolutionFile()
{
}

bool MSBuild_SolutionFile::Generate(
	DatabaseFile& databaseFile,
	WorkspaceFile& workspaceFile,
	std::vector<ProjectFile>& projectFiles,
	IdeHelper::BuildWorkspaceMatrix& buildMatrix
)
{
	Platform::Path solutionDirectory =
		workspaceFile.Get_Workspace_Location();

	Platform::Path solutionLocation =
		solutionDirectory.AppendFragment(
			workspaceFile.Get_Workspace_Name() + ".sln", true);

	std::vector<std::string> configurations =
		workspaceFile.Get_Configurations_Configuration();

	std::vector<EPlatform> platforms =
		workspaceFile.Get_Platforms_Platform();

	std::stringstream output;

	std::vector<Platform::Path> files = workspaceFile.Get_WorkspaceFiles_File();

	// Resolve VPath's
	std::vector<ConfigFile::KeyValuePair> virtualPaths =
		workspaceFile.Get_WorkspaceVirtualPaths();

	std::vector<IdeHelper::VPathPair> vpathFilters =
		IdeHelper::ExpandVPaths(virtualPaths, files);

	// Get a list of folders.
	std::vector<IdeHelper::ProjectGroupFolder> folders =
		IdeHelper::GetGroupFolders(workspaceFile, projectFiles, vpathFilters);

	VbNode root;

	// Header
	root.Single("");
	root.Single("Microsoft Visual Studio Solution File, Format Version %s", m_version.c_str());
	root.Single("# %s", m_versionName.c_str());

	// Project block
	for (ProjectFile& file : projectFiles)
	{
		std::string typeGuid =
			MSBuild::GetProjectTypeGuid(file.Get_Project_Language());
		std::string name =
			file.Get_Project_Name();

		Platform::Path projectLocation;
		
		projectLocation = file.Get_Project_Location().
			AppendFragment(file.Get_Project_Name() + ".vcxproj", true);

		std::string relativeLocation =
			solutionDirectory.RelativeTo(projectLocation).ToString();

		std::string guid = Strings::Guid({ 
			workspaceFile.Get_Workspace_Name(), 
			file.Get_Project_Name() 
		});

		VbNode& projectNode =
			root.Node("Project")
			.Attribute(true, "%s", typeGuid.c_str())
			.Value(true, "%s", name.c_str())
			.Value(true, "%s", relativeLocation.c_str())
			.Value(true, "%s", guid.c_str());

		VbNode& depsNode =
			projectNode.Node("ProjectSection")
			.Attribute(false, "ProjectDependencies")
			.Value(false, "postProject");

		MB_UNUSED_PARAMETER(depsNode);

		/*
		for (std::string dependency : file.Get_Dependencies_Dependency())
		{
			ProjectFile* projectDependency = nullptr;
			if (!IdeHelper::GetProjectDependency(
				workspaceFile, 
				projectFiles, 
				&file, 
				projectDependency, 
				dependency))
			{
				return false;
			}

			std::string depGuid = Strings::Guid({ 
				workspaceFile.Get_Workspace_Name(), 
				projectDependency->Get_Project_Name() 
			});

			depsNode.Single(depGuid.c_str())
				.Value(false, "%s", depGuid.c_str());
		}
		*/
	}

	for (IdeHelper::ProjectGroupFolder& folder : folders)
	{
		std::string guid = Strings::Guid({ 
			workspaceFile.Get_Workspace_Name(), 
			folder.name, 
			"folder" 
		});

		VbNode& projectNode = root.Node("Project")
			.Attribute(true, "%s", MSBuild::k_GuidSolutionFolder)
			.Value(true, "%s", folder.baseName.c_str())
			.Value(true, "%s", folder.baseName.c_str())
			.Value(true, "%s", guid.c_str());

		VbNode& itemsNode =
			projectNode.Node("ProjectSection")
			.Attribute(false, "SolutionItems")
			.Value(false, "preProject");

		for (auto pair : vpathFilters)
		{
			if (pair.second == folder.name)
			{
				std::string relativePath =
					solutionDirectory.RelativeTo(pair.first).ToString();

				itemsNode
					.Single("%s", relativePath.c_str())
					.Value(false, "%s", relativePath.c_str());
			}
		}
	}

	// pre solution block
	VbNode& globalNode =
		root.Node("Global");

	VbNode& globalConfigPreSolutionNode =
		globalNode.Node("GlobalSection")
		.Attribute(false, "SolutionConfigurationPlatforms")
		.Value(false, "preSolution");

	for (std::string& config : configurations)
	{
		for (EPlatform& platform : platforms)
		{
			std::string platformStr = MSBuild::GetPlatformID(platform);
			std::string humanReadablePlatformStr = IdeHelper::GetPlatformHumanReadableId(platform);

			globalConfigPreSolutionNode
				.Single("%s|%s", config.c_str(), humanReadablePlatformStr.c_str())
				.Value(false, "%s|%s", config.c_str(), humanReadablePlatformStr.c_str());
		}
	}

	// Post solution block.
	VbNode& globalConfigPostSolutionNode =
		globalNode.Node("GlobalSection")
		.Attribute(false, "ProjectConfigurationPlatforms")
		.Value(false, "postSolution");

	for (IdeHelper::BuildProjectMatrix& matrix : buildMatrix)
	{
		for (IdeHelper::BuildProjectPair& pair : matrix)
		{
			std::string projectGuid = Strings::Guid({ 
				workspaceFile.Get_Workspace_Name(), 
				pair.projectFile.Get_Project_Name() 
			});

			std::string platformStr = MSBuild::GetPlatformID(pair.platform);
			std::string humanReadablePlatformStr = IdeHelper::GetPlatformHumanReadableId(pair.platform);

			globalConfigPostSolutionNode
				.Single("%s.%s|%s.ActiveCfg", projectGuid.c_str(), pair.config.c_str(), humanReadablePlatformStr.c_str())
				.Value(false, "%s|%s", pair.config.c_str(), platformStr.c_str());

			if (pair.shouldBuild)
			{
				globalConfigPostSolutionNode
					.Single("%s.%s|%s.Build.0", projectGuid.c_str(), pair.config.c_str(), humanReadablePlatformStr.c_str())
					.Value(false, "%s|%s", pair.config.c_str(), platformStr.c_str());
			}
			if (pair.shouldDeploy)
			{
				globalConfigPostSolutionNode
					.Single("%s.%s|%s.Deploy.0", projectGuid.c_str(), pair.config.c_str(), humanReadablePlatformStr.c_str())
					.Value(false, "%s|%s", pair.config.c_str(), platformStr.c_str());
			}
		}
	}

	// Solution properties
	VbNode& solutionsPropertiesNode =
		globalNode.Node("GlobalSection")
		.Attribute(false, "SolutionProperties")
		.Value(false, "preSolution");

	solutionsPropertiesNode
		.Single("HideSolutionNode")
		.Value(false, "FALSE");

	// Nest projects
	VbNode& nestedProjectsNode =
		globalNode.Node("GlobalSection")
		.Attribute(false, "NestedProjects")
		.Value(false, "preSolution");

	for (IdeHelper::ProjectGroupFolder& folder : folders)
	{
		if (folder.parentName != "")
		{
			std::string guid = Strings::Guid({ 
				workspaceFile.Get_Workspace_Name(), 
				folder.name, 
				"folder" 
			});

			std::string parentGuid = Strings::Guid({ 
				workspaceFile.Get_Workspace_Name(),
				folder.parentName, 
				"folder" 
			});

			nestedProjectsNode
				.Single("%s", guid.c_str())
				.Value(false, "%s", parentGuid.c_str());
		}
	}

	for (ProjectFile& file : projectFiles)
	{
		std::string group = file.Get_Project_Group().ToString();
		if (!group.empty())
		{
			std::string guid = Strings::Guid({ 
				workspaceFile.Get_Workspace_Name(), 
				file.Get_Project_Name() 
			});

			std::string parentGuid = Strings::Guid({ 
				workspaceFile.Get_Workspace_Name(), 
				group, 
				"folder" 
			});

			nestedProjectsNode
				.Single("%s", guid.c_str())
				.Value(false, "%s", parentGuid.c_str());
		}
	}

	// Generate result.
	if (!databaseFile.StoreFile(
		workspaceFile,
		solutionLocation,
		root.ToString().c_str()))
	{
		return false;
	}

	return true;
}

}; // namespace MicroBuild