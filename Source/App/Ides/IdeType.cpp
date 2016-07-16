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
#include "App/Ides/IdeType.h"

namespace MicroBuild {

IdeType::IdeType()
	: m_shortName("")
{
}

std::string IdeType::GetShortName()
{
	return m_shortName;
}

void IdeType::SetShortName(const std::string& value)
{
	m_shortName = value;
}


bool IdeType::WriteFile(
	WorkspaceFile& workspaceFile,
	Platform::Path& directory,
	Platform::Path& location,
	const char* data
	)
{
	// Dump evaluation result out to file.
	TextStream stream;
	stream.Write("%s", data);

	// Ensure location directory is created.
	if (!directory.Exists())
	{
		if (!directory.CreateAsDirectory())
		{
			workspaceFile.ValidateError(
				"Failed to create directory '%s'.\n",
				directory.ToString().c_str());

			return false;
		}
	}

	// Write the solution file to the location.
	if (!stream.WriteToFile(location))
	{
		workspaceFile.ValidateError(
			"Failed to write to file '%s'.\n",
			location.ToString().c_str());

		return false;
	}

	return true;
}

ProjectFile* IdeType::GetProjectByName(
	std::vector<ProjectFile>& projectFiles,
	const std::string& name)
{
	for (ProjectFile& file : projectFiles)
	{
		if (Strings::CaseInsensitiveEquals(file.Get_Project_Name(), name))
		{
			return &file;
		}
	}
	return nullptr;
}

bool IdeType::GetProjectDependency(
	WorkspaceFile& workspaceFile,
	std::vector<ProjectFile>& projectFiles,
	ProjectFile* activeFile,
	ProjectFile*& output,
	std::string name
)
{
	ProjectFile* dependency =
		GetProjectByName(projectFiles, name);

	if (dependency == activeFile)
	{
		workspaceFile.ValidateError(
			"Project '%s' appears to be dependent on itself.",
			name.c_str());

		return false;
	}

	if (dependency == nullptr)
	{
		workspaceFile.ValidateError(
			"Project dependency '%s' could not be found.",
			name.c_str());

		return false;
	}
	
	output = dependency;

	return true;
}

void IdeType::GetGroupFoldersInternal(
	Platform::Path group,
	std::vector<ProjectGroupFolder>& result
)
{
	std::vector<std::string> frags = group.GetFragments();

	std::string folder = "";
	for (std::string frag : frags)
	{
		std::string parentFolder = folder;

		if (folder != "")
		{
			folder += Platform::Path::Seperator;
		}
		folder += frag;

		bool bExists = false;

		for (ProjectGroupFolder& existingFolder : result)
		{
			if (Strings::CaseInsensitiveEquals(folder, existingFolder.name))
			{
				bExists = true;
				break;
			}
		}

		if (!bExists)
		{
			std::string baseName = folder;
			size_t slashOffset = baseName.find_last_of(Platform::Path::Seperator);
			if (slashOffset != std::string::npos)
			{
				baseName = baseName.substr(slashOffset + 1);
			}

			ProjectGroupFolder newFolder;
			newFolder.baseName = baseName;
			newFolder.name = folder;
			newFolder.parentName = parentFolder;
			result.push_back(newFolder);
		}
	}
}

std::vector<ProjectGroupFolder> IdeType::GetGroupFolders(
	WorkspaceFile& workspaceFile,
	std::vector<ProjectFile>& projectFiles,
	std::vector<VPathPair>& vpaths
)
{
	UNUSED_PARAMETER(workspaceFile);

	std::vector<ProjectGroupFolder> result;

	for (ProjectFile& project : projectFiles)
	{
		GetGroupFoldersInternal(project.Get_Project_Group(), result);
	}

	for (auto vpath : vpaths)
	{
		GetGroupFoldersInternal(vpath.second, result);
	}

	return result;
}

bool IdeType::CreateBuildMatrix(
	WorkspaceFile& workspaceFile,
	std::vector<ProjectFile>& projectFiles,
	BuildWorkspaceMatrix& output)
{
	std::vector<std::string> configurations =
		workspaceFile.Get_Configurations_Configuration();

	std::vector<EPlatform> platforms =
		workspaceFile.Get_Platforms_Platform();

	for (ProjectFile& file : projectFiles)
	{
		BuildProjectMatrix projectMatrix;
		projectMatrix.resize(configurations.size() * platforms.size());

		int configIndex = 0;

		// Resolve each configuration/platform combination and extract the 
		// information that we require.
		for (std::string& config : configurations)
		{
			for (EPlatform& platform : platforms)
			{
				BuildProjectPair& pair = projectMatrix[configIndex++];

				pair.projectFile = file;
				pair.projectFile.Set_Target_Configuration(config);
				pair.projectFile.Set_Target_Platform(platform);

				pair.config = config;
				pair.platform = platform;

				pair.projectFile.Resolve();
				if (!pair.projectFile.Validate())
				{
					return false;
				}

				pair.shouldBuild = pair.projectFile.Get_Project_ShouldBuild();
			}
		}

		output.push_back(projectMatrix);
	}

	return true;
}

bool IdeType::ArePlatformsValid(ProjectFile& file,
	std::vector<EPlatform> toCheck,
	std::vector<EPlatform> validOptions)
{
	for (EPlatform platform : toCheck)
	{
		if (std::find(validOptions.begin(), validOptions.end(), platform) == 
			validOptions.end())
		{
			file.ValidateError(
				"Platform '%s' is not valid for this project file type.",
				CastToString(platform).c_str());
			return false;
		}
	}
	return true;
}

std::vector<IdeType::VPathPair> IdeType::ExpandVPaths(
	std::vector<ConfigFile::KeyValuePair>& vpaths,
	std::vector<Platform::Path>& files)
{
	std::vector<IdeType::VPathPair> result;

	Platform::Path commonPath;
	Platform::Path::GetCommonPath(files, commonPath);

	for (Platform::Path& path : files)
	{
		bool bMatches = false;

		for (ConfigFile::KeyValuePair& value : vpaths)
		{
			Platform::Path filterPath;

			if (path.Matches(value.second, &filterPath))
			{
				std::string filter = value.first;

				// Replace wildcards in filter path.
				while (true)
				{
					size_t offset = filter.find('*');
					if (offset == std::string::npos)
					{
						break;
					}
					filter.replace(offset, offset, filterPath.ToString().c_str());
				}

				// Normalize the path.
				filter = Platform::Path(filter).ToString();

				IdeType::VPathPair pair(
					path.ToString(),
					filter
				);

				result.push_back(pair);
				bMatches = true;

				break;
			}
		}

		// If no match the filter is just the uncommon part of the path.
		if (!bMatches)
		{
			std::string filter = 
				path.GetUncommonPath(commonPath).GetDirectory().ToString();

			IdeType::VPathPair pair(
				path.ToString(),
				filter
			);
		}
	}

	return result;
}

std::vector<std::string> IdeType::SortFiltersByType(
	std::vector<VPathPair>& vpaths,
	Platform::Path& rootPath,
	std::map<std::string, std::string>& sourceFilterMap,
	std::map<std::string, std::string>& includeFilterMap,
	std::map<std::string, std::string>& noneFilterMap)
{
	std::vector<std::string> filters;

	for (auto vpath : vpaths)
	{
		Platform::Path relativePath =
			rootPath.RelativeTo(vpath.first);

		std::vector<std::string> fragments = 
			Strings::Split(Platform::Path::Seperator, vpath.second);

		std::string	filter = "";

		for (auto iter = fragments.begin(); iter != fragments.end(); iter++)
		{
			if (!filter.empty())
			{
				filter += "\\";
			}
			filter += *iter;

			if (std::find(filters.begin(), filters.end(), filter) == filters.end())
			{
				filters.push_back(filter);
			}
		}

		if (relativePath.IsSourceFile())
		{
			sourceFilterMap[relativePath.ToString()] = filter;
		}
		else if (relativePath.IsIncludeFile())
		{
			includeFilterMap[relativePath.ToString()] = filter;
		}
		else
		{
			noneFilterMap[relativePath.ToString()] = filter;
		}
	}

	return filters;
}

} // namespace MicroBuild