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

	TextStream stream;

	std::string defaultConfigId = 
		configurations[0] + "_" + CastToString(platforms[0]);

	// Files.
	std::vector<Platform::Path> files = 
		projectFile.Get_Files_File();

	// Header
	stream.WriteLine(".NOTPARALLEL:");
	stream.WriteLine("");
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
	stream.WriteLine(".PHONY clean");
	stream.WriteLine("");

	// Generate configuration/platform unique settings.
	for (auto config : configurations)
	{
		for (auto platform : platforms)
		{
			std::string id = config + "_" + CastToString(platform);
			stream.WriteLine("ifeq ($(config),%s)", id.c_str());
			stream.Indent();

			stream.WriteLine("");

			stream.WriteLine("TARGETDIR = ");
			stream.WriteLine("TARGET = ");
			stream.WriteLine("OBJDIR = ");
			stream.WriteLine("DEFINES = ");
			stream.WriteLine("INCLUDES = ");
			stream.WriteLine("FORCE_INCLUDES = ");
			stream.WriteLine("CPPFLAGS = ");
			stream.WriteLine("CFLAGS = ");
			stream.WriteLine("CXXFLAGS = ");
			stream.WriteLine("LIBS = ");
			stream.WriteLine("LDDEPS = ");
			stream.WriteLine("LDFLAGS = ");
			stream.WriteLine("LINKCMD = ");

			stream.WriteLine("");

			stream.Undent();
			stream.WriteLine("all: $(TARGETDIR) $(OBJDIR) $(TARGET)");
			stream.Indent();
				stream.WriteLine("@:");
			stream.Undent();
			stream.Indent();

			stream.WriteLine("");

			stream.Undent();
			stream.WriteLine("endif");
			stream.WriteLine("");
		}
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
		stream.WriteLine("$(ObjDir)/%s.o \\", file.GetBaseName().c_str());
	}
	stream.Undent();
	stream.WriteLine("");
	
	// Write out the link recipie.
	stream.WriteLine("$(TARGET): $(GCH) $(OBJECTS) $(LDDEPS)");
	stream.Indent();
		stream.WriteLine("@echo \"Linking %s\"", projectName.c_str());
		stream.WriteLine("$(SILENT) $(LINKCMD)");
	stream.Undent();
	stream.WriteLine("");

	// Write out the directory recipies.
	stream.WriteLine("$(TARGETDIR):");
	stream.Indent();
		stream.WriteLine("@echo \"Creating $(TARGETDIR)\"");
		stream.Undent();
			stream.WriteLine("ifeq (posix,$(SHELLTYPE))");
			stream.Indent();
				stream.WriteLine("$(SILENT) mkdir -p $(TARGETDIR)");		
			stream.Undent();
			stream.WriteLine("else");
			stream.Indent();
				stream.WriteLine("$(SILENT) mkdir $(subst /,\\\\,$(TARGETDIR))");		
			stream.Undent();
			stream.WriteLine("endif");
		stream.Indent();
	stream.Undent();
	stream.WriteLine("");

	stream.WriteLine("$(OBJDIR):");
	stream.Indent();
		stream.WriteLine("@echo \"Creating $(OBJDIR)\"");
		stream.Undent();
			stream.WriteLine("ifeq (posix,$(SHELLTYPE))");
			stream.Indent();
				stream.WriteLine("$(SILENT) mkdir -p $(OBJDIR)");		
			stream.Undent();
			stream.WriteLine("else");
			stream.Indent();
				stream.WriteLine("$(SILENT) mkdir $(subst /,\\\\,$(OBJDIR))");		
			stream.Undent();
			stream.WriteLine("endif");
		stream.Indent();
	stream.Undent();
	stream.WriteLine("");

	// Write out the clean recipies.
	stream.WriteLine("clean:");
	stream.Indent();
		stream.WriteLine("@echo \"Cleaning %s\"", projectName.c_str());
		stream.Undent();
			stream.WriteLine("ifeq (posix,$(SHELLTYPE))");
			stream.Indent();
				stream.WriteLine("$(SILENT) rm -f $(TARGET)");		
				stream.WriteLine("$(SILENT) rm -rf $(OBJDIR)");		
			stream.Undent();
			stream.WriteLine("else");
			stream.Indent();
				stream.WriteLine("$(SILENT) if exist $(subst /,\\\\,$(TARGET)) del $(subst /,\\\\,$(TARGET))");		
				stream.WriteLine("$(SILENT) if exist $(subst /,\\\\,$(OBJDIR)) rmdir /s /q $(subst /,\\\\,$(OBJDIR))");		
			stream.Undent();
			stream.WriteLine("endif");
		stream.Indent();
	stream.Undent();
	stream.WriteLine("");

	// Write out the PCH recipie.
	stream.WriteLine("ifneq (,$(PCH))");
	stream.Indent();
		stream.Undent();
			stream.WriteLine("$(OBJECTS): $(GCH) $(PCH)");
			stream.WriteLine("$(GCH): $(PCH)");
		stream.Indent();
		stream.Indent();
			stream.WriteLine("@echo $(notdir $<)");
			stream.WriteLine("$(SILENT) $(CXX) -x c++-header $(ALL_CXXFLAGS) -o \"$@\" -MF \"$(@:%%.gch=%%.d)\" -c \"$<\"");
		stream.Undent();
	stream.Undent();
	stream.WriteLine("endif");
	stream.WriteLine("");

	// Write out the object recipies.
	for (auto file : files)
	{
		stream.WriteLine("$(OBJDIR)/%s.o: %s", 
			file.GetBaseName().c_str(),
			projectDirectory.RelativeTo(file).ToString().c_str()
		);
	
		stream.Indent();
			stream.WriteLine("@echo $(notdir $<)");
			// todo: type specific
			stream.WriteLine("$(SILENT) $(CXX) $(CXXFLAGS) $(FORCE_INCLUDES) -o \"$@\" -MF \"$(@:%%.o=%%.d)\" -c \"$<\"");
		stream.Undent();
	
		stream.WriteLine("");
	}

	// Include all dependency files if they exist.
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
