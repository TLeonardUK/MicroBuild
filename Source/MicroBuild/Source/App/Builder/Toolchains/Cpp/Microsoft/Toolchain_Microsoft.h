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

#include "Core/Platform/Process.h"

namespace MicroBuild {
	
// Microsoft C++ toolchain.
class Toolchain_Microsoft
	: public Toolchain
{
private:

	Platform::Path m_envVarBatchFilePath;
	std::string m_version;

	Platform::Path m_resourceCompilerPath;

	bool m_bUseDefaultToolchain;

protected:

	// Attempts to locate the toolchain on the users computer, returns true
	// if its found and available for use, otherwise false.
	bool FindToolchain();

	// Called by FindToolchain to find the locations of specific
	// versions of the toolchain.
	bool FindV140Toolchain();
	bool FindV141Toolchain();

	// Parse dependency information from the stdout.
	bool ParseDependencyOutput(BuilderFileInfo& file, std::string& input);

	// Parse message information from the stdout.
	bool ParseMessageOutput(BuilderFileInfo& file, std::string& input);

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
	Toolchain_Microsoft(ProjectFile& file, uint64_t configurationHash, bool bUseDefaultToolchain = false);
	
	virtual bool Init() override;
	virtual bool Archive(std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile) override;
	virtual bool Link(std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile) override;
	virtual bool CompileVersionInfo(BuilderFileInfo& fileInfo) override;

	bool CreateVersionInfoScript(Platform::Path iconPath, Platform::Path rcScriptPath);

}; 

}; // namespace MicroBuild
