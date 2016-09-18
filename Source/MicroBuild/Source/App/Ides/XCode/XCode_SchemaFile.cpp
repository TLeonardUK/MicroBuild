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

// todo: project dependencies

#include "PCH.h"
#include "App/Ides/XCode/XCode_SchemaFile.h"
#include "Core/Helpers/TextStream.h"
#include "Core/Helpers/XmlNode.h"

namespace MicroBuild {

XCode_SchemaFile::XCode_SchemaFile()
{
}

XCode_SchemaFile::~XCode_SchemaFile()
{
}

bool XCode_SchemaFile::Generate(
	DatabaseFile& databaseFile,
	WorkspaceFile& workspaceFile,
	ProjectFile& projectFile,
    std::vector<ProjectFile>& projectFiles,
    IdeHelper::BuildProjectMatrix& buildMatrix
)
{
    MB_UNUSED_PARAMETER(buildMatrix);
    MB_UNUSED_PARAMETER(projectFiles);

	Platform::Path projectDirectory =
		projectFile.Get_Project_Location();

	Platform::Path projectBaseLocation =
		projectDirectory
		.AppendFragment(projectFile.Get_Project_Name() + ".xcodeproj", true);
    
    Platform::Path schemaDirectory =
		projectBaseLocation
		.AppendFragment("xcshareddata", true)
		.AppendFragment("xcschemes", true);
    
    Platform::Path schemaLocation =
		schemaDirectory
		.AppendFragment(projectFile.Get_Project_Name() + ".xcscheme", true);
    
	std::string rootUuid = projectBaseLocation.ToString();
	std::string targetId = 
		Strings::Uuid(24, { rootUuid, "PBXNativeTarget" });
    
	XmlNode root;

	// Header
	root.Node("?xml")
		.Attribute("version", "1.0")
		.Attribute("encoding", "UTF-8");

	XmlNode& scheme =
		root.Node("Scheme")
		.Attribute("LastUpgradeVersion", "0730")
		.Attribute("version", "1.3");
    
    // Build action.
    {
        XmlNode& action =
            scheme.Node("BuildAction")
            .Attribute("parallelizeBuildables", "YES")
            .Attribute("buildImplicitDependencies", "YES");

        XmlNode& buildActionEntries =
            action.Node("BuildActionEntries");

        XmlNode& buildActionEntry =
            buildActionEntries.Node("BuildActionEntry")
            .Attribute("buildForTesting", "YES")
            .Attribute("buildForRunning", "YES")
            .Attribute("buildForProfiling", "YES")
            .Attribute("buildForArchiving", "YES")
            .Attribute("buildForAnalyzing", "YES");

        buildActionEntry.Node("BuildableReference")
            .Attribute("BuildableIdentifier", "primary")
            .Attribute("BlueprintIdentifier", "%s", targetId.c_str())
            .Attribute("BuildableName", "%s", projectFile.Get_Project_Name().c_str())
            .Attribute("BlueprintName", "%s", projectFile.Get_Project_Name().c_str())
            .Attribute("ReferencedContainer", "container:%s.xcodeproj", projectFile.Get_Project_Name().c_str());
    }
    
    // Test action.
    {
        XmlNode& action =
            scheme.Node("TestAction")
            .Attribute("buildConfiguration", "Debug")
            .Attribute("selectedDebuggerIdentifier", "Xcode.DebuggerFoundation.Debugger.LLDB")
            .Attribute("selectedLauncherIdentifier", "Xcode.DebuggerFoundation.Launcher.LLDB")
            .Attribute("shouldUseLaunchSchemeArgsEnv", "YES");
        
        action.Node("Testables");
    
        XmlNode& macroExpanasion =
            action.Node("MacroExpansion");
        
        macroExpanasion.Node("BuildableReference")
            .Attribute("BuildableIdentifier", "primary")
            .Attribute("BlueprintIdentifier", "%s", targetId.c_str())
            .Attribute("BuildableName", "%s", projectFile.Get_Project_Name().c_str())
            .Attribute("BlueprintName", "%s", projectFile.Get_Project_Name().c_str())
            .Attribute("ReferencedContainer", "container:%s.xcodeproj", projectFile.Get_Project_Name().c_str());
        
        action.Node("AdditionalOptions");
    }
    
    // Launch action.
    {
        XmlNode& action =
            scheme.Node("LaunchAction")
            .Attribute("buildConfiguration", "Release")
            .Attribute("selectedDebuggerIdentifier", "Xcode.DebuggerFoundation.Debugger.LLDB")
            .Attribute("selectedLauncherIdentifier", "Xcode.DebuggerFoundation.Launcher.LLDB")
            .Attribute("launchStyle", "0")
            .Attribute("useCustomWorkingDirectory", "NO")
            .Attribute("ignoresPersistentStateOnLaunch", "NO")
            .Attribute("debugDocumentVersioning", "YES")
            .Attribute("debugServiceExtension", "internal")
            .Attribute("allowLocationSimulation", "YES");
            
        XmlNode& buildableProductRunnable =
            action.Node("BuildableProductRunnable")
            .Attribute("runnableDebuggingMode", "0");

        buildableProductRunnable.Node("BuildableReference")
            .Attribute("BuildableIdentifier", "primary")
            .Attribute("BlueprintIdentifier", "%s", targetId.c_str())
            .Attribute("BuildableName", "%s", projectFile.Get_Project_Name().c_str())
            .Attribute("BlueprintName", "%s", projectFile.Get_Project_Name().c_str())
            .Attribute("ReferencedContainer", "container:%s.xcodeproj", projectFile.Get_Project_Name().c_str());
        
        action.Node("CommandLineArguments");
        action.Node("AdditionalOptions");
    }
    
    // Profile action.
    {
        XmlNode& action =
            scheme.Node("ProfileAction")
            .Attribute("buildConfiguration", "Release")
            .Attribute("shouldUseLaunchSchemeArgsEnv", "YES")
            .Attribute("savedToolIdentifier", "")
            .Attribute("useCustomWorkingDirectory", "NO")
            .Attribute("debugDocumentVersioning", "YES");
        
        XmlNode& buildableProductRunnable =
            action.Node("BuildableProductRunnable")
            .Attribute("runnableDebuggingMode", "0");

        buildableProductRunnable.Node("BuildableReference")
            .Attribute("BuildableIdentifier", "primary")
            .Attribute("BlueprintIdentifier", "%s", targetId.c_str())
            .Attribute("BuildableName", "%s", projectFile.Get_Project_Name().c_str())
            .Attribute("BlueprintName", "%s", projectFile.Get_Project_Name().c_str())
            .Attribute("ReferencedContainer", "container:%s.xcodeproj", projectFile.Get_Project_Name().c_str());
    }
    
    // Analyse Action.
    {
        scheme.Node("AnalyzeAction")
            .Attribute("buildConfiguration", "Debug");
    }
    
    // Archive Action.
    {
        scheme.Node("ArchiveAction")
            .Attribute("buildConfiguration", "Release")
            .Attribute("revealArchiveInOrganizer", "YES");
    }
    
	// Generate result.
	if (!databaseFile.StoreFile(
		workspaceFile,
		schemaLocation,
		root.ToString().c_str()))
	{
		return false;
	}

	return true;
}

}; // namespace MicroBuild
