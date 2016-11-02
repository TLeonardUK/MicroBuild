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
#include "Core/Platform/Platform.h"

#ifdef MB_PLATFORM_WINDOWS

#include <Windows.h>

namespace MicroBuild {
namespace Platform {

void DebugOutput(const char* output)
{
	OutputDebugStringA(output);
}

void RelaxCpu()
{
	_mm_pause();
}

int GetConcurrencyFactor()
{
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);

	return sysinfo.dwNumberOfProcessors;
}

bool IsOperatingSystem64Bit()
{
#if defined(MB_ARCHITECTURE_X64)
	return true;
#else
	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	static LPFN_ISWOW64PROCESS Function_IsWow64Process;
	static bool bCachedResult = false;
	static BOOL bResult = false;

	if (!bCachedResult)
	{
		Function_IsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
			GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

		if (Function_IsWow64Process)
		{
			if (!Function_IsWow64Process(GetCurrentProcess(), &bResult))
			{
				bResult = false;
			}
		}
		else
		{
			bResult = false;
		}

		bCachedResult = true;
	}

	return !!bResult;
#endif
}

#undef SetEnvironmentVariable
void SetEnvironmentVariable(const std::string& tag, const std::string& value)
{
	::SetEnvironmentVariableA(tag.c_str(), value.c_str());
}

}; // namespace Platform
}; // namespace MicroBuild

#endif // MB_PLATFORM_WINDOWS