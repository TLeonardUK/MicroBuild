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
#include "App/Ides/Make/Make_SolutionFile.h"
#include "Core/Helpers/TextStream.h"

namespace MicroBuild {

Make_SolutionFile::Make_SolutionFile()
{
}

Make_SolutionFile::~Make_SolutionFile()
{
}

bool Make_SolutionFile::Generate(
	DatabaseFile& databaseFile,
	WorkspaceFile& workspaceFile,
	std::vector<ProjectFile>& projectFiles,
	IdeHelper::BuildWorkspaceMatrix& buildMatrix
)
{
	Platform::Path solutionDirectory =
		workspaceFile.Get_Workspace_Location();

	Platform::Path solutionLocation =
		solutionDirectory.AppendFragment("Makefile", true);

	std::vector<std::string> configurations =
		workspaceFile.Get_Configurations_Configuration();

	std::vector<EPlatform> platforms =
		workspaceFile.Get_Platforms_Platform();

	std::stringstream output;

	std::vector<Platform::Path> files = 
		workspaceFile.Get_WorkspaceFiles_File();

	// Resolve VPath's
	std::vector<ConfigFile::KeyValuePair> virtualPaths =
		workspaceFile.Get_WorkspaceVirtualPaths();

	std::vector<IdeHelper::VPathPair> vpathFilters =
		IdeHelper::ExpandVPaths(virtualPaths, files);

	// Get a list of folders.
	std::vector<IdeHelper::ProjectGroupFolder> folders =
		IdeHelper::GetGroupFolders(workspaceFile, projectFiles, vpathFilters);

	TextStream stream(true);

	std::string defaultConfigId = 
		configurations[0] + "_" + CastToString(platforms[0]);

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

	// Write out each config options.
	for (auto config : configurations)
	{
		for (auto platform : platforms)
		{
			std::string id = config + "_" + CastToString(platform);
			stream.WriteLine("ifeq ($(config),%s)", id.c_str());
			stream.Indent();

			for (IdeHelper::BuildProjectMatrix& matrix : buildMatrix)
			{
				for (IdeHelper::BuildProjectPair& pair : matrix)
				{
					if (pair.config == config && 
						pair.platform == platform)
					{
						if (pair.shouldBuild)
						{
							stream.WriteLine("%s_config = %s", 
								pair.projectFile.Get_Project_Name().c_str(),
								id.c_str());
						}
						else
						{
							stream.WriteLine("%s_config = ", 
								pair.projectFile.Get_Project_Name().c_str());
						}
					}
				}
			}

			stream.Undent();
			stream.WriteLine("endif");
		}
	}
	stream.WriteLine("");

	// Write out all recipies.
	stream.Write("PROJECTS := ");
	for (ProjectFile& file : projectFiles)
	{
		std::string projectName = file.Get_Project_Name();
		stream.Write("%s ", projectName.c_str());
	}
	stream.WriteLine("");
	stream.WriteLine("");
	stream.WriteLine(".PHONY: all clean help $(PROJECTS)");
	stream.WriteLine("");
	stream.WriteLine("all: $(PROJECTS)");

	// Write out project recipies.
	for (ProjectFile& file : projectFiles)
	{
		std::string projectName = file.Get_Project_Name();

		Platform::Path projectLocation;
			projectLocation = file.Get_Project_Location().
				AppendFragment(file.Get_Project_Name() + ".Makefile", true);

		Platform::Path relativeLocation =
			solutionDirectory.RelativeTo(projectLocation);

		std::vector<std::string> dependencyNames;
		for (std::string dependency : file.Get_Dependencies_Dependency())
		{
			ProjectFile* projectDependency = nullptr;
			if (!IdeHelper::GetProjectDependency(
				workspaceFile, 
				projectFiles, 
				&file, 
				projectDependency, 
				dependency))
			{
				return false;
			}

			dependencyNames.push_back(projectDependency->Get_Project_Name());
		}

		stream.WriteLine("");
		stream.WriteLine("%s: %s", projectName.c_str(), Strings::Join(dependencyNames, " ").c_str());
		stream.WriteLine("ifneq (,$(%s_config))", projectName.c_str());
		stream.WriteLine("\t@echo \"==== Building %s ($(%s_config)) ====\"", projectName.c_str(), projectName.c_str());
		stream.WriteLine("\t@${MAKE} --no-print-directory -C %s -f %s config=$(%s_config)", 					
			relativeLocation.GetDirectory().ToString().c_str(),
			relativeLocation.GetFilename().c_str(),
			projectName.c_str());
		stream.WriteLine("endif");
	}

	// Clean recipie
	stream.WriteLine("");
	stream.WriteLine("clean:");
	for (ProjectFile& file : projectFiles)
	{
		Platform::Path projectLocation;
			projectLocation = file.Get_Project_Location().
				AppendFragment(file.Get_Project_Name() + ".Makefile", true);

		Platform::Path relativeLocation =
			solutionDirectory.RelativeTo(projectLocation);

		stream.WriteLine("\t@${MAKE} --no-print-directory -C %s -f %s clean", 
			relativeLocation.GetDirectory().ToString().c_str(),
			relativeLocation.GetFilename().c_str()
		);
	}

	// Help recipie	
	stream.WriteLine("");
	stream.WriteLine("help:");
	stream.WriteLine("\t@echo \"Usage: make [config=name] [target]\"");
	stream.WriteLine("\t@echo \"\"");
	stream.WriteLine("\t@echo \"CONFIGURATIONS:\"");
	for (auto config : configurations)
	{
		for (auto platform : platforms)
		{
			std::string id = config + "_" + CastToString(platform);
    			stream.WriteLine("\t@echo \"\t%s\"", id.c_str());
		}
	}
	stream.WriteLine("\t@echo \"\"");
	stream.WriteLine("\t@echo \"TARGETS:\"");
	stream.WriteLine("\t@echo \"\tall (default)\"");
	stream.WriteLine("\t@echo \"\tclean\"");
	for (ProjectFile& file : projectFiles)
	{
		stream.WriteLine("\t@echo \"\t%s\"", file.Get_Project_Name().c_str());
	}
	stream.WriteLine("\t@echo \"\"");

	// Generate result.
	if (!databaseFile.StoreFile(
		workspaceFile,
		solutionLocation,
		stream.ToString().c_str()))
	{
		return false;
	}

	return true;
}

}; // namespace MicroBuild
