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
#include "App/Ides/Make/Make_CsProjectFile.h"

namespace MicroBuild {

Make_CsProjectFile::Make_CsProjectFile()
{

}

Make_CsProjectFile::~Make_CsProjectFile()
{

}

void Make_CsProjectFile::WriteCommands(
	TextStream& stream,
	const std::vector<std::string>& commands
)
{
	for (auto cmd : commands)
	{
		stream.WriteLine("\t$(SILENT) %s", cmd.c_str());
	}
}

bool Make_CsProjectFile::Generate(
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

		std::string id = matrix.config + "_" + CastToString(matrix.platform);
		stream.WriteLine("ifeq ($(config),%s)", id.c_str());
		stream.Indent();

		stream.WriteLine("");

		// Platform toolsets.
		switch (matrix.projectFile.Get_Build_PlatformToolset())
		{
			case EPlatformToolset::Default:
				// Fallthrough
			case EPlatformToolset::Mono:
			{
				stream.WriteLine("CSC = mcs");
				break;
			}
			default:
			{
				projectFile.ValidateError(
					"Toolset '%s' is not valid for make csharp projects.",
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

		// Preprocessor defines.
		stream.Write("DEFINES = ");
		for (auto define : matrix.projectFile.Get_Defines_Define())
		{
			stream.Write("-define:%s ", define.c_str());
		}
		stream.WriteLine("");

		// Library directories.
		stream.Write("LIBRARY_DIRS = ");
		for (Platform::Path& path : matrix.projectFile.Get_SearchPaths_LibraryDirectory())
		{
			stream.Write("-L \"%s\" ", projectDirectory.RelativeTo(path).ToString().c_str());
		}
		stream.WriteLine("");
		
		// Library includes
		stream.Write("REFERENCES = ");
		for (Platform::Path& path : matrix.projectFile.Get_References_Reference())
		{
			if (path.IsRelative())
			{
				stream.Write("-r:\"%s\" ", path.ToString().c_str());
			}
			else
			{
				stream.Write("-r:\"%s\" ", projectDirectory.RelativeTo(path).ToString().c_str());
			}
		}
		stream.WriteLine("");

		std::vector<std::string> csflags;

		switch (matrix.projectFile.Get_Project_LanguageVersion())
		{
		case ELanguageVersion::Default:
			break;
		case ELanguageVersion::CSharp_1_0:
			csflags.push_back("-langversion:ISO-1");
			break;
		case ELanguageVersion::CSharp_2_0:
			csflags.push_back("-langversion:ISO-2");
			break;
		case ELanguageVersion::CSharp_3_0:
			csflags.push_back("-langversion:ISO-3");
			break;
		case ELanguageVersion::CSharp_4_0:
			csflags.push_back("-langversion:ISO-4");
			break;
		case ELanguageVersion::CSharp_5_0:
			csflags.push_back("-langversion:ISO-5");
			break;
		case ELanguageVersion::CSharp_6_0:
			csflags.push_back("-langversion:ISO-6");
			break;
		default:
			projectFile.ValidateError(
				"Project language version '%s' is not valid for this project type.",
				CastToString(projectFile.Get_Project_LanguageVersion()).c_str());
			return false;
		}

		switch (projectFile.Get_Project_OutputType())
		{
		case EOutputType::Executable:
			csflags.push_back("-target:winexe");
			break;
		case EOutputType::ConsoleApp:
			csflags.push_back("-target:exe");
			break;
		case EOutputType::DynamicLib:
			csflags.push_back("-target:library");
			break;
		default:
			projectFile.ValidateError(
				"Output type '%s' is not valid for make C++ projects.",
				CastToString(projectFile.Get_Project_OutputType()).c_str());
			return false;
		}		

		// warning level
		switch (matrix.projectFile.Get_Build_WarningLevel())
		{
		case EWarningLevel::None:
			csflags.push_back("-w:0");
			break;
		case EWarningLevel::Low:
			csflags.push_back("-w:1");
			break;
		case EWarningLevel::Medium:
			csflags.push_back("-w:2");
			break;
		case EWarningLevel::High:
			csflags.push_back("-w:3");
			break;
		case EWarningLevel::Default:
			// fallthrough
		case EWarningLevel::Verbose:
			csflags.push_back("-w:4");
			break;
		default:
			projectFile.ValidateError(
				"Warning level '%s' is not valid for make csharp projects.",
				CastToString(matrix.projectFile.Get_Build_WarningLevel()).c_str());
			return false;
		}

		// warnings as errors
		if (matrix.projectFile.Get_Flags_CompilerWarningsFatal() ||
			matrix.projectFile.Get_Flags_LinkerWarningsFatal())
		{			
			csflags.push_back("-warnaserror");
		}

		// disabled warnings
		for (auto warning : matrix.projectFile.Get_DisabledWarnings_DisabledWarning())
		{
			csflags.push_back(Strings::Format("-nowarn:%s", warning.c_str()));
		}

		// additional compiler options
		csflags.push_back(matrix.projectFile.Get_Build_CompilerArguments());

		// additional linker options
		csflags.push_back(matrix.projectFile.Get_Build_LinkerArguments());

		// optimization
		switch (matrix.projectFile.Get_Build_OptimizationLevel())
		{
		case EOptimizationLevel::None:
			// Fallthrough
		case EOptimizationLevel::Debug:
			csflags.push_back("-optimize-");
			break;
		case EOptimizationLevel::PreferSize:
			// Fallthrough
		case EOptimizationLevel::PreferSpeed:
			// Fallthrough
		case EOptimizationLevel::Full:
			csflags.push_back("-optimize+");
			break;
		default:
			projectFile.ValidateError(
				"Optimization level '%s' is not valid for make csharp projects.",
				CastToString(matrix.projectFile.Get_Build_OptimizationLevel()).c_str());
			return false;
		}

		// Unsafe code
		if (matrix.projectFile.Get_Flags_AllowUnsafeCode())
		{
			csflags.push_back("-unsafe+");
		}

		// debug information
		if (matrix.projectFile.Get_Flags_GenerateDebugInformation())
		{			
			csflags.push_back("-debug+");
		}
		else
		{		
			csflags.push_back("-debug-");			
		}

		stream.WriteLine("CSFLAGS += %s", Strings::Join(csflags, " ").c_str());
		stream.WriteLine("ALL_CSFLAGS = $(DEFINES) $(LIBRARY_DIRS) $(REFERENCES) $(CSFLAGS)");

		stream.WriteLine("");

		stream.Undent();
		stream.WriteLine("all: $(TARGETDIR) prebuild prelink $(TARGET)");
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
	stream.WriteLine("CS_FILES := \\");
	stream.Indent();
	for (auto file : files)
	{
		if (file.IsSourceFile())
		{
			stream.WriteLine("%s \\", 
				Strings::SpacesEscaped(projectDirectory.RelativeTo(file).ToString()).c_str());
		}
	}
	stream.Undent();
	stream.WriteLine("");
	
	// output type
	stream.WriteLine("$(TARGET): $(CS_FILES)");
	stream.WriteLine("\t@echo \"Building %s\"", projectName.c_str());
	stream.WriteLine("\t$(SILENT) $(CSC) $(ALL_CSFLAGS) -out:\"$@\" $(CS_FILES)");

	// Postbuild commands.
	WriteCommands(stream, projectFile.Get_PostBuildCommands_Command());
	stream.WriteLine("");

	// Prebuild/Prelink recipies.
	stream.WriteLine("prebuild:");
	WriteCommands(stream, projectFile.Get_PreBuildCommands_Command());
	stream.WriteLine("");

	stream.WriteLine("postlink:");
	WriteCommands(stream, projectFile.Get_PreLinkCommands_Command());
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

	// Write out the clean recipies.
	stream.WriteLine("clean:");
	stream.WriteLine("\t@echo \"Cleaning %s\"", projectName.c_str());
	stream.WriteLine("ifeq (posix,$(SHELLTYPE))");
	stream.WriteLine("\t$(SILENT) rm -f $(TARGET)");		
	stream.WriteLine("else");
	stream.WriteLine("\t$(SILENT) if exist $(subst /,\\\\,$(TARGET)) del $(subst /,\\\\,$(TARGET))");		
	stream.WriteLine("endif");
	stream.WriteLine("");

	// Write out the object recipies.
	for (auto file : files)
	{
		if (file.IsSourceFile())
		{
			stream.WriteLine("%s:", 
				Strings::SpacesEscaped(projectDirectory.RelativeTo(file).ToString()).c_str()
			);
	
			stream.WriteLine("\t@:");
		}
	}

	// Generate result.
	if (!databaseFile.StoreFile(
		workspaceFile,
		projectLocation,
		stream.ToString().c_str()))
	{
		return false;
	}

	return true;
}

}; // namespace MicroBuild
