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
#include "App/Ides/IdeType.h"

namespace MicroBuild {

IdeType::IdeType()
	: m_shortName("")
{
}

IdeType::~IdeType()
{
}

std::string IdeType::GetShortName()
{
	return m_shortName;
}

void IdeType::SetShortName(const std::string& value)
{
	m_shortName = value;
}


bool IdeType::Clean(WorkspaceFile& workspaceFile,
    DatabaseFile& databaseFile)
{
	MB_UNUSED_PARAMETER(workspaceFile);
	MB_UNUSED_PARAMETER(databaseFile);

	// Unimplemented.

	return true;
}

bool IdeType::Build(
	WorkspaceFile& workspaceFile,
	bool bRebuild,
	const std::string& configuration,
	const std::string& platform,
    DatabaseFile& databaseFile
)
{
	MB_UNUSED_PARAMETER(workspaceFile);
	MB_UNUSED_PARAMETER(bRebuild);
	MB_UNUSED_PARAMETER(configuration);
	MB_UNUSED_PARAMETER(platform);
	MB_UNUSED_PARAMETER(databaseFile);

	// Unimplemented.

	return true;
}

} // namespace MicroBuild