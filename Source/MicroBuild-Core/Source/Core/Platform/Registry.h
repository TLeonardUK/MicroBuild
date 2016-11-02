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

#include "Core/Platform/Path.h"
#include "Core/Helpers/Strings.h"
#include "Core/Helpers/StringConverter.h"

namespace MicroBuild {
namespace Platform {

// What area of the registry to extract data from.
enum ERegistryHive
{
	ClassesRoot,
	CurrentConfig,
	CurrentUser,
	LocalMachine,
	Users
};

namespace Registry {

// Gets a value from the registry and attempts to cast it to the given type.
// Returns true if the value was successfully retrieved.
bool GetRawValue(ERegistryHive Hive, Platform::Path Key, std::string ValueName, std::string& Output);

// Gets a value from the registry and attempts to cast it to the given type.
// Returns true if the value was successfully retrieved.
template<typename ResultType>
bool GetValue(ERegistryHive Hive, Platform::Path Key, std::string ValueName, ResultType& Output)
{
	std::string RawValue;
	if (!GetRawValue(Hive, Key, ValueName, RawValue))
	{
		return false;
	}

	Output = CastFromString<ResultType>(RawValue);
	return true;
}

// Returns true if the given key exists.
bool KeyExists(ERegistryHive Hive, Platform::Path Key);

}; // namespace Registry
}; // namespace Platform
}; // namespace MicroBuild