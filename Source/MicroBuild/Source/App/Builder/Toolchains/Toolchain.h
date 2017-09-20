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
#include "Core/Platform/Process.h"

#include <string>
#include <memory>

namespace MicroBuild {
	
class BuildTask;

// Stores some general version number information that the builder embeds in the output file.
struct VersionNumberInfo
{
	std::string Changelist;
	std::string ShortString;
	std::time_t LastChangeTime;
	int			TotalChangelists;
};

// Base class for all internal builder toolchains.
class Toolchain
{
protected:
	bool m_bAvailable;
	bool m_bCanDistribute;
	bool m_bRequiresCompileStep;
	bool m_bRequiresVersionInfo;
	bool m_bGeneratesPchObject;
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
	
	// Gets all the generic arguments required to compile a file.
	virtual void GetBaseCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args);
	
	// Gets arguments to send to compiler for generating a pch.
	virtual void GetPchCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args);

	// Gets arguments to send to compiler for generating an object file.
	virtual void GetSourceCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args);
	
	// Gets arguments to send to linker for generating an executable file.
	virtual void GetLinkArguments(const std::vector<BuilderFileInfo>& sourceFiles, std::vector<std::string>& args);
	
	// Gets arguments to send to archiver for generating an library file.
	virtual void GetArchiveArguments(const std::vector<BuilderFileInfo>& sourceFiles, std::vector<std::string>& args);

	// Updates the output files dependencies based on a linking operation.
	virtual void UpdateLinkDependencies(const std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile);

	// Writes the arguments into a response file and starts the process using the response file.
	bool OpenResponseFileProcess(Platform::Process& process, const Platform::Path& responseFilePath, const Platform::Path& exePath, const Platform::Path& workingDir, const std::vector<std::string>& arguments, bool bRedirectStdout = false);

	// Writes the arguments into a response file and starts the process using the response file.
	bool GetResponseFileAction(BuildAction& action, const Platform::Path& responseFilePath, const Platform::Path& exePath, const Platform::Path& workingDir, const std::vector<std::string>& arguments, bool bRedirectStdout = false);
	
public:

	// Constructor.
	Toolchain(ProjectFile& file, uint64_t configurationHash);

	// Overrides the project info  this toolchain was originally constructed with, useful so you can recycle toolchain instances and avoid
	// some of the initialization cost.
	void SetProjectInfo(ProjectFile& file, uint64_t configurationHash);

	// Returns a description that describes this toolchain and its version.
	std::string GetDescription();

	// Returns true if this toolchain is available for use.
	bool IsAvailable();

	// If true the task can be distributed to other computers using a build accelerator.
	bool CanDistribute();

	// Returns true if we require an individual compile-call for each
	// source file, if not we jump straight to the Archive/Link step.
	bool RequiresCompileStep();

	// Returns true if we require an version info generation step.
	bool RequiresVersionInfo();

	// Gets all the build tasks required to buidl the current project.
	virtual std::vector<std::shared_ptr<BuildTask>> GetTasks(std::vector<BuilderFileInfo>& files, uint64_t configurationHash, BuilderFileInfo& outputFile, VersionNumberInfo& versionInfo);

	// Attempts to find the toolchain files, returns true if everything is available,	
	// otherwise false.
	virtual bool Init() = 0;

	// Compiles the PCH described in the project file.	
	virtual void GetCompilePchAction(BuildAction& action, BuilderFileInfo& fileInfo);

	// Compiles the given file contained in the project.
	virtual void GetCompileAction(BuildAction& action, BuilderFileInfo& fileInfo, BuilderFileInfo& pchFileInfo);
	
	// Compiles any files required to output version information.
	virtual void GetCompileVersionInfoAction(BuildAction& action, BuilderFileInfo& fileInfo, VersionNumberInfo versionInfo);

	// Archives all the source files provided into a single lib.
	virtual void GetArchiveAction(BuildAction& action, std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile);

	// Links all the source files provided into a sinmgle executable.
	virtual void GetLinkAction(BuildAction& action, std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile);

	// Some general paths that most toolchains use, this just makes them a bit cleaner to access.
	Platform::Path GetOutputPath();
	Platform::Path GetPchPath();
	Platform::Path GetPchObjectPath();
	Platform::Path GetVersionInfoObjectPath();
	Platform::Path GetPdbPath();
	Platform::Path GetOutputPdbPath();

    // Writes out all messages that were emitted while building this file.
    void PrintMessages(BuilderFileInfo& file);

    // Parses the output of a compilation stage, extracting dependencies/errors and anything else of use. 
    // Output may be modified to remove redundent information.
    // All extracted information will be stored in the fileInfo passed in.
    // Returns true if output is invalid and we need to abort asap.
    virtual bool ParseOutput(BuilderFileInfo& file, std::string& output);

}; 

}; // namespace MicroBuild
