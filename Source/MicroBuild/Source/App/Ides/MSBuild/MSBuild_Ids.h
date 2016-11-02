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

#include "App/Ides/IdeType.h"

namespace MicroBuild {
namespace MSBuild {

// Universal app guid.
extern const char* k_GuiUapProject;

// CSharp project guid.
extern const char* k_GuidCSharpProject;

// CPP project guid.
extern const char* k_GuidCppProject;

// Solution folder guid.
extern const char* k_GuidSolutionFolder;

// Converts a platform enum into an msbuild platform id.
std::string GetPlatformID(EPlatform platform);

// Converts a platform enum into a dotnet target id.
std::string GetPlatformDotNetTarget(EPlatform platform);

// Finds the correct project type guid based on language.
std::string GetProjectTypeGuid(ELanguage language, bool bUsingInternalBuildTool = false);

}; // namespace MSBuild
}; // namespace MicroBuild