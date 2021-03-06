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

// Represents an installed instance of the .net framework.
struct DotNetFramework
{
	Platform::Path FrameworkPath;
	Platform::Path AssemblyPath;
};

// DotNet C# toolchain.
class Toolchain_DotNet
	: public Toolchain
{
private:

	std::string m_version;

protected:

	// Attempts to locate the toolchain on the users computer, returns true
	// if its found and available for use, otherwise false.
	bool FindToolchain();

	// Gets a map of available frameworks on the users system and the directory
	// they are contained in.
	std::map<EPlatformToolset, DotNetFramework> GetAvailableFrameworks();
	
	// Gets all the generic arguments required to compile a file.
	virtual void GetLinkArguments(const std::vector<BuilderFileInfo>& files, std::vector<std::string>& args) override;

public:
	Toolchain_DotNet(ProjectFile& file, uint64_t configurationHash);
	
	virtual bool Init() override;
	virtual void GetArchiveAction(BuildAction& action, std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile) override;
	virtual void GetLinkAction(BuildAction& action, std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile) override;

}; 

}; // namespace MicroBuild