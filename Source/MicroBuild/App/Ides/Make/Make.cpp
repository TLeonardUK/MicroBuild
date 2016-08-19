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
#include "App/Ides/Make/Make.h"
#include "App/Ides/Make/Make_CsProjectFile.h"
#include "App/Ides/Make/Make_CppProjectFile.h"
#include "App/Ides/Make/Make_SolutionFile.h"
#include "Core/Helpers/TextStream.h"
#include "Core/Platform/Process.h"

namespace MicroBuild {

Ide_Make::Ide_Make()
{
	SetShortName("make");
}

Ide_Make::~Ide_Make()
{
}

bool Ide_Make::Clean(
	WorkspaceFile& workspaceFile,
    DatabaseFile& databaseFile)
{
    UNUSED_PARAMETER(databaseFile);

	Platform::Path solutionDirectory =
		workspaceFile.Get_Workspace_Location();

	Platform::Path solutionLocation =
		solutionDirectory.AppendFragment("Makefile", true);

	std::stringstream commandLine;
	commandLine << "make";
	commandLine << " -f" << solutionLocation.GetFilename().c_str();
	commandLine << " clean";

	std::vector<std::string> arguments;
	arguments.push_back("-c");
	arguments.push_back(commandLine.str());

	Platform::Process process;
	if (process.Open("/bin/sh", solutionDirectory, arguments, false))
	{
		process.Wait();

		int exitCode = process.GetExitCode();
		if (exitCode == 0)
		{
			return true;
		}
		else
		{
			Log(LogSeverity::Fatal, "make failed with exit code %i.\n", exitCode);
			return false;
		}
	}
	else
	{
		Log(LogSeverity::Fatal, "Failed to start msbuild process.\n");
		return false;
	}

	return false;
}

bool Ide_Make::Build(
	WorkspaceFile& workspaceFile,
	bool bRebuild,
	const std::string& configuration,
	const std::string& platform,
    DatabaseFile& databaseFile)
{
	UNUSED_PARAMETER(bRebuild);
    UNUSED_PARAMETER(databaseFile);

	Platform::Path solutionDirectory =
		workspaceFile.Get_Workspace_Location();

	Platform::Path solutionLocation =
		solutionDirectory.AppendFragment("Makefile", true);

	std::stringstream commandLine;
	commandLine << "make";
	commandLine << " -f" << solutionLocation.GetFilename().c_str();
	commandLine << " config=" << configuration.c_str() << "_" << platform.c_str();

	std::vector<std::string> arguments;
	arguments.push_back("-c");
	arguments.push_back(commandLine.str());

	Platform::Process process;
	if (process.Open("/bin/sh", solutionDirectory, arguments, false))
	{
		process.Wait();

		int exitCode = process.GetExitCode();
		if (exitCode == 0)
		{
			return true;
		}
		else
		{
			Log(LogSeverity::Fatal, "make failed with exit code %i.\n", exitCode);
			return false;
		}
	}
	else
	{
		Log(LogSeverity::Fatal, "Failed to start msbuild process.\n");
		return false;
	}

	return false;
}

bool Ide_Make::Generate(
	DatabaseFile& databaseFile,
	WorkspaceFile& workspaceFile,
	std::vector<ProjectFile>& projectFiles)
{
	IdeHelper::BuildWorkspaceMatrix matrix;
	if (!IdeHelper::CreateBuildMatrix(workspaceFile, projectFiles, matrix))
	{
		return false;
	}

	int index = 0;
	for (ProjectFile& file : projectFiles)
	{
		switch (file.Get_Project_Language())
		{
		case ELanguage::Cpp:
			{
				Make_CppProjectFile projectFile;

				if (!projectFile.Generate(
					databaseFile,
					workspaceFile,
					file,
					matrix[index]))
				{
					return false;
				}

				break;
			}
		case ELanguage::CSharp:
			{
				Make_CsProjectFile projectFile;

				if (!projectFile.Generate(
					databaseFile,
					workspaceFile,
					file,
					matrix[index]))
				{
					return false;
				}

				break;
			}
		default:
			{
				file.ValidateError(
					"Language '%s' is not valid for make projects.",
					CastToString(file.Get_Project_Language()).c_str());
				return false;
			}
		}

		index++;
	}

	Make_SolutionFile solutionFile;

	if (!solutionFile.Generate(
		databaseFile,
		workspaceFile,
		projectFiles,
		matrix))
	{
		return false;
	}
    return true;
}

}; // namespace MicroBuild
