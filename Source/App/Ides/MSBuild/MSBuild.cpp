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

#pragma once

#include "PCH.h"
#include "App/Ides/MSBuild/MSBuild.h"
#include "App/Ides/MSBuild/VisualStudio_2015.h"
#include "Core/Templates/TemplateEvaluator.h"
#include "Core/Helpers/TextStream.h"

namespace MicroBuild {

const char* MSBuildVersion_SolutionTemplates[static_cast<int>(MSBuildVersion::COUNT)] = {
	#include "App/Ides/MSBuild/Templates/MSBuild_Solution_V12.template"
};

const char* MSBuildVersion_ProjectTemplates[static_cast<int>(MSBuildVersion::COUNT)] = {
	#include "App/Ides/MSBuild/Templates/MSBuild_Project_V12.template"
};

Ide_MSBuild::Ide_MSBuild()
	: m_msBuildVersion(MSBuildVersion::Version12)
{
}

Ide_MSBuild::~Ide_MSBuild()
{
}

void Ide_MSBuild::SetMSBuildVersion(MSBuildVersion version)
{
	m_msBuildVersion = version;
}

std::string Ide_MSBuild::GetPlatformID(EPlatform platform)
{
	switch (platform)
	{
	case EPlatform::Windows_x86:
		{
			return "Win32";
		}
	case EPlatform::Windows_x64:
		{
			return "x64";
		}
	case EPlatform::Windows_AnyCPU:
		{
			return "Any CPU";
		}
	}
	return "";
}

std::string Ide_MSBuild::GetProjectGUID(
	const std::string& workspaceName, 
	const std::string& projectName)
{
	// To match VS we would want to generate a unique GUID each time the 
	// project files are re-created, but that seems bleh. So I'm just going to
	// make a fairly simple deterministic-guid that shouldn't change between rusn.
	unsigned int workspaceHash = Strings::Hash(workspaceName);
	unsigned int projectHash = Strings::Hash(projectName);

 	return Strings::Format("{%08X-1234-5678-9101-1121%08X}",
		workspaceHash,
		projectHash);
}

bool Ide_MSBuild::GenerateMSBuildSolutionFile(
	WorkspaceFile& workspaceFile,
	std::vector<ProjectFile>& projectFiles
	)
{
	Platform::Path solutionDirectory = workspaceFile.Get_Workspace_Location();
	Platform::Path solutionLocation = 
		solutionDirectory.AppendFragment(
			workspaceFile.Get_Workspace_Name() + ".sln");

	std::vector<std::string> configurations = 
		workspaceFile.Get_Configurations_Configuration();

	std::vector<EPlatform> platforms =
		workspaceFile.Get_Platforms_Platform();

	std::vector<std::string> platformIdentifiers;

	// Convert platform enum to msbuild platform ids.
	for (EPlatform platform : platforms)
	{
		std::string platformString = GetPlatformID(platform);
		if (platformString == "")
		{
			StringCast(platform, platformString);

			workspaceFile.ValidateError(
				"Platform '%s' is not a valid platform for an msbuild project.",
				platformString);
		
			return false;
		}
		platformIdentifiers.push_back(platformString);
	}

	// Generate some GUID's for each project.
	std::vector<std::string> projectGuids;
	for (ProjectFile& project : projectFiles)
	{
		std::string guid = GetProjectGUID(
			workspaceFile.Get_Workspace_Name(), project.Get_Project_Name());
		projectGuids.push_back(guid);
	}

	// Fill evaluator will all variables required to build solution.
	TemplateEvaluator evaluator;

	int projectsIndex = evaluator.AddArgument("projects", "");
	for (unsigned int i = 0; i < projectFiles.size(); i++)
	{
		ProjectFile& project = projectFiles[i];
		int itemIndex = evaluator.AddArgument("", "", projectsIndex);
		evaluator.AddArgument("guid", projectGuids[i], itemIndex);
		evaluator.AddArgument("name", project.Get_Project_Name(), itemIndex);
		evaluator.AddArgument("location", project.Get_Project_Location().ToString(), itemIndex);
		evaluator.AddArgument("parentGuid","", itemIndex);
	}

	int configurationsIndex = evaluator.AddArgument("configurations", "");
	for (unsigned int i = 0; i < configurations.size(); i++)
	{
		int itemIndex = evaluator.AddArgument("", "", configurationsIndex);
		evaluator.AddArgument("id", configurations[i], itemIndex);
	}

	int platformsIndex = evaluator.AddArgument("platforms", "");
	for (unsigned int i = 0; i < platforms.size(); i++)
	{
		int itemIndex = evaluator.AddArgument("", "", platformsIndex);
		evaluator.AddArgument("id", platformIdentifiers[i], itemIndex);
	}

	int nestedProjectsIndex = evaluator.AddArgument("nestedProjects", "");
	for (unsigned int i = 0; i < configurations.size(); i++)
	{
		//int itemIndex = evaluator.AddArgument("", "", nestedProjectsIndex);
	//	evaluator.AddArgument("parentGuid", "", itemIndex);
	//	evaluator.AddArgument("guid", "", itemIndex);
	}

	evaluator.AddArgument("hideSolutionNode", "TRUE");

	// Generate the solution for this project.
	if (!evaluator.Evaluate(
			solutionLocation.ToString(),
			MSBuildVersion_SolutionTemplates[static_cast<int>(m_msBuildVersion)]))
	{
		workspaceFile.ValidateError(
			"Internal error, msbuild solution template appears to be broken. "
			"Tell a developer if possible!");
	}

	// Dump evaluation result out to file.
	TextStream stream;
	stream.Write("%s", evaluator.GetResult().c_str());

	// Ensure location directory is created.
	if (!solutionDirectory.Exists())
	{
		if (!solutionDirectory.CreateAsDirectory())
		{
			Log(LogSeverity::Fatal, 
				"Failed to create workspace directory '%s'.\n",
				solutionDirectory.ToString().c_str());

			return false;
		}
	}

	// Write the solution file to the location.
	if (!stream.WriteToFile(solutionLocation))
	{
		Log(LogSeverity::Fatal,
			"Failed to write workspace to file '%s'.\n",
			solutionLocation.ToString().c_str());

		return false;
	}

	return true;
}

}; // namespace MicroBuild