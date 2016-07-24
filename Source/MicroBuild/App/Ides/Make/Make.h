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

// Permits generating of makefile workspaces.
class Ide_Make 
	: public IdeType
{
public:

	Ide_Make();
	~Ide_Make();

	virtual bool Generate(
		DatabaseFile& databaseFile,
		WorkspaceFile& workspaceFile,
		std::vector<ProjectFile>& projectFiles) override;

	virtual bool Clean(
		WorkspaceFile& workspaceFile) override;

	virtual bool Build(
		WorkspaceFile& workspaceFile,
		bool bRebuild,
		const std::string& configuration,
		const std::string& platform) override;

protected:

private:

};

}; // namespace MicroBuild