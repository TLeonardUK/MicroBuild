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

// todo: emit this info somewhere obvious:
//
// Supports builiding:
//		Platforms:		x86, x64, Arm, Arm64, AnyCPU
//		SubSystems:		Native / WinRT

// todo: Break this file up, its a massive clusterfuck at the moment,
//		 there is a bunch of code that should be easy to break out
//		 into more generic modules. Definitely make platform support
//		 independent.

#include "PCH.h"
#include "App/Project/ProjectFile.h"
#include "App/Ides/MSBuild/MSBuild.h"
#include "App/Ides/MSBuild/MSBuild_Ids.h"
#include "App/Ides/MSBuild/MSBuild_CsProjectFile.h"
#include "App/Ides/MSBuild/MSBuild_SlnSolutionFile.h"
#include "App/Ides/MSBuild/MSBuild_VcxFiltersFile.h"
#include "App/Ides/MSBuild/MSBuild_VcxProjectFile.h"
#include "App/Ides/MSBuild/Versions/VisualStudio_2015.h"
#include "Core/Helpers/TextStream.h"
#include "Core/Helpers/XmlNode.h"
#include "Core/Helpers/JsonNode.h"
#include "Core/Helpers/VbNode.h"
#include "Core/Platform/Process.h"

namespace MicroBuild {

Ide_MSBuild::Ide_MSBuild()
    : m_msBuildVersion(MSBuildVersion::Version12)
	, m_defaultToolset(EPlatformToolset::Default)
{
}

Ide_MSBuild::~Ide_MSBuild()
{
}

void Ide_MSBuild::SetMSBuildVersion(MSBuildVersion version)
{
    m_msBuildVersion = version;
}

void Ide_MSBuild::SetHeaderShortName(const std::string& value)
{
	m_headerShortName = value;
}

void Ide_MSBuild::SetHeaderVersion(const std::string& value)
{
	m_headerVersion = value;
}

void Ide_MSBuild::SetDefaultToolset(EPlatformToolset toolset)
{
	m_defaultToolset = toolset;
}

void Ide_MSBuild::SetDefaultToolsetString(const std::string& toolset)
{
	m_defaultToolsetString = toolset;
}

bool Ide_MSBuild::Generate(
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
				MSBuild_VcxProjectFile vcxFile(
					m_defaultToolset, 
					m_defaultToolsetString
				);

				if (!vcxFile.Generate(
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
				MSBuild_CsProjectFile csFile(
					m_defaultToolsetString
				);

				if (!csFile.Generate(
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
					"Language '%s' is not valid for msbuild projects.",
					CastToString(file.Get_Project_Language()).c_str());
				return false;
			}
		}

		index++;
    	}

	MSBuild_SlnSolutionFile slnFile(
		m_headerVersion, 
		m_headerShortName
	);

    if (!slnFile.Generate(
		databaseFile,
        	workspaceFile,
        	projectFiles,
		matrix
        ))
    {
        return false;
    }

    return true;
}

bool Ide_MSBuild::Clean(
    WorkspaceFile& workspaceFile,
    DatabaseFile& databaseFile)
{
    MB_UNUSED_PARAMETER(databaseFile);

	Platform::Path solutionDirectory =
		workspaceFile.Get_Workspace_Location();

	Platform::Path solutionLocation =
		solutionDirectory.AppendFragment(
			workspaceFile.Get_Workspace_Name() + ".sln", true);

	std::vector<std::string> arguments;
	arguments.push_back(solutionLocation.ToString());
	arguments.push_back("/t:Clean");

	Platform::Process process;
	if (process.Open(GetMSBuildLocation(), solutionDirectory, arguments, false))
	{
		process.Wait();

		int exitCode = process.GetExitCode();
		if (exitCode == 0)
		{
			return true;
		}
		else
		{
			Log(LogSeverity::Fatal, "msbuild failed with exit code %i.\n", exitCode);
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

bool Ide_MSBuild::Build(
	WorkspaceFile& workspaceFile,
	bool bRebuild,
	const std::string& configuration,
	const std::string& platform,
    DatabaseFile& databaseFile)
{
    MB_UNUSED_PARAMETER(databaseFile);

	Platform::Path solutionDirectory =
		workspaceFile.Get_Workspace_Location();

	Platform::Path solutionLocation =
		solutionDirectory.AppendFragment(
			workspaceFile.Get_Workspace_Name() + ".sln", true);

	std::vector<std::string> arguments;
	arguments.push_back(solutionLocation.ToString());

	EPlatform platformId = CastFromString<EPlatform>(platform);

	if (bRebuild)
	{
		arguments.push_back("/t:Rebuild");
	}
	else
	{
		arguments.push_back("/t:Build");
	}

	arguments.push_back(
		Strings::Format("/p:Configuration=%s", configuration.c_str())
	);

	arguments.push_back(
		Strings::Format("/p:Platform=%s", MSBuild::GetPlatformDotNetTarget(platformId).c_str())
	);

	Platform::Process process;
	if (process.Open(GetMSBuildLocation(), solutionDirectory, arguments, false))
	{
		process.Wait();

		int exitCode = process.GetExitCode();
		if (exitCode == 0)
		{
			return true;
		}
		else
		{
			Log(LogSeverity::Fatal, "msbuild failed with exit code %i.\n", exitCode);
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

}; // namespace MicroBuild
