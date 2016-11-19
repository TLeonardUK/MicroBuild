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
#include "App/Ides/XCode/XCode_ProjectFile.h"
#include "App/Ides/XCode/XCode_SolutionFile.h"
#include "App/Ides/XCode/XCode_SchemaFile.h"
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
		XCode_ProjectFile projectFile;

		if (!projectFile.Generate(
			databaseFile,
			workspaceFile,
			file,
            projectFiles,
			matrix[index]))
		{
			return false;
		}

        XCode_SchemaFile schemaFile;

        if (!schemaFile.Generate(
            databaseFile,
            workspaceFile,
            file,
            projectFiles,
            matrix[index]))
        {
            return false;
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
