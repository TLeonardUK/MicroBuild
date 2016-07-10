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
	case EPlatform::Windows_ARM:
		{
			return "ARM";
		}
    }
    return "";
}

std::string Ide_MSBuild::GetPlatformDotNetTarget(EPlatform platform)
{
    switch (platform)
    {
    case EPlatform::Windows_x86:
        {
            return "x86";
        }
    case EPlatform::Windows_x64:
        {
            return "x64";
        }
    case EPlatform::Windows_AnyCPU:
        {
            return "AnyCPU";
        }
	case EPlatform::Windows_ARM:
		{
			return "ARM";
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

	// Get a list of folders.
	std::vector<ProjectGroupFolder> folders =
		GetGroupFolders(workspaceFile, projectFiles);

	// Header
	output << "\n";
	output << "Microsoft Visual Studio Solution File, Format Version " << m_headerVersion << "\n";
	output << "# " << m_headerShortName << "\n";

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

		output << "Project(\"" << typeGuid << "\") = \"" << name << "\", \"" << relativeLocation << "\", \"" << guid << "\"" << "\n";
		output << "\t" << "ProjectSelection(ProjectDependencies) = postProject" << "\n";

		for (std::string dependency : file.Get_Dependencies_Dependency())
		{
			ProjectFile* projectDependency;
			if (!GetProjectDependency(workspaceFile, projectFiles, &file, projectDependency, dependency))
			{
				return false;
			}

			std::string depGuid = Strings::Guid({ workspaceFile.Get_Workspace_Name(), projectDependency->Get_Project_Name() });
			output << "\t\t" << depGuid << " = " << depGuid << "\n";
		}

		output << "\t" << "EndProjectSelection" << "\n";
		output << "EndProject" << "\n";
	}

	for (ProjectGroupFolder& folder : folders)
	{
		std::string guid =
			Strings::Guid({ workspaceFile.Get_Workspace_Name(), folder.name, "folder" });

		output << "Project(\"" << GUID_SOLUTION_FOLDER << "\") = \"" << folder.baseName << "\", \"" << folder.baseName << "\", \"" << guid << "\"" << "\n";
		output << "EndProject" << "\n";
	}

	// Global block
	output << "Global" << "\n";
	output << "\t" << "GlobalSection(SolutionConfigurationPlatforms) = preSolution" << "\n";

	for (std::string& config : configurations)
	{
		for (EPlatform& platform : platforms)
		{
			std::string platformStr = GetPlatformID(platform);

			output << "\t\t" << config << "|" << platformStr << " = " << config << "|" << platformStr << "\n";
		}
	}

	output << "\t" << "EndGlobalSection" << "\n";
	output << "\t" << "GlobalSection(ProjectConfigurationPlatforms) = postSolution" << "\n";

	for (BuildProjectMatrix& matrix : buildMatrix)
	{
		for (BuildProjectPair& pair : matrix)
		{
			std::string projectGuid = 
				Strings::Guid({ workspaceFile.Get_Workspace_Name(), pair.projectFile.Get_Project_Name() });

			std::string platformStr = GetPlatformID(pair.platform);

			output << "\t\t" << projectGuid << "." << pair.config << "|" << platformStr << ".ActiveCfg = " << pair.config << "|" << platformStr << "\n";

			if (pair.shouldBuild)
			{
				output << "\t\t" << projectGuid << "." << pair.config << "|" << platformStr << ".Build.0 = " << pair.config << "|" << platformStr << "\n";
			}
		}
	}

	output << "\t" << "EndGlobalSection" << "\n";
	output << "\t" << "GlobalSection(SolutionProperties) = preSolution" << "\n";
	output << "\t\tHideSolutionNode = FALSE" << "\n";
	output << "\t" << "EndGlobalSection" << "\n";
	output << "\t" << "GlobalSection(NestedProjects) = preSolution" << "\n";

	for (ProjectGroupFolder& folder : folders)
	{
		if (folder.parentName != "")
		{
			std::string guid =
				Strings::Guid({ workspaceFile.Get_Workspace_Name(), folder.name, "folder" });

			std::string parentGuid =
				Strings::Guid({ workspaceFile.Get_Workspace_Name(), folder.parentName, "folder" });

			output << "\t\t" << guid << " = " << parentGuid << "\n";
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

			output << "\t\t" << guid << " = " << parentGuid << "\n";
		}
	}

	output << "\t" << "EndGlobalSection" << "\n";
	output << "EndGlobal" << "\n";

	// Generate result.
	if (!WriteFile(
		workspaceFile,
		solutionDirectory,
		solutionLocation,
		output.str().c_str()))
	{
		return false;
	}

	return true;
}

bool Ide_MSBuild::Generate_Csproj(
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

	std::stringstream output;

	std::string projectGuid = Strings::Guid({
		workspaceFile.Get_Workspace_Name(),
		projectFile.Get_Project_Name() });

	// Check platform is supported.
	if (!ArePlatformsValid(projectFile, platforms, { 
		EPlatform::Windows_AnyCPU, 
		EPlatform::Windows_ARM,
		EPlatform::Windows_x86,
		EPlatform::Windows_x64
	}))
	{
		return false;
	}

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

	// Header
	output << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << "\n";
	output << "<Project DefaultTargets=\"Build\" ToolsVersion=\"14.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">" << "\n";

	// Globals
	output << "\t" << "<PropertyGroup>" << "\n";
	output << "\t\t" << "<Configuration Condition=\" '$(Configuration)' == '' \">" << CastToString(configurations[0]) << "</Configuration>" << "\n";
    output << "\t\t" << "<Platform Condition=\" '$(Platform)' == '' \">" << GetPlatformDotNetTarget(platforms[0]) << "</Platform>" << "\n";
	output << "\t\t" << "<ProjectGuid>" << projectGuid << "</ProjectGuid>" << "\n";
	switch (projectFile.Get_Project_OutputType())
	{
	case EOutputType::Executable:
		output << "\t\t" << "<OutputType>WinExe</OutputType>" << "\n";
		break;
	case EOutputType::ConsoleApp:
		output << "\t\t" << "<OutputType>Exe</OutputType>" << "\n";
		break;
	case EOutputType::DynamicLib:
		output << "\t\t" << "<OutputType>Library</OutputType>" << "\n";
		break;
	default:
		projectFile.ValidateError(
			"Output type '%s' is not valid for msbuild .NET projects.",
			CastToString(projectFile.Get_Project_OutputType()).c_str());
		return false;
	}

	output << "\t\t" << "<AppDesignerFolder>Properties</AppDesignerFolder>" << "\n";

	switch (projectFile.Get_Build_PlatformToolset())
	{
	case EPlatformToolset::Default:
		break;
	case EPlatformToolset::DotNet_2_0:
		output << "\t\t" << "<TargetFrameworkVersion>v2.0</TargetFrameworkVersion>" << "\n";
		break;
	case EPlatformToolset::DotNet_3_0:
		output << "\t\t" << "<TargetFrameworkVersion>v3.0</TargetFrameworkVersion>" << "\n";
		break;
	case EPlatformToolset::DotNet_3_5:
		output << "\t\t" << "<TargetFrameworkVersion>v3.5</TargetFrameworkVersion>" << "\n";
		break;
	case EPlatformToolset::DotNet_4_0:
		output << "\t\t" << "<TargetFrameworkVersion>v4.0</TargetFrameworkVersion>" << "\n";
		break;
	case EPlatformToolset::DotNet_4_5:
		output << "\t\t" << "<TargetFrameworkVersion>v4.5</TargetFrameworkVersion>" << "\n";
		break;
	case EPlatformToolset::DotNet_4_5_1:
		output << "\t\t" << "<TargetFrameworkVersion>v4.5.1</TargetFrameworkVersion>" << "\n";
		break;
	case EPlatformToolset::DotNet_4_5_2:
		output << "\t\t" << "<TargetFrameworkVersion>v4.5.2</TargetFrameworkVersion>" << "\n";
		break;
	case EPlatformToolset::DotNet_4_6:
		output << "\t\t" << "<TargetFrameworkVersion>v4.6</TargetFrameworkVersion>" << "\n";
		break;
	default:
		projectFile.ValidateError(
			"Platform toolset '%s' is not valid for msbuild .NET projects.",
			CastToString(projectFile.Get_Build_PlatformToolset()).c_str());
		return false;
	}

	output << "\t\t" << "<FileAlignment>512</FileAlignment>" << "\n";
	output << "\t\t" << "<AutoGenerateBindingRedirects>true</AutoGenerateBindingRedirects>" << "\n";

	output << "\t\t" << "<RootNamespace>" << projectFile.Get_Project_RootNamespace() << "</RootNamespace>" << "\n";
	output << "\t" << "</PropertyGroup>" << "\n";

	// Property Grid
	for (auto matrix : buildMatrix)
	{
		std::string platformId = GetPlatformID(matrix.platform);

				Platform::Path outDir = matrix.projectFile.Get_Project_OutputDirectory();
		Platform::Path intDir = matrix.projectFile.Get_Project_IntermediateDirectory();

		Platform::Path outDirRelative = solutionDirectory.RelativeTo(outDir);
		Platform::Path intDirRelative = solutionDirectory.RelativeTo(intDir);

		std::vector<std::string> defines = matrix.projectFile.Get_Defines_Define();
		
		output << "\t" << "<PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='" << matrix.config << "|" << platformId << "'\" Label=\"Configuration\">" << "\n";
		output << "\t\t" << "<DebugSymbols>" << CastToString(matrix.projectFile.Get_Flags_GenerateDebugInformation()) << "</DebugSymbols>" << "\n";
		output << "\t\t" << "<BaseOutputPath>$(SolutionDir)\\" << outDirRelative.ToString() << "\\</BaseOutputPath>" << "\n";
		output << "\t\t" << "<OutputPath>$(BaseOutputPath)\\</OutputPath>" << "\n";
		output << "\t\t" << "<BaseIntermediateOutputPath>$(SolutionDir)\\" << intDirRelative.ToString() << "\\</BaseIntermediateOutputPath>" << "\n";
		output << "\t\t" << "<IntermediateOutputPath>$(BaseIntermediateOutputPath)\\</IntermediateOutputPath>" << "\n";
		output << "\t\t" << "<DefineConstants>" << Strings::Join(defines, ";") << "</DefineConstants>" << "\n";
		output << "\t\t" << "<DebugType>" << (matrix.projectFile.Get_Flags_GenerateDebugInformation() ? "full" : "pdbonly") << "</DebugType>" << "\n";
		output << "\t\t" << "<PlatformTarget>" << GetPlatformDotNetTarget(matrix.platform) << "</PlatformTarget>" << "\n";
		output << "\t\t" << "<ErrorReport>prompt</ErrorReport>" << "\n";		
		output << "\t\t" << "<WarningsAsErrors>" << (matrix.projectFile.Get_Flags_CompilerWarningsFatal() ? "true" : "false") << "</WarningsAsErrors>" << "\n";
		output << "\t\t" << "<CodeAnalysisRuleSet>MinimumRecommendedRules.ruleset</CodeAnalysisRuleSet>" << "\n";
		output << "\t\t" << "<Prefer32Bit>" << CastToString(matrix.projectFile.Get_Flags_Prefer32Bit()) << "</Prefer32Bit>" << "\n";
		output << "\t\t" << "<AllowUnsafeBlocks>" << CastToString(matrix.projectFile.Get_Flags_AllowUnsafeCode()) << "</AllowUnsafeBlocks>" << "\n";

		if (matrix.projectFile.Get_Build_OptimizationLevel() == EOptimizationLevel::None ||
			matrix.projectFile.Get_Build_OptimizationLevel() == EOptimizationLevel::Debug)
		{
			output << "\t\t" << "<Optimize>false</Optimize>" << "\n";
		}
		else
		{
			output << "\t\t" << "<Optimize>true</Optimize>" << "\n";
		}

		switch (projectFile.Get_Project_LanguageVersion())
		{
		case ELanguageVersion::Default:
			break;
		case ELanguageVersion::CSharp_1_0:
			output << "\t\t" << "<LangVersion>ISO-1</LangVersion>" << "\n";
			break;
		case ELanguageVersion::CSharp_2_0:
			output << "\t\t" << "<LangVersion>ISO-2</LangVersion>" << "\n";
			break;
		case ELanguageVersion::CSharp_3_0:
			output << "\t\t" << "<LangVersion>3</LangVersion>" << "\n";
			break;
		case ELanguageVersion::CSharp_4_0:
			output << "\t\t" << "<LangVersion>4</LangVersion>" << "\n";
			break;
		case ELanguageVersion::CSharp_5_0:
			output << "\t\t" << "<LangVersion>5</LangVersion>" << "\n";
			break;
		case ELanguageVersion::CSharp_6_0:
			output << "\t\t" << "<LangVersion>6</LangVersion>" << "\n";
			break;
		default:
			projectFile.ValidateError(
				"Project language version '%s' is not valid for this project type.",
				CastToString(projectFile.Get_Project_LanguageVersion()).c_str());
			return false;
		}

		output << "\t" << "</PropertyGroup>" << "\n";
	}

	// Startup Property Grid
	output << "\t" << "<PropertyGroup>" << "\n";
	output << "\t\t" << "<StartupObject />" << "\n";
	output << "\t" << "</PropertyGroup>" << "\n";

	// References
	output << "\t" << "<ItemGroup>" << "\n";
	for (auto file : projectFile.Get_References_Reference())
	{
		output << "\t\t" << "<Reference Include=\"" << file.ToString() << "\" />" << "\n";
	}
	output << "\t" << "</ItemGroup>" << "\n";

	// Source files
	output << "\t" << "<ItemGroup>" << "\n";
	for (auto file : sourceFiles)
	{
		output << "\t\t" << "<Compile Include=\"" << file << "\" />" << "\n";
	}
	output << "\t" << "</ItemGroup>" << "\n";

	// Misc file.
	output << "\t" << "<ItemGroup>" << "\n";
	for (auto file : miscFiles)
	{
		output << "\t\t" << "<None Include=\"" << file << "\" />" << "\n";
	}
	output << "\t" << "</ItemGroup>" << "\n";


	output << "\t" << "<Import Project=\"$(MSBuildToolsPath)\\Microsoft.CSharp.targets\" />" << "\n";
	output << "</Project>" << "\n";

	// Generate result.
	if (!WriteFile(
		workspaceFile,
		projectDirectory,
		projectLocation,
		output.str().c_str()))
	{
		return false;
	}

	return true;
}

bool Ide_MSBuild::Generate_Vcxproj(
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

	std::stringstream output;

	std::string projectGuid = Strings::Guid({
		workspaceFile.Get_Workspace_Name(),
		projectFile.Get_Project_Name() });

	// Check platform is supported.
	if (!ArePlatformsValid(projectFile, platforms, {
		EPlatform::Windows_AnyCPU,
		EPlatform::Windows_ARM,
		EPlatform::Windows_x86,
		EPlatform::Windows_x64
	}))
	{
		return false;
	}

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

	// Header
	output << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << "\n";
	output << "<Project DefaultTargets=\"Build\" ToolsVersion=\"14.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">" << "\n";

	// Build Matrix
	output << "\t" << "<ItemGroup Label=\"ProjectConfigurations\">" << "\n";	
	for (auto matrix : buildMatrix)
	{
		std::string platformId = GetPlatformID(matrix.platform);

		output << "\t\t" << "<ProjectConfiguration Include=\"" << matrix.config << "|" << platformId << "\">" << "\n";
		output << "\t\t\t" << "<Configuration>" << matrix.config << "</Configuration>" << "\n";
		output << "\t\t\t" << "<Platform>" << platformId << "</Platform>" << "\n";
		output << "\t\t" << "</ProjectConfiguration>" << "\n";
	}
	output << "\t" << "</ItemGroup>" << "\n";
	
	// Globals
	output << "\t" << "<PropertyGroup Label=\"Globals\">" << "\n";
	output << "\t\t" << "<ProjectGuid>" << projectGuid << "</ProjectGuid>" << "\n";
	output << "\t\t" << "<IgnoreWarnCompileDuplicatedFilename>true</IgnoreWarnCompileDuplicatedFilename>" << "\n";
	output << "\t\t" << "<Keyword>Win32Proj</Keyword>" << "\n";
	output << "\t\t" << "<RootNamespace>" << projectFile.Get_Project_RootNamespace() <<  "</RootNamespace>" << "\n";
	output << "\t" << "</PropertyGroup>" << "\n";

	output << "\t" << "<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />" << "\n";

	// Property Grid
	for (auto matrix : buildMatrix)
	{
		std::string platformId = GetPlatformID(matrix.platform);

		output << "\t" << "<PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='" << matrix.config << "|" << platformId << "'\" Label=\"Configuration\">" << "\n";

		// Output type.
		switch (matrix.projectFile.Get_Project_OutputType())
		{
		case EOutputType::Executable:
			// Fallthrough
		case EOutputType::ConsoleApp:
			output << "\t\t" << "<ConfigurationType>Application</ConfigurationType>" << "\n";
			break;
		case EOutputType::DynamicLib:
			output << "\t\t" << "<ConfigurationType>DynamicLibrary</ConfigurationType>" << "\n";
			break;
		case EOutputType::StaticLib:
			output << "\t\t" << "<ConfigurationType>StaticLibrary</ConfigurationType>" << "\n";
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
			output << "\t\t" << "<UseDebugLibraries>true</UseDebugLibraries>" << "\n";
		}
		else
		{
			output << "\t\t" << "<UseDebugLibraries>false</UseDebugLibraries>" << "\n";
		}

		// Character set.
		switch (matrix.projectFile.Get_Build_CharacterSet())
		{
		case ECharacterSet::Default:
			break;
		case ECharacterSet::Unicode:
			output << "\t\t" << "<CharacterSet>Unicode</CharacterSet>" << "\n";
			break;
		case ECharacterSet::MBCS:
			output << "\t\t" << "<CharacterSet>MultiByte</CharacterSet>" << "\n";
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
			output << "\t\t" << "<PlatformToolset>" << CastToString(m_defaultToolset) << "</PlatformToolset>" << "\n";
			break;
		case EPlatformToolset::v140:
			output << "\t\t" << "<PlatformToolset>v140</PlatformToolset>" << "\n";
			break;
		case EPlatformToolset::v140_xp:
			output << "\t\t" << "<PlatformToolset>v140_xp</PlatformToolset>" << "\n";
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
			output << "\t\t" << "<WholeProgramOptimization>false</WholeProgramOptimization>" << "\n";
		}
		else
		{
			output << "\t\t" << "<WholeProgramOptimization>true</WholeProgramOptimization>" << "\n";
		}

		output << "\t" << "</PropertyGroup>" << "\n";
	}

	output << "\t" << "<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />" << "\n";

	output << "\t" << "<ImportGroup Label=\"ExtensionSettings\">" << "\n";
	output << "\t" << "</ImportGroup>" << "\n";

	// Item definition group.
	for (auto matrix : buildMatrix)
	{
		std::string platformId = GetPlatformID(matrix.platform);

		output << "\t" << "<ImportGroup Label=\"PropertySheets\" Condition=\"'$(Configuration)|$(Platform)'=='" << matrix.config << "|" << platformId << "'\">" << "\n";
		output << "\t\t" << "<Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')\" Label=\"LocalAppDataPlatform\" />" << "\n";
		output << "\t" << "</ImportGroup>" << "\n";
	}

	output << "\t" << "<PropertyGroup Label=\"UserMacros\" />" << "\n";

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

		output << "\t" << "<PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='" << matrix.config << "|" << platformId << "'\">" << "\n";

		// Incremental linking
		if (matrix.projectFile.Get_Flags_LinkTimeOptimization())
		{
			output << "\t\t" << "<LinkIncremental>false</LinkIncremental>" << "\n";
		}
		else
		{
			output << "\t\t" << "<LinkIncremental>" << (matrix.projectFile.Get_Build_OptimizationLevel() != EOptimizationLevel::Full ? "true" : "false") << "</LinkIncremental>" << "\n";
		}

		// Output information.
		output << "\t\t" << "<OutDir>$(SolutionDir)\\" << outDirRelative.ToString() << "\\</OutDir>" << "\n";
		output << "\t\t" << "<IntDir>$(SolutionDir)\\" << intDirRelative.ToString() << "\\</IntDir>" << "\n";
		output << "\t\t" << "<TargetName>" << matrix.projectFile.Get_Project_OutputName() << "</TargetName>" << "\n";
		output << "\t\t" << "<TargetExt>" << matrix.projectFile.Get_Project_OutputExtension() << "</TargetExt>" << "\n";

		// Search paths.
		if (includePaths.size() > 0)
		{
			output << "\t\t" << "<IncludePath>" << Strings::Join(includePaths, ";") << ";$(IncludePath)</IncludePath>" << "\n";
		}
		if (libraryPaths.size() > 0)
		{
			output << "\t\t" << "<LibraryPath>" << Strings::Join(libraryPaths, ";") << ";$(LibraryPath)</LibraryPath>" << "\n";
		}

		output << "\t" << "</PropertyGroup>" << "\n";
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
	
		output << "\t" << "<ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='" << matrix.config << "|" << platformId << "'\">" << "\n";
		output << "\t\t" << "<ClCompile>" << "\n";

			// Precompiled header.
			if (precompiledHeader.ToString() != "")
			{
				output << "\t\t\t" << "<PrecompiledHeader>Use</PrecompiledHeader>" << "\n";
				output << "\t\t\t" << "<PrecompiledHeaderFile>" << precompiledHeader.GetFilename() << "</PrecompiledHeaderFile>" << "\n";
			}

			// General settings.
			if (matrix.projectFile.Get_Defines_Define().size() > 0)
			{
				std::vector<std::string> defines = matrix.projectFile.Get_Defines_Define();
				output << "\t\t\t" << "<PreprocessorDefinitions>" << Strings::Join(defines, ";") << ";$(PreprocessorDefinitions)</PreprocessorDefinitions>" << "\n";
			}
			output << "\t\t\t" << "<MinimalRebuild>false</MinimalRebuild>" << "\n";
			output << "\t\t\t" << "<MultiProcessorCompilation>true</MultiProcessorCompilation>" << "\n";
			output << "\t\t\t" << "<RuntimeTypeInfo>" << (matrix.projectFile.Get_Flags_RuntimeTypeInfo() ? "true" : "false") << "</RuntimeTypeInfo>" << "\n";
			output << "\t\t\t" << "<ExceptionHandling>" << (matrix.projectFile.Get_Flags_Exceptions() ? "Async" : "false") << "</ExceptionHandling>" << "\n";

			// Standard library.
			if (matrix.projectFile.Get_Build_OptimizationLevel() == EOptimizationLevel::None ||
				matrix.projectFile.Get_Build_OptimizationLevel() == EOptimizationLevel::Debug)
			{
				if (matrix.projectFile.Get_Flags_StaticRuntime())
				{
					output << "\t\t\t" << "<RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>" << "\n";
				}
				else
				{
					output << "\t\t\t" << "<RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>" << "\n";
				}
			}
			else
			{
				if (matrix.projectFile.Get_Flags_StaticRuntime())
				{
					output << "\t\t\t" << "<RuntimeLibrary>MultiThreaded</RuntimeLibrary>" << "\n";
				}
				else
				{
					output << "\t\t\t" << "<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>" << "\n";
				}
			}

			// Debug database.
			if (matrix.projectFile.Get_Flags_GenerateDebugInformation())
			{
				output << "\t\t\t" << "<DebugInformationFormat>ProgramDatabase</DebugInformationFormat>" << "\n";
			}
			else
			{
				output << "\t\t\t" << "<DebugInformationFormat>None</DebugInformationFormat>" << "\n";
			}

			// Warning level.
			switch (matrix.projectFile.Get_Build_WarningLevel())
			{
			case EWarningLevel::Default:
				break;
			case EWarningLevel::None:
				output << "\t\t\t" << "<WarningLevel>TurnOffAllWarnings</WarningLevel>" << "\n";
				break;
			case EWarningLevel::Low:
				output << "\t\t\t" << "<WarningLevel>Level1</WarningLevel>" << "\n";
				break;
			case EWarningLevel::Medium:
				output << "\t\t\t" << "<WarningLevel>Level3</WarningLevel>" << "\n";
				break;
			case EWarningLevel::High:
				output << "\t\t\t" << "<WarningLevel>Level4</WarningLevel>" << "\n";
				break;
			case EWarningLevel::Verbose:
				output << "\t\t\t" << "<WarningLevel>EnableAllWarnings</WarningLevel>" << "\n";
				break;
			default:
				projectFile.ValidateError(
					"Warning level '%s' is not valid for msbuild C++ projects.",
					CastToString(matrix.projectFile.Get_Build_WarningLevel()).c_str());
				return false;
			}

			output << "\t\t\t" << "<TreatWarningAsError>" << (matrix.projectFile.Get_Flags_CompilerWarningsFatal() ? "true" : "false") << "</TreatWarningAsError>" << "\n";

			if (matrix.projectFile.Get_DisabledWarnings_DisabledWarning().size() > 0)
			{
				std::vector<std::string> disabledWarnings = matrix.projectFile.Get_DisabledWarnings_DisabledWarning();
				output << "\t\t\t" << "<DisableSpecificWarnings>" << Strings::Join(disabledWarnings, ";") << ";$(DisableSpecificWarnings)</DisableSpecificWarnings>" << "\n";
			}

			// Optimization.
			switch (matrix.projectFile.Get_Build_OptimizationLevel())
			{
			case EOptimizationLevel::None:
				// Fallthrough
			case EOptimizationLevel::Debug:
				output << "\t\t\t" << "<Optimization>Disabled</Optimization>" << "\n";
				break;
			case EOptimizationLevel::PreferSize:
				output << "\t\t\t" << "<Optimization>MinSpace</Optimization>" << "\n";
				break;
			case EOptimizationLevel::PreferSpeed:
				output << "\t\t\t" << "<Optimization>MaxSpeed</Optimization>" << "\n";
				break;
			case EOptimizationLevel::Full:
				output << "\t\t\t" << "<Optimization>Full</Optimization>" << "\n";
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
				output << "\t\t\t" << "<AdditionalOptions>" << matrix.projectFile.Get_Build_CompilerArguments() << ";$(AdditionalOptions)</AdditionalOptions>" << "\n";
			}

			if (forcedIncludes.size() > 0)
			{
				output << "\t\t\t" << "<ForcedIncludeFiles>" << Strings::Join(forcedIncludes, ";") << ";$(ForcedIncludeFiles)</ForcedIncludeFiles>" << "\n";
			}

		output << "\t\t" << "</ClCompile>" << "\n";
		output << "\t\t" << "<Link>" << "\n";

			// Warnings.
			output << "\t\t\t" << "<TreatLinkerWarningAsErrors>" << (matrix.projectFile.Get_Flags_LinkerWarningsFatal() ? "true" : "false") << "</TreatLinkerWarningAsErrors>" << "\n";

			// Optimization.
			if (matrix.projectFile.Get_Flags_LinkTimeOptimization())
			{
				output << "\t\t\t" << "<LinkTimeCodeGeneration>UseLinkTimeCodeGeneration</LinkTimeCodeGeneration>" << "\n";
				output << "\t\t\t" << "<WholeProgramOptimization>true</WholeProgramOptimization>" << "\n";

			}
			else
			{
				output << "\t\t\t" << "<WholeProgramOptimization>false</WholeProgramOptimization>" << "\n";
			}

			// Sub system
			switch (matrix.projectFile.Get_Project_OutputType())
			{
			case EOutputType::Executable:
				output << "\t\t\t" << "<SubSystem>Windows</SubSystem>" << "\n";
				break;
			case EOutputType::ConsoleApp:
				// Fallthrough
			case EOutputType::DynamicLib:
				// Fallthrough
			case EOutputType::StaticLib:
				output << "\t\t\t" << "<SubSystem>Console</SubSystem>" << "\n";
				break;
			default:
				projectFile.ValidateError(
					"Output type '%s' is not valid for msbuild C++ projects.",
					CastToString(matrix.projectFile.Get_Project_OutputType()).c_str());
				return false;
			}

			// Debug information.
			output << "\t\t\t" << "<GenerateDebugInformation>" << (matrix.projectFile.Get_Flags_GenerateDebugInformation() ? "true" : "false") << "</GenerateDebugInformation>" << "\n";

			// Libraries.
			if (additionalLibraries.size() > 0)
			{
				output << "\t\t\t" << "<AdditionalDependencies>" << Strings::Join(additionalLibraries, ";") << ";$(AdditionalDependencies)</AdditionalDependencies>" << "\n";
			}

			// Additional options.
			output << "\t\t\t" << "<EntryPointSymbol>mainCRTStartup</EntryPointSymbol>" << "\n";
			
			if (!matrix.projectFile.Get_Build_LinkerArguments().empty())
			{
				output << "\t\t\t" << "<AdditionalOptions>" << matrix.projectFile.Get_Build_LinkerArguments() << ";$(AdditionalOptions)</AdditionalOptions>" << "\n";
			}

		output << "\t\t" << "</Link>" << "\n";
		output << "\t" << "</ItemDefinitionGroup>" << "\n";
	}

	// Include files
	output << "\t" << "<ItemGroup>" << "\n";
	for (auto file : includeFiles)
	{
		output << "\t\t" << "<ClInclude Include=\"" << file << "\" />" << "\n";
	}
	output << "\t" << "</ItemGroup>" << "\n";

	// Source files
	output << "\t" << "<ItemGroup>" << "\n";
	for (auto file : sourceFiles)
	{
		if (file == precompiledSourceFile)
		{
			output << "\t\t" << "<ClCompile Include=\"" << file << "\">" << "\n";
			output << "\t\t\t" << "<PrecompiledHeader>Create</PrecompiledHeader>" << "\n";
			output << "\t\t" << "</ClCompile>" << "\n";
		}
		else
		{
			output << "\t\t" << "<ClCompile Include=\"" << file << "\" />" << "\n";
		}
	}
	output << "\t" << "</ItemGroup>" << "\n";

	// Misc file.
	output << "\t" << "<ItemGroup>" << "\n";
	for (auto file : miscFiles)
	{
		output << "\t\t" << "<None Include=\"" << file << "\" />" << "\n";
	}
	output << "\t" << "</ItemGroup>" << "\n";


	output << "\t" << "<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />" << "\n";
	output << "\t" << "<ImportGroup Label=\"ExtensionTargets\">" << "\n";
	output << "\t" << "</ImportGroup>" << "\n";
	output << "</Project>" << "\n";

	// Generate result.
	if (!WriteFile(
		workspaceFile,
		projectDirectory,
		projectLocation,
		output.str().c_str()))
	{
		return false;
	}

    return true;
}

bool Ide_MSBuild::Generate_Vcxproj_Filters(
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

	std::stringstream output;

	std::vector<Platform::Path> files = projectFile.Get_Files_File();
	std::vector<std::string> filters;
	std::map<std::string, std::string> sourceFilterMap;
	std::map<std::string, std::string> includeFilterMap;
	std::map<std::string, std::string> noneFilterMap;

	// Find common path.
	Platform::Path commonPath;
	Platform::Path::GetCommonPath(files, commonPath);

	// Generate filter list.
	for (Platform::Path& file : files)
	{
		Platform::Path relativePath =
			solutionDirectory.RelativeTo(file);

		// Generate a list of unique filters.
		std::string filter = file.GetUncommonPath(commonPath).ToString();

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

	// Header
	output << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << "\n";
	output << "<Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">" << "\n";

	// Filter block.
	output << "\t" << "<ItemGroup>" << "\n";

	for (std::string& filter : filters)
	{
		std::string guid =
			Strings::Guid({ workspaceFile.Get_Workspace_Name(), filter, "filter" });

		output << "\t\t" << "<Filter Include=\"" << filter << "\">" << "\n";
		output << "\t\t\t" << "<UniqueIdentifier>" << guid << "</UniqueIdentifier>" << "\n";
		output << "\t\t" << "</Filter>" << "\n";
	}

	output << "\t" << "</ItemGroup>" << "\n";

	// Source files
	output << "\t" << "<ItemGroup>" << "\n";
	for (auto pair : sourceFilterMap)
	{
		
		output << "\t\t" << "<ClCompile Include=\"$(SolutionDir)\\" << pair.first << "\">" << "\n";
		output << "\t\t\t" << "<Filter>" << pair.second << "</Filter>" << "\n";
		output << "\t\t" << "</ClCompile>" << "\n";
	}
	output << "\t" << "</ItemGroup>" << "\n";

	// Header files
	output << "\t" << "<ItemGroup>" << "\n";
	for (auto pair : includeFilterMap)
	{

		output << "\t\t" << "<ClInclude Include=\"$(SolutionDir)\\" << pair.first << "\">" << "\n";
		output << "\t\t\t" << "<Filter>" << pair.second << "</Filter>" << "\n";
		output << "\t\t" << "</ClInclude>" << "\n";
	}
	output << "\t" << "</ItemGroup>" << "\n";

	// none files
	output << "\t" << "<ItemGroup>" << "\n";
	for (auto pair : noneFilterMap)
	{

		output << "\t\t" << "<None Include=\"$(SolutionDir)\\" << pair.first << "\">" << "\n";
		output << "\t\t\t" << "<Filter>" << pair.second << "</Filter>" << "\n";
		output << "\t\t" << "</None>" << "\n";
	}
	output << "\t" << "</ItemGroup>" << "\n";

	output << "</Project>" << "\n";

	// Generate result.
	if (!WriteFile(
		workspaceFile,
		projectDirectory,
		projectFiltersLocation,
		output.str().c_str()))
	{
		return false;
	}

	return true;
}

bool Ide_MSBuild::Generate(
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
					workspaceFile,
					file,
					matrix[index]
					))
				{
					return false;
				}

				if (!Generate_Vcxproj_Filters(
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
        workspaceFile,
        projectFiles,
		matrix
        ))
    {
        return false;
    }

    return true;
}

}; // namespace MicroBuild