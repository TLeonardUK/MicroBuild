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
#include "App/Workspace/WorkspaceFile.h"

namespace MicroBuild {

WorkspaceFile::WorkspaceFile()
{
}

WorkspaceFile::~WorkspaceFile()
{
}

std::vector<std::string> WorkspaceFile::GetConfigurations()
{
	return GetValues("Configurations", "Configuration");
}

std::vector<std::string> WorkspaceFile::GetPlatform()
{
	return GetValues("Platforms", "Platform");
}

std::vector<Platform::Path> WorkspaceFile::GetProjectPaths()
{
	std::vector<std::string> values = GetValues("Projects", "Project");
	std::vector<Platform::Path> result;
	for (std::string val : values)
	{
		Platform::Path path = val;

		std::vector<Platform::Path> matches = 
			Platform::Path::MatchFilter(ResolvePath(path));

		result.insert(result.begin(), matches.begin(), matches.end()); 
	}
	return result;	 
}

Platform::Path WorkspaceFile::ResolvePath(Platform::Path& value)
{
	if (value.IsAbsolute())
	{
		return value;
	}
	else
	{
		return GetPath() + std::string(1, Platform::Path::Seperator) + value;
	}
}

void WorkspaceFile::Resolve()
{
	SetOrAddValue("Workspace", "Directory", GetPath().GetDirectory().ToString());
	SetOrAddValue("Workspace", "File", GetPath().ToString());

	ConfigFile::Resolve();
}

}; // namespace MicroBuild