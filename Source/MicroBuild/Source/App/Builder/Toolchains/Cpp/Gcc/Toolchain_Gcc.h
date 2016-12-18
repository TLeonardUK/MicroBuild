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
#include "App/Builder/Toolchains/Cpp/Microsoft/Toolchain_Microsoft.h"

namespace MicroBuild {
	
// GCC C++ toolchain.
class Toolchain_Gcc
	: public Toolchain
{
protected:
	std::string m_version;

	bool m_useStartEndGroup;

	Platform::Path m_windowsResourceCompilerPath;

#if defined(MB_PLATFORM_WINDOWS)
	Toolchain_Microsoft m_microsoftToolchain;
#endif

protected:	
	
#if defined(MB_PLATFORM_WINDOWS)

	// Initializes our internal microsoft toolchain, we use this toolchain when on windows
	// for the resource compiler and the other tools that there are not equivilents of.
	bool InitMicrosoftToolchain();

#endif

	// Parse message information from the stdout.
	bool ParseMessageOutput(BuilderFileInfo& file, std::string& input);

	// Attempts to locate the toolchain on the users computer, returns true
	// if its found and available for use, otherwise false.
	virtual bool FindToolchain();
	
	// Gets all the generic arguments required to compile a file.
	virtual void GetBaseCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args) override;
	
	// Gets arguments to send to compiler for generating a pch.
	virtual void GetPchCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args) override;

	// Gets arguments to send to compiler for generating an object file.
	virtual void GetSourceCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args) override;
	
	// Gets arguments to send to linker for generating an executable file.
	virtual void GetLinkArguments(const std::vector<BuilderFileInfo>& sourceFiles, std::vector<std::string>& args) override;
	
	// Gets arguments to send to archiver for generating an library file.
	virtual void GetArchiveArguments(const std::vector<BuilderFileInfo>& sourceFiles, std::vector<std::string>& args) override;	
	
	// Splits an stdout capture int dependencies and raw data.
	virtual bool ParseOutput(BuilderFileInfo& file, std::string& output) override;

public:
	Toolchain_Gcc(ProjectFile& file, uint64_t configurationHash);
	
	static bool ParseDependencyFile(BuilderFileInfo& file, std::string& input);

	virtual bool Init() override;
	virtual bool CompileVersionInfo(BuilderFileInfo& fileInfo) override;

}; 

}; // namespace MicroBuild
