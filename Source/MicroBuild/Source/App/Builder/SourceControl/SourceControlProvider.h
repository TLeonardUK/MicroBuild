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

#include "Core/Platform/Path.h"

#include <ctime>

namespace MicroBuild {
	
// Represents an individual changelist retrieved from a source control provider.
struct SourceControlChangelist
{
	std::string Id;
	std::string Author;
	std::string Description;
	std::time_t	Date;
};

// Provides an interface to interact with and query a source control server.
class ISourceControlProvider
{
public:
	
	// Connects to the source control that is controlling the 
	// given local folder. Returns true on success.
	virtual bool Connect(const Platform::Path& rootPath) = 0;

	// Queries a file for its changelist. Returns true on success.
	virtual bool GetChangelist(const Platform::Path& path, SourceControlChangelist& changelist) = 0;

	// Queries the path for all changelists that effect it, including sub-paths.
	virtual bool GetTotalChangelists(int& totalChangelists) = 0;

	// Queries the history for a given path. Returns true on success. Offset and Count can be
	// used to page through the history in chunks.
	virtual bool GetHistory(const Platform::Path& path, std::vector<SourceControlChangelist>& history, int count, int offset) = 0;

	// Attempts to checkout the given file. File is checked out to the default changelist.
	virtual bool Checkout(const Platform::Path& path) = 0;

	// Attempts to commit the given list of files with the given commit message.
	virtual bool Commit(const std::vector<Platform::Path>& files, const std::string& commitMessage) = 0;

	// Attempts to check if the given file exists in source control.
	virtual bool Exists(const Platform::Path& path, bool& bExists) = 0;

	// Attempts to add the given file to source control.
	virtual bool Add(const Platform::Path& path) = 0;

	// Attempts to syncronize the local and remote repositories. In git this results in a pull followed
	// by a push. Fails if there are conflics.
	virtual bool Sync() = 0;


}; 

}; // namespace MicroBuild
