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
#include "App/Ides/IdeHelper.h"

namespace MicroBuild {
namespace IdeHelper {

ProjectFile* GetProjectByName(
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

ProjectFile* GetProjectByName(
	std::vector<ProjectFile*>& projectFiles,
	const std::string& name)
{
	for (ProjectFile* file : projectFiles)
	{
		if (Strings::CaseInsensitiveEquals(file->Get_Project_Name(), name))
		{
			return file;
		}
	}
	return nullptr;
}

bool GetProjectDependency(
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

bool GetProjectDependency(
	WorkspaceFile& workspaceFile,
	std::vector<ProjectFile*>& projectFiles,
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

void GetGroupFoldersInternal(
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

std::vector<ProjectGroupFolder> GetGroupFolders(
	WorkspaceFile& workspaceFile,
	std::vector<ProjectFile>& projectFiles,
	std::vector<VPathPair>& vpaths
)
{
	MB_UNUSED_PARAMETER(workspaceFile);

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

bool CreateBuildMatrix(
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
				pair.projectFile.Set_Target_PlatformName(IdeHelper::ResolvePlatformName(platform));
				pair.projectFile.Set_Target_MicroBuildExecutable(Platform::Path::GetExecutablePath());

				pair.config = config;
				pair.platform = platform;

				pair.projectFile.Resolve();
				if (!pair.projectFile.Validate())
				{
					return false;
				}

				pair.shouldBuild = pair.projectFile.Get_Project_ShouldBuild();
				pair.shouldDeploy = pair.projectFile.Get_Project_ShouldDeploy();
			}
		}

		output.push_back(projectMatrix);
	}

	// We do auto-link dependency resolution here to be somewhat lazy, 
	// we need to structure this a bit better.
	for (std::string& config : configurations)
	{
		for (EPlatform& platform : platforms)
		{
			// Get a list of projects for this configuration/platform pair.
			std::vector<ProjectFile*> configProjectFiles;
			for (BuildProjectMatrix& projectMatrix : output)
			{
				for (BuildProjectPair& projectPair : projectMatrix)
				{
					if (projectPair.config == config && projectPair.platform == platform)
					{
						configProjectFiles.push_back(&projectPair.projectFile);
					}
				}
			}

			// Resolve dependencies for each one.
			for (auto project : configProjectFiles)
			{
				std::vector<Platform::Path> outputDeps 
					= project->Get_Libraries_Library();

				std::vector<Platform::Path> newDeps;

				if (project->Get_Dependencies_AutoLink())
				{
					std::vector<std::string> dependencies = 
						project->Get_Dependencies_Dependency();

					for (std::string dependency : dependencies)
					{
						ProjectFile* projectDependency = nullptr;
						
						if (!IdeHelper::GetProjectDependency(
							workspaceFile,
							configProjectFiles,
							project,
							projectDependency,
							dependency))
						{
							return false;
						}

						if ((projectDependency->Get_Project_OutputType() == EOutputType::DynamicLib ||
							projectDependency->Get_Project_OutputType() == EOutputType::StaticLib) &&
							projectDependency->Get_Project_ShouldBuild())
						{
							Platform::Path outputPath = projectDependency->Get_Project_OutputDirectory()
								.AppendFragment(
									Strings::Format("%s%s", 
										projectDependency->Get_Project_OutputName().c_str(), 
										projectDependency->Get_Project_OutputExtension().c_str()
									),
									true
								);

							newDeps.push_back(outputPath);
						}
					}
				}

				outputDeps.insert(outputDeps.begin(), newDeps.begin(), newDeps.end());
				project->Set_Libraries_Library(outputDeps);
			}
		}
	}

	return true;
}

bool ArePlatformsValid(ProjectFile& file,
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

std::vector<VPathPair> ExpandVPaths(
	std::vector<ConfigFile::KeyValuePair>& vpaths,
	std::vector<Platform::Path>& files)
{
	std::vector<VPathPair> result;

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

				VPathPair pair(
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

			VPathPair pair(
				path.ToString(),
				filter
			);

			result.push_back(pair);
		}
	}

	return result;
}

std::vector<std::string> SortFiltersByType(
	std::vector<VPathPair>& vpaths,
	Platform::Path& rootPath,
	std::map<std::string, std::string>& filterMap)
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

		filterMap[relativePath.ToString()] = filter;
	}

	return filters;
}

std::string ResolvePlatformName(EPlatform platform)
{
	std::string name = "";

	switch (platform)
	{
	case EPlatform::ARM:
	case EPlatform::ARM64:
	case EPlatform::x86:
	case EPlatform::x64:
	case EPlatform::AnyCPU:
	case EPlatform::Native:
	case EPlatform::Universal86:
	case EPlatform::Universal64:
	case EPlatform::Universal:
		{
#if defined(MB_PLATFORM_WINDOWS)
			name = "Windows";
#elif defined(MB_PLATFORM_LINUX)
			name = "Linux";
#elif defined(MB_PLATFORM_MACOS)
			name = "MacOS";
#else
			#error Unimplemented platform.
#endif
			break;
		}
	default:
		{
			name = CastToString(platform);
			break;
		}
	}

	return name;
}

}; // namespace IdeHelper
}; // namespace MicroBuild
