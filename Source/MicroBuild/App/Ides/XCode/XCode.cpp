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
#include "App/Ides/XCode/XCode.h"
#include "App/Ides/XCode/XCode_CsProjectFile.h"
#include "App/Ides/XCode/XCode_CppProjectFile.h"
#include "App/Ides/XCode/XCode_SolutionFile.h"
#include "Core/Helpers/TextStream.h"
#include "Core/Platform/Process.h"

namespace MicroBuild {

Ide_XCode::Ide_XCode()
{
	SetShortName("xcode");
}

Ide_XCode::~Ide_XCode()
{
}

bool Ide_XCode::Clean(
	WorkspaceFile& workspaceFile,
    DatabaseFile& databaseFile)
{
	Platform::Path solutionDirectory =
		workspaceFile.Get_Workspace_Location();

    std::string workspaceName =
        workspaceFile.Get_Workspace_Name() + ".xcworkspace";

	Platform::Path solutionLocation =
		solutionDirectory
		.AppendFragment(workspaceName, true);

    // Clean each project contained in the workspace.
    std::vector<std::string> configurations = databaseFile.Get_Workspace_Configuration();
    std::vector<EPlatform> platforms = databaseFile.Get_Workspace_Platform();
    std::vector<std::string> projects = databaseFile.Get_Workspace_Project();
	for (std::string project : projects)
	{
        for (std::string config : configurations)
        {
            for (EPlatform platform : platforms)
            {
                std::vector<std::string> arguments;
                arguments.push_back("-workspace");
                arguments.push_back(solutionLocation.ToString());
                arguments.push_back("-scheme");
                arguments.push_back(project);
                arguments.push_back("-configuration");
                arguments.push_back(config + "_" + CastToString(platform));
                arguments.push_back("clean");

                Platform::Process process;
                if (process.Open("/usr/bin/xcodebuild", solutionDirectory, arguments, false))
                {
                    process.Wait();

                    int exitCode = process.GetExitCode();
                    if (exitCode != 0)
                    {
                        Log(LogSeverity::Fatal, "xcodebuild failed with exit code %i.\n", exitCode);
                        return false;
                    }
                }
                else
                {
                    Log(LogSeverity::Fatal, "Failed to start xcodebuild process.\n");
                    return false;
                }
            }
        }
    }

	return true;
}

bool Ide_XCode::Build(
	WorkspaceFile& workspaceFile,
	bool bRebuild,
	const std::string& configuration,
	const std::string& platform,
    DatabaseFile& databaseFile)
{
	Platform::Path solutionDirectory =
		workspaceFile.Get_Workspace_Location();

    std::string workspaceName =
        workspaceFile.Get_Workspace_Name() + ".xcworkspace";

	Platform::Path solutionLocation =
		solutionDirectory
		.AppendFragment(workspaceName, true);

    // Clean each project contained in the workspace.
    std::vector<std::string> projects = databaseFile.Get_Workspace_Project();
	for (std::string project : projects)
	{
        std::vector<std::string> arguments;
        arguments.push_back("-workspace");
        arguments.push_back(solutionLocation.ToString());
        arguments.push_back("-scheme");
        arguments.push_back(project);
        arguments.push_back("-configuration");
        arguments.push_back(configuration + "_" + platform);
        if (bRebuild)
        {
            arguments.push_back("clean");
        }
        arguments.push_back("build");

        Platform::Process process;
        if (process.Open("/usr/bin/xcodebuild", solutionDirectory, arguments, false))
        {
            process.Wait();

            int exitCode = process.GetExitCode();
            if (exitCode != 0)
            {
                Log(LogSeverity::Fatal, "xcodebuild failed with exit code %i.\n", exitCode);
                return false;
            }
        }
        else
        {
            Log(LogSeverity::Fatal, "Failed to start xcodebuild process.\n");
            return false;
        }
    }

	return true;
}

bool Ide_XCode::Generate(
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
				XCode_CppProjectFile projectFile;

				if (!projectFile.Generate(
					databaseFile,
					workspaceFile,
					file,
                    projectFiles,
					matrix[index]))
				{
					return false;
				}

				break;
			}
		case ELanguage::CSharp:
			{
				XCode_CsProjectFile projectFile;

				if (!projectFile.Generate(
					databaseFile,
					workspaceFile,
					file,
                    projectFiles,
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

	XCode_SolutionFile solutionFile;

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
