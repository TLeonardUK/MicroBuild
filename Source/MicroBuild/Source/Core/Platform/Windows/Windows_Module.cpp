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
#include "Core/Platform/Module.h"
#include "Core/Helpers/Strings.h"

#ifdef MB_PLATFORM_WINDOWS

#include <Windows.h>

namespace MicroBuild {
namespace Platform {

struct Windows_Module
{
	HMODULE hModule;
};

Module::Module()
{
	m_impl = new Windows_Module();

	Windows_Module* data = reinterpret_cast<Windows_Module*>(m_impl);
	data->hModule = nullptr;
}

Module::~Module()
{
	if (IsOpen())
	{
		Close();
	}

	delete reinterpret_cast<Module*>(m_impl);
}


bool Module::IsOpen()
{
	Windows_Module* data = reinterpret_cast<Windows_Module*>(m_impl);
	return (data->hModule != nullptr);
}

bool Module::Open(const Path& command)
{
	Windows_Module* data = reinterpret_cast<Windows_Module*>(m_impl);
	assert(!IsOpen());

	data->hModule = LoadLibrary(command.ToString().c_str());
	if (!data->hModule)
	{
		Log(LogSeverity::Warning,
			"LoadLibrary failed with 0x%08x.\n", GetLastError());

		return false;
	}

	return true;
}

void Module::Close()
{
	Windows_Module* data = reinterpret_cast<Windows_Module*>(m_impl);
	assert(IsOpen());

	BOOL result = CloseHandle(data->hModule);
	if (!result)
	{
		Log(LogSeverity::Warning,
			"CloseHandle failed with 0x%08x.\n", GetLastError());
	}
	data->hModule = nullptr;
}

void* Module::GetSymbol(const std::string& name)
{
	Windows_Module* data = reinterpret_cast<Windows_Module*>(m_impl);
	assert(IsOpen());

	return (void*)GetProcAddress(data->hModule, name.c_str());
}

}; // namespace Platform
}; // namespace MicroBuild

#endif // MB_PLATFORM_WINDOWS