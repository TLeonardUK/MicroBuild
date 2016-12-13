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
#include "App/Ides/MSBuild/MSBuild_ProjectFile.h"
#include "App/Ides/MSBuild/MSBuild_VcxFiltersFile.h"
#include "Core/Helpers/TextStream.h"
#include "Core/Helpers/XmlNode.h"
#include "Core/Helpers/JsonNode.h"

namespace MicroBuild {

MSBuild_ProjectFile::MSBuild_ProjectFile(
	EPlatformToolset defaultToolset,
	std::string defaultToolsetString
)
	: m_defaultToolset(defaultToolset)
	, m_defaultToolsetString(defaultToolsetString)
{
}

MSBuild_ProjectFile::~MSBuild_ProjectFile()
{
}

bool MSBuild_ProjectFile::Generate(
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
	std::vector<std::string> allFiles;


	for (Platform::Path& path : projectFile.Get_Files_File())
	{
		std::string relativePath = "$(SolutionDir)" +
			solutionDirectory.RelativeTo(path).ToString();

		allFiles.push_back(relativePath);
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
		platformId = IdeHelper::GetPlatformHumanReadableId(matrix.platform);

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
	globals.Node("Keyword").Value("MakeFileProj");

	// Undocumented hackery going on here:
	// As we have our own custom platforms, and we are building them through makefiles we are just
	// going to silence the "platform not found" warnings visual studio gives. It seems preferable
	// to creating a bunch of unneccessary project property files and tampering with visual studio.
	globals.Node("PlatformTargetsFound").Value("True");
	globals.Node("PlatformPropsFound").Value("True");	 
	globals.Node("ToolsetTargetsFound").Value("True");
		
	// Imports
	project.Node("Import")
		.Attribute("Project", "$(VCTargetsPath)\\Microsoft.Cpp.Default.props");

	// Property Grid
	for (auto matrix : buildMatrix)
	{
		std::string platformId = MSBuild::GetPlatformID(matrix.platform);
		platformId = IdeHelper::GetPlatformHumanReadableId(matrix.platform);

		XmlNode& propertyGroup =
			project.Node("PropertyGroup")
			.Attribute("Condition", "'$(Configuration)|$(Platform)'=='%s|%s'", matrix.config.c_str(), platformId.c_str())
			.Attribute("Label", "Configuration");

		propertyGroup.Node("ConfigurationType").Value("Makefile");

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

		// Platform tooltype.
		switch (matrix.projectFile.Get_Build_PlatformToolset())
		{
		case EPlatformToolset::Default:
			propertyGroup.Node("PlatformToolset").Value("%s", CastToString(m_defaultToolset).c_str());
			break;
		case EPlatformToolset::MSBuild_v140:
			propertyGroup.Node("PlatformToolset").Value("v140");
			break;
		default:
			// Anny others will be dealt with on a per-platform basis, eg, AnyCPU etc.
			break;
		}
	}

	// Imports
	project.Node("Import")
		.Attribute("Project", "$(VCTargetsPath)\\Microsoft.Cpp.props");

	// Extension settings
	project.Node("ImportGroup")
		.Attribute("Label", "ExtensionSettings");

	// Shared settings
	project.Node("ImportGroup")
		.Attribute("Label", "Shared");

	// Property sheets
	XmlNode& propSheetsNode = 
		project.Node("ImportGroup")
		.Attribute("Label", "PropertySheets");

	propSheetsNode.Node("Import")
		.Attribute("Project", "$(UserRootDir)\\Microsoft.Cpp.Win32.user.props") // Forcing win32 because we don't care about platform specific config for this.
		.Attribute("Label", "LocalAppDataPlatform");
	
	// Macros
	project.Node("ImportGroup")
		.Attribute("Label", "UserMacros");

	// NMake property groups.
	for (auto matrix : buildMatrix)
	{
		std::string platformId = MSBuild::GetPlatformID(matrix.platform);
		platformId = IdeHelper::GetPlatformHumanReadableId(matrix.platform);

		Platform::Path outDir = matrix.projectFile.Get_Project_OutputDirectory();
		Platform::Path intDir = matrix.projectFile.Get_Project_IntermediateDirectory();

		Platform::Path outDirRelative = solutionDirectory.RelativeTo(outDir);
		Platform::Path intDirRelative = solutionDirectory.RelativeTo(intDir);

		std::vector<std::string> forcedIncludes;
		std::vector<std::string> includePaths;

		std::vector<std::string> defines = matrix.projectFile.Get_Defines_Define();

		XmlNode& propertyGroup =
			project.Node("PropertyGroup")
			.Attribute("Condition", "'$(Configuration)|$(Platform)'=='%s|%s'", matrix.config.c_str(), platformId.c_str());

		for (Platform::Path& path : matrix.projectFile.Get_ForcedIncludes_ForcedInclude())
		{
			forcedIncludes.push_back("$(SolutionDir)\\" + solutionDirectory.RelativeTo(path).ToString());
		}

		for (Platform::Path& path : matrix.projectFile.Get_SearchPaths_IncludeDirectory())
		{
			Platform::Path relativePath = solutionDirectory.RelativeTo(path).ToString();
			if (relativePath.IsRelative())
			{
				includePaths.push_back("$(SolutionDir)\\" + relativePath.ToString() + "\\");
			}
			else
			{
				includePaths.push_back(relativePath.ToString());
			}
		}

		Platform::Path relativeMicroBuildPath = solutionDirectory.RelativeTo(Platform::Path::GetExecutablePath());

		propertyGroup.Node("NMakeBuildCommandLine").Value("%s Build %s %s -c=%s -p=%s --silent", 
			Strings::Quoted("$(SolutionDir)\\" + relativeMicroBuildPath.ToString()).c_str(), 
			Strings::Quoted(workspaceFile.Get_Workspace_File().ToString()).c_str(),
			projectFile.Get_Project_Name().c_str(),
			matrix.config.c_str(),
			CastToString(matrix.platform).c_str()
		);
		propertyGroup.Node("NMakeOutput").Value("$(SolutionDir)%s\\%s%s", outDirRelative.ToString().c_str(), matrix.projectFile.Get_Project_OutputName().c_str(), matrix.projectFile.Get_Project_OutputExtension().c_str());
		propertyGroup.Node("NMakeCleanCommandLine").Value("%s Clean %s %s -c=%s -p=%s --silent",
			Strings::Quoted("$(SolutionDir)\\" + relativeMicroBuildPath.ToString()).c_str(), 
			Strings::Quoted(workspaceFile.Get_Workspace_File().ToString()).c_str(),
			projectFile.Get_Project_Name().c_str(),
			matrix.config.c_str(),
			CastToString(matrix.platform).c_str()
		);
		propertyGroup.Node("NMakeReBuildCommandLine").Value("%s Build %s %s -c=%s -p=%s -r --silent",
			Strings::Quoted("$(SolutionDir)\\" + relativeMicroBuildPath.ToString()).c_str(), 
			Strings::Quoted(workspaceFile.Get_Workspace_File().ToString()).c_str(),
			projectFile.Get_Project_Name().c_str(),
			matrix.config.c_str(),
			CastToString(matrix.platform).c_str()
		);
		propertyGroup.Node("NMakePreprocessorDefinitions").Value("%s;$(NMakePreprocessorDefinitions)", Strings::Join(defines, ";").c_str());
		propertyGroup.Node("NMakeIncludeSearchPath").Value("%s;$(NMakeIncludeSearchPath)", Strings::Join(includePaths, ";").c_str());
		propertyGroup.Node("NMakeForcedIncludes").Value("%s;$(NMakeForcedIncludes)", Strings::Join(forcedIncludes, ";").c_str());
	}

	// Empty item group (nmake project seems to generate this by default?).
	project.Node("ItemDefinitionGroup");

	// Create list of files.
	XmlNode& groupNode = 
		project.Node("ItemGroup");

	for (auto file : allFiles)
	{
		groupNode.Node("None")
			.Attribute("Include", "%s", file.c_str());
	}
		
	// We store which groups each file goes into so we can easily
	// generate a filters file.
	std::vector<MSBuildFileGroup> fileGroupList;

	MSBuildFileGroup group;
	for (auto file : allFiles)
	{
		MSBuildFile groupFile;
		groupFile.TypeId = "None";
		groupFile.Path = file;
		group.Files.push_back(groupFile);
	}
	fileGroupList.push_back(group);

	// Imports
	project.Node("Import")
		.Attribute("Project", "$(VCTargetsPath)\\Microsoft.Cpp.targets");

	// Extension settings
	project.Node("ImportGroup")
		.Attribute("Label", "ExtensionSettings");

	// Generate result.
	if (!databaseFile.StoreFile(
		workspaceFile,
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
