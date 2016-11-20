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
#include "App/Builder/Toolchains/Cpp/Gcc/Toolchain_Gcc.h"

namespace MicroBuild {
	
// Clang C++ toolchain.
class Toolchain_Clang
	: public Toolchain_Gcc
{
protected:
	std::string m_version;

protected:	
	
	// Attempts to locate the toolchain on the users computer, returns true
	// if its found and available for use, otherwise false.
	virtual bool FindToolchain() override;
	
public:
	Toolchain_Clang(ProjectFile& file, uint64_t configurationHash);

	virtual bool Init() override;
	virtual bool CompileVersionInfo(BuilderFileInfo& fileInfo) override;

}; 

}; // namespace MicroBuild