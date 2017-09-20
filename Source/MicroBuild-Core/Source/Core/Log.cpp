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
#include "Core/Log.h"
#include "Core/Helpers/Strings.h"
#include "Core/Platform/Platform.h"

#include <stdarg.h>

namespace MicroBuild {

bool g_logVerboseOn = false;
bool g_logSilentOn = false;

void LogSetVerbose(bool bVerbose)
{
	g_logVerboseOn = bVerbose;
}

bool LogGetVerbose()
{
	return g_logVerboseOn;
}

void LogSetSilent(bool bSilent)
{
	g_logSilentOn = bSilent;
}

bool LogGetSilent()
{
	return g_logSilentOn;
}

void Log(LogSeverity severity, const char* format, ...)
{
	if (severity == LogSeverity::Verbose && !g_logVerboseOn)
	{
		return;
	}
	if ((int)severity <= (int)LogSeverity::Info && g_logSilentOn)
	{
		return;
	}

	va_list list;
	va_start(list, format);
	std::string result = Strings::FormatVa(format, list);
	va_end(list);

	printf("%s", result.c_str());
	Platform::DebugOutput(result.c_str());

	// I'm assuming here we won't be doing this much, and we won't be logging 
	// incrementally, so this should be fine.
	fflush(stdout);
}

}; // namespace MicroBuild
