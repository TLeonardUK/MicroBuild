/*
Ludo Game Engine
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
#include "Core/Platform/Registry.h"

#ifdef MB_PLATFORM_MACOS

namespace MicroBuild {
namespace Platform {	
namespace Registry {

bool GetRawValue(ERegistryHive Hive, Platform::Path Group, std::string KeyName, std::string& Output)
{
	MB_UNUSED_PARAMETER(Hive);
	MB_UNUSED_PARAMETER(Group);
	MB_UNUSED_PARAMETER(KeyName);
	MB_UNUSED_PARAMETER(Output);

	// Registry does not exist on macos, abort!

	return false;
}

bool KeyExists(ERegistryHive Hive, Platform::Path Group)
{
	MB_UNUSED_PARAMETER(Hive);
	MB_UNUSED_PARAMETER(Group);

	// Registry does not exist on macos, abort!

	return false;
}

}; // namespace Registry
}; // namespace Platform
}; // namespace MicroBuild

#endif