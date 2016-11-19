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
#include "Schemas/Project/ProjectFile.h"
#include "App/Ides/MSBuild/MSBuild.h"
#include "App/Ides/MSBuild/MSBuild_Ids.h"
#include "App/Ides/MSBuild/MSBuild_SolutionFile.h"
#include "App/Ides/MSBuild/MSBuild_VcxFiltersFile.h"
#include "App/Ides/MSBuild/MSBuild_ProjectFile.h"
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
		MSBuild_ProjectFile ibtFile(
			m_defaultToolset,
			m_defaultToolsetString
		);

		if (!ibtFile.Generate(
			databaseFile,
			workspaceFile,
			file,
			matrix[index]))
		{
			return false;
		}

		index++;
    }

	MSBuild_SolutionFile slnFile(
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

}; // namespace MicroBuild
