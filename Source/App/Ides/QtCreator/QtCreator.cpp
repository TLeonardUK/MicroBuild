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
#include "App/Ides/QtCreator/QtCreator.h"
#include "Core/Helpers/TextStream.h"

namespace MicroBuild {

Ide_QtCreator::Ide_QtCreator()
{
}

Ide_QtCreator::~Ide_QtCreator()
{
}

bool Ide_QtCreator::Generate(
    WorkspaceFile& workspaceFile,
    std::vector<ProjectFile>& projectFiles)
{
	UNUSED_PARAMETER(workspaceFile);
	UNUSED_PARAMETER(projectFiles);
	return false;
}

}; // namespace MicroBuild