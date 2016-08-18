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
	WorkspaceFile& workspaceFile) 
{
	UNUSED_PARAMETER(workspaceFile);

	// TODO

	return false;
}

bool Ide_XCode::Build(
	WorkspaceFile& workspaceFile,
	bool bRebuild,
	const std::string& configuration,
	const std::string& platform) 
{
	UNUSED_PARAMETER(workspaceFile);
	UNUSED_PARAMETER(bRebuild);
	UNUSED_PARAMETER(configuration);
	UNUSED_PARAMETER(platform);

	// TODO

	return false;
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
