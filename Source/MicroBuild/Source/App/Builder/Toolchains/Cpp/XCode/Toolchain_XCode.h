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

#include "App/Builder/Toolchains/Toolchain.h"
#include "App/Builder/Toolchains/Cpp/Clang/Toolchain_Clang.h"

namespace MicroBuild {
	
// XCode C++ toolchain, a wierd offshoot of clang with some extra linking steps. 
class Toolchain_XCode
	: public Toolchain_Clang
{
protected:
	std::string m_version;
	Platform::Path m_isysRoot;

protected:	
	
	// Attempts to locate the toolchain on the users computer, returns true
	// if its found and available for use, otherwise false.
	virtual bool FindToolchain() override;

	// Attempts to find an executable thats part of the xcode toolchain. Returns true
	// if found, and available for use, otherwise false.
	bool FindXCodeExe(const std::string& exeName, Platform::Path& output);
	
	// Gets all the generic arguments required to compile a file.
	virtual void GetBaseCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args) override;
	
public:
	Toolchain_XCode(ProjectFile& file, uint64_t configurationHash);

	virtual bool Init() override;

}; 

}; // namespace MicroBuild
