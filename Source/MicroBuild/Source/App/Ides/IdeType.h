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

#include "App/Database/DatabaseFile.h"
#include "App/Workspace/WorkspaceFile.h"
#include "App/Project/ProjectFile.h"
#include "Core/Helpers/TextStream.h"
#include "App/Ides/IdeHelper.h"

namespace MicroBuild {

// Base class for all IDE targets.
class IdeType
{
public:

	IdeType();
	virtual ~IdeType();

	// Gets the short-named use on the command line and in config files
	// to refer to this IDE.
	std::string GetShortName();

	// Takes a fully resolved workspace file and generates the project
	// files it defines.
	virtual bool Generate(
		DatabaseFile& databaseFile,
		WorkspaceFile& workspaceFile, 
		std::vector<ProjectFile>& projectFiles) = 0;

	// Requests that the ide cleans up any intermediate files. This is
	// an optional implementation as some targets do not support cleaning. 
	virtual bool Clean(
		WorkspaceFile& workspaceFile,
        DatabaseFile& databaseFile);

	// Rebuilds a workspace file that has previously been generated. This is
	// an optional implementation as some targets do not support command
	// line building.
	virtual bool Build(
		WorkspaceFile& workspaceFile,
		bool bRebuild,
		const std::string& configuration,
		const std::string& platform,
        DatabaseFile& databaseFile
	);

protected:

	void SetShortName(const std::string& value);

private:

	std::string m_shortName;

};

}; // namespace MicroBuild