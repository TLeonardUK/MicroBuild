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

namespace MicroBuild {

// Contains the code required to generate a Makefile file.
class Make_ProjectFile
{
public:

	Make_ProjectFile();
	~Make_ProjectFile();

	// Generates a basic msbuild solution file that links to the given
	// project files.
	bool Generate(
		DatabaseFile& databaseFile,
		WorkspaceFile& workspaceFile,
		ProjectFile& projectFile,
		IdeHelper::BuildProjectMatrix& buildMatrix
	);

private:

	void WriteCommands(
		TextStream& stream,
		const std::vector<std::string>& commands
	);

};

}; // namespace MicroBuild