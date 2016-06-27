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
#include "Core/Platform/Path.h"

#ifdef MB_PLATFORM_LINUX

#include <Windows.h>

namespace MicroBuild {
namespace Platform {

char Path::Seperator = '/';

std::vector<Path> Path::GetFiles() const
{
	// todo
}

std::vector<Path> Path::GetDirectories() const
{
	// todo
}

bool Path::IsFile() const
{
	// todo
}

bool Path::IsDirectory() const
{
	// todo
}

bool Path::CreateAsDirectory() const
{
	// todo
}

bool Path::CreateAsFile() const
{
	// todo
}

bool Path::Exists() const
{
	// todo
}

bool Path::Copy(const Path& Destination) const
{
	// todo
}

bool Path::Delete() const
{
	// todo
}

std::time_t Path::GetModifiedTime() const
{
	// todo
}

}; // namespace Platform
}; // namespace MicroBuild

#endif // MB_PLATFORM_LINUX