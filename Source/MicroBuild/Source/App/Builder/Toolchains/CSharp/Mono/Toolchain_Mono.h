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

namespace MicroBuild {
	
// Mono C# toolchain.
class Toolchain_Mono
	: public Toolchain
{
private:

	std::string m_version;

protected:

	// Attempts to locate the toolchain on the users computer, returns true
	// if its found and available for use, otherwise false.
	bool FindToolchain();

	// Gets all the generic arguments required to compile a file.
	virtual void GetLinkArguments(const std::vector<BuilderFileInfo>& files, std::vector<std::string>& args) override;

public:
	Toolchain_Mono(ProjectFile& file, uint64_t configurationHash);
	
	virtual bool Init() override;
	virtual bool Archive(std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile) override;
	virtual bool Link(std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile) override;

}; 

}; // namespace MicroBuild