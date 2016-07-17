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
#include "Core/Templates/TemplateEvaluator.h"

namespace MicroBuild {

// Permits generating of monodevelop workspaces.
class Ide_MonoDevelop
	: public IdeType
{
public:

	Ide_MonoDevelop();
	~Ide_MonoDevelop();

	virtual bool Generate(
		DatabaseFile& databaseFile,
		WorkspaceFile& workspaceFile,
		std::vector<ProjectFile>& projectFiles) override;

protected:

private:

};

}; // namespace MicroBuild