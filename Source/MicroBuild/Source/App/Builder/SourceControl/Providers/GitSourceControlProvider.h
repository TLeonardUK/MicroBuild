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

#include "App/Builder/SourceControl/SourceControlProvider.h"

namespace MicroBuild {
	
// Provides an interface to interact with and query a git server.
class GitSourceControlProvider 
	: public ISourceControlProvider
{
public:

	GitSourceControlProvider();
	
	virtual bool Connect(const Platform::Path& rootPath) override;
	virtual bool GetChangelist(const Platform::Path& path, SourceControlChangelist& changelistId) override;
	virtual bool GetTotalChangelists(const Platform::Path& path, int& totalChangelists) override;
	virtual bool GetHistory(const Platform::Path& path, std::vector<SourceControlChangelist>& history, int count, int offset) override;

private:

	// Attempts to run the given git command and optionally returns the stdout.
	bool RunGitCommand(
		const std::vector<std::string>& arguments,
		const Platform::Path& folder,
		std::string* output = nullptr
	);

	// Attempts to parse changelists out of the given output, output is assumed to 
	// be in m_logFormat format.
	bool ParseChangelists(
		const std::string& output, 
		std::vector<SourceControlChangelist>& changelists
	);

	Platform::Path m_rootPath;

	std::string m_logFormat;

}; 

}; // namespace MicroBuild
