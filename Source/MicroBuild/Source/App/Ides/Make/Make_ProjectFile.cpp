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

void Make_ProjectFile::WriteCommands(
	TextStream& stream,
	const std::vector<std::string>& commands
)
{
	for (auto cmd : commands)
	{
		stream.WriteLine("\t$(SILENT) %s", cmd.c_str());
	}
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

		Platform::Path relativeMicroBuildPath = projectDirectory.RelativeTo(Platform::Path::GetExecutablePath());	

		std::string id = matrix.config + "_" + CastToString(matrix.platform);
		stream.WriteLine("ifeq ($(config),%s)", id.c_str());
		stream.Indent();
		stream.WriteLine("");
		stream.WriteLine("MB_EXE = %s", relativeMicroBuildPath.ToString().c_str());
		stream.WriteLine("MB_WORKSPACE_FILE = %s", workspaceFile.Get_Workspace_File().ToString().c_str());
		stream.WriteLine("MB_PROJECT_NAME = %s", projectFile.Get_Project_Name().c_str());
		stream.WriteLine("MB_PROJECT_CONFIG = %s", matrix.config.c_str());
		stream.WriteLine("MB_PROJECT_PLATFORM = %s", CastToString(matrix.platform).c_str());
		stream.WriteLine("");
		stream.Undent();
		stream.WriteLine("endif");
		stream.WriteLine("");
	}

	// Write out the everything recipie.
	stream.WriteLine("all: build");
	stream.WriteLine("\t@:");
	stream.WriteLine("");

	// Write out the build recipie.
	stream.WriteLine("build:");
	stream.WriteLine("\t$(SILENT) $(MB_EXE) Build $(MB_WORKSPACE_FILE) $(MB_PROJECT_NAME) -c=$(MB_PROJECT_CONFIG) -p=$(MB_PROJECT_PLATFORM) --silent");	
	stream.WriteLine("\t");

	// Write out the build recipie.
	stream.WriteLine("rebuild:");
	stream.WriteLine("\t$(SILENT) $(MB_EXE) Build $(MB_WORKSPACE_FILE) $(MB_PROJECT_NAME) -c=$(MB_PROJECT_CONFIG) -p=$(MB_PROJECT_PLATFORM) -r --silent");	
	stream.WriteLine("\t");
	
	// Write out the clean recipies.
	stream.WriteLine("clean:");	
	stream.WriteLine("\t$(SILENT) $(MB_EXE) Clean $(MB_WORKSPACE_FILE) $(MB_PROJECT_NAME) -c=$(MB_PROJECT_CONFIG) -p=$(MB_PROJECT_PLATFORM) --silent");	
	stream.WriteLine("");

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
