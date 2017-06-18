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

#include "App/Builder/Toolchains/ToolchainOutputParser.h"
#include "Core/Commands/Command.h"
#include "Core/Platform/Path.h"

#include <mutex>

namespace MicroBuild {

// Stores information on a dependency of a MetadataFileInfo
// structure.
struct BuilderDependencyInfo
{
public:
	Platform::Path	SourcePath;
	uint64_t		Hash;

};

// Stores information on an individual file that needs to 
// have meta data generated for it.
struct BuilderFileInfo 
{
private:
	static std::map<uint64_t, std::time_t> m_modifiedTimeCache;
	static std::map<uint64_t, bool> m_fileExistsCache;
	static std::mutex m_fileCacheLock;

public:

	BuilderFileInfo();

	// Source file of header the metadata is generated from.
	Platform::Path						SourcePath;

	// The file that is output when the source file is compiled.
	Platform::Path						OutputPath;

	// Manifest file used to keep track of incremental build state.
	Platform::Path						ManifestPath;

	// True if the source file needs its metadata regenerating.
	bool								bOutOfDate;

	// Hash which changes when the source file changes. Used to determine
	// when the file is out of date.
	uint64_t							Hash;

	// Keeps track of the hashes and paths to all other files the
	// source file is dependent on.
	std::vector<BuilderDependencyInfo>	Dependencies;

	// List of dependency paths that were extracted from the stdout.
	std::vector<Platform::Path>			OutputDependencyPaths;

	// Messages that occured while trying to build the file.
	std::vector<ToolchainOutputMessage>	Messages;

	// How many messages in the above array are errors.
	int ErrorCount;

	// How many messages in the above array are warnings.
	int WarningCount;

	// How many messages in the above array are informational.
	int InfoCount;

	// Adds a message to the file info.
	void AddMessage(const ToolchainOutputMessage& message);

	// Loads hash and dependency data from the manifest file.
	bool LoadManifest();

	// Stores hash and dependency data into the manifest file.
	bool StoreManifest();

	// Calculates the state-hash for a given file. Used to figure
	// out of a file is stale and needs regenerating.
	static uint64_t CalculateFileHash(const Platform::Path& path, uint64_t configurationHash);

	// Goes through a list of source files an generates an array of FileInfo
	// structures for them using the given properties.
	static std::vector<BuilderFileInfo> GetMultipleFileInfos(
		const std::vector<Platform::Path>& paths,
		Platform::Path rootDirectory,
		Platform::Path outputDirectory,
		uint64_t configurationHash,
		bool bNoIntermediateFiles
	);

	// Checks if a given file info is out of date.
	static bool CheckOutOfDate(BuilderFileInfo& file, uint64_t configurationHash, bool bNoIntermediateFiles);

	// Gets the modified time for a given file, and stores 
	static std::time_t GetCachedModifiedTime(const Platform::Path& path);

	// Gets the existance state for a given file.
	static bool GetCachedPathExists(const Platform::Path& path);
};

// Individual command line execution for a build step.
struct BuildAction
{
	Platform::Path Tool;
	Platform::Path WorkingDirectory;
	std::vector<std::string> Arguments;
	BuilderFileInfo FileInfo;
	std::string Output;
	std::string StatusMessage;
	int ExitCode;

	std::function<bool(BuildAction& Action)> PostProcessDelegate;

	BuildAction()
	{
	}
};

}; // namespace MicroBuild
