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

namespace MicroBuild {

// Severity of the log message, determines color and what priority the log 
// has for printing (based on -verbose flag). 
enum class LogSeverity
{
	Verbose,
	Info,
	SilentInfo,
	Fatal,
	Warning,
	Success
};

// Toggles verbose logging, if on all LogSeverity::Verbose entries will be 
// dumped, otherwise they will be omitted.
void LogSetVerbose(bool bVerbose);
bool LogGetVerbose();

// Toggles silent logging, all but errors will be supressed.
void LogSetSilent(bool bSilent);
bool LogGetSilent();

// Writes a log to stdout in the same style as printf. Seveirty determines
// if it will be printed (based on -verbose flag) and what color it will be
// printed in.
void Log(LogSeverity severity, const char* format, ...);

}; // namespace MicroBuild