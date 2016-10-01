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
namespace Platform {

// Writes output to the platform specific debug output (vs output window etc).
void DebugOutput(const char* output);

// Gets the given environment variable based on its tag.
std::string GetEnvironmentVariable(const std::string& tag);

// Relaxes the CPU, mainly used for busy-waits.
void RelaxCpu();

// Gets the ideal number of threads that should be run in parallel to get
// maximum performance.
int GetConcurrencyFactor();

}; // namespace Platform
}; // namespace MicroBuild