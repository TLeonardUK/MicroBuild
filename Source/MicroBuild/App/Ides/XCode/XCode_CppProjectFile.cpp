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
#include "App/Ides/XCode/XCode_CppProjectFile.h"
#include "Core/Helpers/TextStream.h"
#include "Core/Helpers/PlistNode.h"
#include "Core/Helpers/Strings.h"

namespace MicroBuild {

XCode_CppProjectFile::XCode_CppProjectFile()
{

}

XCode_CppProjectFile::~XCode_CppProjectFile()
{

}
/*
void XCode_CppProjectFile::Write_PBXBuildFile(
	PlistNode& root,
	const std::string& rootUuid,
	const std::vector<Platform::Path>& files
)
{
	for (Platform::Path& path : files)
	{
		std::string fileId = 
			Strings::Uuid(28, { rootUuid.ToString(), path.ToString(), "PBXBuildFile" });

		std::string fileRefId = 
			Strings::Uuid(28, { rootUuid.ToString(), path.ToString(), "PBXFileReference" });

		PlistNode& buildFile = 
			root.Node("%s", fileId.c_str());

		buildFile.Node("isa").Value("PBXBuildFile");		
		buildFile.Node("fileRef").Value("%s", fileRefId);
	}
}

void XCode_CppProjectFile::Write_PBXFileReference(
	PlistNode& root,
	const std::string& rootUuid,
	const std::vector<Platform::Path>& files,
	const Platform::Path& projectLocation
)
{
	for (Platform::Path& path : files)
	{	
		std::string fileRefId = 
			Strings::Uuid(28, { rootUuid.ToString(), path.ToString(), "PBXFileReference" });

		PlistNode& buildFile = 
			root.Node("%s", fileRefId.c_str());

		std::string relativePath = 
			projectLocation.RelativeTo(path).ToString();

		buildFile.Node("isa").Value("PBXFileReference");	
		buildFile.Node("lastKnownFileType").Value(FileTypeFromPath(path).c_str());
		buildFile.Node("name").Value("%s", path.GetBaseName().ToString().c_str());
		buildFile.Node("path").Value("%s", relativePath.ToString().c_str());
		buildFile.Node("sourceTree").Value("\"<group>\"");
	}

	std::string fileRefId = 
		Strings::Uuid(28, { rootUuid, "PBXFileReference", "BuildTarget" });

	Platform::Path outDir = matrix.projectFile.Get_Project_OutputDirectory();
	Platform::Path outDirRelative = projectLocation.RelativeTo(outDir);
	Platform::Path targetPath = 
		outDirRelative.AppendFragment(
		matrix.projectFile.Get_Project_OutputName() + 
		matrix.projectFile.Get_Project_OutputExtension(), true);

	buildFile.Node("isa").Value("PBXFileReference");	
	buildFile.Node("explicitFileType").Value("compiled.mach-o.executable");
	buildFile.Node("includeInIndex").Value("0");		
	buildFile.Node("name").Value("%s", targetPath.GetBaseName().ToString().c_str());
	buildFile.Node("path").Value("%s", targetPath.ToString().c_str());
	buildFile.Node("sourceTree").Value("\"<group>\"");
}

void XCode_CppProjectFile::Write_PBXGroup(
	PlistNode& root,
	const std::string& rootUuid,
	const std::vector<std::string>& filters,
	const std::string& rootGroupId
)
{
	for (std::string& filter : filters)
	{
		std::string groupId =
			Strings::Uuid(28, { rootUuid.ToString(), filter, "PBXGroup" });

		PlistNode& buildFile = 
			root.Node("%s", groupId.c_str());

		buildFile.Node("isa").Value("PBXGroup");

		PListNode& childrenNode = buildFile.Node("children");		

		// TODO: Add children

		buildFile.Node("name").Value("%s", filter.c_str());	
		buildFile.Node("sourceTree").Value("\"<group>\"");	
	}

	// Add root group
	{
		PlistNode& buildFile = 
			root.Node("%s", rootGroupId.c_str());

		buildFile.Node("isa").Value("PBXGroup");

		PListNode& childrenNode = buildFile.Node("children");		

		// TODO: Add children

		buildFile.Node("name").Value("Source");	
		buildFile.Node("sourceTree").Value("\"<group>\"");	
	}
}

void XCode_CppProjectFile::Write_PBXNativeTarget(
	PlistNode& root,
	const std::string& rootUuid,
	const std::string& id,
	const std::string& configListId
)
{
	PlistNode& targetNode = 
		root.Node("%s", id.c_str());

	targetNode.Node("isa").Value("PBXNativeTarget");
	targetNode.Node("buildConfigurationList").Value("%s", configListId.c_str()); 
	targetNode.Node("buildPhases"); // todo
	targetNode.Node("buildRules"); // todo
	targetNode.Node("dependencies"); // todo
	targetNode.Node("name").Value(""); // todo
	targetNode.Node("productInstallPath").Value(""); // todo
	targetNode.Node("productName").Value(""); // todo
	targetNode.Node("productReference").Value(""); // todo
	targetNode.Node("productType").Value(""); // todo
}

void XCode_CppProjectFile::Write_PBXProject(
	PlistNode& root,
	const std::string& rootUuid,
	const std::string& id,
	const std::string& configListId,
	const std::string& mainGroupId
)
{
	PlistNode& targetNode = 
		root.Node("%s", id.c_str());

	targetNode.Node("isa").Value("PBXProject");
	targetNode.Node("buildConfigurationList").Value("%s", configListId.c_str()); 
	targetNode.Node("compatibilityVersion").Value("\"Xcode 3.2\"");
	targetNode.Node("hasScannedForEncodings").Value("1"); 
	targetNode.Node("mainGroup").Value(mainGroupId); 
	targetNode.Node("projectDirPath").Value("\"\""); 
	targetNode.Node("projectRoot").Value("\"\""); 

	PlistNode& childNode = targetNode.Node("targets"); 

	std::string fileRefId = 
		Strings::Uuid(28, { rootUuid, "PBXFileReference", "BuildTarget" });

	childNode.Node("%s", fileRefId.c_str());
}

void XCode_CppProjectFile::Write_XCBuildConfigurationList_Project(
	PlistNode& root,
	const std::string& rootUuid, 
	const std::string& id,
	IdeHelper::BuildProjectMatrix& buildMatrix
)
{
	PlistNode& targetNode = 
		root.Node("%s", id.c_str());

	targetNode.Node("isa").Value("XCConfigurationList");
	targetNode.Node("defaultConfigurationIsVisible").Value("0"); 
	targetNode.Node("defaultConfigurationName").Value("%s_%s", configurations[0].c_str(), CastToString(platforms[0]).c_str()); 

	PlistNode& configNode = targetNode.Node("buildConfigurations"); 

	for (auto matrix : buildMatrix)
	{
		std::string platformId = (matrix.platform);

		std::string configId = 
			Strings::Uuid(28, { rootUuid, "PBXProject", "XCConfigurationList", matrix.config.c_str(), platformId.c_str() });

		configNode.Node("%s", configId.c_str());
	}	
}

void XCode_CppProjectFile::Write_XCBuildConfigurationList_Target(
	PlistNode& root,
	const std::string& rootUuid, 
	const std::string& id,
	IdeHelper::BuildProjectMatrix& buildMatrix
)
{
	PlistNode& targetNode = 
		root.Node("%s", targetConfigListId.c_str());

	targetNode.Node("isa").Value("XCConfigurationList");
	targetNode.Node("defaultConfigurationIsVisible").Value("0");
	targetNode.Node("defaultConfigurationName").Value("%s", configurations[0].c_str()); 

	PlistNode& configNode = targetNode.Node("buildConfigurations"); 
	for (auto matrix : buildMatrix)
	{
		std::string platformId = (matrix.platform);

		std::string configId = 
			Strings::Uuid(28, { rootUuid, "PBXProject", "XCConfigurationList", matrix.config.c_str(), platformId.c_str() });

		configNode.Node("%s", configId.c_str());
	}
}

void XCode_CppProjectFile::Write_XCBuildConfiguration(
	PlistNode& root,
	const std::string& rootUuid, 
	IdeHelper::BuildProjectMatrix& buildMatrix
)
{
	for (auto matrix : buildMatrix)
	{
		std::string platformId = (matrix.platform);

		std::string configId = 
			Strings::Uuid(28, { rootUuid, "PBXProject", "XCConfigurationList", matrix.config.c_str(), platformId.c_str() });

		PlistNode& targetNode = 
			root.Node("%s", configId.c_str());

		targetNode.Node("isa").Value("XCBuildConfiguration");
		
		PlistNode& settingsNode = 
			targetNode.Node("buildSettings"); 

		// TODO: Settings
		
		targetNode.Node("name").Value("%s_%s", matrix.config.c_str(), platformId.c_str());
	}
}

void XCode_CppProjectFile::Write_PBXHeadersBuildPhase(
	PlistNode& root,
	const std::string& rootUuid, 
	const std::string& id,
	std::vector<Platform::Path>& files,
)
{
	PlistNode& targetNode = 
		root.Node("%s", id.c_str());

	targetNode.Node("isa").Value("PBXHeadersBuildPhase");
	targetNode.Node("buildActionMask").value("2147483647");
	
	PlistNode& filesNode = targetNode.Node("files"); 

	// TODO: Precompiled headers.

	targetNode.Node("runOnlyForDeploymentPostprocessing").value(0);
}

void XCode_CppProjectFile::Write_PBXSourcesBuildPhase(
	PlistNode& root,
	const std::string& rootUuid, 
	const std::string& id,
	std::vector<Platform::Path>& files,
)
{
	PlistNode& targetNode = 
		root.Node("%s", id.c_str());

	targetNode.Node("isa").Value("PBXSourcesBuildPhase");
	targetNode.Node("buildActionMask").value("2147483647");
	
	PlistNode& filesNode = targetNode.Node("files"); 
	for (Platform::Path& path : projectFile.Get_Files_File())
	{
		if (path.IsSourceFile())
		{
			std::string fileId = 
				Strings::Uuid(28, { rootUuid.ToString(), path.ToString(), "PBXBuildFile" });
				
			filesNode.Node("%s", fileId.c_str());
		}
	}

	targetNode.Node("runOnlyForDeploymentPostprocessing").value(0);
}
*/
    
bool XCode_CppProjectFile::Generate(
	DatabaseFile& databaseFile,
	WorkspaceFile& workspaceFile,
	ProjectFile& projectFile,
	IdeHelper::BuildProjectMatrix& buildMatrix
)
{
    /*
	Platform::Path solutionDirectory =
		workspaceFile.Get_Workspace_Location();

	Platform::Path projectDirectory =
		projectFile.Get_Project_Location();

	Platform::Path projectLocation =
		projectDirectory
		.AppendFragment(projectFile.Get_Project_Name() + ".xcodeproj", true)
		.AppendFragment("project.pbxproj", true);

	std::vector<std::string> configurations =
		workspaceFile.Get_Configurations_Configuration();

	std::vector<EPlatform> platforms =
		workspaceFile.Get_Platforms_Platform();

	std::vector<Platform::Path> files = 
		projectFile.Get_Files_File();

	// Find common path.
	std::vector<ConfigFile::KeyValuePair> virtualPaths =
		projectFile.Get_VirtualPaths();

	std::vector<IdeHelper::VPathPair> vpathFilters =
		IdeHelper::ExpandVPaths(virtualPaths, files);

	// Node ID's
	std::string rootUuid = projectLocation.ToString();

	std::string targetId = 
		Strings::Uuid(28, { rootUuid, "PBXNativeTarget" });
	std::string projectId = 
		Strings::Uuid(14, { rootUuid, "PBXProject" });
	std::string headerBuildPhaseId = 
		Strings::Uuid(28, { rootUuid, "PBXHeadersBuildPhase" });
	std::string sourceBuildPhaseId = 
		Strings::Uuid(28, { rootUuid, "PBXSourcesBuildPhase" });
	std::string projectConfigListId = 
		Strings::Uuid(14, { rootUuid, "PBXProject", "XCConfigurationList" });
	std::string targetConfigListId = 
		Strings::Uuid(28, { rootUuid, "PBXNativeTarget", "XCConfigurationList" });
	std::string rootGroupId =
		Strings::Uuid(28, { rootUuid, "@Root", "PBXGroup" });

	// Add header.
	PlistNode root;
	root.Node("archiveVersion").Value("1");
	root.Node("classes");
	root.Node("objectVersion").Value("46");
	PlistNode& objectsNode = root.Node("objects");

	// PBXBuildFile Section
	Write_PBXBuildFile(objectsNode, rootUUID, files);

	// PBXFileReference Section
	Write_PBXFileReference(objectsNode, rootUUID, files, projectLocation);

	// PBXGroup Section
	Write_PBXGroup(objectsNode, rootUUID, filters, rootGroupId);

	// PBXNativeTarget Section
	Write_PBXNativeTarget(objectsNode, rootUUID, targetId, targetConfigListId);

	// PBXProject Section
	Write_PBXProject(objectsNode, rootUUID, projectId, projectConfigListId, rootGroupId);

	// PBXHeadersBuildPhase Section
	Write_PBXHeadersBuildPhase(objectsNode, rootUUID, headerBuildPhaseId, files);

	// PBXSourcesBuildPhase Section
	Write_PBXSourcesBuildPhase(objectsNode, rootUUID, sourceBuildPhaseId, files);

	// XCBuildConfiguration Section
	Write_XCBuildConfiguration(objectsNode, rootUUID, buildMatrix);

	// XCConfigurationList Section	
	Write_XCBuildConfigurationList_Project(objectsNode, rootUUID, projectConfigListId, buildMatrix);
	Write_XCBuildConfigurationList_Target(objectsNode, rootUUID, targetConfigListId, buildMatrix);

	// Root object value.
	root.Node("rootObject").Value("%s", projectId.c_str());

	// Generate result.
	if (!databaseFile.StoreFile(
		workspaceFile,
		projectDirectory,
		projectLocation,
		root.ToString().c_str()))
	{
		return false;
	}

	return false;*/
    
    UNUSED_PARAMETER(databaseFile);
    UNUSED_PARAMETER(workspaceFile);
    UNUSED_PARAMETER(projectFile);
    UNUSED_PARAMETER(buildMatrix);
    
    return true;
}

}; // namespace MicroBuild
