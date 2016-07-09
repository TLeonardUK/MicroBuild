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

#include "PCH.h"
#include "App/Ides/MSBuild/MSBuild.h"
#include "App/Ides/MSBuild/VisualStudio_2015.h"
#include "Core/Helpers/TextStream.h"

namespace MicroBuild {

#define GUID_SOLUTION_FOLDER "{2150E333-8FDC-42A3-9474-1A3956D46DE8}"
#define GUID_CPP_PROJECT	 "{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}"
#define GUID_CSHARP_PROJECT	 "{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}"

const char* MSBuildVersion_SolutionTemplates[static_cast<int>(MSBuildVersion::COUNT)] = {
    #include "App/Ides/MSBuild/Templates/MSBuild_Solution_V12.template"
};

const char* MSBuildVersion_ProjectTemplates[static_cast<int>(MSBuildVersion::COUNT)] = {
    #include "App/Ides/MSBuild/Templates/MSBuild_Project_V12.template"
};

const char* MSBuildVersion_ProjectFiltersTemplates[static_cast<int>(MSBuildVersion::COUNT)] = {
	#include "App/Ides/MSBuild/Templates/MSBuild_Project_Filters_V12.template"
};

Ide_MSBuild::Ide_MSBuild()
    : m_msBuildVersion(MSBuildVersion::Version12)
{
}

Ide_MSBuild::~Ide_MSBuild()
{
}

void Ide_MSBuild::SetMSBuildVersion(MSBuildVersion version)
{
    m_msBuildVersion = version;
}

std::string Ide_MSBuild::GetPlatformID(EPlatform platform)
{
    switch (platform)
    {
    case EPlatform::Windows_x86:
        {
            return "Win32";
        }
    case EPlatform::Windows_x64:
        {
            return "x64";
        }
    case EPlatform::Windows_AnyCPU:
        {
            return "Any CPU";
        }
    }
    return "";
}

std::string Ide_MSBuild::GetProjectGuid(
    const std::string& workspaceName, 
    const std::string& projectName)
{
    // To match VS we would want to generate a unique GUID each time the 
    // project files are re-created, but that seems bleh. So I'm just going to
    // make a fairly simple deterministic-guid that shouldn't change between rusn.
    unsigned int workspaceHash = Strings::Hash(workspaceName);
    unsigned int projectHash = Strings::Hash(projectName);

    return Strings::Format("{%08X-0000-0000-0000-0000%08X}",
        workspaceHash,
        projectHash);
}

std::string Ide_MSBuild::GetFilterGuid(
	const std::string& workspaceName,
	const std::string& projectName, 
	const std::string& filter)
{
	unsigned int workspaceHash = Strings::Hash(workspaceName);
	unsigned int projectHash = Strings::Hash(projectName);
	unsigned int filterHash = Strings::Hash(filter);

	unsigned int filterHi = ((filterHash >> 16) & 0x0000FFFF);
	unsigned int filterLo = ((filterHash) & 0x0000FFFF);

	return Strings::Format("{%08X-0000-%04X-%04X-0000%08X}",
		workspaceHash,
		filterLo,
		filterHi,
		projectHash);
}

std::string Ide_MSBuild::GetProjectTypeGuid(ELanguage language)
{
    switch (language)
    {
    case ELanguage::CSharp:	
        return GUID_CSHARP_PROJECT;
    case ELanguage::Cpp:
        return GUID_CPP_PROJECT;
    default:
        return "";
    }
}

ProjectFile* Ide_MSBuild::GetProjectByName(
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

bool Ide_MSBuild::GenerateSolutionFile(
    WorkspaceFile& workspaceFile,
    std::vector<ProjectFile>& projectFiles,
    MSBuildWorkspaceMatrix& buildMatrix
    )
{
    Platform::Path solutionDirectory = workspaceFile.Get_Workspace_Location();
    Platform::Path solutionLocation = 
        solutionDirectory.AppendFragment(
            workspaceFile.Get_Workspace_Name() + ".sln", true);

    std::vector<std::string> configurations = 
        workspaceFile.Get_Configurations_Configuration();

    std::vector<EPlatform> platforms =
        workspaceFile.Get_Platforms_Platform();

    // Fill evaluator will all variables required to build solution.
    TemplateEvaluator evaluator;

    // Work out all the solution folder fragments we need to build
    // the solution hierarchy.
    int projectsIndex = evaluator.AddArgument("projects", "");
    int nestedProjectsIndex = evaluator.AddArgument("nestedProjects", "");

    std::vector<std::string> folders;

    for (ProjectFile& project : projectFiles)
    {
        Platform::Path group = project.Get_Project_Group();
        std::vector<std::string> frags = group.GetFragments();

        std::string folder = "";
        for (std::string frag : frags)
        {
            std::string parentFolder = folder;

            if (folder != "")
            {
                folder += "/";
            }
            folder += frag;

            bool bExists = false;

            for (std::string existingFolder : folders)
            {
                if (Strings::CaseInsensitiveEquals(folder, existingFolder))
                {
                    bExists = true;
                    break;
                }
            }

            if (!bExists)
            {
                std::string folderGuid = GetProjectGuid(
                    workspaceFile.Get_Workspace_Name(),
                    folder + ".folder"
                );

                std::string parentFolderGuid = GetProjectGuid(
                    workspaceFile.Get_Workspace_Name(),
                    parentFolder + ".folder"
                );

                std::string baseName = folder;
                size_t slashOffset = baseName.find_last_of('/');
                if (slashOffset != std::string::npos)
                {
                    baseName = baseName.substr(slashOffset + 1);
                }

                int itemIndex = evaluator.AddArgument("", "", projectsIndex);
                evaluator.AddArgument(
                    "guid", folderGuid, itemIndex);
                evaluator.AddArgument(
                    "name", baseName, itemIndex);
                evaluator.AddArgument(
                    "location", "", itemIndex);
                evaluator.AddArgument(
                    "parentGuid", "", itemIndex);
                evaluator.AddArgument(
                    "typeGuid", GUID_SOLUTION_FOLDER, itemIndex);
                evaluator.AddArgument(
                    "dependencyGuids", "", itemIndex);

                // Folder->Parent Folder nesting.
                if (!parentFolder.empty())
                {
                    int nestedItemIndex = 
                        evaluator.AddArgument("", "", nestedProjectsIndex);
                    evaluator.AddArgument(
                        "guid", folderGuid, nestedItemIndex);
                    evaluator.AddArgument(
                        "parentGuid", parentFolderGuid, nestedItemIndex);
                }

                folders.push_back(folder);
            }
        }

        // Project->Folder nesting.
        if (!folder.empty())
        {
            std::string guid = GetProjectGuid(
                workspaceFile.Get_Workspace_Name(), 
                project.Get_Project_Name()
			);

            std::string folderGuid = GetProjectGuid(
                workspaceFile.Get_Workspace_Name(),
                folder + ".folder"
            );

            int nestedItemIndex =
                evaluator.AddArgument("", "", nestedProjectsIndex);
            evaluator.AddArgument(
                "guid", guid, nestedItemIndex);
            evaluator.AddArgument(
                "parentGuid", folderGuid, nestedItemIndex);
        }
    }
    
    // Project variables.
    for (ProjectFile& project : projectFiles)
    {
        std::string guid = GetProjectGuid(
            workspaceFile.Get_Workspace_Name(), project.Get_Project_Name());

        std::string typeGuid = GetProjectTypeGuid(
            project.Get_Project_Language());

        if (typeGuid == "")
        {
            StringCast(project.Get_Project_Language(), typeGuid);

            workspaceFile.ValidateError(
                "Platform '%s' is not a valid platform for an msbuild project.",
                typeGuid.c_str());

            return false;
        }

        Platform::Path vcxprojLocation = 
            project.Get_Project_Location().
            AppendFragment(project.Get_Project_Name() + ".vcxproj", true);

        Platform::Path vcxprojLocationRelative = 
            solutionDirectory.RelativeTo(vcxprojLocation);

        int itemIndex = evaluator.AddArgument("", "", projectsIndex);
        evaluator.AddArgument(
            "guid", guid, itemIndex);
        evaluator.AddArgument(
            "name", project.Get_Project_Name(), itemIndex);
        evaluator.AddArgument(
            "location", vcxprojLocationRelative.ToString(), itemIndex);
        evaluator.AddArgument(
            "parentGuid","", itemIndex);
        evaluator.AddArgument(
            "typeGuid", typeGuid, itemIndex);

        // Figure out the guids of all dependent projects.
        int dependencyGuidIndex = evaluator.AddArgument("dependencyGuids", "", itemIndex);
        std::vector<std::string> dependencyProjectNames = 
            project.Get_Dependencies_Dependency();

        for (std::string dependencyName : dependencyProjectNames)
        {
            ProjectFile* dependency = 
                GetProjectByName(projectFiles, dependencyName);
            
            if (dependency == &project)
            {
                workspaceFile.ValidateError(
                    "Project '%s' appears to be dependent on itself.",
                    dependencyName.c_str());

                return false;
            }

            if (dependency == nullptr)
            {
                workspaceFile.ValidateError(
                    "Project dependency '%s' could not be found.",
                    dependencyName.c_str());

                return false;
            }
            else
            {
                std::string dependencyGuid = GetProjectGuid(
                    workspaceFile.Get_Workspace_Name(), 
                    dependency->Get_Project_Name());
                
                evaluator.AddArgument("", dependencyGuid, dependencyGuidIndex);
            }
        }
    }

    // Configuration variables.
    int configurationsIndex = evaluator.AddArgument("configurations", "");
    for (unsigned int i = 0; i < configurations.size(); i++)
    {
        int itemIndex = evaluator.AddArgument("", "", configurationsIndex);
        evaluator.AddArgument("id", configurations[i], itemIndex);
    }

    // Platform variables.
    int platformsIndex = evaluator.AddArgument("platforms", "");
    for (unsigned int i = 0; i < platforms.size(); i++)
    {
        std::string platformString = GetPlatformID(platforms[i]);
        if (platformString == "")
        {
            StringCast(platforms[i], platformString);

            workspaceFile.ValidateError(
                "Platform '%s' is not a valid platform for an msbuild project.",
                platformString);

            return false;
        }

        int itemIndex = evaluator.AddArgument("", "", platformsIndex);
        evaluator.AddArgument("id", platformString, itemIndex);
    }

    // Build matrix settings.
    int buildMatrixIndex = evaluator.AddArgument("buildMatrix", "");
    for (unsigned int i = 0; i < buildMatrix.size(); i++)
    {
        MSBuildProjectMatrix& projectMatrix = buildMatrix[i];
        ProjectFile& project = projectFiles[i];

        std::string projectGuid = GetProjectGuid(
            workspaceFile.Get_Workspace_Name(),
            project.Get_Project_Name());

        for (unsigned int k = 0; k < projectMatrix.size(); k++)
        {
            MSBuildProjectPair& pair = projectMatrix[k];

            int itemIndex = evaluator.AddArgument("", "", buildMatrixIndex);
            evaluator.AddArgument(
                "projectGuid", projectGuid, itemIndex);
            evaluator.AddArgument(
                "configId", pair.config, itemIndex);
            evaluator.AddArgument(
                "platformId", GetPlatformID(pair.platform), itemIndex);
            evaluator.AddArgument(
                "shouldBuild", pair.shouldBuild ? "1" : "0", itemIndex);
        }
    }

    // Misc variables.
    evaluator.AddArgument("hideSolutionNode", "FALSE");

    // Generate result.
    if (!GenerateTemplateFile(
        workspaceFile,
        solutionDirectory,
        solutionLocation,
        MSBuildVersion_SolutionTemplates[static_cast<int>(m_msBuildVersion)],
        evaluator))
    {
        return false;
    }

    return true;
}

bool Ide_MSBuild::GenerateProjectFile(
    WorkspaceFile& workspaceFile,
    ProjectFile& projectFile,
    MSBuildProjectMatrix& buildMatrix
    )
{
    Platform::Path projectDirectory = 
        projectFile.Get_Project_Location();

    Platform::Path projectLocation =
        projectDirectory.AppendFragment(
            projectFile.Get_Project_Name() + ".vcxproj", true);

	Platform::Path projectFiltersLocation =
		projectDirectory.AppendFragment(
			projectFile.Get_Project_Name() + ".vcxproj.filters", true);

    std::vector<std::string> configurations =
        workspaceFile.Get_Configurations_Configuration();

    std::vector<EPlatform> platforms =
        workspaceFile.Get_Platforms_Platform();

	std::string projectGuid = GetProjectGuid(
		workspaceFile.Get_Workspace_Name(),
		projectFile.Get_Project_Name());

    // Fill evaluator will all variables required to build.
    TemplateEvaluator evaluator;

    // Resolve each configuration/platform combination and extract the 
    // information that we require.
    int configurationsIndex = evaluator.AddArgument("configurations", "");
    for (std::string& config : configurations)
    {
        for (EPlatform& platform : platforms)
        {
            projectFile.Set_Target_Configuration(config);
            projectFile.Set_Target_Platform(platform);

            projectFile.Resolve();
            if (!projectFile.Validate())
            {				
                return false;
            }

            MSBuildProjectPair pair;
            pair.config = config;
            pair.platform = platform;
            pair.shouldBuild = projectFile.Get_Project_ShouldBuild();
            buildMatrix.push_back(pair);

			// Build evaluator context.
			int itemIndex = evaluator.AddArgument("", "", configurationsIndex);
			evaluator.AddArgument(
				"configurationId", config, itemIndex);
			evaluator.AddArgument(
				"platformId", GetPlatformID(platform), itemIndex);
        }
    }

	// Project globals.
	int projectIndex = evaluator.AddArgument("project", "");
	evaluator.AddArgument(
		"guid", projectGuid, projectIndex);
	evaluator.AddArgument(
		"name", projectFile.Get_Project_Name(), projectIndex);

	// Figure out what the base root of the files is, we will use this
	// as the point to generate filters from.
	std::vector<Platform::Path> filePaths = projectFile.Get_Files_File();
	
	Platform::Path commonPath;
	Platform::Path::GetCommonPath(filePaths, commonPath);

	// File lists.
	int includeFilesIndex = evaluator.AddArgument("includeFiles", "");
	int sourceFilesIndex = evaluator.AddArgument("sourceFiles", "");
	int miscFilesIndex = evaluator.AddArgument("miscFIles", "");

	std::vector<std::string> filters;

	for (Platform::Path& path : filePaths)
	{
		std::string extension = path.GetExtension();

		int parentIndex = -1;

		if (path.IsSourceFile())
		{
			parentIndex = sourceFilesIndex;
		}
		else if (path.IsIncludeFile())
		{
			parentIndex = includeFilesIndex;
		}
		else
		{
			parentIndex = miscFilesIndex;
		}

		Platform::Path relativePath = 
			projectDirectory.RelativeTo(path);

		// Generate a list of unique filters.
		std::string filter = path.GetUncommonPath(commonPath).ToString();

		std::vector<std::string> fragments = Strings::Split(Platform::Path::Seperator, filter);

		filter = "";
		for (auto iter = fragments.begin(); iter != fragments.end() - 1; iter++)
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

		// Create file data.
		int itemIndex = evaluator.AddArgument("", "", parentIndex);
		evaluator.AddArgument(
			"path", relativePath.ToString(), itemIndex);

		evaluator.AddArgument(
			"filter", filter, itemIndex);
	}

	// File filter list.
	int fileFitersIndex = evaluator.AddArgument("fileFilters", "");

	for (auto filter : filters)
	{
		std::string filterGuid = GetFilterGuid(
			workspaceFile.Get_Workspace_Name(),
			projectFile.Get_Project_Name(),
			filter);

		int itemIndex = evaluator.AddArgument("", "", fileFitersIndex);
		evaluator.AddArgument(
			"path", filter, itemIndex);

		evaluator.AddArgument(
			"uniqueId", filterGuid, itemIndex);		
	}

    // Generate project result.
    if (!GenerateTemplateFile(
        workspaceFile,
        projectDirectory, 
        projectLocation, 
        MSBuildVersion_ProjectTemplates[static_cast<int>(m_msBuildVersion)],
        evaluator))
    {
        return false;
    }

	// Generate filters result.
	if (!GenerateTemplateFile(
		workspaceFile,
		projectDirectory,
		projectFiltersLocation,
		MSBuildVersion_ProjectFiltersTemplates[static_cast<int>(m_msBuildVersion)],
		evaluator))
	{
		return false;
	}

    return true;
}

bool Ide_MSBuild::GenerateTemplateFile(
    WorkspaceFile& workspaceFile,
    Platform::Path& directory,
    Platform::Path& location,
    const char* templateData,
    TemplateEvaluator& evaluator
    )
{
    // Generate the solution for this project.
    if (!evaluator.Evaluate(
        location.ToString(),
        templateData))
    {
        workspaceFile.ValidateError(
            "Internal error, msbuild project template appears to be broken. "
            "Tell a developer if possible!");

        return false;
    }

    // Dump evaluation result out to file.
    TextStream stream;
    stream.Write("%s", evaluator.GetResult().c_str());

    // Ensure location directory is created.
    if (!directory.Exists())
    {
        if (!directory.CreateAsDirectory())
        {
            workspaceFile.ValidateError(
                "Failed to create project directory '%s'.\n",
                directory.ToString().c_str());

            return false;
        }
    }

    // Write the solution file to the location.
    if (!stream.WriteToFile(location))
    {
        workspaceFile.ValidateError(
            "Failed to write project to file '%s'.\n",
            location.ToString().c_str());

        return false;
    }

    return true;
}

bool Ide_MSBuild::Generate(
    WorkspaceFile& workspaceFile,
    std::vector<ProjectFile>& projectFiles)
{	
    MSBuildWorkspaceMatrix workspaceMatrix;

    for (ProjectFile& file : projectFiles)
    {
        MSBuildProjectMatrix projectMatrix;

        if (!GenerateProjectFile(
            workspaceFile,
            file,
            projectMatrix
            ))
        {
            return false;
        }

        workspaceMatrix.push_back(projectMatrix);
    }

    if (!GenerateSolutionFile(
        workspaceFile,
        projectFiles,
        workspaceMatrix
        ))
    {
        return false;
    }

    return true;
}

}; // namespace MicroBuild