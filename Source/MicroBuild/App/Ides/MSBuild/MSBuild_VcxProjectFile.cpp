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
#include "App/Ides/MSBuild/MSBuild.h"
#include "App/Ides/MSBuild/MSBuild_Ids.h"
#include "App/Ides/MSBuild/MSBuild_VcxProjectFile.h"
#include "App/Ides/MSBuild/MSBuild_VcxFiltersFile.h"
#include "Core/Helpers/TextStream.h"
#include "Core/Helpers/XmlNode.h"
#include "Core/Helpers/JsonNode.h"

namespace MicroBuild {

MSBuild_VcxProjectFile::MSBuild_VcxProjectFile(
	EPlatformToolset defaultToolset,
	std::string defaultToolsetString
)
	: m_defaultToolset(defaultToolset)
	, m_defaultToolsetString(defaultToolsetString)
{
}

MSBuild_VcxProjectFile::~MSBuild_VcxProjectFile()
{
}

bool MSBuild_VcxProjectFile::Generate(
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
	std::vector<std::string> xamlFiles;
	std::vector<std::string> imageFiles;

	std::string precompiledSourceFile;
	std::string appxManifestFile;
	std::string packageSigningKey =
		solutionDirectory.RelativeTo(projectFile.Get_WinRT_PackageSigningKey()).ToString();

	std::string defaultAppxManifest;
	std::string defaultPackageSignedKey;

	for (Platform::Path& path : projectFile.Get_Files_File())
	{
		std::string relativePath = "$(SolutionDir)" +
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
		.Attribute("ToolsVersion", "")
		.Attribute("xmlns", "http://schemas.microsoft.com/developer/msbuild/2003");

	// Build Matrix
	XmlNode& projectConfig =
		project.Node("ItemGroup")
		.Attribute("Label", "ProjectConfiguration");

	for (auto matrix : buildMatrix)
	{
		std::string platformId = MSBuild::GetPlatformID(matrix.platform);

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

	// WinRT support.
	if (projectFile.Get_Project_Subsystem() == EPlatformSubSystem::WinRT)
	{
		globals.Node("DefaultLanguage").Value("%s", projectFile.Get_WinRT_Locale().c_str());
		globals.Node("MinimumVisualStudioVersion").Value("%s", m_defaultToolsetString.c_str());
		globals.Node("AppContainerApplication").Value("true");
		globals.Node("ApplicationType").Value("Windows Store");
		globals.Node("WindowsTargetPlatformVersion").Value("%s", projectFile.Get_WinRT_PlatformVersion().c_str());
		globals.Node("WindowsTargetPlatformMinVersion").Value("%s", projectFile.Get_WinRT_PlatformMinimumVersion().c_str());
		globals.Node("ApplicationTypeRevision").Value("10.0");
	}

	// Imports
	project.Node("Import")
		.Attribute("Project", "$(VCTargetsPath)\\Microsoft.Cpp.Default.props");

	// Property Grid
	for (auto matrix : buildMatrix)
	{
		std::string platformId = MSBuild::GetPlatformID(matrix.platform);

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
		case EPlatformToolset::v140_clang_3_7:
			propertyGroup.Node("PlatformToolset").Value("v140_clang_3_7");
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

		// WinRT support.
		if (projectFile.Get_Project_Subsystem() == EPlatformSubSystem::WinRT)
		{
			propertyGroup.Node("UseDotNetNativeToolchain").Value(true);
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
		std::string platformId = MSBuild::GetPlatformID(matrix.platform);

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

	// WinRT.
	if (projectFile.Get_Project_Subsystem() == EPlatformSubSystem::WinRT)
	{
		XmlNode& winrtNode =
			project.Node("PropertyGroup");

		winrtNode
			.Node("PackageCertificateKeyFile")
			.Value("%s", defaultPackageSignedKey.c_str());
	}

	// Output property group.
	for (auto matrix : buildMatrix)
	{
		std::string platformId = MSBuild::GetPlatformID(matrix.platform);

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
		std::string platformId = MSBuild::GetPlatformID(matrix.platform);

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
			else
			{
				compileGroup.Node("PrecompiledHeader").Value("NotUsing");
			}

			// General settings.
			if (matrix.projectFile.Get_Defines_Define().size() > 0)
			{
				std::vector<std::string> defines = matrix.projectFile.Get_Defines_Define();
				compileGroup.Node("PreprocessorDefinitions").Value("%s;$(PreprocessorDefinitions)", Strings::Join(defines, ";").c_str());
			}
			compileGroup.Node("MinimalRebuild").Value("false");
			compileGroup.Node("MultiProcessorCompilation").Value("true");

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

			// RTTI
			compileGroup
				.Node("RuntimeTypeInfo")
				.Value(matrix.projectFile.Get_Flags_RuntimeTypeInfo());

			// Toolchain specific values.
			switch (matrix.projectFile.Get_Build_PlatformToolset())
			{
			case EPlatformToolset::v140_clang_3_7:
				{
					// Warning level.
					switch (matrix.projectFile.Get_Build_WarningLevel())
					{
					case EWarningLevel::Default:
						break;
					case EWarningLevel::None:
						compileGroup.Node("WarningLevel").Value("TurnOffAllWarnings");
						break;
					case EWarningLevel::Low:
						compileGroup.Node("WarningLevel").Value("EnableAllWarnings");
						break;
					case EWarningLevel::Medium:
						compileGroup.Node("WarningLevel").Value("EnableAllWarnings");
						break;
					case EWarningLevel::High:
						compileGroup.Node("WarningLevel").Value("EnableAllWarnings");
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

					// Exception handling.
					compileGroup
						.Node("ExceptionHandling")
						.Value((matrix.projectFile.Get_Flags_Exceptions() ? "Disabled" : "Enabled"));

					// Debug database.
					if (matrix.projectFile.Get_Flags_GenerateDebugInformation())
					{
						compileGroup.Node("DebugInformationFormat").Value("FullDebug");
					}
					else
					{
						compileGroup.Node("DebugInformationFormat").Value("None");
					}

					break;
				}
			case EPlatformToolset::Default:
				{
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

					// Exception handling.
					compileGroup
						.Node("ExceptionHandling")
						.Value((matrix.projectFile.Get_Flags_Exceptions() ? "Async" : "false"));

					// Debug database.
					if (matrix.projectFile.Get_Flags_GenerateDebugInformation())
					{
						compileGroup.Node("DebugInformationFormat").Value("ProgramDatabase");
					}
					else
					{
						compileGroup.Node("DebugInformationFormat").Value("None");
					}

					break;
				}
			default:
				{
					break;
				}
			}

			compileGroup.Node("TreatWarningAsError").Value(matrix.projectFile.Get_Flags_CompilerWarningsFatal());

			std::vector<std::string> disabledWarnings =
				matrix.projectFile.Get_DisabledWarnings_DisabledWarning();

			if (projectFile.Get_Project_Subsystem() == EPlatformSubSystem::WinRT)
			{
				disabledWarnings.push_back("4453");
				disabledWarnings.push_back("28204");
			}

			if (disabledWarnings.size() > 0)
			{
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
			std::string addtionalOptions = "";
			if (projectFile.Get_Project_Subsystem() == EPlatformSubSystem::WinRT)
			{
				addtionalOptions += "/bigobj ";
			}
			if (!matrix.projectFile.Get_Build_CompilerArguments().empty())
			{
				addtionalOptions += matrix.projectFile.Get_Build_CompilerArguments();
			}

			if (!addtionalOptions.empty())
			{
				compileGroup.Node("AdditionalOptions")
					.Value("%s $(AdditionalOptions)", addtionalOptions.c_str());
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

	// We store which groups each file goes into so we can easily
	// generate a filters file.
	std::vector<MSBuildFileGroup> fileGroupList;

	// Include files
	if (includeFiles.size() > 0)
	{
		MSBuildFileGroup group;

		XmlNode& includeFileGroup = project.Node("ItemGroup");
		for (auto file : includeFiles)
		{
			XmlNode& fileNode =
				includeFileGroup.Node("ClInclude")
				.Attribute("Include", "%s", file.c_str());

			MSBuildFile groupFile;
			groupFile.TypeId = "ClInclude";
			groupFile.Path = file;
			group.Files.push_back(groupFile);

			if (projectFile.Get_Project_Subsystem() == EPlatformSubSystem::WinRT)
			{
				size_t xamlOffset = file.find(".xaml.");
				if (xamlOffset != std::string::npos)
				{
					fileNode.Node("DependentUpon")
						.Value("%s", file.substr(0, xamlOffset + 5).c_str());
				}
			}
		}

		fileGroupList.push_back(group);
	}

	// Xaml files
	if (xamlFiles.size() > 0)
	{
		MSBuildFileGroup group;

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

				MSBuildFile groupFile;
				groupFile.TypeId = "ApplicationDefinition";
				groupFile.Path = file;
				group.Files.push_back(groupFile);
			}
			else
			{
				XmlNode& fileNode = xamlFileGroup.Node("Page")
					.Attribute("Include", "%s", file.c_str());

				fileNode
					.Node("SubType")
					.Value("Designer");

				MSBuildFile groupFile;
				groupFile.TypeId = "Page";
				groupFile.Path = file;
				group.Files.push_back(groupFile);
			}
		}

		fileGroupList.push_back(group);
	}

	// None files.
	if (miscFiles.size() > 0)
	{
		MSBuildFileGroup group;

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

					MSBuildFile groupFile;
					groupFile.TypeId = "AppxManifest";
					groupFile.Path = file;
					group.Files.push_back(groupFile);

					continue;
				}
			}

			miscFileGroup.Node("None")
				.Attribute("Include", "%s", file.c_str());

			MSBuildFile groupFile;
			groupFile.TypeId = "Include";
			groupFile.Path = file;
			group.Files.push_back(groupFile);
		}

		fileGroupList.push_back(group);
	}

	// Image files.
	if (imageFiles.size() > 0)
	{
		MSBuildFileGroup group;

		XmlNode& imageFileGroup = project.Node("ItemGroup");
		for (auto file : imageFiles)
		{
			imageFileGroup.Node("Image")
				.Attribute("Include", "%s", file.c_str());

			MSBuildFile groupFile;
			groupFile.TypeId = "Image";
			groupFile.Path = file;
			group.Files.push_back(groupFile);
		}

		fileGroupList.push_back(group);
	}

	// Source files
	if (sourceFiles.size() > 0)
	{
		MSBuildFileGroup group;

		XmlNode& sourceFileGroup = project.Node("ItemGroup");
		for (auto file : sourceFiles)
		{
			XmlNode& fileNode =
				sourceFileGroup.Node("ClCompile")
				.Attribute("Include", "%s", file.c_str());

			MSBuildFile groupFile;
			groupFile.TypeId = "ClCompile";
			groupFile.Path = file;
			group.Files.push_back(groupFile);

			if (file == precompiledSourceFile)
			{
				fileNode.Node("PrecompiledHeader").Value("Create");
			}

			if (projectFile.Get_Project_Subsystem() == EPlatformSubSystem::WinRT)
			{
				size_t xamlOffset = file.find(".xaml.");
				if (xamlOffset != std::string::npos)
				{
					fileNode.Node("DependentUpon")
						.Value("%s", file.substr(0, xamlOffset + 5).c_str());
				}
			}
		}

		fileGroupList.push_back(group);
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

	// Generate the filters file.
	MSBuild_VcxFiltersFile filtersFile(m_defaultToolsetString);

	if (!filtersFile.Generate(
		databaseFile,
		workspaceFile,
		projectFile,
		buildMatrix,
		fileGroupList
	))
	{
		return false;
	}

	return true;
}

}; // namespace MicroBuild