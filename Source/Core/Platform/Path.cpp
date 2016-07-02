/*
Ludo Game Engine
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
#include "Core/Platform/Path.h"
#include "Core/Helpers/Strings.h"

namespace MicroBuild {
namespace Platform {

Path::Path()
	: m_mount("")
	, m_filename("")
	, m_extension("")
{
}
Path::Path(const char* InValue)
	: Path((std::string)InValue)
{	
}

Path::Path(const std::string& InValue)
	: m_mount("")
	, m_filename("")
	, m_extension("")
{
	std::vector<std::string> Seperators;
	Seperators.push_back("/");
	Seperators.push_back("\\");

	std::string Value = InValue;

	// Parse out the mount.
	size_t ColonIndex = Value.find(":");
	if (ColonIndex != std::string::npos)
	{
		Strings::SplitOnIndex(Value, ColonIndex, m_mount, Value);
	}

	// Parse out the extension.
	size_t PeriodIndex = Value.find_last_of(".");
	bool bExtensionIsFilename = false;

	if (PeriodIndex != std::string::npos)
	{
		size_t LastSeperatorIndex = Value.find_last_of("/\\");
		if (LastSeperatorIndex == PeriodIndex - 1)
		{
			bExtensionIsFilename = true;
		}
		if (LastSeperatorIndex == std::string::npos || 
			LastSeperatorIndex < PeriodIndex)
		{
			Strings::SplitOnIndex(Value, PeriodIndex, Value, m_extension);
		}
	}

	// Split up directories/filename.	
	while (!Value.empty())
	{
		size_t SliceIndex = Value.find_first_of("/\\");
		if (SliceIndex != std::string::npos)
		{
			std::string Segment;
			Strings::SplitOnIndex(Value, SliceIndex, Segment, Value);
			if (!Segment.empty())
			{
				m_directories.push_back(Segment);
			}
		}
		else
		{
			if (!Value.empty())
			{
				m_directories.push_back(Value);
			}
			break;
		}
	}

	if (m_directories.size() >= 1 && !bExtensionIsFilename)
	{
		m_filename = *m_directories.rbegin();
		m_directories.pop_back();
	}

	UpdateCachedString();
}

void Path::UpdateCachedString()
{
	std::string Result = "";

	if (!m_mount.empty())
	{
		Result = m_mount + ":";
		Result += Seperator;
	}

	if (!m_directories.empty())
	{
		for (std::string Directory : m_directories)
		{
			Result += Directory + Seperator;
		}
	}

	Result += m_filename;

	if (!m_extension.empty())
	{
		Result += "." + m_extension;
	}

	m_cachedString = Result;
}

std::string Path::ToString() const
{
	return m_cachedString;
}

Path Path::operator +(const Path& Other) const
{
	assert(Other.IsRelative());

	Path Result;
	Result.m_mount = m_mount;
	Result.m_directories = m_directories;

	if (!m_filename.empty())
	{
		if (!m_extension.empty())
		{
			Result.m_directories.push_back(m_filename + "." + m_extension);
		}
		else
		{
			Result.m_directories.push_back(m_filename);
		}
	}
	else if (!m_extension.empty())
	{
		Result.m_directories.push_back("." + m_extension);
	}

	Result.m_directories.insert(Result.m_directories.end(),
		Other.m_directories.begin(),
		Other.m_directories.end());

	Result.m_filename = Other.m_filename;
	Result.m_extension = Other.m_extension;

	Result.UpdateCachedString();

	return Result;
}

bool Path::operator ==(const Path& Other) const
{
	return (m_cachedString == Other.m_cachedString);
}

bool Path::operator !=(const Path& Other) const
{
	return !(operator ==(Other));
}

std::string Path::GetBaseName() const
{
	return m_filename;
}

std::string Path::GetExtension() const
{
	return m_extension;
}

std::string Path::GetMount() const
{
	return m_mount;
}

std::string Path::GetFilename() const
{
	if (!m_extension.empty())
	{
		return m_filename + "." + m_extension;
	}
	else
	{
		return m_filename;
	}
}

Path Path::GetDirectory() const
{
	Path Result;
	Result.m_mount = m_mount;
	Result.m_directories = m_directories;

	if (Result.m_directories.size() > 0)
	{
		Result.m_filename = *Result.m_directories.rbegin();
		Result.m_directories.pop_back();
		Result.m_extension = "";

		// Parse out the extension.
		size_t PeriodIndex = Result.m_filename.find_last_of(".");
		if (PeriodIndex != std::string::npos)
		{
			Strings::SplitOnIndex(Result.m_filename, PeriodIndex, 
				Result.m_filename, Result.m_extension);
		}
	}
	else
	{
		Result.m_filename = "";
		Result.m_extension = "";
	}

	Result.UpdateCachedString();
	return Result;
}

Path Path::ChangeExtension(const std::string& Value) const
{
	Path Result = *this;
	Result.m_extension = Value;
	Result.UpdateCachedString();
	return Result;
}

Path Path::ChangeBaseName(const std::string& Value) const
{
	Path Result = *this;
	Result.m_filename = Value;
	Result.UpdateCachedString();
	return Result;
}

Path Path::ChangeMount(const std::string& Value) const
{
	Path Result = *this;
	Result.m_mount = Value;
	Result.UpdateCachedString();
	return Result;
}

Path Path::ChangeFilename(const std::string& Value) const
{
	Path Result = *this;

	size_t PeriodIndex = Result.m_filename.find_last_of(".");
	if (PeriodIndex != std::string::npos)
	{
		Strings::SplitOnIndex(Value, PeriodIndex, 
			Result.m_filename, Result.m_extension);
	}
	else
	{
		Result.m_filename = Value;
		Result.m_extension = "";
	}

	Result.UpdateCachedString();
	return Result;
}

Path Path::ChangeDirectory(const Path& Value) const
{
	Path Result = *this;

	Result.m_mount = Value.m_mount;
	Result.m_directories = Value.m_directories;

	if (!Value.m_filename.empty())
	{
		if (!Value.m_extension.empty())
		{
			Result.m_directories.push_back(
				Value.m_filename + "." + Value.m_extension);
		}
		else
		{
			Result.m_directories.push_back(Value.m_filename);
		}
	}

	Result.UpdateCachedString();
	return Result;
}

Path Path::AppendFragment(const std::string& Value) const
{
	Path Result = *this;

	// Filename gets pushed into directory stack.
	if (!Result.m_filename.empty() && !Result.m_extension.empty())
	{
		Result.m_directories.push_back(
			Result.m_filename + "." + Result.m_extension
		);
	}

	// Filename now becomes appended value.
	size_t PeriodIndex = Value.find_last_of(".");
	if (PeriodIndex != std::string::npos)
	{
		Strings::SplitOnIndex(Value, PeriodIndex,
			Result.m_filename, Result.m_extension);
	}
	else
	{
		Result.m_filename = Value;
		Result.m_extension = "";
	}

	Result.m_cachedString += Seperator;
	Result.m_cachedString += Value;

	return Result;
}


bool Path::IsEmpty() const
{
	return m_mount.empty() && m_filename.empty() 
		&& m_extension.empty() && m_directories.empty();
}

bool Path::IsRelative() const
{
	return m_mount.empty();
}

bool Path::IsAbsolute() const
{
	return !IsRelative();
}

bool Path::IsRoot() const
{
	return !m_mount.empty() && m_filename.empty() 
		&& m_extension.empty() && m_directories.empty();
}

Path Path::RelativeTo(const Path& Destination) const
{
	// If its a different mount, we have to be absolute.
	if (m_mount != Destination.m_mount)
	{
		return Destination;
	}

	// If either us or the destination are absolute, its not 
	// possible, just return destination.
	if (IsRelative() || Destination.IsRelative())
	{
		return Destination;
	}

	Path Result;
	Result.m_mount = "";
	Result.m_extension = Destination.m_extension;
	Result.m_filename = Destination.m_filename;

	// Work out which directories are matching.
	int MatchingDirs = 0;
	size_t MinDirCount = min(
		Destination.m_directories.size(), 
		m_directories.size());

	for (int i = 0; i < MinDirCount; i++)
	{
		if (Destination.m_directories[i] == m_directories[i])
		{
			MatchingDirs++;
		}
		else
		{
			break;
		}
	}

	size_t DirsToGoUp = m_directories.size() - MatchingDirs;
	for (int i = 0; i < DirsToGoUp; i++)
	{
		Result.m_directories.push_back("..");
	}

	size_t DirsToAdd = Destination.m_directories.size() - MatchingDirs;
	for (int i = 0; i < DirsToAdd; i++)
	{
		Result.m_directories.push_back(
			Destination.m_directories[MatchingDirs + i]);
	}

	Result.UpdateCachedString();

	return Result;
}

struct PathMatchFragment
{
	Path full;
	std::string remaining;
};

void MatchFilter_GetDirectories_r(const Path& base, std::vector<Path>& results)
{
	std::string seperatorString(1, Path::Seperator);
	std::vector<Path> dirs = base.GetDirectories();
	for (Path& path : dirs)
	{
		Path fullPath = base + seperatorString + path;
		results.push_back(fullPath);
		MatchFilter_GetDirectories_r(fullPath, results);
	}
}

std::vector<Path> MatchFilter_r(
	const Path& base, 
	const std::vector<std::string>& matches)
{
	std::vector<Path> result;

	std::string seperatorString(1, Path::Seperator);

	std::string matchType = matches[0];
	
	// Nothing to match again? Game over.
	if (matches.size() == 0)
	{
		result.push_back(base);
		return result;
	}

	// Try and match up as much as possible.
	else
	{
		std::vector<PathMatchFragment> potentialMatches;

		std::vector<Path> dirs = base.GetDirectories();
		std::vector<Path> files = base.GetFiles();
		
		for (Path& path : dirs)
		{
			PathMatchFragment frag;
			frag.full = base + seperatorString + path;
			frag.remaining = path.ToString();
			potentialMatches.push_back(frag);
		}

		for (Path& path : files)
		{
			PathMatchFragment frag;
			frag.full = base + seperatorString + path;
			frag.remaining = path.ToString();
			potentialMatches.push_back(frag);
		}
		
		bool bFinished = false;

		std::vector<std::string> matchStack(matches);
		while (matchStack.size() > 0 && !bFinished)
		{
			std::string match = matchStack[0];
			matchStack.erase(matchStack.begin());

			// Non-recursive match.
			if (match == "*")
			{
				// If we are at the end, or next is a directory, we can 
				// just accept all potentials.
				if (matchStack.size() == 0 || 
					matchStack[0] == seperatorString)
				{
					for (unsigned int i = 0; i < potentialMatches.size(); i++)
					{
						PathMatchFragment& frag = potentialMatches[i];
						frag.remaining = "";
					}
				}
				else
				{
					std::string nextFragment = matchStack[0];

					// Try and match up any potentials.
					for (unsigned int i = 0; i < potentialMatches.size(); i++)
					{
						PathMatchFragment& frag = potentialMatches[i];
						
						size_t offset = frag.remaining.find(
							nextFragment.c_str()
						);

						if (offset != std::string::npos)
						{
							frag.remaining.erase(
								frag.remaining.begin(),
								frag.remaining.begin() + offset);
						}
						else
						{
							potentialMatches.erase(potentialMatches.begin() + i);
							i--;
						}
					}
				}
			}
			
			// Recursive match.
			else if (match == "**")
			{
				// Get a list of all directories below this sub-directory, 
				// try and perform a match for the reset of the stack on each
				// of these directories.
				std::vector<Path> dirs;

				MatchFilter_GetDirectories_r(base, dirs);

				// If we have a seperator next, we don't want
				// to pass in the base path as matches have to be at least
				// one directory level down.
				if (matchStack.size() > 0 && 
					matchStack[0] == seperatorString)
				{
					matchStack.erase(matchStack.begin());
				}
				else
				{
					dirs.push_back(base);
				}

				for (unsigned int i = 0; i < dirs.size(); i++)
				{
					std::vector<Path> subResult =
						MatchFilter_r(dirs[i], matchStack);

					for (Path& path : subResult)
					{
						result.push_back(path);
					}
				}

				// No more matching at this level.
				bFinished = true;
			}

			// Expected directory.
			else if (match == seperatorString)
			{
				// Recurse into each directory.
				for (unsigned int i = 0; i < potentialMatches.size(); i++)
				{
					PathMatchFragment& frag = potentialMatches[i];
					if (frag.remaining.empty())
					{						
						if (frag.full.IsDirectory())
						{
							std::vector<Path> subResult = 
								MatchFilter_r(frag.full, matchStack);

							for (Path& subPath : subResult)
							{
								result.push_back(subPath);
							}
						}
					}
				}

				// No more matching at this level.
				bFinished = true;
			}
			
			// Literal match.
			else
			{
				for (unsigned int i = 0; i < potentialMatches.size(); i++) 
				{
					PathMatchFragment& frag = potentialMatches[i];

					if (frag.remaining.size() >= match.size() &&
						frag.remaining.substr(0, match.size()) == match)
					{
						frag.remaining.erase(
							frag.remaining.begin(), 
							frag.remaining.begin() + match.size());
					}
					else
					{
						potentialMatches.erase(potentialMatches.begin() + i);
						i--;
					}
				}
			}
		}

		// If we are not finished, but we are out of matches, see if any 
		// potential matches are now empty.
		if (!bFinished)
		{
			for (unsigned int i = 0; i < potentialMatches.size(); i++)
			{
				PathMatchFragment& frag = potentialMatches[i];
				if (frag.remaining.empty())
				{
					result.push_back(frag.full);
				}
			}
		}
	}
	
	return result;
}

std::vector<Path> Path::MatchFilter(const Path& path)
{
	std::vector<std::string> matchStack;
	std::string pathString = path.ToString();
	size_t offset = 0;

	std::vector<Path> result;

	// No match filters, early-out.
	if (path.ToString().find('*') == std::string::npos)
	{
		result.push_back(path);
		return result;
	}

	// Split up into * ** and path fragments
	while (offset < pathString.size())
	{
		size_t seperatorIndex = pathString.find_first_of(Seperator, offset);
		size_t matchIndex = pathString.find_first_of('*', offset);

		size_t splitIndex = seperatorIndex;
		if (splitIndex == std::string::npos || 
			matchIndex < seperatorIndex)
		{
			splitIndex = matchIndex;
		}

		if (splitIndex == std::string::npos)
		{
			matchStack.push_back(pathString.substr(offset));
			break;
		}
		else
		{
			std::string split = pathString.substr(splitIndex, 1);

			std::string fragment = pathString.substr(offset, splitIndex - offset);
			if (!fragment.empty())
			{
				matchStack.push_back(fragment);
			}

			offset = splitIndex + 1;

			if (split == "*")
			{
				if (*matchStack.rbegin() == "*" ||
					*matchStack.rbegin() == "**")
				{
					Log(LogSeverity::Fatal, 
						"Wildcard followed by another wildcard in value '%s', "
						"this is ambiguous and cannot be expanded.", 
						pathString.c_str());

					return result;
				}

				if (offset < pathString.size())
				{
					if (pathString[offset] == '*')
					{
						split += "*";
						offset++;
					}
				}
			}

			if (!split.empty())
			{
				matchStack.push_back(split);
			}
		}
	}

	// If we only have one split, we are done.
	if (matchStack.size() == 1)
	{
		result.push_back(matchStack[0]);
	}
	else
	{
		// Start at the first directory before a match value.
		unsigned int firstValidStartIndex = 0;
		std::string seperatorString(1, Seperator);

		for (unsigned int i = 0; i < matchStack.size(); i++)
		{
			std::string sub = matchStack[i];
			if (sub == seperatorString)
			{
				firstValidStartIndex = i;
			}
			else if (sub == "*" || sub == "**")
			{
				break;
			}
		}

		std::string subValue = "";
		for (unsigned int i = 0; i <= firstValidStartIndex; i++)
		{
			subValue += matchStack[i];
		}

		matchStack.erase(
			matchStack.begin(),
			matchStack.begin() + (firstValidStartIndex + 1)
		);

		result = MatchFilter_r(subValue, matchStack);
	}

	return result;
}

}; // namespace Platform
}; // namespace MicroBuild