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
#include "Core/Helpers/Time.h"

#include <sstream>

namespace MicroBuild {
namespace Platform {

Path::Path()
	: m_raw("")
{
}
Path::Path(const char* InValue)
	: Path((std::string)InValue)
{	
}

Path::Path(const std::string& InValue)
	: m_raw(InValue)
{
	Normalize();
}

std::string Path::ToString() const
{
	return m_raw;
}

Path Path::operator +(const Path& Other) const
{
	return Strings::Format("%s%c%s", m_raw.c_str(), Seperator, Other.m_raw.c_str());
}

Path Path::operator +(const std::string& Other) const
{
	return Strings::Format("%s%c%s", m_raw.c_str(), Seperator, Other.c_str());
}

bool Path::operator ==(const Path& Other) const
{
	return (m_raw == Other.m_raw);
}

bool Path::operator !=(const Path& Other) const
{
	return !(operator ==(Other));
}

std::string Path::GetBaseName() const
{
	std::string result = m_raw;
	
	size_t lastDirOffset = result.find_last_of(Seperator);
	if (lastDirOffset != std::string::npos)
	{
		result.erase(result.begin(), result.begin() + lastDirOffset + 1);
	}

	size_t lastDotOffset = result.find_last_of('.');
	if (lastDotOffset != std::string::npos)
	{
		result.erase(result.begin() + lastDotOffset, result.end());
	}

	return result;
}

std::string Path::GetExtension() const
{
	size_t lastDotOffset = m_raw.find_last_of('.');
	if (lastDotOffset != std::string::npos)
	{
		return m_raw.substr(lastDotOffset + 1);
	}

	return "";
}

std::string Path::GetMount() const
{
	if (m_raw.size() > 0)
	{
		if (m_raw[0] == Seperator)
		{
			return { Seperator };
		}
		else if (m_raw.size() > 1 && m_raw[1] == ':')
		{
			return m_raw.substr(0, 2);
		}
	}
	
	return "";
}

std::string Path::GetFilename() const
{
	size_t lastDirOffset = m_raw.find_last_of(Seperator);
	if (lastDirOffset != std::string::npos)
	{
		return m_raw.substr(lastDirOffset + 1);
	}
	
	return "";
}

Path Path::GetDirectory() const
{
	size_t lastDirOffset = m_raw.find_last_of(Seperator);
	if (lastDirOffset != std::string::npos)
	{
		return m_raw.substr(0, lastDirOffset);
	}

	return "";
}

Path Path::ChangeExtension(const std::string& Value) const
{
	size_t lastDotOffset = m_raw.find_last_of('.');
	if (lastDotOffset != std::string::npos)
	{
		return Strings::Format(
			"%s.%s",
			m_raw.substr(0, lastDotOffset).c_str(),
			Value.c_str()
		);
	}
	else
	{
		return Strings::Format(
			"%s.%s",
			m_raw,
			Value.c_str()
		);
	}
}

Path Path::ChangeBaseName(const std::string& Value) const
{
	std::string result = m_raw;

	size_t leftOffset = 0;
	size_t rightOffset = 0;

	size_t lastDirOffset = result.find_last_of(Seperator);
	if (lastDirOffset != std::string::npos)
	{
		leftOffset = lastDirOffset + 1;
	}
	else
	{
		leftOffset = 0;
	}

	size_t lastDotOffset = result.find_last_of('.');
	if (lastDotOffset != std::string::npos)
	{
		rightOffset = lastDotOffset;
	}
	else
	{
		rightOffset = result.size();
	}

	return result.replace(
		leftOffset, 
		rightOffset - leftOffset, 
		Value
	);
}

Path Path::ChangeMount(const std::string& Value) const
{
	if (m_raw.size() > 0)
	{
		if (m_raw[0] == Seperator)
		{
			return Strings::Format("%s%s",
				m_raw.substr(1),
				Value.c_str());
		}
		else if (m_raw.size() > 1 && m_raw[1] == ':')
		{
			return Strings::Format("%s%s",
				m_raw.substr(2),
				Value.c_str());
		}
	}

	return *this;
}

Path Path::ChangeFilename(const std::string& Value) const
{
	size_t lastDirOffset = m_raw.find_last_of(Seperator);
	if (lastDirOffset != std::string::npos)
	{
		return m_raw.substr(0, lastDirOffset + 1) + Value;
	}
	else
	{
		return Value;
	}
}

Path Path::ChangeDirectory(const Path& Value) const
{
	size_t lastDirOffset = m_raw.find_last_of(Seperator);
	if (lastDirOffset != std::string::npos)
	{
		return Strings::Format("%s%c%s", 
			Value.m_raw.c_str(),
			Seperator,
			m_raw.substr(lastDirOffset).c_str());
	}
	else
	{
		return Strings::Format("%s%c%s",
			Value.m_raw.c_str(),
			Seperator,
			m_raw.c_str());
	}
}

Path Path::AppendFragment(const std::string& Value, bool bAddDeliminator) const
{
	if (bAddDeliminator)
	{
		return Strings::Format("%s%c%s", m_raw.c_str(), Seperator, Value.c_str());

	}
	else
	{
		return Strings::Format("%s%s", m_raw.c_str(), Value.c_str());
	}
}

bool Path::IsEmpty() const
{
	return m_raw.empty();
}

bool Path::IsRelative() const
{
	return GetMount() == "";
}

bool Path::IsAbsolute() const
{
	return !IsRelative();
}

bool Path::IsRoot() const
{
	if (m_raw.size() == 1 && m_raw[0] == Seperator)
	{
		return true;
	}
	else if (m_raw.size() == 2 && m_raw[1] == ':')
	{
		return true;
	}
	return false;
}

bool Path::IsSourceFile() const
{
	static const char* extensions[] = {
		"cpp",
		"c",
		"cc",
		"c++",
		"mm",
		"m",
		"cxx",
		"cs",
		"asm",
		"js",
		"fs",
		"vb",
		"py",
		"lua",
		"vbs",
		"r",
		"d",
		"php",
		"cob",
		"pb",
		"cbl",
		"pas",
		"perl",
		"tcl",
		"objc",
		"cp",
		"j",
		"java",
		"qs",
		"cls",
		"frm",
		"rb",
		nullptr
	};
	
	std::string extension = Strings::ToLowercase(GetExtension());

	for (int i = 0; extensions[i] != nullptr; i++)
	{
		if (extensions[i] == extension)
		{
			return true;
		}
	}

	return false;
}

bool Path::IsIncludeFile()  const
{
	static const char* extensions[] = {
		"h",
		"hpp",
		"inc",
		"hrc",
		"hxx",
		nullptr
	};

	std::string extension = Strings::ToLowercase(GetExtension());

	for (int i = 0; extensions[i] != nullptr; i++)
	{
		if (extensions[i] == extension)
		{
			return true;
		}
	}

	return false;
}

std::vector<std::string> Path::GetFragments() const
{
	return Strings::Split(Seperator, m_raw);
}

void Path::Normalize()
{
	char correctSeperator = Seperator;
	char incorrectSeperator = '/';

	if (correctSeperator == incorrectSeperator)
	{
		incorrectSeperator = '\\';	
	}

	// Replace incorrect directory seperators.
	while (true)
	{
		size_t incorrectOffset = m_raw.find(incorrectSeperator);
		if (incorrectOffset != std::string::npos)
		{
			m_raw[incorrectOffset] = correctSeperator;
		}
		else
		{
			break;
		}
	}

	// Replace trailing slash.
	if (m_raw[m_raw.size() - 1] == correctSeperator)
	{
		m_raw.resize(m_raw.size() - 1);
	}

	// Replace repeated directory seperators.
	std::string repeatedSeperator = { correctSeperator, correctSeperator };
	while (true)
	{
		size_t offset = m_raw.find(repeatedSeperator);
		if (offset != std::string::npos)
		{
			m_raw.erase(offset, 1);
		}
		else
		{
			break;
		}
	}

	// Replace all parent seperators.
	if (IsAbsolute() && m_raw.find("/.") != std::string::npos)
	{
		std::string result = "";
		std::string fragment = "";
		int skipCount = 0;

		for (int i = (int)m_raw.size() - 1; i >= 0; i--)
		{
			char chr = m_raw[i];
			if (chr == correctSeperator)
			{
				if (fragment == "..")
				{
					skipCount++;
				}
				else if (fragment == ".")
				{
					// Don't skip anything, but don't append fragment.
				}
				else
				{
					if (skipCount > 0)
					{
						skipCount--;
					}
					else
					{
						if (result.empty())
						{
							result = fragment;
						}
						else
						{
							result = fragment + correctSeperator + result;
						}
					}
				}
				fragment = "";
			}
			else
			{
				fragment = chr + fragment;
			}
		}

		if (!fragment.empty())
		{
			if (skipCount > 0)
			{
				Log(LogSeverity::Info,
					"Path '%s' attempted to reference directory above root!", 
					m_raw.c_str());
			}
			result = fragment + correctSeperator + result;
		}

		m_raw = result;
	}
}

Path Path::RelativeTo(const Path& Destination) const
{
	// If its a different mount, we have to be absolute.
	if (GetMount() != Destination.GetMount())
	{
		return Destination;
	}

	// If either of us is relative we have no root to determine relative
	// paths from, so its going to have to be absolute.
	if (IsRelative() || Destination.IsRelative())
	{
		return Destination;
	}

	std::vector<std::string> fragments = GetFragments();
	std::vector<std::string> destFragments = Destination.GetFragments();

	int fragmentsDirCount = (int)fragments.size();
	int destFragmentsDirCount = (int)destFragments.size();

	// Work out which directories are matching.
	int matchingDirs = 0;
	size_t minDirCount = min(fragmentsDirCount, destFragmentsDirCount);

	for (unsigned int i = 0; i < minDirCount; i++)
	{
		if (fragments[i] == destFragments[i])
		{
			matchingDirs++;
		}
		else
		{
			break;
		}
	}

	std::string result = "";

	size_t DirsToGoUp = fragmentsDirCount - matchingDirs;
	for (unsigned int i = 0; i < DirsToGoUp; i++)
	{
		if (!result.empty())
		{
			result += Seperator;
		}
		result += "..";
	}

	size_t dirsToAdd = destFragmentsDirCount - matchingDirs;
	for (unsigned int i = 0; i < dirsToAdd; i++)
	{
		if (!result.empty())
		{
			result += Seperator;
		}
		result += destFragments[matchingDirs + i];
	}

	return result;
}

bool Path::GetCommonPath(std::vector<Path>& paths, Path& result)
{
	size_t maxOffset = INT_MAX;
	for (Path& path : paths)
	{
		maxOffset = min(maxOffset, path.ToString().size());
	}

	std::string matchPath = "";
	size_t matchOffset = 0; 

	while (true)
	{
		size_t offset = paths[0].ToString().find(Seperator, matchOffset);
		if (offset == std::string::npos)
		{
			offset = paths[0].ToString().size();
		}

		std::string main = paths[0].ToString().substr(0, offset);

		bool bMatching = true;

		for (int i = 1; i < (int)paths.size(); i++)
		{
			std::string sub = paths[i].ToString().substr(0, offset);
			size_t subOffset = paths[i].ToString().find(Seperator, matchOffset);

			if (offset != subOffset && sub != main)
			{
				bMatching = false;
				break;
			}
		}

		if (bMatching)
		{
			matchPath = main;
			matchOffset = offset + 1;

			if (offset == maxOffset)
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

	result = matchPath;
	return true;
}

Path Path::GetUncommonPath(Path& commonPath)
{
	Path uncommon = m_raw.substr(commonPath.m_raw.size() + 1);
	return uncommon;
}

struct PathMatchFragment
{
	std::string full;
	std::string remaining;
	bool valid;
	bool isDirectory;
};

void MatchFilter_GetDirectories_r(const Path& base, std::vector<Path>& results)
{
	static std::string seperatorString(1, Path::Seperator);

	std::vector<std::string> dirs = base.GetDirectories();

	results.reserve(results.size() + dirs.size());

	for (std::string& path : dirs)
	{
		Path fullPath = base.AppendFragment(path, true);
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

		std::vector<std::string> dirs = base.GetDirectories();
		std::vector<std::string> files = base.GetFiles();
		
		potentialMatches.reserve(dirs.size() + files.size());

		for (std::string& path : dirs)
		{
			PathMatchFragment frag;
			frag.full = path;
			frag.remaining = path;
			frag.valid = true;
			frag.isDirectory = true;
			potentialMatches.push_back(frag);
		}

		for (std::string& path : files)
		{
			PathMatchFragment frag;
			frag.full = path;
			frag.remaining = path;
			frag.valid = true;
			frag.isDirectory = false;
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
						if (frag.valid)
						{
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
								potentialMatches[i].valid = false;
							}
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

				// Add a non-recursive match to check against for each directory.
				matchStack.insert(matchStack.begin(), "*");

				for (unsigned int i = 0; i < dirs.size(); i++)
				{
					std::vector<Path> subResult =
						MatchFilter_r(dirs[i], matchStack);

					result.reserve(subResult.size() + result.size());

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
					if (frag.remaining.empty() && frag.valid)
					{						
						if (frag.isDirectory)
						{
							std::vector<Path> subResult = 
								MatchFilter_r(base + seperatorString + frag.full, matchStack);

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
						potentialMatches[i].valid = false;
					}
				}
			}
		}

		// If we are not finished, but we are out of matches, see if any 
		// potential matches are now empty.
		if (!bFinished)
		{
			result.reserve(potentialMatches.size());

			for (unsigned int i = 0; i < potentialMatches.size(); i++)
			{
				PathMatchFragment& frag = potentialMatches[i];
				if (frag.remaining.empty() && frag.valid)
				{
					result.push_back(base.AppendFragment(frag.full, true));
				}
			}
		}
	}
	
	return result;
}

std::vector<Path> Path::MatchFilter(const Path& path)
{
	Time::TimedScope scope("Match Filter");

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
				if (matchStack.size() > 0 && 
					(*matchStack.rbegin() == "*" || *matchStack.rbegin() == "**"))
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