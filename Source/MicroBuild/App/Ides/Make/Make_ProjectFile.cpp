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
#include "App/Ides/Make/Make_ProjectFile.h"

namespace MicroBuild {

Make_ProjectFile::Make_ProjectFile()
{

}

Make_ProjectFile::~Make_ProjectFile()
{

}

bool Make_ProjectFile::Generate(
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
			projectFile.Get_Project_Name() + ".Makefile", true);

	std::vector<std::string> configurations =
		workspaceFile.Get_Configurations_Configuration();

	std::vector<EPlatform> platforms =
		workspaceFile.Get_Platforms_Platform();

	std::string projectGuid = Strings::Guid({
		workspaceFile.Get_Workspace_Name(),
		projectFile.Get_Project_Name() });

	std::string projectName = 
		projectFile.Get_Project_Name();

	TextStream stream(true);

	std::string defaultConfigId = 
		configurations[0] + "_" + CastToString(platforms[0]);

	// Files.
	std::vector<Platform::Path> files = 
		projectFile.Get_Files_File();

	// Header
	stream.WriteLine("ifndef config");
	stream.Indent();
		stream.WriteLine("config = %s", defaultConfigId.c_str());
	stream.Undent();
	stream.WriteLine("endif");
	stream.WriteLine("");
	stream.WriteLine("ifndef verbose");
	stream.Indent();
		stream.WriteLine("SILENT = @");
	stream.Undent();
	stream.WriteLine("endif");
	stream.WriteLine("");
	stream.WriteLine(".PHONY: clean");
	stream.WriteLine("");

	// Generate configuration/platform unique settings.
	for (auto matrix : buildMatrix)
	{
		std::string relativeOutputDirectory =
			projectDirectory.RelativeTo(matrix.projectFile.Get_Project_OutputDirectory()).ToString();

		std::string relativeObjectDirectory =
			projectDirectory.RelativeTo(matrix.projectFile.Get_Project_IntermediateDirectory()).ToString();

		std::string id = matrix.config + "_" + CastToString(matrix.platform);
		stream.WriteLine("ifeq ($(config),%s)", id.c_str());
		stream.Indent();

		stream.WriteLine("");

		// Platform toolsets.
		switch (matrix.projectFile.Get_Build_PlatformToolset())
		{
			case EPlatformToolset::Default:
				// Fallthrough
			case EPlatformToolset::GCC:
			{
				stream.WriteLine("CXX = g++");
				stream.WriteLine("CC = gcc");
				stream.WriteLine("AR = ar");
				break;
			}
			case EPlatformToolset::Clang:
			{
				stream.WriteLine("CXX = clang++");
				stream.WriteLine("CC = clang");
				stream.WriteLine("AR = ar");
				break;
			}
			default:
			{
				projectFile.ValidateError(
					"Toolset '%s' is not valid for make C++ projects.",
					CastToString(matrix.projectFile.Get_Build_PlatformToolset()).c_str());
				return false;
			}
		}

		stream.WriteLine("");

		// Directory paths.
		stream.WriteLine("TARGETDIR = %s", relativeOutputDirectory.c_str());

		stream.WriteLine("TARGET = $(TARGETDIR)/%s%s", 
			matrix.projectFile.Get_Project_OutputName().c_str(),
			matrix.projectFile.Get_Project_OutputExtension().c_str());

		stream.WriteLine("OBJDIR = %s", relativeObjectDirectory.c_str());

		// Preprocessor defines.
		stream.Write("DEFINES = ");
		for (auto define : matrix.projectFile.Get_Defines_Define())
		{
			stream.Write("-D%s ", define.c_str());
		}
		stream.WriteLine("");

		// Include directories.
		stream.Write("INCLUDE_DIRS = ");
		for (Platform::Path& path : matrix.projectFile.Get_SearchPaths_IncludeDirectory())
		{
			stream.Write("-I\"%s\" ", projectDirectory.RelativeTo(path).ToString().c_str());
		}
		stream.WriteLine("");

		// Library directories.
		stream.Write("LIBRARY_DIRS = ");
		for (Platform::Path& path : matrix.projectFile.Get_SearchPaths_LibraryDirectory())
		{
			stream.Write("-L\"%s\" ", projectDirectory.RelativeTo(path).ToString().c_str());
		}
		stream.WriteLine("");
		
		// Forced includes.
		stream.Write("FORCE_INCLUDES = ");
		for (Platform::Path& path : matrix.projectFile.Get_ForcedIncludes_ForcedInclude())
		{
			stream.Write("-include \"%s\" ", projectDirectory.RelativeTo(path).ToString().c_str());
		}
		stream.WriteLine("");

		// Library includes
		stream.Write("LIBRARIES = ");
		for (Platform::Path& path : matrix.projectFile.Get_Libraries_Library())
		{
			if (path.IsRelative())
			{
				stream.Write("-l\"%s\" ", path.ToString().c_str());
			}
			else
			{
				stream.Write("\"%s\" ", projectDirectory.RelativeTo(path).ToString().c_str());
			}
		}
		stream.WriteLine("");

		Platform::Path precompiledHeader = 
			matrix.projectFile.Get_Build_PrecompiledHeader();
		if (precompiledHeader.ToString() != "")
		{
			stream.WriteLine("PCH = %s",
				projectDirectory.RelativeTo(precompiledHeader).ToString().c_str()
			);
		}

		std::vector<std::string> cflags;
		std::vector<std::string> cxxflags;
		std::vector<std::string> ldflags;

		// rtti
		if (matrix.projectFile.Get_Flags_RuntimeTypeInfo())
		{
			cxxflags.push_back("-frtti");
			ldflags.push_back("-frtti");			
		}
		else
		{
			cxxflags.push_back("-fno-rtti");
			ldflags.push_back("-fno-rtti");						
		}

		// exceptions
		if (matrix.projectFile.Get_Flags_Exceptions())
		{
			cxxflags.push_back("-fexceptions");
			ldflags.push_back("-fexceptions");	
		}
		else
		{
			cxxflags.push_back("-fno-exceptions");
			ldflags.push_back("-fno-exceptions");				
		}

		// warning level
		switch (matrix.projectFile.Get_Build_WarningLevel())
		{
		case EWarningLevel::Default:
		case EWarningLevel::None:
		case EWarningLevel::Low:
		case EWarningLevel::Medium:
			// All of these just use the default warning level in gcc.
			break;
		case EWarningLevel::High:
			cflags.push_back("-Wall");
			break;
		case EWarningLevel::Verbose:
			cflags.push_back("-Wextra");
			break;
		default:
			projectFile.ValidateError(
				"Warning level '%s' is not valid for make C++ projects.",
				CastToString(matrix.projectFile.Get_Build_WarningLevel()).c_str());
			return false;
		}

		// warnings as errors
		if (matrix.projectFile.Get_Flags_CompilerWarningsFatal())
		{			
			cflags.push_back("-Werror");
		}

		// disabled warnings
		for (auto warning : matrix.projectFile.Get_DisabledWarnings_DisabledWarning())
		{
			cflags.push_back(Strings::Format("-Wno-%s", warning.c_str()));
		}

		// additional compiler options
		cflags.push_back(matrix.projectFile.Get_Build_CompilerArguments());

		// additional linker options
		ldflags.push_back(matrix.projectFile.Get_Build_LinkerArguments());

		// optimization
		switch (matrix.projectFile.Get_Build_OptimizationLevel())
		{
		case EOptimizationLevel::None:
			// Fallthrough
		case EOptimizationLevel::Debug:
			cflags.push_back("-Og");
			ldflags.push_back("-Og");
			break;
		case EOptimizationLevel::PreferSize:
			cflags.push_back("-Os");
			ldflags.push_back("-Os");
			break;
		case EOptimizationLevel::PreferSpeed:
			cflags.push_back("-Ofast");
			ldflags.push_back("-Ofast");
			break;
		case EOptimizationLevel::Full:
			cflags.push_back("-O3");
			ldflags.push_back("-O3");
			break;
		default:
			projectFile.ValidateError(
				"Optimization level '%s' is not valid for make C++ projects.",
				CastToString(matrix.projectFile.Get_Build_OptimizationLevel()).c_str());
			return false;
		}

		// link time code gen
		if (matrix.projectFile.Get_Flags_LinkTimeOptimization())
		{			
			cflags.push_back("-flto");
			ldflags.push_back("-flto");
		}
		else
		{
			cflags.push_back("-fno-lto");
			ldflags.push_back("-fno-lto");			
		}
		
		// linker warnings as errors
		if (matrix.projectFile.Get_Flags_LinkerWarningsFatal())
		{
			ldflags.push_back("-Werror");
		}

		// debug information
		if (matrix.projectFile.Get_Flags_GenerateDebugInformation())
		{			
			ldflags.push_back("-g");
		}

		// architecture depth
		switch (matrix.platform)
		{
		case EPlatform::ARM64:
		case EPlatform::x64:
			cflags.push_back("-m64");
			ldflags.push_back("-m64");
			ldflags.push_back("-L/usr/lib64");
			break;
		default:
			cflags.push_back("-m32");
			ldflags.push_back("-m32");
			ldflags.push_back("-L/usr/lib32");
			break;
		}

		// C++ standard
		switch (matrix.projectFile.Get_Project_LanguageVersion())
		{
		case ELanguageVersion::Default:
			// Fallthrough
		case ELanguageVersion::Cpp_11:
			cxxflags.push_back("-std=c++11");
			break;
		case ELanguageVersion::Cpp_98:
			cxxflags.push_back("-std=c++98");
			break;
		case ELanguageVersion::Cpp_14:
			cxxflags.push_back("-std=c++14");
			break;
		default:
			projectFile.ValidateError(
				"Language standard '%s' is not valid for make C++ projects.",
				CastToString(matrix.projectFile.Get_Project_LanguageVersion()).c_str());
			return false;
		}

		// Need PIC if we are making a shared lib.
		if (matrix.projectFile.Get_Project_OutputType() == EOutputType::DynamicLib)
		{
			cflags.push_back("-fPIC");
			ldflags.push_back("-fPIC");
			ldflags.push_back("-shared");
		}

		stream.WriteLine("CFLAGS += -MMD -MP %s", Strings::Join(cflags, " ").c_str());
		stream.WriteLine("CXXFLAGS += %s", Strings::Join(cxxflags, " ").c_str());
		stream.WriteLine("LDFLAGS += %s", Strings::Join(ldflags, " ").c_str());

		stream.WriteLine("ALL_CFLAGS = $(DEFINES) $(INCLUDE_DIRS) $(CFLAGS) $(FORCE_INCLUDES)");
		stream.WriteLine("ALL_CXXFLAGS = $(DEFINES) $(INCLUDE_DIRS) $(CFLAGS) $(CXXFLAGS) $(FORCE_INCLUDES)");
		stream.WriteLine("ALL_LDFLAGS = $(LDFLAGS) $(LIBRARY_DIRS) $(LIBRARIES)");

		stream.WriteLine("");

		stream.Undent();
		stream.WriteLine("all: $(TARGETDIR) $(OBJDIR) $(TARGET)");
		stream.WriteLine("\t@:");
		stream.Indent();

		stream.WriteLine("");

		stream.Undent();
		stream.WriteLine("endif");
		stream.WriteLine("");
	}

	// Shell type determination.
	stream.WriteLine("SHELLTYPE := msdos");
	stream.WriteLine("ifeq (,$(ComSpec)$(COMSPEC))");
	stream.Indent();
	stream.WriteLine("SHELLTYPE := posix");
	stream.Undent();
	stream.WriteLine("endif");
	stream.WriteLine("ifeq (/bin,$(findstring /bin,$(SHELL)))");
	stream.Indent();
	stream.WriteLine("SHELLTYPE := posix");
	stream.Undent();
	stream.WriteLine("endif");
	stream.WriteLine("");

	// Write out the object list.
	stream.WriteLine("OBJECTS := \\");
	stream.Indent();
	for (auto file : files)
	{
		if (file.IsSourceFile())
		{
			stream.WriteLine("$(OBJDIR)/%s.o \\", file.GetBaseName().c_str());
		}
	}
	stream.Undent();
	stream.WriteLine("");
	
	// output type
	stream.WriteLine("$(TARGET): $(GCH) $(OBJECTS) $(LDDEPS)");
	switch (projectFile.Get_Project_OutputType())
	{
	case EOutputType::Executable:
		// Fallthrough
	case EOutputType::ConsoleApp:
		stream.WriteLine("\t@echo \"Linking %s\"", projectName.c_str());
		stream.WriteLine("\t$(SILENT) $(CXX) -o \"$@\" $(OBJECTS) $(ALL_LDFLAGS) $(LIBRARIES)");
		break;
	case EOutputType::DynamicLib:
		break;
	case EOutputType::StaticLib:
		stream.WriteLine("\t@echo \"Archiving %s\"", projectName.c_str());
		stream.WriteLine("\t$(SILENT) $(AR) rcs \"$@\" $(OBJECTS)");
		break;
	default:
		projectFile.ValidateError(
			"Output type '%s' is not valid for make C++ projects.",
			CastToString(projectFile.Get_Project_OutputType()).c_str());
		return false;
	}		
	stream.WriteLine("");

	// Write out the directory recipies.
	stream.WriteLine("$(TARGETDIR):");
	stream.WriteLine("\t@echo \"Creating $(TARGETDIR)\"");
	stream.WriteLine("ifeq (posix,$(SHELLTYPE))");
	stream.WriteLine("\t$(SILENT) mkdir -p $(TARGETDIR)");		
	stream.WriteLine("else");
	stream.WriteLine("\t$(SILENT) mkdir $(subst /,\\\\,$(TARGETDIR))");		
	stream.WriteLine("endif");
	stream.WriteLine("");

	stream.WriteLine("$(OBJDIR):");
	stream.WriteLine("\t@echo \"Creating $(OBJDIR)\"");
	stream.WriteLine("ifeq (posix,$(SHELLTYPE))");
	stream.WriteLine("\t$(SILENT) mkdir -p $(OBJDIR)");		
	stream.WriteLine("else");
	stream.WriteLine("\t$(SILENT) mkdir $(subst /,\\\\,$(OBJDIR))");		
	stream.WriteLine("endif");
	stream.WriteLine("");

	// Write out the clean recipies.
	stream.WriteLine("clean:");
	stream.WriteLine("\t@echo \"Cleaning %s\"", projectName.c_str());
	stream.WriteLine("ifeq (posix,$(SHELLTYPE))");
	stream.WriteLine("\t$(SILENT) rm -f $(TARGET)");		
	stream.WriteLine("\t$(SILENT) rm -rf $(OBJDIR)");		
	stream.WriteLine("else");
	stream.WriteLine("\t$(SILENT) if exist $(subst /,\\\\,$(TARGET)) del $(subst /,\\\\,$(TARGET))");		
	stream.WriteLine("\t$(SILENT) if exist $(subst /,\\\\,$(OBJDIR)) rmdir /s /q $(subst /,\\\\,$(OBJDIR))");		
	stream.WriteLine("endif");
	stream.WriteLine("");

	// Write out the PCH recipie.
	stream.WriteLine("ifneq (,$(PCH))");
	stream.WriteLine("$(OBJECTS): $(GCH) $(PCH)");
	stream.WriteLine("$(GCH): $(PCH)");
	stream.WriteLine("\t@echo $(notdir $<)");
	stream.WriteLine("\t$(SILENT) $(CXX) -x c++-header $(ALL_CXXFLAGS) -o \"$@\" -MF \"$(@:%%.gch=%%.d)\" -c \"$<\"");
	stream.WriteLine("endif");
	stream.WriteLine("");

	// Write out the object recipies.
	for (auto file : files)
	{
		if (file.IsSourceFile())
		{
			stream.WriteLine("$(OBJDIR)/%s.o: %s", 
				file.GetBaseName().c_str(),
				projectDirectory.RelativeTo(file).ToString().c_str()
			);
	
			stream.WriteLine("\t@echo $(notdir $<)");
			if (file.IsCFile())
			{
				stream.WriteLine("\t$(SILENT) $(CC) $(ALL_CFLAGS) -o \"$@\" -MF \"$(@:%%.o=%%.d)\" -c \"$<\"");
			}
			else if (file.IsCppFile())
			{
				stream.WriteLine("\t$(SILENT) $(CXX) $(ALL_CXXFLAGS) -o \"$@\" -MF \"$(@:%%.o=%%.d)\" -c \"$<\"");
			}
			else 
			{
				stream.WriteLine("\t$(SILENT) $(CXX) $(ALL_CXXFLAGS) -o \"$@\" -MF \"$(@:%%.o=%%.d)\" -c \"$<\"");
			}
		}
	}

	// Include all dependency files if they exist.		
	stream.WriteLine("");
	stream.WriteLine("-include $(OBJECTS:%%.o=%%.d)");
	stream.WriteLine("ifneq (,$(PCH))");
	stream.Indent();
	stream.WriteLine("-include $(OBJDIR)/$(notdir ($PCH)).d");
	stream.Undent();
	stream.WriteLine("endif");

	// Generate result.
	if (!databaseFile.StoreFile(
		workspaceFile,
		projectDirectory,
		projectLocation,
		stream.ToString().c_str()))
	{
		return false;
	}

	return true;
}

}; // namespace MicroBuild
