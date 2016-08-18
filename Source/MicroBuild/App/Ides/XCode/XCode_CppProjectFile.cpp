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

// todo:
//  c# projects
//  other sdk targets (ios etc)
//  link through framework link not linker args
//  workspace virtual paths
//  clean this whole thing up

#include "PCH.h"
#include "App/Ides/XCode/XCode_CppProjectFile.h"
#include "Core/Helpers/TextStream.h"
#include "Core/Helpers/PlistNode.h"
#include "Core/Helpers/Strings.h"

#include <map>

namespace MicroBuild {

XCode_CppProjectFile::XCode_CppProjectFile()
{

}

XCode_CppProjectFile::~XCode_CppProjectFile()
{

}
    
std::string XCode_CppProjectFile::FileTypeFromPath(
	const Platform::Path& path
)
{
    std::string ext = Strings::ToLowercase(path.GetExtension());
    
    std::map<std::string, std::string> map;
    
    map["c"] =             "sourcecode.c.c";
    map["cc"] =            "sourcecode.cpp.cpp";
    map["cpp"] =           "sourcecode.cpp.cpp";
    map["cxx"] =           "sourcecode.cpp.cpp";
    map["c++"] =           "sourcecode.cpp.cpp";
    map["asm"] =           "sourcecode.asm.asm";
    map["s"] =             "sourcecode.asm.asm";
    map["m"] =             "sourcecode.c.objc";
    map["mm"] =            "sourcecode.cpp.objc";
    map["h"] =             "sourcecode.c.h";
    map["hpp"] =           "sourcecode.c.h";
    map["hxx"] =           "sourcecode.c.h";
    map["nib"] =           "wrapper.nib";
    map["framework"] =     "wrapper.framework";
    map["html"] =          "text.html";
    map["plist"] =         "text.plist.xml";
    map["strings"] =       "text.plist.strings";
    map["xib"] =           "file.xib";
    map["storyboard"] =    "file.storyboard";
    map["xcassets"] =      "folder.assetcatalog";
    map["icns"] =          "image.icns";
    map["gif"] =           "image.gif";
    map["png"] =           "image.png";
    map["bmp"] =           "image.bmp";
    map["jpeg"] =          "image.jpeg";
    map["jpg"] =           "image.jpeg";
    map["wav"] =           "audio.wav";

    auto iter = map.find(ext);
    if (iter != map.end())
    {
        return iter->second;
    }
    else
    {
        return "text";
    }
}

std::string XCode_CppProjectFile::FileTypeFromOutput(
    const EOutputType& type
)
{
    switch (type)
    {
        case EOutputType::ConsoleApp:
            return "compiled.mach-o.executable";
        case EOutputType::Executable:
            return "wrapper.application";
        case EOutputType::DynamicLib:
            return "compiled.mach-o.dylib";
        case EOutputType::StaticLib:
            // Fallthrough
        default:
            return "archive.ar";
    }
}

std::string XCode_CppProjectFile::ProductTypeFromOutput(
    const EOutputType& type
)
{
    switch (type)
    {
        case EOutputType::ConsoleApp:
            return "com.apple.product-type.tool";
        case EOutputType::Executable:
            return "com.apple.product-type.application";
        case EOutputType::DynamicLib:
            return "com.apple.product-type.library.dynamic";
        case EOutputType::StaticLib:
            // Fallthrough
        default:
            return "com.apple.product-type.library.static";
    }
}

void XCode_CppProjectFile::Write_PBXBuildFile(
	PlistNode& root,
	const std::string& rootUuid,
    const std::map<std::string, std::string>& filterMap
)
{
	for (auto file : filterMap)
	{
		std::string fileId = 
			Strings::Uuid(24, { rootUuid, file.first, "PBXBuildFile" });

		std::string fileRefId = 
			Strings::Uuid(24, { rootUuid, file.first, "PBXFileReference" });

		PlistNode& buildFile = 
			root.Node("%s", fileId.c_str());

		buildFile.Node("isa").Value("PBXBuildFile");		
		buildFile.Node("fileRef").Value("%s", fileRefId.c_str());
	}
}
    
void XCode_CppProjectFile::Write_PBXFileReference(
	PlistNode& root,
	const std::string& rootUuid,
    const std::map<std::string, std::string>& filterMap,
	const Platform::Path& projectLocation,
    const std::string& projectName,
    const EOutputType& outputType,
    std::vector<ProjectFile*>& dependencies
)
{
    // Add container item.
    for (auto project : dependencies)
    {
		std::string fileRefId =
			Strings::Uuid(24, { rootUuid, "PBXFileReference", "Dependency", project->Get_Project_Name() });
        
		std::string proxyId =
			Strings::Uuid(24, { rootUuid, "PBXContainerItemProxy", "Project", project->Get_Project_Name() });
        
		std::string targetId =
			Strings::Uuid(24, { rootUuid, "PBXContainerItemProxy", "Target", project->Get_Project_Name() });
        
		std::string proxyDependenyId =
			Strings::Uuid(24, { rootUuid, "PBXTargetDependency", "Project", project->Get_Project_Name() });
        
		std::string targetDependencyId =
			Strings::Uuid(24, { rootUuid, "PBXTargetDependency", "Target", project->Get_Project_Name() });
        
        Platform::Path projectDirectory =
            project->Get_Project_Location();

        Platform::Path projectBaseLocation =
            projectDirectory
            .AppendFragment(project->Get_Project_Name() + ".xcodeproj", true);
        
        std::string projectRootUuid = projectBaseLocation.ToString();
        std::string remoteProjectId = Strings::Uuid(24, { projectRootUuid, "PBXProject" });
        std::string remoteTargetId = Strings::Uuid(24, { projectRootUuid, "PBXNativeTarget" });
        
        {
            PlistNode& buildFile = root.Node("%s", proxyId.c_str());
            buildFile.Node("isa").Value("PBXContainerItemProxy");
            buildFile.Node("containerPortal").Value("%s", fileRefId.c_str());
            buildFile.Node("proxyType").Value("2");
            buildFile.Node("remoteGlobalIDString").Value("%s", remoteProjectId.c_str());
            buildFile.Node("remoteInfo").Value("%s", project->Get_Project_Name().c_str());
        }
        
        {
            PlistNode& buildFile = root.Node("%s", targetId.c_str());
            buildFile.Node("isa").Value("PBXContainerItemProxy");
            buildFile.Node("containerPortal").Value("%s", fileRefId.c_str());
            buildFile.Node("proxyType").Value("1");
            buildFile.Node("remoteGlobalIDString").Value("%s", remoteTargetId.c_str());
            buildFile.Node("remoteInfo").Value("%s", project->Get_Project_Name().c_str());
        }
        
        {
            PlistNode& buildFile = root.Node("%s", proxyDependenyId.c_str());
            buildFile.Node("isa").Value("PBXTargetDependency");
            buildFile.Node("name").Value("%s", project->Get_Project_Name().c_str());
            buildFile.Node("targetProxy").Value("%s", proxyId.c_str());
        }
        
        {
            PlistNode& buildFile = root.Node("%s", targetDependencyId.c_str());
            buildFile.Node("isa").Value("PBXTargetDependency");
            buildFile.Node("name").Value("%s", project->Get_Project_Name().c_str());
            buildFile.Node("targetProxy").Value("%s", targetId.c_str());
        }
    }
    
    // Add dependency file references.
    for (auto project : dependencies)
    {
		std::string fileRefId =
			Strings::Uuid(24, { rootUuid, "PBXFileReference", "Dependency", project->Get_Project_Name() });
        
        Platform::Path projectDirectory =
            project->Get_Project_Location();

        Platform::Path projectBaseLocation =
            projectDirectory
            .AppendFragment(project->Get_Project_Name() + ".xcodeproj", true);
        
        Platform::Path projectRelativeLocation =
            projectLocation.RelativeTo(projectBaseLocation);
        
        PlistNode& buildFile = root.Node("%s", fileRefId.c_str());
        buildFile.Node("isa").Value("PBXFileReference");
        buildFile.Node("lastKnownFileType").Value("\"wrapper.pb-project\"");
        buildFile.Node("name").Value("%s.xcodeproj", project->Get_Project_Name().c_str());
        buildFile.Node("path").Value("%s", projectRelativeLocation.ToString().c_str());
        buildFile.Node("sourceTree").Value("SOURCE_ROOT");
    }
    
    // Add target file reference.
	{
		std::string fileRefId =
			Strings::Uuid(24, { rootUuid, "PBXFileReference", "BuildTarget", projectName });
        
		PlistNode& buildFile = 
			root.Node("%s", fileRefId.c_str());

        buildFile.Node("isa").Value("PBXFileReference");
        buildFile.Node("explicitFileType").Value("\"%s\"", FileTypeFromOutput(outputType).c_str());
        buildFile.Node("includeInIndex").Value("0");
        buildFile.Node("path").Value("%s", projectName.c_str());
        buildFile.Node("sourceTree").Value("BUILT_PRODUCTS_DIR");
	}

    // Add all project file references.
	for (auto file : filterMap)
	{	
		std::string fileRefId = 
			Strings::Uuid(24, { rootUuid, file.first, "PBXFileReference" });

		PlistNode& buildFile = 
			root.Node("%s", fileRefId.c_str());

        Platform::Path path = file.first;

		buildFile.Node("isa").Value("PBXFileReference");	
		buildFile.Node("lastKnownFileType").Value(FileTypeFromPath(path).c_str());
		buildFile.Node("name").Value("%s", path.GetFilename().c_str());
		buildFile.Node("path").Value("%s", path.ToString().c_str());
		buildFile.Node("sourceTree").Value("SOURCE_ROOT");
	}
}
    
void XCode_CppProjectFile::Write_PBXGroup(
	PlistNode& root,
	const std::string& rootUuid,
	const std::vector<std::string>& filters,
    const std::map<std::string, std::string>& filterMap,
	const std::string& rootGroupId,
    const std::string& rootName,
    const std::string& projectName,
    std::vector<ProjectFile*>& dependencies
)
{
    bool bAddedRoot = false;

    std::string targetId =
        Strings::Uuid(24, { rootUuid, "PBXFileReference", "BuildTarget", projectName });
    
    PlistNode* rootChildNode = nullptr;
    
	for (const std::string& filter : filters)
	{
		std::string groupId =
			Strings::Uuid(24, { rootUuid, filter, "PBXGroup" });

        if (filter.empty())
        {
            groupId = rootGroupId;
        }

		PlistNode& buildFile = 
			root.Node("%s", groupId.c_str());

		buildFile.Node("isa").Value("PBXGroup");

		PlistNode& childrenNode = buildFile.Array("children");
        
        // Add sub-groups
        for (const std::string& subFilter : filters)
        {
            Platform::Path filterPath = filter;
            Platform::Path subFilterPath = subFilter;
            
            if (subFilterPath.GetDirectory().ToString() == filterPath.ToString())
            {
                std::string fileRefId =
                    Strings::Uuid(24, { rootUuid, subFilter, "PBXGroup" });
                
                childrenNode.Node("").Value("%s", fileRefId.c_str());
            }
        }
        
        // Add files.
        for (auto file : filterMap)
		{
            if (file.second == filter)
            {
                std::string fileRefId =
                    Strings::Uuid(24, { rootUuid, file.first, "PBXFileReference" });
                
                childrenNode.Node("").Value("%s", fileRefId.c_str());
            }
        }
        
        Platform::Path filterPath = filter;
        if (filter.empty())
        {
            buildFile.Node("name").Value("%s", rootName.c_str());
            rootChildNode = &childrenNode;
            bAddedRoot = true;
		}
        else
        {
            buildFile.Node("name").Value("%s", filterPath.GetFilename().c_str());
        }
        
        buildFile.Node("sourceTree").Value("\"<group>\"");
	}

	// Add root group
    if (!bAddedRoot)
	{
		PlistNode& buildFile = 
			root.Node("%s", rootGroupId.c_str());

		buildFile.Node("isa").Value("PBXGroup");

		PlistNode& childrenNode = buildFile.Array("children");
        
        // Add sub-groups
        for (const std::string& subFilter : filters)
        {
            if (subFilter.find("\\") == std::string::npos &&
                subFilter.find("/") == std::string::npos)
            {
                std::string fileRefId =
                    Strings::Uuid(24, { rootUuid, subFilter, "PBXGroup" });
                
                childrenNode.Node("").Value("%s", fileRefId.c_str());
            }
        }
        
        // Add files
        for (auto file : filterMap)
		{
            if (file.second.find("\\") == std::string::npos &&
                file.second.find("/") == std::string::npos)
            {
                std::string fileRefId =
                    Strings::Uuid(24, { rootUuid, file.first, "PBXFileReference" });
                
                childrenNode.Node("").Value("%s", fileRefId.c_str());
            }
        }
        
        rootChildNode = &childrenNode;
        
		buildFile.Node("name").Value("%s", rootName.c_str());
		buildFile.Node("sourceTree").Value("SOURCE_ROOT");
	}
    
    // Add dependencies to root.
    rootChildNode->Node("").Value("%s", targetId.c_str());
    
    for (auto project : dependencies)
    {
		std::string fileRefId =
			Strings::Uuid(24, { rootUuid, "PBXReferenceProxy", "Dependency", project->Get_Project_Name() });

        rootChildNode->Node("").Value("%s", fileRefId.c_str());
    }
}

void XCode_CppProjectFile::Write_PBXProject(
	PlistNode& root,
	const std::string& rootUuid,
	const std::string& id,
	const std::string& configListId,
	const std::string& mainGroupId,
    std::vector<ProjectFile*>& dependencies
)
{
	PlistNode& targetNode = 
		root.Node("%s", id.c_str());

	targetNode.Node("isa").Value("PBXProject");
	targetNode.Node("buildConfigurationList").Value("%s", configListId.c_str()); 
	targetNode.Node("compatibilityVersion").Value("\"Xcode 3.2\"");
	targetNode.Node("hasScannedForEncodings").Value("1"); 
	targetNode.Node("mainGroup").Value("%s", mainGroupId.c_str());
    targetNode.Node("productRefGroup").Value("%s", mainGroupId.c_str());
	targetNode.Node("projectDirPath").Value("\"\""); 
	targetNode.Node("projectRoot").Value("\"\""); 

    if (dependencies.size() > 0)
    {
        PlistNode& referencesNode =
            targetNode.Array("projectReferences");
    
        for (auto project : dependencies)
        {
            PlistNode& childNode =
                referencesNode.Dict("");
    
            std::string fileRefId =
                Strings::Uuid(24, { rootUuid, "PBXFileReference", "Dependency", project->Get_Project_Name() });
        
            std::string rootGroupId =
                Strings::Uuid(24, { rootUuid, "", "PBXGroup" });

            childNode.Node("ProductGroup").Value("%s", rootGroupId.c_str());
            childNode.Node("ProductRef").Value("%s", fileRefId.c_str());
        }
    }

	PlistNode& childNode = targetNode.Array("targets"); 

	std::string targetId = 
		Strings::Uuid(24, { rootUuid, "PBXNativeTarget" });

	childNode.Node("").Value("%s", targetId.c_str());
}

void XCode_CppProjectFile::Write_PBXReferenceProxy(
	PlistNode& root,
	const std::string& rootUuid,
    std::vector<ProjectFile*>& dependencies
)
{
    for (auto project : dependencies)
    {
		std::string fileRefId =
			Strings::Uuid(24, { rootUuid, "PBXReferenceProxy", "Dependency", project->Get_Project_Name() });
        
		std::string proxyId =
			Strings::Uuid(24, { rootUuid, "PBXContainerItemProxy", "Project", project->Get_Project_Name() });
        
        PlistNode& targetNode =
            root.Node("%s", fileRefId.c_str());
        
        targetNode.Node("isa").Value("PBXReferenceProxy");
        targetNode.Node("fileType").Value("\"%s\"", FileTypeFromOutput(project->Get_Project_OutputType()).c_str());
        targetNode.Node("name").Value("%s", project->Get_Project_Name().c_str());
        targetNode.Node("path").Value("%s", project->Get_Project_Name().c_str());
        targetNode.Node("remoteRef").Value("%s", proxyId.c_str());
        targetNode.Node("sourceTree").Value("BUILD_PRODUCTS_DIR");
    }
}

void XCode_CppProjectFile::Write_PBXNativeTarget(
	PlistNode& root,
	const std::string& rootUuid,
	const std::string& id,
	const std::string& configListId,
    const std::string& projectName,
    const EOutputType& outputType,
    const std::string& sourcePhaseId,
    const std::string& frameworksPhaseId,
    const std::string& resourcesPhaseId,
    std::vector<ProjectFile*>& dependencies
)
{
    std::string configId =
        Strings::Uuid(24, { rootUuid, "PBXFileReference", "BuildTarget", projectName });
    
	PlistNode& targetNode = 
		root.Node("%s", id.c_str());

	targetNode.Node("isa").Value("PBXNativeTarget");
	targetNode.Node("buildConfigurationList").Value("%s", configListId.c_str()); 
	
    PlistNode& buildPhaseNode =
        targetNode.Array("buildPhases");
    
    buildPhaseNode.Node("").Value("%s", resourcesPhaseId.c_str());
    buildPhaseNode.Node("").Value("%s", sourcePhaseId.c_str());
    buildPhaseNode.Node("").Value("%s", frameworksPhaseId.c_str());
    
	targetNode.Array("buildRules");
    
    if (dependencies.size() > 0)
    {
        PlistNode& dependencyNode =
            targetNode.Array("dependencies");
    
        for (auto project : dependencies)
        {
            std::string targetDependencyId =
                Strings::Uuid(24, { rootUuid, "PBXTargetDependency", "Target", project->Get_Project_Name() });
        
            dependencyNode.Node("").Value("%s", targetDependencyId.c_str());
        }
    }
    
	targetNode.Node("name").Value("%s", projectName.c_str());
	targetNode.Node("productName").Value("%s", projectName.c_str());
	targetNode.Node("productReference").Value("%s", configId.c_str());
	targetNode.Node("productType").Value("\"%s\"", ProductTypeFromOutput(outputType).c_str());
}

void XCode_CppProjectFile::Write_XCBuildConfigurationList_Project(
	PlistNode& root,
	const std::string& rootUuid, 
	const std::string& id,
	IdeHelper::BuildProjectMatrix& buildMatrix,
    const std::vector<EPlatform>& platforms,
    const std::vector<std::string>& configurations
)
{
	PlistNode& targetNode = 
		root.Node("%s", id.c_str());

	targetNode.Node("isa").Value("XCConfigurationList");

	PlistNode& configNode = targetNode.Array("buildConfigurations"); 

	for (auto matrix : buildMatrix)
	{
		std::string platformId = CastToString(matrix.platform);

		std::string configId = 
			Strings::Uuid(24, { rootUuid, "PBXProject", "XCConfigurationList", matrix.config.c_str(), platformId.c_str() });

		configNode.Node("").Value("%s", configId.c_str());
	}	

    targetNode.Node("defaultConfigurationIsVisible").Value("0"); 
    targetNode.Node("defaultConfigurationName").Value("%s_%s", configurations[0].c_str(), CastToString(platforms[0]).c_str()); 
}

void XCode_CppProjectFile::Write_XCBuildConfigurationList_Target(
	PlistNode& root,
	const std::string& rootUuid, 
	const std::string& id,
	IdeHelper::BuildProjectMatrix& buildMatrix,
    const std::vector<EPlatform>& platforms,
    const std::vector<std::string>& configurations
)
{
	PlistNode& targetNode = 
		root.Node("%s", id.c_str());

	targetNode.Node("isa").Value("XCConfigurationList");

	PlistNode& configNode = targetNode.Array("buildConfigurations"); 
	for (auto matrix : buildMatrix)
	{
		std::string platformId = CastToString(matrix.platform);

		std::string configId = 
			Strings::Uuid(24, { rootUuid, "PBXProject", "XCConfigurationList", "BuildTarget", matrix.config.c_str(), platformId.c_str() });

		configNode.Node("").Value("%s", configId.c_str());
	}

    targetNode.Node("defaultConfigurationIsVisible").Value("0");
    targetNode.Node("defaultConfigurationName").Value("%s_%s", configurations[0].c_str(), CastToString(platforms[0]).c_str());
}

bool XCode_CppProjectFile::Write_XCBuildConfiguration(
	PlistNode& root,
	const std::string& rootUuid, 
	IdeHelper::BuildProjectMatrix& buildMatrix,
    const Platform::Path& projectBaseDirectory
)
{
    // Write the project config list.
	for (auto matrix : buildMatrix)
	{
		std::string platformId =
            CastToString(matrix.platform);

		std::string targetId =
			Strings::Uuid(24, { rootUuid, "PBXProject", "XCConfigurationList", matrix.config.c_str(), platformId.c_str() });

		PlistNode& targetNode =
			root.Node("%s", targetId.c_str());

		targetNode.Node("isa").Value("XCBuildConfiguration");

		PlistNode& settingsNode = 
			targetNode.Dict("buildSettings");
        
        settingsNode.Node("ALWAYS_SEARCH_USER_PATHS").Value("NO");
        settingsNode.Node("CLANG_ANALYZER_NONNULL").Value("YES");
        settingsNode.Node("CLANG_CXX_LIBRARY").Value("\"libc++\"");
        settingsNode.Node("CLANG_ENABLE_MODULES").Value("YES");
        settingsNode.Node("CLANG_ENABLE_OBJ_ARC").Value("YES");
        settingsNode.Node("CODE_SIGN_IDENTITY").Value("\"-\"");
        settingsNode.Node("COPY_PHASE_STRIP").Value("NO");
        settingsNode.Node("ENABLE_NS_ASSERTIONS").Value("NO");
        settingsNode.Node("ENABLE_STRICT_OBJC_MSGSEND").Value("YES");
        settingsNode.Node("GCC_C_LANGUAGE_STANDARD").Value("gnu99");
        settingsNode.Node("GCC_NO_COMMON_BLOCKS").Value("YES");
        settingsNode.Node("MTL_ENABLE_DEBUG_INFO").Value("NO");

        // Debug database.
        if (matrix.projectFile.Get_Flags_GenerateDebugInformation())
        {
            settingsNode.Node("DEBUG_INFORMATION_FORMAT").Value("\"dwarf-with-dsym\"");
            settingsNode.Node("GCC_GENERATE_DEBUGGING_SYMBOLS").Value("YES");
        }
        else
        {
            settingsNode.Node("DEBUG_INFORMATION_FORMAT").Value("\"dwarf\"");
            settingsNode.Node("GCC_GENERATE_DEBUGGING_SYMBOLS").Value("NO");
        }

		// C++ standard
		switch (matrix.projectFile.Get_Project_LanguageVersion())
		{
		case ELanguageVersion::Default:
			break;
		case ELanguageVersion::Cpp_11:
			settingsNode.Node("CLANG_CXX_LANGUAGE_STANDARD").Value("\"c++0x\"");
			break;
		case ELanguageVersion::Cpp_98:
			settingsNode.Node("CLANG_CXX_LANGUAGE_STANDARD").Value("\"c++98\"");
			break;
		case ELanguageVersion::Cpp_14:
			settingsNode.Node("CLANG_CXX_LANGUAGE_STANDARD").Value("\"c++14\"");
			break;
		default:
			matrix.projectFile.ValidateError(
				"Language standard '%s' is not valid for xcode C++ projects.",
				CastToString(matrix.projectFile.Get_Project_LanguageVersion()).c_str());
			return false;
		}

		// LTO
		if (matrix.projectFile.Get_Build_OptimizationLevel() == EOptimizationLevel::None ||
			matrix.projectFile.Get_Build_OptimizationLevel() == EOptimizationLevel::Debug)
		{
			settingsNode.Node("LLVM_LTO").Value("NO");
		}
		else
		{
			settingsNode.Node("LLVM_LTO").Value("YES");
		}

        // Search paths.
		Platform::Path outDir = matrix.projectFile.Get_Project_OutputDirectory();
		Platform::Path intDir = matrix.projectFile.Get_Project_IntermediateDirectory();

		Platform::Path outDirRelative = projectBaseDirectory.RelativeTo(outDir);
		Platform::Path intDirRelative = projectBaseDirectory.RelativeTo(intDir);

		std::vector<std::string> includePaths;
		std::vector<std::string> libraryPaths;

		// Output information.
		settingsNode.Node("SYMROOT").Value("%s", outDirRelative.ToString().c_str());
		settingsNode.Node("OBJROOT").Value("%s", intDirRelative.ToString().c_str());
		settingsNode.Node("CONFIGURATION_BUILD_DIR").Value("%s", outDirRelative.ToString().c_str());
		settingsNode.Node("CONFIGURATION_TEMP_DIR").Value("%s", intDirRelative.ToString().c_str());
		settingsNode.Node("PRODUCT_NAME").Value("%s",
            matrix.projectFile.Get_Project_OutputName().c_str()
            );
		settingsNode.Node("EXECUTABLE_PREFIX").Value("\"\"");
        
        
        std::string ext = matrix.projectFile.Get_Project_OutputExtension();
        if (ext.length() > 0 && ext[0] == '.')
        {
            ext = ext.substr(1);
        }
		settingsNode.Node("EXECUTABLE_EXTENSION").Value("\"%s\"", ext.c_str());
        
		// Search paths.
        if (matrix.projectFile.Get_SearchPaths_IncludeDirectory().size() > 0)
        {
            PlistNode& headersDirNode = settingsNode.Array("HEADER_SEARCH_PATHS");
            for (Platform::Path& path : matrix.projectFile.Get_SearchPaths_IncludeDirectory())
            {
                headersDirNode.Node("").Value("%s", projectBaseDirectory.RelativeTo(path).ToString().c_str());
            }
        }

        if (matrix.projectFile.Get_SearchPaths_LibraryDirectory().size() > 0)
        {
            PlistNode& librariesDirNode = settingsNode.Array("LIBRARY_SEARCH_PATHS");
            for (Platform::Path& path : matrix.projectFile.Get_SearchPaths_LibraryDirectory())
            {
                librariesDirNode.Node("").Value("%s", projectBaseDirectory.RelativeTo(path).ToString().c_str());
            }
        }
    
        // Defines
        if (matrix.projectFile.Get_Defines_Define().size() > 0)
        {
            PlistNode& definesNode = settingsNode.Array("GCC_PREPROCESSOR_DEFINITIONS");
            std::vector<std::string> defines = matrix.projectFile.Get_Defines_Define();
            for (std::string& define : defines)
            {
                definesNode.Node("").Value("\"%s\"", define.c_str());
            }
            definesNode.Node("").Value("\"$(inherited)\"");
        }

        // RTTI
        settingsNode.Node("GCC_ENABLE_CPP_RTTI").Value(matrix.projectFile.Get_Flags_RuntimeTypeInfo() ? "YES" : "NO");
        settingsNode.Node("GCC_ENABLE_CPP_EXCEPTIONS").Value(matrix.projectFile.Get_Flags_Exceptions() ? "YES" : "NO");

        // Warning level.
        switch (matrix.projectFile.Get_Build_WarningLevel())
        {
        case EWarningLevel::None:
            break;
        case EWarningLevel::Default:
            // Fallthrough
        case EWarningLevel::Low:
            // Fallthrough
        case EWarningLevel::Medium:
            // Fallthrough
        case EWarningLevel::High:
            // Fallthrough
        case EWarningLevel::Verbose:
            settingsNode.Node("CLANG_WARN_BOOL_CONVERSION").Value("YES");
            settingsNode.Node("CLANG_WARN_CONSTANT_CONVERSION").Value("YES");
            settingsNode.Node("CLANG_WARN_DIRECT_OBJC_ISA_USAGE").Value("YES_ERROR");
            settingsNode.Node("CLANG_WARN_EMPTY_BODY").Value("YES");
            settingsNode.Node("CLANG_WARN_ENUM_CONVERSION").Value("YES");
            settingsNode.Node("CLANG_WARN_INT_CONVERSION").Value("YES");
            settingsNode.Node("CLANG_WARN_OBJC_ROOT_CLASS").Value("YES_ERROR");
            settingsNode.Node("CLANG_WARN_UNREACHABLE_CODE").Value("YES");
            settingsNode.Node("CLANG_WARN__DUPLICATE_METHOD_MATCH").Value("YES");
            settingsNode.Node("GCC_WARN_64_TO_32_BIT_CONVERSION").Value("YES");
            settingsNode.Node("GCC_WARN_ABOUT_RETURN_TYPE").Value("YES_ERROR");
            settingsNode.Node("GCC_WARN_UNDECLARED_SELECTOR").Value("YES");
            settingsNode.Node("GCC_WARN_UNINITIALIZED_AUTOS").Value("YES_AGGRESSIVE");
            settingsNode.Node("GCC_WARN_UNUSED_FUNCTION").Value("YES");
            settingsNode.Node("GCC_WARN_UNUSED_VARIABLE").Value("YES");
            break;
        default:
            matrix.projectFile.ValidateError(
                "Warning level '%s' is not valid for xcode C++ projects.",
                CastToString(matrix.projectFile.Get_Build_WarningLevel()).c_str());
            return false;
        }
        
        // Warnings-as-errors
        settingsNode.Node("GCC_TREAT_WARNINGS_AS_ERRORS").Value(matrix.projectFile.Get_Flags_CompilerWarningsFatal() ? "YES" : "NO");

        // Disabled warnings.
        std::vector<std::string> disabledWarnings =
            matrix.projectFile.Get_DisabledWarnings_DisabledWarning();

        for (std::string& id : disabledWarnings)
        {
            settingsNode.Node("%s", id.c_str()).Value("NO");
        }

        // Optimization.
        switch (matrix.projectFile.Get_Build_OptimizationLevel())
        {
        case EOptimizationLevel::None:
            // Fallthrough
        case EOptimizationLevel::Debug:
            settingsNode.Node("GCC_OPTIMIZATION_LEVEL").Value("0");
            break;
        case EOptimizationLevel::PreferSize:
            settingsNode.Node("GCC_OPTIMIZATION_LEVEL").Value("S");
            break;
        case EOptimizationLevel::PreferSpeed:
            settingsNode.Node("GCC_OPTIMIZATION_LEVEL").Value("3");
            break;
        case EOptimizationLevel::Full:
            settingsNode.Node("GCC_OPTIMIZATION_LEVEL").Value("fast");
            break;
        default:
            matrix.projectFile.ValidateError(
                "Optimization level '%s' is not valid for xcode C++ projects.",
                CastToString(matrix.projectFile.Get_Build_OptimizationLevel()).c_str());
            return false;
        }

        // Additional options.
        std::string compilerArguments = "";
        std::string linkerArguments = "";
        
		for (Platform::Path& path : matrix.projectFile.Get_ForcedIncludes_ForcedInclude())
		{
            compilerArguments += " -include " + Strings::Quoted(projectBaseDirectory.RelativeTo(path).ToString());
		}
        
		for (Platform::Path& path : matrix.projectFile.Get_Libraries_Library())
		{
			if (path.IsRelative())
			{
				linkerArguments += " -l" + path.ToString();
			}
			else
			{
				linkerArguments += " " + Strings::Quoted(projectBaseDirectory.RelativeTo(path).ToString());
			}
		}
        
        settingsNode.Node("OTHER_CFLAGS").Value("%s", Strings::Quoted(matrix.projectFile.Get_Build_CompilerArguments() + compilerArguments).c_str());
        settingsNode.Node("OTHER_LDFLAGS").Value("%s", Strings::Quoted(matrix.projectFile.Get_Build_LinkerArguments() + linkerArguments).c_str());

        // Forced Includes / Additional Libs
		Platform::Path precompiledHeader = matrix.projectFile.Get_Build_PrecompiledHeader();
		Platform::Path precompiledSource = matrix.projectFile.Get_Build_PrecompiledSource();

        // Precompiled header.
        if (precompiledHeader.ToString() != "")
        {
            settingsNode.Node("GCC_PRECOMPILE_PREFIX_HEADER").Value("YES");
            settingsNode.Node("GCC_PREFIX_HEADER").Value("%s", projectBaseDirectory.RelativeTo(precompiledHeader).ToString().c_str());
        }
        else
        {
            settingsNode.Node("GCC_PRECOMPILE_PREFIX_HEADER").Value("NO");
        }

        // Platform settings
        switch (matrix.platform)
        {
        case EPlatform::Native:
            settingsNode.Node("ARCHS").Value("\"$(NATIVE_ARCH_ACTUAL)\"");
            settingsNode.Node("SUPPORTED_PLATFORMS").Value("macosx");
            settingsNode.Node("MACOSX_DEPLOYMENT_TARGET").Value("10.11"); // todo: make a config option.
            settingsNode.Node("SDKROOT").Value("macosx");
            break;
        case EPlatform::x86:
            settingsNode.Node("ARCHS").Value("\"i386\"");
            settingsNode.Node("SUPPORTED_PLATFORMS").Value("macosx");
            settingsNode.Node("MACOSX_DEPLOYMENT_TARGET").Value("10.11"); // todo: make a config option.
            settingsNode.Node("SDKROOT").Value("macosx");
            break;
        case EPlatform::x64:
            settingsNode.Node("ARCHS").Value("\"x86_64\"");
            settingsNode.Node("SUPPORTED_PLATFORMS").Value("macosx");
            settingsNode.Node("MACOSX_DEPLOYMENT_TARGET").Value("10.11"); // todo: make a config option.
            settingsNode.Node("SDKROOT").Value("macosx");
            break;
        case EPlatform::Universal86:
            settingsNode.Node("ARCHS").Value("\"$(ARCHS_STANDARD_32_BIT)\"");
            settingsNode.Node("SUPPORTED_PLATFORMS").Value("macosx");
            settingsNode.Node("MACOSX_DEPLOYMENT_TARGET").Value("10.11"); // todo: make a config option.
            settingsNode.Node("SDKROOT").Value("macosx");
            break;
        case EPlatform::Universal64:
            settingsNode.Node("ARCHS").Value("\"$(ARCHS_STANDARD_64_BIT)\"");
            settingsNode.Node("SUPPORTED_PLATFORMS").Value("macosx");
            settingsNode.Node("MACOSX_DEPLOYMENT_TARGET").Value("10.11"); // todo: make a config option.
            settingsNode.Node("SDKROOT").Value("macosx");
            break;
        case EPlatform::Universal:
            settingsNode.Node("ARCHS").Value("\"$(ARCHS_STANDARD_32_64_BIT)\"");
            settingsNode.Node("SUPPORTED_PLATFORMS").Value("macosx");
            settingsNode.Node("MACOSX_DEPLOYMENT_TARGET").Value("10.11"); // todo: make a config option.
            settingsNode.Node("SDKROOT").Value("macosx");
            break;
        default:
            matrix.projectFile.ValidateError(
                "Platform '%s' is not valid for xcode C++ projects.",
                CastToString(matrix.platform).c_str());
            return false;
        }

        targetNode.Node("name").Value("%s_%s", matrix.config.c_str(), platformId.c_str());        
	}
    
    // Write the target config list.
	for (auto matrix : buildMatrix)
	{
		std::string platformId =
            CastToString(matrix.platform);

		std::string targetId =
			Strings::Uuid(24, { rootUuid, "PBXProject", "XCConfigurationList", "BuildTarget", matrix.config.c_str(), platformId.c_str() });

		PlistNode& targetNode =
			root.Node("%s", targetId.c_str());

		targetNode.Node("isa").Value("XCBuildConfiguration");
        targetNode.Dict("buildSettings");
		targetNode.Node("name").Value("%s_%s", matrix.config.c_str(), platformId.c_str());
	}
    
    return true;
}

void XCode_CppProjectFile::Write_PBXSourcesBuildPhase(
	PlistNode& root,
	const std::string& rootUuid, 
	const std::string& id,
    const std::map<std::string, std::string>& filterMap
)
{
	PlistNode& targetNode = 
		root.Node("%s", id.c_str());

	targetNode.Node("isa").Value("PBXSourcesBuildPhase");
	targetNode.Node("buildActionMask").Value("2147483647");
	
	PlistNode& filesNode = targetNode.Array("files");

    for (auto file : filterMap)
	{
        Platform::Path path = file.first;
        if (path.IsSourceFile())
        {
            std::string fileId = 
                Strings::Uuid(24, { rootUuid, file.first, "PBXBuildFile" });

            filesNode.Node("").Value("%s", fileId.c_str());
        }
    }

	targetNode.Node("runOnlyForDeploymentPostprocessing").Value(0);
}

void XCode_CppProjectFile::Write_PBXFrameworksBuildPhase(
    PlistNode& root,
    const std::string& rootUuid,
    const std::string& id,
    const std::map<std::string, std::string>& filterMap
)
{
    PlistNode& targetNode =
		root.Node("%s", id.c_str());

	targetNode.Node("isa").Value("PBXFrameworksBuildPhase");
	targetNode.Node("buildActionMask").Value("2147483647");
	PlistNode& filesNode = targetNode.Array("files");
    
    // todo: Do we need to do anything here or can we keep it all in the linker arguments?
    UNUSED_PARAMETER(filesNode);
    UNUSED_PARAMETER(filterMap);
    UNUSED_PARAMETER(rootUuid);
    
	targetNode.Node("runOnlyForDeploymentPostprocessing").Value("0");
}
    
void XCode_CppProjectFile::Write_PBXResourcesBuildPhase(
    PlistNode& root,
    const std::string& rootUuid,
    const std::string& id,
    const std::map<std::string, std::string>& filterMap
)
{
    PlistNode& targetNode =
		root.Node("%s", id.c_str());

	targetNode.Node("isa").Value("PBXResourcesBuildPhase");
	targetNode.Node("buildActionMask").Value("2147483647");
	PlistNode& filesNode = targetNode.Array("files");
    
    for (auto file : filterMap)
	{
        Platform::Path path = file.first;
        if (path.IsResourceFile())
        {
            std::string fileId = 
                Strings::Uuid(24, { rootUuid, file.first, "PBXBuildFile" });

            filesNode.Node("").Value("%s", fileId.c_str());
        }
    }

	targetNode.Node("runOnlyForDeploymentPostprocessing").Value("0");
}
    
bool XCode_CppProjectFile::Generate(
	DatabaseFile& databaseFile,
	WorkspaceFile& workspaceFile,
	ProjectFile& projectFile,
    std::vector<ProjectFile>& projectFiles,
	IdeHelper::BuildProjectMatrix& buildMatrix
)
{
	Platform::Path solutionDirectory =
		workspaceFile.Get_Workspace_Location();

	Platform::Path projectDirectory =
		projectFile.Get_Project_Location();

	Platform::Path projectBaseLocation =
		projectDirectory
		.AppendFragment(projectFile.Get_Project_Name() + ".xcodeproj", true);

	Platform::Path projectLocation =
		projectBaseLocation
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

	// Generate filter list.
	std::map<std::string, std::string> filterMap;
	std::vector<std::string> filters = IdeHelper::SortFiltersByType(
		vpathFilters,
		projectDirectory,
		filterMap
	);
    
	// Node ID's
	std::string rootUuid = projectBaseLocation.ToString();

	std::string targetId = 
		Strings::Uuid(24, { rootUuid, "PBXNativeTarget" });
	std::string projectId = 
		Strings::Uuid(24, { rootUuid, "PBXProject" });
	std::string sourceBuildPhaseId = 
		Strings::Uuid(24, { rootUuid, "PBXSourcesBuildPhase" });
	std::string frameworksPhaseId =
		Strings::Uuid(24, { rootUuid, "PBXFrameworksBuildPhase" });
	std::string resourcesPhaseId =
		Strings::Uuid(24, { rootUuid, "PBXResourcesBuildPhase" });
	std::string projectConfigListId = 
		Strings::Uuid(24, { rootUuid, "PBXProject", "XCConfigurationList" });
	std::string targetConfigListId = 
		Strings::Uuid(24, { rootUuid, "PBXNativeTarget", "XCConfigurationList" });
	std::string rootGroupId =
		Strings::Uuid(24, { rootUuid, "@Root", "PBXGroup" });
    
    // Get dependency list.
    std::vector<ProjectFile*> dependencies;
    for (std::string dependency : projectFile.Get_Dependencies_Dependency())
    {
        ProjectFile* projectDependency = nullptr;
        if (!IdeHelper::GetProjectDependency(
            workspaceFile,
            projectFiles,
            &projectFile,
            projectDependency,
            dependency))
        {
            return false;
        }
        
        dependencies.push_back(projectDependency);
    }

	// Add header.
	PlistNode root;
	root.Node("archiveVersion").Value("1");
	root.Node("classes");
	root.Node("objectVersion").Value("46");
	PlistNode& objectsNode = root.Dict("objects");
    
	// PBXBuildFile Section
	Write_PBXBuildFile(objectsNode, rootUuid, filterMap);
    
    // PBXFileReference Section
    Write_PBXFileReference(objectsNode, rootUuid, filterMap, projectDirectory, projectFile.Get_Project_Name(), projectFile.Get_Project_OutputType(), dependencies);
    
	// PBXGroup Section
	Write_PBXGroup(objectsNode, rootUuid, filters, filterMap, rootGroupId, projectFile.Get_Project_Name(), projectFile.Get_Project_Name(), dependencies);

	// PBXNativeTarget Section
	Write_PBXNativeTarget(objectsNode, rootUuid, targetId,  targetConfigListId, projectFile.Get_Project_Name(), projectFile.Get_Project_OutputType(), sourceBuildPhaseId, frameworksPhaseId, resourcesPhaseId, dependencies);
    
    // PBXProject Section
	Write_PBXProject(objectsNode, rootUuid, projectId, projectConfigListId, rootGroupId, dependencies);
    
    // PBXReferenceProxy Section
	Write_PBXReferenceProxy(objectsNode, rootUuid, dependencies);

	// PBXSourcesBuildPhase Section
	Write_PBXSourcesBuildPhase(objectsNode, rootUuid, sourceBuildPhaseId, filterMap);

	// PBXFrameworksBuildPhase Section
	Write_PBXFrameworksBuildPhase(objectsNode, rootUuid, frameworksPhaseId, filterMap);

	// PBXResourcesBuildPhase Section
	Write_PBXResourcesBuildPhase(objectsNode, rootUuid, resourcesPhaseId, filterMap);

	// XCBuildConfiguration Section
	if (!Write_XCBuildConfiguration(objectsNode, rootUuid, buildMatrix, projectDirectory))
    {
        return false;
    }

	// XCConfigurationList Section	
	Write_XCBuildConfigurationList_Project(objectsNode, rootUuid, projectConfigListId, buildMatrix, platforms, configurations);
 
	Write_XCBuildConfigurationList_Target(objectsNode, rootUuid, targetConfigListId, buildMatrix, platforms, configurations);
    
	// Root object value.
	root.Node("rootObject").Value("%s", projectId.c_str());

	// Generate result.
	if (!databaseFile.StoreFile(
		workspaceFile,
		projectLocation,
		root.ToString().c_str()))
	{
		return false;
	}

	return true;
}

}; // namespace MicroBuild
