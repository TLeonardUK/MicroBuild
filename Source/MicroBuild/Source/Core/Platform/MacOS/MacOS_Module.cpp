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

#ifdef MB_PLATFORM_MACOS

#define _GNU_SOURCE
#include <dlfcn.h>

namespace MicroBuild {
namespace Platform {

struct MacOS_Module
{
	void* hModule;
};

Module::Module()
{
	m_impl = new MacOS_Module();

	MacOS_Module* data = reinterpret_cast<MacOS_Module*>(m_impl);
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
	MacOS_Module* data = reinterpret_cast<MacOS_Module*>(m_impl);
	return (data->hModule != nullptr);
}

bool Module::Open(const Path& command)
{
	MacOS_Module* data = reinterpret_cast<MacOS_Module*>(m_impl);
	assert(!IsOpen());

	data->hModule = dlopen(command.ToString().c_str(), RTLD_LAZY);
	if (!data->hModule)
	{
		Log(LogSeverity::Warning, "dlopen failed with %s.\n", dlerror());
		return false;
	}

	return true;
}

void Module::Close()
{
	MacOS_Module* data = reinterpret_cast<MacOS_Module*>(m_impl);
	assert(IsOpen());

	int result = dlclose(data->hModule);
	if (result != 0)
	{
		Log(LogSeverity::Warning, "dlclose failed with %s.\n", dlerror());
	}
	data->hModule = nullptr;
}

void* Module::GetSymbol(const std::string& name)
{
	MacOS_Module* data = reinterpret_cast<MacOS_Module*>(m_impl);
	assert(IsOpen());

	return (void*)dlsym(data->hModule, name.c_str());
}

}; // namespace Platform
}; // namespace MicroBuild

#endif // MB_PLATFORM_LINUX