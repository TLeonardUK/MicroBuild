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
#include "App/Builder/Toolchains/Toolchain_Clang.h"

namespace MicroBuild {

Toolchain_Clang::Toolchain_Clang(ProjectFile& file)
	: Toolchain(file)
{
}

bool Toolchain_Clang::CompilePch(BuilderFileInfo& fileInfo) 
{
	MB_UNUSED_PARAMETER(fileInfo);
	// todo
	return false;
}

bool Toolchain_Clang::Compile(BuilderFileInfo& fileInfo, BuilderFileInfo& pchFileInfo)
{
	MB_UNUSED_PARAMETER(fileInfo);
	MB_UNUSED_PARAMETER(pchFileInfo);
	// todo
	return false;
}

bool Toolchain_Clang::Archive(std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile)
{
	MB_UNUSED_PARAMETER(files);
	MB_UNUSED_PARAMETER(outputFile);
	// todo
	return false;
}

bool Toolchain_Clang::Link(std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile)
{
	MB_UNUSED_PARAMETER(files);
	MB_UNUSED_PARAMETER(outputFile);
	// todo
	return false;
}

}; // namespace MicroBuild