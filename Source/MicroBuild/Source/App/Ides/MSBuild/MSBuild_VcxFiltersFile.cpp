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
#include "App/Ides/MSBuild/MSBuild_VcxFiltersFile.h"
#include "App/Ides/MSBuild/MSBuild.h"
#include "App/Ides/MSBuild/MSBuild_Ids.h"
#include "App/Ides/MSBuild/MSBuild_VcxProjectFile.h"
#include "Core/Helpers/TextStream.h"
#include "Core/Helpers/XmlNode.h"
#include "Core/Helpers/JsonNode.h"

namespace MicroBuild {

MSBuild_VcxFiltersFile::MSBuild_VcxFiltersFile(
	std::string defaultToolsetString
)
	: m_defaultToolsetString(defaultToolsetString)
{
}

MSBuild_VcxFiltersFile::~MSBuild_VcxFiltersFile()
{
}

bool MSBuild_VcxFiltersFile::Generate(
	DatabaseFile& databaseFile,
	WorkspaceFile& workspaceFile,
	ProjectFile& projectFile,
	IdeHelper::BuildProjectMatrix& buildMatrix,
	std::vector<MSBuildFileGroup>& groups
)
{
	MB_UNUSED_PARAMETER(buildMatrix);

	Platform::Path solutionDirectory =
		workspaceFile.Get_Workspace_Location();

	Platform::Path projectDirectory =
		projectFile.Get_Project_Location();

	Platform::Path projectFiltersLocation =
		projectDirectory.AppendFragment(
			projectFile.Get_Project_Name() + ".vcxproj.filters", true);

	std::vector<Platform::Path> files = projectFile.Get_Files_File();

	// Find common path.
	std::vector<ConfigFile::KeyValuePair> virtualPaths =
		projectFile.Get_VirtualPaths();

	std::vector<IdeHelper::VPathPair> vpathFilters =
		IdeHelper::ExpandVPaths(virtualPaths, files);

	// Generate filter list.
	std::map<std::string, std::string> filterMap;
	std::vector<std::string> filters = IdeHelper::SortFiltersByType(
		vpathFilters,
		solutionDirectory,
		filterMap
	);

	XmlNode root;

	// Header
	root.Node("?xml")
		.Attribute("version", "1.0")
		.Attribute("encoding", "utf-8");

	XmlNode& project =
		root.Node("Project")
		.Attribute("ToolsVersion", "%s", m_defaultToolsetString.c_str())
		.Attribute("xmlns", "http://schemas.microsoft.com/developer/msbuild/2003");

	// Filter block.
	XmlNode& filterGroup = project.Node("ItemGroup");
	for (std::string& filter : filters)
	{
		std::string guid =
			Strings::Guid({ workspaceFile.Get_Workspace_Name(), filter, "folder" });

		XmlNode& itemNode =
			filterGroup.Node("Filter")
			.Attribute("Include", "%s", filter.c_str());

		itemNode.Node("UniqueIdentifier").Value("%s", guid.c_str());
	}

	// Dump out each group.
	std::string filePrefix = "$(SolutionDir)";

	for (MSBuildFileGroup& group : groups)
	{
		XmlNode& sourceGroup = project.Node("ItemGroup");
		for (MSBuildFile& file : group.Files)
		{
			XmlNode& itemNode =
				sourceGroup.Node(file.TypeId.c_str())
				.Attribute("Include", "%s", file.Path.c_str());

			std::string nonPrefixedPath = file.Path.substr(filePrefix.size());

			itemNode
				.Node("Filter")
				.Value("%s", filterMap[nonPrefixedPath].c_str());
		}
	}

	// Generate result.
	if (!databaseFile.StoreFile(
		workspaceFile,
		projectFiltersLocation,
		root.ToString().c_str()))
	{
		return false;
	}

	return true;
}

}; // namespace MicroBuild