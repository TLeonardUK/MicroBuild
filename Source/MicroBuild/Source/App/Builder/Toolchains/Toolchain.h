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

#include "Schemas/Project/ProjectFile.h"
#include "App/Builder/Tasks/BuildTask.h"
#include "App/Builder/BuilderFileInfo.h"

#include <string>
#include <memory>

namespace MicroBuild {
	
class BuildTask;

// Base class for all internal builder toolchains.
class Toolchain
{
protected:
	bool m_bAvailable;
	bool m_bRequiresCompileStep;
	bool m_bRequiresVersionInfo;
	std::string m_description;
	ProjectFile& m_projectFile;
	
	Platform::Path m_compilerPath;
	Platform::Path m_linkerPath;
	Platform::Path m_archiverPath;

	std::vector<Platform::Path> m_standardIncludePaths;
	std::vector<Platform::Path> m_standardLibraryPaths;

	uint64_t m_configurationHash;

protected:
	
	// Extracts dependencies from stdout capture and updates the entries in the manifest.
	void UpdateDependencyManifest(BuilderFileInfo& fileInfo, std::vector<Platform::Path>& dependencies, std::vector<BuilderFileInfo*> inherits);
	
	// Tries to find the fuill path to the library by searching project then system library folders.
	Platform::Path FindLibraryPath(const Platform::Path& path);
	
	// Gets the dependencies for a given source file, passes in the stdout incase dependencies are logged in it. 
	virtual void ExtractDependencies(const BuilderFileInfo& file, const std::string& input, std::string& rawInput, std::vector<Platform::Path>& dependencies);
	
	// Gets all the generic arguments required to compile a file.
	virtual void GetBaseCompileArguments(std::vector<std::string>& args);
	
	// Gets arguments to send to compiler for generating a pch.
	virtual void GetPchCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args);

	// Gets arguments to send to compiler for generating an object file.
	virtual void GetSourceCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args);
	
	// Gets arguments to send to linker for generating an executable file.
	virtual void GetLinkArguments(const std::vector<BuilderFileInfo>& sourceFiles, std::vector<std::string>& args);
	
	// Gets arguments to send to archiver for generating an library file.
	virtual void GetArchiveArguments(const std::vector<BuilderFileInfo>& sourceFiles, std::vector<std::string>& args);
	
public:

	// Constructor.
	Toolchain(ProjectFile& file, uint64_t configurationHash);

	// Returns a description that describes this toolchain and its version.
	std::string GetDescription();

	// Returns true if this toolchain is available for use.
	bool IsAvailable();

	// Returns true if we require an individual compile-call for each
	// source file, if not we jump straight to the Archive/Link step.
	bool RequiresCompileStep();

	// Returns true if we require an version info generation step.
	bool RequiresVersionInfo();

	// Gets all the build tasks required to buidl the current project.
	virtual std::vector<std::shared_ptr<BuildTask>> GetTasks(std::vector<BuilderFileInfo>& files, uint64_t configurationHash, BuilderFileInfo& outputFile);

	// Attempts to find the toolchain files, returns true if everything is available,	
	// otherwise false.
	virtual bool Init() = 0;

	// Compiles the PCH described in the project file.	
	virtual bool CompilePch(BuilderFileInfo& fileInfo);

	// Compiles the given file contained in the project.
	virtual bool Compile(BuilderFileInfo& fileInfo, BuilderFileInfo& pchFileInfo);
	
	// Compiles any files required to output version information.
	virtual bool CompileVersionInfo(BuilderFileInfo& fileInfo);

	// Archives all the source files provided into a single lib.
	virtual bool Archive(std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile);

	// Links all the source files provided into a sinmgle executable.
	virtual bool Link(std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile);

}; 

}; // namespace MicroBuild