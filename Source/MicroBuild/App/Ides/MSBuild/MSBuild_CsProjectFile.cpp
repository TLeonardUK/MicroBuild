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
#include "App/Ides/MSBuild/MSBuild_Ids.h"
#include "App/Ides/MSBuild/MSBuild_CsProjectFile.h"
#include "Core/Helpers/TextStream.h"
#include "Core/Helpers/XmlNode.h"
#include "Core/Helpers/JsonNode.h"

namespace MicroBuild {

MSBuild_CsProjectFile::MSBuild_CsProjectFile(
	std::string defaultToolset
)
	: m_defaultToolsetString(defaultToolset)
{
}

MSBuild_CsProjectFile::~MSBuild_CsProjectFile()
{
}

bool MSBuild_CsProjectFile::Generate(
	DatabaseFile& databaseFile,
	WorkspaceFile& workspaceFile,
	ProjectFile& projectFile,
	IdeHelper::BuildProjectMatrix& buildMatrix
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

	// Build a file->liunk map.
	std::map<std::string, std::string> fileLinkMap;

	// Split into file-type buckets.
	std::vector<std::string> sourceFiles;
	std::vector<std::string> miscFiles;
	std::vector<std::string> xamlFiles;
	std::vector<std::string> imageFiles;

	std::string appxManifestFile;
	std::string packageSigningKey =
		solutionDirectory.RelativeTo(projectFile.Get_WinRT_PackageSigningKey()).ToString();

	std::string defaultAppxManifest;
	std::string defaultPackageSignedKey;

	for (Platform::Path& path : files)
	{
		std::string nonPrefixedPath =
			solutionDirectory.RelativeTo(path).ToString();

		std::string relativePath = "$(SolutionDir)" +
			nonPrefixedPath;

		std::string filter = filterMap[nonPrefixedPath].c_str();

		std::string linkPath = filter;

		if (!linkPath.empty())
		{
			linkPath = Strings::Format(
				"%s%c%s",
				linkPath.c_str(),
				Platform::Path::Seperator,
				path.GetFilename().c_str()
			);
		}
		else
		{
			linkPath = path.GetFilename().c_str();
		}

		fileLinkMap[relativePath] = linkPath;

		if (path.IsSourceFile())
		{
			sourceFiles.push_back(relativePath);
		}
		else
		{
			if (projectFile.Get_Project_Subsystem() == EPlatformSubSystem::WinRT)
			{
				if (path == projectFile.Get_WinRT_Manifest())
				{
					appxManifestFile = relativePath;
				}

				if (path.GetExtension() == "pfx")
				{
					defaultPackageSignedKey = relativePath;
				}
				else if (path.GetExtension() == "appxmanifest")
				{
					defaultAppxManifest = relativePath;
				}

				if (path.IsXamlFile())
				{
					xamlFiles.push_back(relativePath);
				}
				else if (path.IsImageFile())
				{
					imageFiles.push_back(relativePath);
				}
				else
				{
					miscFiles.push_back(relativePath);
				}
			}
			else
			{
				miscFiles.push_back(relativePath);
			}
		}
	}

	if (appxManifestFile.empty())
	{
		appxManifestFile = defaultAppxManifest;
	}
	if (projectFile.Get_WinRT_PackageSigningKey().IsEmpty())
	{
		packageSigningKey = defaultPackageSignedKey;
	}

	XmlNode root;

	// Header
	root.Node("?xml")
		.Attribute("version", "1.0")
		.Attribute("encoding", "utf-8");

	XmlNode& project =
		root.Node("Project")
		.Attribute("DefaultTargets", "Build")
		.Attribute("ToolsVersion", "%s", m_defaultToolsetString.c_str())
		.Attribute("xmlns", "http://schemas.microsoft.com/developer/msbuild/2003");

	// Import block.
	project.Node("Import")
		.Attribute("Project", "$(MSBuildExtensionsPath)\\$(MSBuildToolsVersion)\\Microsoft.Common.props")
		.Attribute("Condition", "Exists('$(MSBuildExtensionsPath)\\$(MSBuildToolsVersion)\\Microsoft.Common.props')");

	// Globals
	XmlNode& configPropertyGroup =
		project.Node("PropertyGroup");

	configPropertyGroup.Node("Configuration")
		.Attribute("Condition", "'$(Configuration)'==''")
		.Value("%s", CastToString(configurations[0]).c_str());

	configPropertyGroup.Node("Platform")
		.Attribute("Condition", "'$(Platform)'==''")
		.Value("%s", MSBuild::GetPlatformDotNetTarget(platforms[0]).c_str());

	configPropertyGroup.Node("ProjectGuid")
		.Value("%s", projectGuid.c_str());


	// WinRT support.
	if (projectFile.Get_Project_Subsystem() == EPlatformSubSystem::WinRT)
	{
		configPropertyGroup.Node("DefaultLanguage").Value("%s", projectFile.Get_WinRT_Locale().c_str());
		configPropertyGroup.Node("TargetPlatformIdentifier").Value("UAP");
		configPropertyGroup.Node("TargetPlatformVersion").Value("%s", projectFile.Get_WinRT_PlatformVersion().c_str());
		configPropertyGroup.Node("TargetPlatformMinVersion").Value("%s", projectFile.Get_WinRT_PlatformMinimumVersion().c_str());
		configPropertyGroup.Node("MinimumVisualStudioVersion").Value("%s", m_defaultToolsetString.c_str());
		configPropertyGroup.Node("ProjectTypeGuids").Value("%s;%s", MSBuild::k_GuiUapProject, MSBuild::k_GuidCSharpProject);
		configPropertyGroup.Node("PackageCertificateKeyFile").Value("%s", defaultPackageSignedKey.c_str());
	}

	switch (projectFile.Get_Project_OutputType())
	{
	case EOutputType::Executable:
		if (projectFile.Get_Project_Subsystem() == EPlatformSubSystem::WinRT)
		{
			configPropertyGroup.Node("OutputType").Value("AppContainerExe");
		}
		else
		{
			configPropertyGroup.Node("OutputType").Value("WinExe");
		}
		break;
	case EOutputType::ConsoleApp:
		if (projectFile.Get_Project_Subsystem() == EPlatformSubSystem::WinRT)
		{
			configPropertyGroup.Node("OutputType").Value("AppContainerExe");
		}
		else
		{
			configPropertyGroup.Node("OutputType").Value("Exe");
		}
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
		std::string platformId = MSBuild::GetPlatformID(matrix.platform);

		Platform::Path outDir = matrix.projectFile.Get_Project_OutputDirectory();
		Platform::Path intDir = matrix.projectFile.Get_Project_IntermediateDirectory();

		Platform::Path outDirRelative = solutionDirectory.RelativeTo(outDir);
		Platform::Path intDirRelative = solutionDirectory.RelativeTo(intDir);

		std::vector<std::string> defines = matrix.projectFile.Get_Defines_Define();

		if (projectFile.Get_Project_Subsystem() == EPlatformSubSystem::WinRT)
		{
			defines.push_back("WINDOWS_UWP");
			defines.push_back("NETFX_CORE");
		}

		XmlNode& platformConfig =
			project.Node("PropertyGroup")
			.Attribute("Condition", "'$(Configuration)|$(Platform)' == '%s|%s'", matrix.config.c_str(), MSBuild::GetPlatformDotNetTarget(matrix.platform).c_str())
			;//.Attribute("Label", "Configuration");

		platformConfig.Node("DebugSymbols").Value(matrix.projectFile.Get_Flags_GenerateDebugInformation());
		platformConfig.Node("BaseOutputPath").Value("$(SolutionDir)\\%s", outDirRelative.ToString().c_str());
		platformConfig.Node("OutputPath").Value("$(BaseOutputPath)\\");
		//platformConfig.Node("BaseIntermediateOutputPath").Value("$(SolutionDir)\\%s\\", intDirRelative.ToString().c_str());
		//platformConfig.Node("IntermediateOutputPath").Value("$(SolutionDir)\\%s\\", intDirRelative.ToString().c_str());
		//platformConfig.Node("IntermediateOutputPath").Value("$(BaseIntermediateOutputPath)\\");
		platformConfig.Node("DefineConstants").Value("%s", Strings::Join(defines, ";").c_str());
		platformConfig.Node("DebugType").Value(matrix.projectFile.Get_Flags_GenerateDebugInformation() ? "full" : "pdbonly");
		platformConfig.Node("PlatformTarget").Value("%s", MSBuild::GetPlatformDotNetTarget(matrix.platform).c_str());
		platformConfig.Node("ErrorReport").Value("prompt");
		platformConfig.Node("WarningsAsErrors").Value(matrix.projectFile.Get_Flags_CompilerWarningsFatal());
		platformConfig.Node("CodeAnalysisRuleSet").Value("MinimumRecommendedRules.ruleset");
		platformConfig.Node("Prefer32Bit").Value(matrix.projectFile.Get_Flags_Prefer32Bit());
		platformConfig.Node("AllowUnsafeBlocks").Value(matrix.projectFile.Get_Flags_AllowUnsafeCode());

		std::vector<std::string> disabledWarnings =
			matrix.projectFile.Get_DisabledWarnings_DisabledWarning();

		if (matrix.projectFile.Get_Project_Subsystem() == EPlatformSubSystem::WinRT)
		{
			disabledWarnings.push_back("2008");
		}

		if (disabledWarnings.size() > 0)
		{
			platformConfig.Node("NoWarn").Value("%s", Strings::Join(disabledWarnings, ";").c_str());
		}

		if (matrix.projectFile.Get_Project_Subsystem() == EPlatformSubSystem::WinRT)
		{
			platformConfig.Node("UseVSHostingProcess").Value("false");
		}

		if (matrix.projectFile.Get_Build_OptimizationLevel() == EOptimizationLevel::None ||
			matrix.projectFile.Get_Build_OptimizationLevel() == EOptimizationLevel::Debug)
		{
			platformConfig.Node("Optimize").Value("false");
		}
		else
		{
			platformConfig.Node("Optimize").Value("true");
		}

		switch (matrix.projectFile.Get_Project_LanguageVersion())
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
	// WinRT references are part of projects.json, which is up to the user to deal with.
	if (projectFile.Get_Project_Subsystem() != EPlatformSubSystem::WinRT)
	{
		XmlNode& referenceFileGroup = project.Node("ItemGroup");
		for (auto file : projectFile.Get_References_Reference())
		{
			referenceFileGroup.Node("Reference")
				.Attribute("Include", "%s", file.ToString().c_str());
		}
	}
	else
	{
		XmlNode& referenceFileGroup = project.Node("ItemGroup");

		referenceFileGroup.Node("None")
			.Attribute("Include", "project.json");

		Platform::Path projectJsonLocation =
			projectDirectory.AppendFragment("project.json", true);

		// Write out reference file.
		JsonNode jsonRoot;

		JsonNode& dependencyRoot = jsonRoot.Node("dependencies");
		for (auto package : projectFile.Get_Packages())
		{
			dependencyRoot
				.Node("%s", package.first.c_str())
				.Value("%s", package.second.c_str());
		}

		JsonNode& frameworkRoot = jsonRoot.Node("frameworks");
		frameworkRoot.Node("uap10.0").Value("");

		JsonNode& runtimesRoot = jsonRoot.Node("runtimes");
		runtimesRoot.Node("win10-arm").Value("");
		runtimesRoot.Node("win10-arm-aot").Value("");
		runtimesRoot.Node("win10-x86").Value("");
		runtimesRoot.Node("win10-x86-aot").Value("");
		runtimesRoot.Node("win10-x64").Value("");
		runtimesRoot.Node("win10-x64-aot").Value("");

		// Generate result.
		if (!databaseFile.StoreFile(
			workspaceFile,
			projectDirectory,
			projectJsonLocation,
			jsonRoot.ToString().c_str()))
		{
			return false;
		}
	}

	// Keep a list of item nodes so we can do global adjustments later.
	std::vector<std::pair<std::string, XmlNode*>> fileNodes;

	// Xaml files
	if (xamlFiles.size() > 0)
	{
		XmlNode& xamlFileGroup = project.Node("ItemGroup");
		for (auto file : xamlFiles)
		{
			Platform::Path filePath = file;

			// It seems the app xaml always has to be named App.xaml, convinent for us.
			if (filePath.GetFilename() == "App.xaml")
			{
				XmlNode& fileNode = xamlFileGroup.Node("ApplicationDefinition")
					.Attribute("Include", "%s", file.c_str());

				fileNode
					.Node("SubType")
					.Value("Designer");

				fileNode
					.Node("Generator")
					.Value("MSBuild:Compile");

				fileNode
					.Node("Link")
					.Value("%s", fileLinkMap[file.c_str()].c_str());
			}
			else
			{
				XmlNode& fileNode = xamlFileGroup.Node("Page")
					.Attribute("Include", "%s", file.c_str());

				fileNode
					.Node("SubType")
					.Value("Designer");

				fileNode
					.Node("Generator")
					.Value("MSBuild:Compile");

				fileNode
					.Node("Link")
					.Value("%s", fileLinkMap[file.c_str()].c_str());
			}
		}
	}

	// Source files
	if (sourceFiles.size() > 0)
	{
		XmlNode& sourceFileGroup = project.Node("ItemGroup");
		for (auto file : sourceFiles)
		{
			XmlNode& fileNode =
				sourceFileGroup.Node("Compile")
				.Attribute("Include", "%s", file.c_str());

			if (projectFile.Get_Project_Subsystem() == EPlatformSubSystem::WinRT)
			{
				size_t xamlOffset = file.find(".xaml.");
				if (xamlOffset != std::string::npos)
				{
					fileNode.Node("DependentUpon")
						.Value("%s", fileLinkMap[file.substr(0, xamlOffset + 5)].c_str());
				}
			}

			fileNode
				.Node("Link")
				.Value("%s", fileLinkMap[file.c_str()].c_str());
		}
	}

	// Misc file.
	if (miscFiles.size() > 0)
	{
		XmlNode& miscFileGroup = project.Node("ItemGroup");
		for (auto file : miscFiles)
		{
			if (projectFile.Get_Project_Subsystem() == EPlatformSubSystem::WinRT)
			{
				if (file == appxManifestFile)
				{
					XmlNode& fileNode = miscFileGroup.Node("AppxManifest")
						.Attribute("Include", "%s", file.c_str());

					fileNode
						.Node("SubType")
						.Value("Designer");

					fileNode
						.Node("Link")
						.Value("%s", fileLinkMap[file.c_str()].c_str());

					continue;
				}
			}

			XmlNode& fileNode = miscFileGroup.Node("None")
				.Attribute("Include", "%s", file.c_str());

			fileNode
				.Node("Link")
				.Value("%s", fileLinkMap[file.c_str()].c_str());
		}
	}

	// Image files.
	if (imageFiles.size() > 0)
	{
		XmlNode& imageFileGroup = project.Node("ItemGroup");
		for (auto file : imageFiles)
		{
			XmlNode& fileNode = imageFileGroup.Node("Content")
				.Attribute("Include", "%s", file.c_str());

			fileNode
				.Node("Link")
				.Value("%s", fileLinkMap[file.c_str()].c_str());
		}
	}

	// Import
	if (projectFile.Get_Project_Subsystem() != EPlatformSubSystem::WinRT)
	{
		project.Node("Import")
			.Attribute("Project", "$(MSBuildToolsPath)\\Microsoft.CSharp.targets");
	}
	else
	{
		XmlNode& versionPropertyGroup =
			project.Node("PropertyGroup")
			.Attribute("Condition", "'$(VisualStudioVersion)'=='' or '$(VisualStudioVersion)' &lt; '%s' ", m_defaultToolsetString.c_str());

		versionPropertyGroup.Node("VisualStudioVersion")
			.Value("%s", m_defaultToolsetString.c_str());

		project.Node("Import")
			.Attribute("Project", "$(MSBuildExtensionsPath)\\Microsoft\\WindowsXaml\\v$(VisualStudioVersion)\\Microsoft.Windows.UI.Xaml.CSharp.targets");
	}

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

}; // namespace MicroBuild