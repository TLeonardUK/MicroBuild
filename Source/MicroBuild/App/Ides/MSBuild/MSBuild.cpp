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
#include "App/Project/ProjectFile.h"
#include "App/Ides/MSBuild/MSBuild.h"
#include "App/Ides/MSBuild/VisualStudio_2015.h"
#include "Core/Helpers/TextStream.h"
#include "Core/Helpers/XmlNode.h"
#include "Core/Helpers/VbNode.h"
#include "Core/Platform/Process.h"

namespace MicroBuild {

#define GUID_SOLUTION_FOLDER "{2150E333-8FDC-42A3-9474-1A3956D46DE8}"
#define GUID_CPP_PROJECT	 "{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}"
#define GUID_CSHARP_PROJECT	 "{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}"

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

void Ide_MSBuild::SetHeaderShortName(const std::string& value)
{
	m_headerShortName = value;
}

void Ide_MSBuild::SetHeaderVersion(const std::string& value)
{
	m_headerVersion = value;
}

void Ide_MSBuild::SetDefaultToolset(EPlatformToolset toolset)
{
	m_defaultToolset = toolset;
}

std::string Ide_MSBuild::GetPlatformID(EPlatform platform)
{
    switch (platform)
    {
    case EPlatform::x86:
        {
            return "Win32";
        }
    case EPlatform::x64:
        {
            return "x64";
        }
    case EPlatform::AnyCPU:
        {
            return "Any CPU";
        }
	case EPlatform::ARM:
		{
			return "ARM";
		}
	case EPlatform::ARM64:
		{
			return "ARM64";
		}
	case EPlatform::PS4:
		{
			return "PS4";
		}
	case EPlatform::XboxOne:
		{
			return "Durango";
		}
	case EPlatform::WiiU:
		{
			return "CAFE";
		}
	case EPlatform::Nintendo3DS:
		{
			return "CTR";
		}
    }
    return "";
}

std::string Ide_MSBuild::GetPlatformDotNetTarget(EPlatform platform)
{
    switch (platform)
    {
    case EPlatform::x86:
        {
            return "Win32";
        }
    case EPlatform::x64:
        {
            return "x64";
        }
    case EPlatform::AnyCPU:
        {
            return "AnyCPU";
        }
	case EPlatform::ARM:
		{
			return "ARM";
		}
	case EPlatform::ARM64:
		{
			return "ARM";
		}
	case EPlatform::PS4:
		{
			return "PS4";
		}
	case EPlatform::XboxOne:
		{
			return "Durango";
		}
	case EPlatform::WiiU:
		{
			return "CAFE";
		}
	case EPlatform::Nintendo3DS:
		{
			return "CTR";
		}
    }
    return "";
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

bool Ide_MSBuild::GenerateSolutionFile(
	DatabaseFile& databaseFile,
    WorkspaceFile& workspaceFile,
    std::vector<ProjectFile>& projectFiles,
    BuildWorkspaceMatrix& buildMatrix
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

	std::vector<VPathPair> vpathFilters =
		ExpandVPaths(virtualPaths, files);

	// Get a list of folders.
	std::vector<ProjectGroupFolder> folders =
		GetGroupFolders(workspaceFile, projectFiles, vpathFilters);

	VbNode root;

	// Header
	root.Single("");
	root.Single("Microsoft Visual Studio Solution File, Format Version %s", m_headerVersion.c_str());
	root.Single("# %s", m_headerShortName.c_str());

	// Project block
	for (ProjectFile& file : projectFiles)
	{
		std::string typeGuid = 
			GetProjectTypeGuid(file.Get_Project_Language());
		std::string name = 
			file.Get_Project_Name();
		
		Platform::Path projectLocation;

		switch (file.Get_Project_Language())
		{
		case ELanguage::Cpp:
			{
				projectLocation = file.Get_Project_Location().
					AppendFragment(file.Get_Project_Name() + ".vcxproj", true);
				break;
			}
		case ELanguage::CSharp:
			{
				projectLocation = file.Get_Project_Location().
					AppendFragment(file.Get_Project_Name() + ".csproj", true);
				break;
			}
		default:
			{
				assert(false);
			}
		}

		std::string relativeLocation =
			solutionDirectory.RelativeTo(projectLocation).ToString();
		
		std::string guid = 
			Strings::Guid({ workspaceFile.Get_Workspace_Name(), file.Get_Project_Name() });

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

		for (std::string dependency : file.Get_Dependencies_Dependency())
		{
			ProjectFile* projectDependency = nullptr;
			if (!GetProjectDependency(workspaceFile, projectFiles, &file, projectDependency, dependency))
			{
				return false;
			}

			std::string depGuid = Strings::Guid({ workspaceFile.Get_Workspace_Name(), projectDependency->Get_Project_Name() });

			depsNode.Single(depGuid.c_str())
				.Value(false, "%s", depGuid.c_str());
		}
	}

	for (ProjectGroupFolder& folder : folders)
	{
		std::string guid =
			Strings::Guid({ workspaceFile.Get_Workspace_Name(), folder.name, "folder" });

		VbNode& projectNode = root.Node("Project")
			.Attribute(true, "%s", GUID_SOLUTION_FOLDER)
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
			std::string platformStr = GetPlatformID(platform);

			globalConfigPreSolutionNode
				.Single("%s|%s", config.c_str(), platformStr.c_str())
				.Value(false, "%s|%s", config.c_str(), platformStr.c_str());
		}
	}

	// Post solution block.
	VbNode& globalConfigPostSolutionNode =
		globalNode.Node("GlobalSection")
		.Attribute(false, "ProjectConfigurationPlatforms")
		.Value(false, "postSolution");

	for (BuildProjectMatrix& matrix : buildMatrix)
	{
		for (BuildProjectPair& pair : matrix)
		{
			std::string projectGuid = 
				Strings::Guid({ workspaceFile.Get_Workspace_Name(), pair.projectFile.Get_Project_Name() });

			std::string platformStr = GetPlatformID(pair.platform);

			globalConfigPostSolutionNode
				.Single("%s.%s|%s.ActiveCfg", projectGuid.c_str(), pair.config.c_str(), platformStr.c_str())
				.Value(false, "%s|%s", pair.config.c_str(), platformStr.c_str());

			if (pair.shouldBuild)
			{
				globalConfigPostSolutionNode
					.Single("%s.%s|%s.Build.0", projectGuid.c_str(), pair.config.c_str(), platformStr.c_str())
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

	for (ProjectGroupFolder& folder : folders)
	{
		if (folder.parentName != "")
		{
			std::string guid =
				Strings::Guid({ workspaceFile.Get_Workspace_Name(), folder.name, "folder" });

			std::string parentGuid =
				Strings::Guid({ workspaceFile.Get_Workspace_Name(), folder.parentName, "folder" });

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
			std::string guid =
				Strings::Guid({ workspaceFile.Get_Workspace_Name(), file.Get_Project_Name() });

			std::string parentGuid =
				Strings::Guid({ workspaceFile.Get_Workspace_Name(), group, "folder" });

			nestedProjectsNode
				.Single("%s", guid.c_str())
				.Value(false, "%s", parentGuid.c_str());
		}
	}

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

bool Ide_MSBuild::Generate_Csproj(
	DatabaseFile& databaseFile,
	WorkspaceFile& workspaceFile,
	ProjectFile& projectFile,
	BuildProjectMatrix& buildMatrix
	)
{
	Platform::Path solutionDirectory =
		workspaceFile.Get_Workspace_Location();

	Platform::Path projectDirectory =
		projectFile.Get_Project_Location();

	Platform::Path projectLocation =
		projectDirectory.AppendFragment(
			projectFile.Get_Project_Name() + ".csproj", true);

	std::vector<std::string> configurations =
		workspaceFile.Get_Configurations_Configuration();

	std::vector<EPlatform> platforms =
		workspaceFile.Get_Platforms_Platform();

	std::string projectGuid = Strings::Guid({
		workspaceFile.Get_Workspace_Name(),
		projectFile.Get_Project_Name() });

	// Files.
	std::vector<std::string> sourceFiles;
	std::vector<std::string> miscFiles;

	for (Platform::Path& path : projectFile.Get_Files_File())
	{
		std::string relativePath = "$(SolutionDir)\\" +
			solutionDirectory.RelativeTo(path).ToString();

		if (path.IsSourceFile())
		{
			sourceFiles.push_back(relativePath);
		}
		else
		{
			miscFiles.push_back(relativePath);
		}
	}

	XmlNode root;

	// Header
	root.Node("?xml")
		.Attribute("version", "1.0")
		.Attribute("encoding", "utf-8");

	XmlNode& project =
		root.Node("Project")
		.Attribute("DefaultTargets", "Build")
		.Attribute("ToolsVersion", "14.0")
		.Attribute("xmlns", "http://schemas.microsoft.com/developer/msbuild/2003");

	// Globals
	XmlNode& configPropertyGroup =
		project.Node("PropertyGroup");

	configPropertyGroup.Node("Configuration")
		.Attribute("Condition", "'$(Configuration)'==''")
		.Value("%s", CastToString(configurations[0]).c_str());

	configPropertyGroup.Node("Platform")
		.Attribute("Condition", "'$(Platform)'==''")
		.Value("%s", GetPlatformDotNetTarget(platforms[0]).c_str());

	configPropertyGroup.Node("ProjectGuid")
		.Value("%s", projectGuid.c_str());

	switch (projectFile.Get_Project_OutputType())
	{
	case EOutputType::Executable:
		configPropertyGroup.Node("OutputType").Value("WinExe");
		break;
	case EOutputType::ConsoleApp:
		configPropertyGroup.Node("OutputType").Value("Exe");
		break;
	case EOutputType::DynamicLib:
		configPropertyGroup.Node("OutputType").Value("Library");
		break;
	default:
		projectFile.ValidateError(
			"Output type '%s' is not valid for msbuild .NET projects.",
			CastToString(projectFile.Get_Project_OutputType()).c_str());
		return false;
	}

	configPropertyGroup.Node("AppDesignerFolder").Value("Properties");

	switch (projectFile.Get_Build_PlatformToolset())
	{
	case EPlatformToolset::Default:
		break;
	case EPlatformToolset::DotNet_2_0:
		configPropertyGroup.Node("TargetFrameworkVersion").Value("v2.0");
		break;
	case EPlatformToolset::DotNet_3_0:
		configPropertyGroup.Node("TargetFrameworkVersion").Value("v3.0");
		break;
	case EPlatformToolset::DotNet_3_5:
		configPropertyGroup.Node("TargetFrameworkVersion").Value("v3.5");
		break;
	case EPlatformToolset::DotNet_4_0:
		configPropertyGroup.Node("TargetFrameworkVersion").Value("v4.0");
		break;
	case EPlatformToolset::DotNet_4_5:
		configPropertyGroup.Node("TargetFrameworkVersion").Value("v4.5");
		break;
	case EPlatformToolset::DotNet_4_5_1:
		configPropertyGroup.Node("TargetFrameworkVersion").Value("v4.5.1");
		break;
	case EPlatformToolset::DotNet_4_5_2:
		configPropertyGroup.Node("TargetFrameworkVersion").Value("v4.5.2");
		break;
	case EPlatformToolset::DotNet_4_6:
		configPropertyGroup.Node("TargetFrameworkVersion").Value("v4.6");
		break;
	default:
		projectFile.ValidateError(
			"Platform toolset '%s' is not valid for msbuild .NET projects.",
			CastToString(projectFile.Get_Build_PlatformToolset()).c_str());
		return false;
	}

	configPropertyGroup.Node("FileAlignment").Value("512");
	configPropertyGroup.Node("AutoGenerateBindingRedirects").Value("true");
	configPropertyGroup.Node("RootNamespace").Value("%s", projectFile.Get_Project_RootNamespace().c_str());

	// Property Grid
	for (auto matrix : buildMatrix)
	{
		std::string platformId = GetPlatformID(matrix.platform);

				Platform::Path outDir = matrix.projectFile.Get_Project_OutputDirectory();
		Platform::Path intDir = matrix.projectFile.Get_Project_IntermediateDirectory();

		Platform::Path outDirRelative = solutionDirectory.RelativeTo(outDir);
		Platform::Path intDirRelative = solutionDirectory.RelativeTo(intDir);

		std::vector<std::string> defines = matrix.projectFile.Get_Defines_Define();

		XmlNode& platformConfig =
			project.Node("PropertyGroup")
			.Attribute("Condition", "'$(Configuration)|$(Platform)'=='%s|%s'", matrix.config.c_str(), GetPlatformDotNetTarget(matrix.platform).c_str())
			.Attribute("Label", "Configuration");

		platformConfig.Node("DebugSymbols").Value(matrix.projectFile.Get_Flags_GenerateDebugInformation());
		platformConfig.Node("BaseOutputPath").Value("$(SolutionDir)\\%s", outDirRelative.ToString().c_str());
		platformConfig.Node("OutputPath").Value("$(BaseOutputPath)\\");
		platformConfig.Node("BaseIntermediateOutputPath").Value("$(SolutionDir)\\%s\\", intDirRelative.ToString().c_str());
		platformConfig.Node("IntermediateOutputPath").Value("$(BaseIntermediateOutputPath)\\");
		platformConfig.Node("DefineConstants").Value("%s", Strings::Join(defines, ";").c_str());
		platformConfig.Node("DebugType").Value(matrix.projectFile.Get_Flags_GenerateDebugInformation() ? "full" : "pdbonly");
		platformConfig.Node("PlatformTarget").Value("%s", GetPlatformDotNetTarget(matrix.platform).c_str());
		platformConfig.Node("ErrorReport").Value("prompt");
		platformConfig.Node("WarningsAsErrors").Value(matrix.projectFile.Get_Flags_CompilerWarningsFatal());
		platformConfig.Node("CodeAnalysisRuleSet").Value("MinimumRecommendedRules.ruleset");
		platformConfig.Node("Prefer32Bit").Value(matrix.projectFile.Get_Flags_Prefer32Bit());
		platformConfig.Node("AllowUnsafeBlocks").Value(matrix.projectFile.Get_Flags_AllowUnsafeCode());

		if (matrix.projectFile.Get_Build_OptimizationLevel() == EOptimizationLevel::None ||
			matrix.projectFile.Get_Build_OptimizationLevel() == EOptimizationLevel::Debug)
		{
			platformConfig.Node("Optimize").Value("false");
		}
		else
		{
			platformConfig.Node("Optimize").Value("true");
		}

		switch (projectFile.Get_Project_LanguageVersion())
		{
		case ELanguageVersion::Default:
			break;
		case ELanguageVersion::CSharp_1_0:
			platformConfig.Node("LangVersion").Value("ISO-1");
			break;
		case ELanguageVersion::CSharp_2_0:
			platformConfig.Node("LangVersion").Value("ISO-2");
			break;
		case ELanguageVersion::CSharp_3_0:
			platformConfig.Node("LangVersion").Value("3");
			break;
		case ELanguageVersion::CSharp_4_0:
			platformConfig.Node("LangVersion").Value("4");
			break;
		case ELanguageVersion::CSharp_5_0:
			platformConfig.Node("LangVersion").Value("5");
			break;
		case ELanguageVersion::CSharp_6_0:
			platformConfig.Node("LangVersion").Value("6");
			break;
		default:
			projectFile.ValidateError(
				"Project language version '%s' is not valid for this project type.",
				CastToString(projectFile.Get_Project_LanguageVersion()).c_str());
			return false;
		}
	}

	// Startup Property Grid
	project.Node("PropertyGroup").Node("StartupObject");

	// References
	XmlNode& referenceFileGroup = project.Node("ItemGroup");
	for (auto file : projectFile.Get_References_Reference())
	{
		referenceFileGroup.Node("Reference")
			.Attribute("Include", "%s", file.ToString().c_str());
	}

	// Source files
	XmlNode& sourceFileGroup = project.Node("ItemGroup");
	for (auto file : sourceFiles)
	{
		sourceFileGroup.Node("Compile")
			.Attribute("Include", "%s", file.c_str());
	}

	// Misc file.
	XmlNode& miscFileGroup = project.Node("ItemGroup");
	for (auto file : miscFiles)
	{
		miscFileGroup.Node("None")
			.Attribute("Include", "%s", file.c_str());
	}

	// Import
	project.Node("Import")
		.Attribute("Project", "$(MSBuildToolsPath)\\Microsoft.CSharp.targets");

	// Generate result.
	if (!databaseFile.StoreFile(
		workspaceFile,
		projectDirectory,
		projectLocation,
		root.ToString().c_str()))
	{
		return false;
	}

	return true;
}

bool Ide_MSBuild::Generate_Vcxproj(
	DatabaseFile& databaseFile,
    WorkspaceFile& workspaceFile,
    ProjectFile& projectFile,
    BuildProjectMatrix& buildMatrix
    )
{
	Platform::Path solutionDirectory =
		workspaceFile.Get_Workspace_Location();

	Platform::Path projectDirectory =
		projectFile.Get_Project_Location();

	Platform::Path projectLocation =
		projectDirectory.AppendFragment(
			projectFile.Get_Project_Name() + ".vcxproj", true);

	std::vector<std::string> configurations =
		workspaceFile.Get_Configurations_Configuration();

	std::vector<EPlatform> platforms =
		workspaceFile.Get_Platforms_Platform();

	std::string projectGuid = Strings::Guid({
		workspaceFile.Get_Workspace_Name(),
		projectFile.Get_Project_Name() });

	// Files.
	std::vector<std::string> includeFiles;
	std::vector<std::string> sourceFiles;
	std::vector<std::string> miscFiles;
	std::string precompiledSourceFile;

	for (Platform::Path& path : projectFile.Get_Files_File())
	{
		std::string relativePath = "$(SolutionDir)\\" +
			solutionDirectory.RelativeTo(path).ToString();

		if (path == projectFile.Get_Build_PrecompiledSource())
		{
			precompiledSourceFile = relativePath;
		}

		if (path.IsSourceFile())
		{
			sourceFiles.push_back(relativePath);
		}
		else if (path.IsIncludeFile())
		{
			includeFiles.push_back(relativePath);
		}
		else
		{
			miscFiles.push_back(relativePath);
		}
	}

	XmlNode root;

	// Header
	root.Node("?xml")
		.Attribute("version", "1.0")
		.Attribute("encoding", "utf-8");
	
	XmlNode& project = 
		root.Node("Project")
		.Attribute("DefaultTargets", "Build")
		.Attribute("ToolsVersion", "14.0")
		.Attribute("xmlns", "http://schemas.microsoft.com/developer/msbuild/2003");

	// Build Matrix
	XmlNode& projectConfig = 
		project.Node("ItemGroup")
		.Attribute("Label", "ProjectConfiguration");

	for (auto matrix : buildMatrix)
	{
		std::string platformId = GetPlatformID(matrix.platform);

		XmlNode& buildConfig =
			projectConfig.Node("ProjectConfiguration")
			.Attribute("Include", "%s|%s", matrix.config.c_str(), platformId.c_str());

		buildConfig.Node("Configuration").Value("%s", matrix.config.c_str());
		buildConfig.Node("Platform").Value("%s", platformId.c_str());
	}

	// Globals.
	XmlNode& globals =
		project.Node("PropertyGroup")
		.Attribute("Label", "Globals");

	globals.Node("ProjectGuid").Value("%s", projectGuid.c_str());
	globals.Node("IgnoreWarnCompileDuplicatedFilename").Value("true");
	globals.Node("Keyword").Value("Win32Proj");
	globals.Node("RootNamespace").Value("%s", projectFile.Get_Project_RootNamespace().c_str());

	// Imports
	project.Node("Import")
		.Attribute("Project", "$(VCTargetsPath)\\Microsoft.Cpp.Default.props");

	// Property Grid
	for (auto matrix : buildMatrix)
	{
		std::string platformId = GetPlatformID(matrix.platform);

		XmlNode& propertyGroup =
			project.Node("PropertyGroup")
			.Attribute("Condition", "'$(Configuration)|$(Platform)'=='%s|%s'", matrix.config.c_str(), platformId.c_str())
			.Attribute("Label", "Configuration");

		// Output type.
		switch (matrix.projectFile.Get_Project_OutputType())
		{
		case EOutputType::Executable:
			// Fallthrough
		case EOutputType::ConsoleApp:
			propertyGroup.Node("ConfigurationType").Value("Application");
			break;
		case EOutputType::DynamicLib:
			propertyGroup.Node("ConfigurationType").Value("DynamicLibrary");
			break;
		case EOutputType::StaticLib:
			propertyGroup.Node("ConfigurationType").Value("StaticLibrary");
			break;
		default:
			projectFile.ValidateError(
				"Output type '%s' is not valid for msbuild C++ projects.",
				CastToString(matrix.projectFile.Get_Project_OutputType()).c_str());
			return false;
		}

		// Debug libraries.
		if (matrix.projectFile.Get_Build_OptimizationLevel() == EOptimizationLevel::None ||
			matrix.projectFile.Get_Build_OptimizationLevel() == EOptimizationLevel::Debug)
		{
			propertyGroup.Node("UseDebugLibraries").Value("true");
		}
		else
		{
			propertyGroup.Node("UseDebugLibraries").Value("false");
		}

		// Character set.
		switch (matrix.projectFile.Get_Build_CharacterSet())
		{
		case ECharacterSet::Default:
			break;
		case ECharacterSet::Unicode:
			propertyGroup.Node("CharacterSet").Value("Unicode");
			break;
		case ECharacterSet::MBCS:
			propertyGroup.Node("CharacterSet").Value("MultiByte");
			break;
		default:
			projectFile.ValidateError(
				"Character set '%s' is not valid for msbuild C++ projects.", 
				CastToString(matrix.projectFile.Get_Build_CharacterSet()).c_str());
			return false;
		}

		// Platform tooltype.
		switch (matrix.projectFile.Get_Build_PlatformToolset())
		{
		case EPlatformToolset::Default:
			propertyGroup.Node("PlatformToolset").Value("%s", CastToString(m_defaultToolset).c_str());
			break;
		case EPlatformToolset::v140:
			propertyGroup.Node("PlatformToolset").Value("v140");
			break;
		case EPlatformToolset::v140_xp:
			propertyGroup.Node("PlatformToolset").Value("v140_xp");
			break;
		default:
			projectFile.ValidateError(
				"Platform toolset '%s' is not valid for msbuild C++ projects.",
				CastToString(matrix.projectFile.Get_Build_PlatformToolset()).c_str());
			return false;
		}

		// LTO
		if (matrix.projectFile.Get_Build_OptimizationLevel() == EOptimizationLevel::None ||
			matrix.projectFile.Get_Build_OptimizationLevel() == EOptimizationLevel::Debug)
		{
			propertyGroup.Node("WholeProgramOptimization").Value("false");
		}
		else
		{
			propertyGroup.Node("WholeProgramOptimization").Value("true");
		}
	}

	// Imports
	project.Node("Import")
		.Attribute("Project", "$(VCTargetsPath)\\Microsoft.Cpp.props");

	// Extension settings
	project.Node("ImportGroup")
		.Attribute("Label", "ExtensionSettings");

	// Item definition group.
	for (auto matrix : buildMatrix)
	{
		std::string platformId = GetPlatformID(matrix.platform);

		XmlNode& itemDefGroup = 
			project.Node("ImportGroup")
			.Attribute("Label", "PropertySheets")
			.Attribute("Condition", "'$(Configuration)|$(Platform)'=='%s|%s'", matrix.config.c_str(), platformId.c_str());

		itemDefGroup.Node("Import")
			.Attribute("Project", "$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props")
			.Attribute("Condition", "exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')")
			.Attribute("Label", "LocalAppDataPlatform");
	}

	// Macros settings
	project.Node("PropertyGroup")
		.Attribute("Label", "UserMacros");

	// Output property group.
	for (auto matrix : buildMatrix)
	{
		std::string platformId = GetPlatformID(matrix.platform);

		Platform::Path outDir = matrix.projectFile.Get_Project_OutputDirectory();
		Platform::Path intDir = matrix.projectFile.Get_Project_IntermediateDirectory();

		Platform::Path outDirRelative = solutionDirectory.RelativeTo(outDir);
		Platform::Path intDirRelative = solutionDirectory.RelativeTo(intDir);

		std::vector<std::string> includePaths;
		std::vector<std::string> libraryPaths;

		for (Platform::Path& path : matrix.projectFile.Get_SearchPaths_IncludeDirectory())
		{
			includePaths.push_back("$(SolutionDir)\\" + solutionDirectory.RelativeTo(path).ToString() + "\\");
		}

		for (Platform::Path& path : matrix.projectFile.Get_SearchPaths_LibraryDirectory())
		{
			libraryPaths.push_back("$(SolutionDir)\\" + solutionDirectory.RelativeTo(path).ToString() + "\\");
		}


		XmlNode& propertyGroup =
			project.Node("PropertyGroup")
			.Attribute("Condition", "'$(Configuration)|$(Platform)'=='%s|%s'", matrix.config.c_str(), platformId.c_str());

		// Incremental linking
		if (matrix.projectFile.Get_Flags_LinkTimeOptimization())
		{
			propertyGroup.Node("LinkIncremental").Value("false");
		}
		else
		{
			propertyGroup.Node("LinkIncremental").Value(matrix.projectFile.Get_Build_OptimizationLevel() != EOptimizationLevel::Full);
		}

		// Output information.
		propertyGroup.Node("OutDir").Value("$(SolutionDir)%s\\", outDirRelative.ToString().c_str());
		propertyGroup.Node("IntDir").Value("$(SolutionDir)%s\\", intDirRelative.ToString().c_str());
		propertyGroup.Node("TargetName").Value("%s", matrix.projectFile.Get_Project_OutputName().c_str());
		propertyGroup.Node("TargetExt").Value("%s", matrix.projectFile.Get_Project_OutputExtension().c_str());

		// Search paths.
		if (includePaths.size() > 0)
		{
			propertyGroup.Node("IncludePath").Value("%s;$(IncludePath)", Strings::Join(includePaths, ";").c_str());
		}
		if (libraryPaths.size() > 0)
		{
			propertyGroup.Node("LibraryPath").Value("%s;$(LibraryPath)", Strings::Join(includePaths, ";").c_str());
		}
	}

	// Compile/Link information.
	for (auto matrix : buildMatrix)
	{
		std::string platformId = GetPlatformID(matrix.platform);

		Platform::Path precompiledHeader = matrix.projectFile.Get_Build_PrecompiledHeader();
		Platform::Path precompiledSource = matrix.projectFile.Get_Build_PrecompiledSource();

		std::vector<std::string> forcedIncludes;
		std::vector<std::string> additionalLibraries;

		for (Platform::Path& path : matrix.projectFile.Get_ForcedIncludes_ForcedInclude())
		{
			forcedIncludes.push_back("$(SolutionDir)\\" + solutionDirectory.RelativeTo(path).ToString());
		}

		for (Platform::Path& path : matrix.projectFile.Get_Libraries_Library())
		{
			if (path.IsRelative())
			{
				additionalLibraries.push_back(path.ToString());
			}
			else
			{
				additionalLibraries.push_back("$(SolutionDir)\\" + solutionDirectory.RelativeTo(path).ToString());
			}
		}
	
		XmlNode& itemDefGroup = 
			project.Node("ItemDefinitionGroup")
			.Attribute("Condition", "'$(Configuration)|$(Platform)'=='%s|%s'", matrix.config.c_str(), platformId.c_str());

		{
			XmlNode& compileGroup =
				itemDefGroup.Node("ClCompile");

			// Precompiled header.
			if (precompiledHeader.ToString() != "")
			{
				compileGroup.Node("PrecompiledHeader").Value("Use");
				compileGroup.Node("PrecompiledHeaderFile").Value("%s", precompiledHeader.GetFilename().c_str());
			}

			// General settings.
			if (matrix.projectFile.Get_Defines_Define().size() > 0)
			{
				std::vector<std::string> defines = matrix.projectFile.Get_Defines_Define();
				compileGroup.Node("PreprocessorDefinitions").Value("%s;$(PreprocessorDefinitions)", Strings::Join(defines, ";").c_str());
			}
			compileGroup.Node("MinimalRebuild").Value("false");
			compileGroup.Node("MultiProcessorCompilation").Value("true");
			compileGroup.Node("RuntimeTypeInfo").Value(matrix.projectFile.Get_Flags_RuntimeTypeInfo());
			compileGroup.Node("ExceptionHandling").Value((matrix.projectFile.Get_Flags_Exceptions() ? "Async" : "false"));

			// Standard library.
			if (matrix.projectFile.Get_Build_OptimizationLevel() == EOptimizationLevel::None ||
				matrix.projectFile.Get_Build_OptimizationLevel() == EOptimizationLevel::Debug)
			{
				if (matrix.projectFile.Get_Flags_StaticRuntime())
				{
					compileGroup.Node("RuntimeLibrary").Value("MultiThreadedDebug");
				}
				else
				{
					compileGroup.Node("RuntimeLibrary").Value("MultiThreadedDebugDLL");
				}
			}
			else
			{
				if (matrix.projectFile.Get_Flags_StaticRuntime())
				{
					compileGroup.Node("RuntimeLibrary").Value("MultiThreaded");
				}
				else
				{
					compileGroup.Node("RuntimeLibrary").Value("MultiThreadedDLL");
				}
			}

			// Debug database.
			if (matrix.projectFile.Get_Flags_GenerateDebugInformation())
			{
				compileGroup.Node("DebugInformationFormat").Value("ProgramDatabase");
			}
			else
			{
				compileGroup.Node("DebugInformationFormat").Value("None");
			}

			// Warning level.
			switch (matrix.projectFile.Get_Build_WarningLevel())
			{
			case EWarningLevel::Default:
				break;
			case EWarningLevel::None:
				compileGroup.Node("WarningLevel").Value("TurnOffAllWarnings");
				break;
			case EWarningLevel::Low:
				compileGroup.Node("WarningLevel").Value("Level1");
				break;
			case EWarningLevel::Medium:
				compileGroup.Node("WarningLevel").Value("Level3");
				break;
			case EWarningLevel::High:
				compileGroup.Node("WarningLevel").Value("Level4");
				break;
			case EWarningLevel::Verbose:
				compileGroup.Node("WarningLevel").Value("EnableAllWarnings");
				break;
			default:
				projectFile.ValidateError(
					"Warning level '%s' is not valid for msbuild C++ projects.",
					CastToString(matrix.projectFile.Get_Build_WarningLevel()).c_str());
				return false;
			}

			compileGroup.Node("TreatWarningAsError").Value(matrix.projectFile.Get_Flags_CompilerWarningsFatal());

			if (matrix.projectFile.Get_DisabledWarnings_DisabledWarning().size() > 0)
			{
				std::vector<std::string> disabledWarnings = matrix.projectFile.Get_DisabledWarnings_DisabledWarning();
				compileGroup.Node("DisableSpecificWarnings").Value("%s;$(DisableSpecificWarnings)", Strings::Join(disabledWarnings, ";").c_str());
			}

			// Optimization.
			switch (matrix.projectFile.Get_Build_OptimizationLevel())
			{
			case EOptimizationLevel::None:
				// Fallthrough
			case EOptimizationLevel::Debug:
				compileGroup.Node("Optimization").Value("Disabled");
				break;
			case EOptimizationLevel::PreferSize:
				compileGroup.Node("Optimization").Value("MinSpace");
				break;
			case EOptimizationLevel::PreferSpeed:
				compileGroup.Node("Optimization").Value("MaxSpeed");
				break;
			case EOptimizationLevel::Full:
				compileGroup.Node("Optimization").Value("Full");
				break;
			default:
				projectFile.ValidateError(
					"Optimization level '%s' is not valid for msbuild C++ projects.",
					CastToString(matrix.projectFile.Get_Build_OptimizationLevel()).c_str());
				return false;
			}

			// Additional options.
			if (!matrix.projectFile.Get_Build_CompilerArguments().empty())
			{
				compileGroup.Node("AdditionalOptions")
					.Value("%s $(AdditionalOptions)", matrix.projectFile.Get_Build_CompilerArguments().c_str());
			}

			if (forcedIncludes.size() > 0)
			{
				compileGroup.Node("ForcedIncludeFiles").Value("%s;$(ForcedIncludeFiles)", Strings::Join(forcedIncludes, ";").c_str());
			}
		}

		{
			XmlNode& linkGroup =
				itemDefGroup.Node("Link");

			// Warnings.
			linkGroup.Node("TreatLinkerWarningAsErrors").Value(matrix.projectFile.Get_Flags_LinkerWarningsFatal());
			
			// Optimization.
			if (matrix.projectFile.Get_Flags_LinkTimeOptimization())
			{
				linkGroup.Node("LinkTimeCodeGeneration").Value("UseLinkTimeCodeGeneration");
				linkGroup.Node("WholeProgramOptimization").Value("true");
			}
			else
			{
				linkGroup.Node("WholeProgramOptimization").Value("false");
			}

			// Sub system
			switch (matrix.projectFile.Get_Project_OutputType())
			{
			case EOutputType::Executable:
				linkGroup.Node("SubSystem").Value("Windows");
				break;
			case EOutputType::ConsoleApp:
				// Fallthrough
			case EOutputType::DynamicLib:
				// Fallthrough
			case EOutputType::StaticLib:
				linkGroup.Node("SubSystem").Value("Console");
				break;
			default:
				projectFile.ValidateError(
					"Output type '%s' is not valid for msbuild C++ projects.",
					CastToString(matrix.projectFile.Get_Project_OutputType()).c_str());
				return false;
			}

			// Debug information.
			linkGroup.Node("GenerateDebugInformation").Value(matrix.projectFile.Get_Flags_GenerateDebugInformation());

			// Libraries.
			if (additionalLibraries.size() > 0)
			{
				linkGroup.Node("AdditionalDependencies").Value("%s;$(AdditionalDependencies)", Strings::Join(additionalLibraries, ";").c_str());
			}

			// Additional options.
			linkGroup.Node("EntryPointSymbol").Value("mainCRTStartup");
			
			if (!matrix.projectFile.Get_Build_LinkerArguments().empty())
			{
				linkGroup.Node("AdditionalOptions").Value("%s $(AdditionalOptions)", matrix.projectFile.Get_Build_LinkerArguments().c_str());
			}
		}
	}

	// Include files
	XmlNode& includeFileGroup = project.Node("ItemGroup");
	for (auto file : includeFiles)
	{
		includeFileGroup.Node("ClInclude")
			.Attribute("Include", "%s", file.c_str());
	}

	// Source files
	XmlNode& sourceFileGroup = project.Node("ItemGroup");
	for (auto file : sourceFiles)
	{
		XmlNode& fileNode =
			sourceFileGroup.Node("ClCompile")
			.Attribute("Include", "%s", file.c_str());

		if (file == precompiledSourceFile)
		{
			fileNode.Node("PrecompiledHeader").Value("Create");
		}
	}
	
	// Misc file.
	XmlNode& miscFileGroup = project.Node("ItemGroup");
	for (auto file : miscFiles)
	{
		miscFileGroup.Node("None")
			.Attribute("Include", "%s", file.c_str());
	}

	// Imports
	project.Node("Import")
		.Attribute("Project", "$(VCTargetsPath)\\Microsoft.Cpp.targets");

	// Extension settings
	project.Node("ImportGroup")
		.Attribute("Label", "ExtensionSettings");

	// Generate result.
	if (!databaseFile.StoreFile(
		workspaceFile,
		projectDirectory,
		projectLocation,
		root.ToString().c_str()))
	{
		return false;
	}

    return true;
}

bool Ide_MSBuild::Generate_Vcxproj_Filters(
	DatabaseFile& databaseFile,
	WorkspaceFile& workspaceFile,
	ProjectFile& projectFile,
	BuildProjectMatrix& buildMatrix
	)
{
	UNUSED_PARAMETER(buildMatrix);

	Platform::Path solutionDirectory =
		workspaceFile.Get_Workspace_Location();

	Platform::Path projectDirectory =
		projectFile.Get_Project_Location();

	Platform::Path projectFiltersLocation =
		projectDirectory.AppendFragment(
			projectFile.Get_Project_Name() + ".vcxproj.filters", true);

	std::vector<std::string> configurations =
		workspaceFile.Get_Configurations_Configuration();

	std::vector<EPlatform> platforms =
		workspaceFile.Get_Platforms_Platform();

	std::vector<Platform::Path> files = projectFile.Get_Files_File();

	// Find common path.
	std::vector<ConfigFile::KeyValuePair> virtualPaths = 
		projectFile.Get_VirtualPaths();

	std::vector<VPathPair> vpathFilters = 
		ExpandVPaths(virtualPaths, files);

	// Generate filter list.
	std::map<std::string, std::string> sourceFilterMap;
	std::map<std::string, std::string> includeFilterMap;
	std::map<std::string, std::string> noneFilterMap;
	std::vector<std::string> filters = SortFiltersByType(
		vpathFilters, 
		solutionDirectory,
		sourceFilterMap,
		includeFilterMap,
		noneFilterMap
	);

	XmlNode root;

	// Header
	root.Node("?xml")
		.Attribute("version", "1.0")
		.Attribute("encoding", "utf-8");

	XmlNode& project =
		root.Node("Project")
		.Attribute("ToolsVersion", "14.0")
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

	// Source files
	XmlNode& sourceGroup = project.Node("ItemGroup");
	for (auto pair : sourceFilterMap)
	{		
		XmlNode& itemNode =
			sourceGroup.Node("ClCompile")
			.Attribute("Include", "$(SolutionDir)\\%s", pair.first.c_str());

		itemNode.Node("Filter").Value("%s", pair.second.c_str());
	}
	
	// Header files
	XmlNode& headerGroup = project.Node("ItemGroup"); 
	for (auto pair : includeFilterMap)
	{
		XmlNode& itemNode =
			headerGroup.Node("ClInclude")
			.Attribute("Include", "$(SolutionDir)\\%s", pair.first.c_str());

		itemNode.Node("Filter").Value("%s", pair.second.c_str());
	}

	// none files
	XmlNode& noneGroup = project.Node("ItemGroup");
	for (auto pair : noneFilterMap)
	{
		XmlNode& itemNode =
			noneGroup.Node("None")
			.Attribute("Include", "$(SolutionDir)\\%s", pair.first.c_str());

		itemNode.Node("Filter").Value("%s", pair.second.c_str());
	}
	
	// Generate result.
	if (!databaseFile.StoreFile(
		workspaceFile,
		projectDirectory,
		projectFiltersLocation,
		root.ToString().c_str()))
	{
		return false;
	}

	return true;
}

bool Ide_MSBuild::Generate(
	DatabaseFile& databaseFile,
    WorkspaceFile& workspaceFile,
    std::vector<ProjectFile>& projectFiles)
{
	BuildWorkspaceMatrix matrix;
	if (!CreateBuildMatrix(workspaceFile, projectFiles, matrix))
	{
		return false;
	}

	int index = 0;
    for (ProjectFile& file : projectFiles)
    {
		switch (file.Get_Project_Language())
		{
		case ELanguage::Cpp:
			{
				if (!Generate_Vcxproj(
					databaseFile,
					workspaceFile,
					file,
					matrix[index]
					))
				{
					return false;
				}

				if (!Generate_Vcxproj_Filters(
					databaseFile,
					workspaceFile,
					file,
					matrix[index]
					))
				{
					return false;
				}

				break;
			}
		case ELanguage::CSharp:
			{
				if (!Generate_Csproj(
					databaseFile,
					workspaceFile,
					file,
					matrix[index]
					))
				{
					return false;
				}

				break;
			}
		default:
			{
				file.ValidateError(
					"Language '%s' is not valid for msbuild projects.",
					CastToString(file.Get_Project_Language()).c_str());
				return false;
			}
		}

		index++;
    }

    if (!GenerateSolutionFile(
		databaseFile,
        workspaceFile,
        projectFiles,
		matrix
        ))
    {
        return false;
    }

    return true;
}

bool Ide_MSBuild::Clean(WorkspaceFile& workspaceFile)
{
	Platform::Path solutionDirectory =
		workspaceFile.Get_Workspace_Location();

	Platform::Path solutionLocation =
		solutionDirectory.AppendFragment(
			workspaceFile.Get_Workspace_Name() + ".sln", true);

	std::vector<std::string> arguments;
	arguments.push_back(solutionLocation.ToString());
	arguments.push_back("/t:Clean");

	Platform::Process process;
	if (process.Open(GetMSBuildLocation(), solutionDirectory, arguments, false))
	{
		process.Wait();

		int exitCode = process.GetExitCode();
		if (exitCode == 0)
		{
			return true;
		}
		else
		{
			Log(LogSeverity::Fatal, "msbuild failed with exit code %i.\n", exitCode);
			return false;
		}
	}
	else
	{
		Log(LogSeverity::Fatal, "Failed to start msbuild process.\n");
		return false;
	}

	return false;
}

bool Ide_MSBuild::Build(
	WorkspaceFile& workspaceFile,
	bool bRebuild,
	const std::string& configuration,
	const std::string& platform)
{
	Platform::Path solutionDirectory =
		workspaceFile.Get_Workspace_Location();

	Platform::Path solutionLocation =
		solutionDirectory.AppendFragment(
			workspaceFile.Get_Workspace_Name() + ".sln", true);

	std::vector<std::string> arguments;
	arguments.push_back(solutionLocation.ToString());

	EPlatform platformId = CastFromString<EPlatform>(platform);

	if (bRebuild)
	{
		arguments.push_back("/t:Rebuild");
	}
	else
	{
		arguments.push_back("/t:Build");
	}

	arguments.push_back(
		Strings::Format("/p:Configuration=%s", configuration.c_str())
	);

	arguments.push_back(
		Strings::Format("/p:Platform=%s", GetPlatformDotNetTarget(platformId).c_str())
	);

	Platform::Process process;
	if (process.Open(GetMSBuildLocation(), solutionDirectory, arguments, false))
	{
		process.Wait();

		int exitCode = process.GetExitCode();
		if (exitCode == 0)
		{
			return true;
		}
		else
		{
			Log(LogSeverity::Fatal, "msbuild failed with exit code %i.\n", exitCode);
			return false;
		}
	}
	else
	{
		Log(LogSeverity::Fatal, "Failed to start msbuild process.\n");
		return false;
	}

	return false;
}

}; // namespace MicroBuild