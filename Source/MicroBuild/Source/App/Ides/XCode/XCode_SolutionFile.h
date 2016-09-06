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

#include "App/Ides/IdeType.h"
#include "Core/Helpers/XmlNode.h"

namespace MicroBuild {

// Contains the code required to generate a XCode file.
class XCode_SolutionFile
{
public:

	XCode_SolutionFile();
	~XCode_SolutionFile();

	// Generates a basic solution file that links to the given
	// project files.
	bool Generate(
		DatabaseFile& databaseFile,
		WorkspaceFile& workspaceFile,
		std::vector<ProjectFile>& projectFiles,
		IdeHelper::BuildWorkspaceMatrix& buildMatrix
	);

private:

	// Recursively travels the project tree and creates the 
	// xml nodes for each project/file.
	void BuildProjectTree(
		XmlNode& rootNode,
		const std::string& rootPath, 
		std::vector<IdeHelper::ProjectGroupFolder>& folders,
		std::vector<IdeHelper::VPathPair>& files,
		Platform::Path& solutionDirectory,
        std::vector<ProjectFile>& projectFiles
	);

};

}; // namespace MicroBuild