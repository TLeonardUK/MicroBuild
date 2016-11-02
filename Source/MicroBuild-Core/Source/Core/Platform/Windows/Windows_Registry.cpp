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
#include "Core/Helpers/Memory.h"

#ifdef MB_PLATFORM_WINDOWS

#include <cstdlib>
#include <Windows.h>

namespace MicroBuild {
namespace Platform {	
namespace Registry {

bool GetRawValue(ERegistryHive Hive, Platform::Path Group, std::string KeyName, std::string& Output)
{
	HKEY hiveKey;

	switch (Hive)
	{
	case ERegistryHive::ClassesRoot:
	{
		hiveKey = HKEY_CLASSES_ROOT;
		break;
	}
	case ERegistryHive::CurrentConfig:
	{
		hiveKey = HKEY_CURRENT_CONFIG;
		break;
	}
	case ERegistryHive::CurrentUser:
	{
		hiveKey = HKEY_CURRENT_USER;
		break;
	}
	case ERegistryHive::LocalMachine:
	{
		hiveKey = HKEY_LOCAL_MACHINE;
		break;
	}
	case ERegistryHive::Users:
	{
		hiveKey = HKEY_USERS;
		break;
	}
	default:
		return false;
	}

	HKEY openedKey;
	if (RegOpenKey(hiveKey, Group.ToString().c_str(), &openedKey) != ERROR_SUCCESS)
	{
		return false;
	}

	DWORD keyType;
	DWORD keySize;

	LONG queryResult = RegQueryValueEx(openedKey, KeyName.c_str(), nullptr, &keyType, nullptr, &keySize);
	if (queryResult != ERROR_SUCCESS)
	{
		LONG closeResult = RegCloseKey(openedKey);
		assert(closeResult == ERROR_SUCCESS);
		return false;
	}

	std::vector<BYTE> keyData;
	keyData.resize(keySize + 1);

	queryResult = RegQueryValueEx(openedKey, KeyName.c_str(), nullptr, &keyType, keyData.data(), &keySize);
	if (queryResult != ERROR_SUCCESS)
	{
		LONG closeResult = RegCloseKey(openedKey);
		assert(closeResult == ERROR_SUCCESS);
		return false;
	}

	keyData[keyData.size() - 1] = '\0';

	LONG closeResult = RegCloseKey(openedKey);
	assert(closeResult == ERROR_SUCCESS);

	Output = "";

	switch (keyType)
	{
	case REG_EXPAND_SZ:
	case REG_MULTI_SZ:
	case REG_SZ:
	case REG_LINK:
	case REG_BINARY:
		{
			Output = std::string((char*)keyData.data(), keySize);
			break;
		}
	case REG_DWORD:
		{
			int value = *reinterpret_cast<int*>(keyData.data());
			Output = Strings::Format("%i", value);
			break;
		}
	case REG_DWORD_BIG_ENDIAN:
		{
			int value = *reinterpret_cast<int*>(keyData.data());
			Memory::SwapEndian<int>(value);
			Output = Strings::Format("%i", value);
			break;
		}
	case REG_QWORD:
		{
			Output = Strings::Format("%ll", *reinterpret_cast<long long*>(keyData.data()));
			break;
		}
	case REG_NONE:
		{
			Output = "";
			break;
		}
	default:
		return false;
	}

	return true;
}

bool KeyExists(ERegistryHive Hive, Platform::Path Group)
{	
	HKEY hiveKey;

	switch (Hive)
	{
	case ERegistryHive::ClassesRoot:
	{
		hiveKey = HKEY_CLASSES_ROOT;
		break;
	}
	case ERegistryHive::CurrentConfig:
	{
		hiveKey = HKEY_CURRENT_CONFIG;
		break;
	}
	case ERegistryHive::CurrentUser:
	{
		hiveKey = HKEY_CURRENT_USER;
		break;
	}
	case ERegistryHive::LocalMachine:
	{
		hiveKey = HKEY_LOCAL_MACHINE;
		break;
	}
	case ERegistryHive::Users:
	{
		hiveKey = HKEY_USERS;
		break;
	}
	default:
		return false;
	}

	HKEY openedKey;
	if (RegOpenKey(hiveKey, Group.ToString().c_str(), &openedKey) != ERROR_SUCCESS)
	{
		return false;
	}
	
	LONG closeResult = RegCloseKey(openedKey);
	assert(closeResult == ERROR_SUCCESS);

	return true;
}

}; // namespace Registry
}; // namespace Platform
}; // namespace MicroBuild

#endif