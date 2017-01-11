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
#include "Core/Platform/Platform.h"
#include "Core/Platform/Path.h"
#include "Core/Helpers/Strings.h"
#include "Core/Helpers/Time.h"

#include <sstream>

#ifdef MB_PLATFORM_WINDOWS
#include <direct.h>
#else
#include <unistd.h>
#endif

namespace MicroBuild {
namespace Platform {

Path g_executablePath;

Path::Path()
	: m_raw("")
{
}
Path::Path(const char* InValue)
	: m_raw(InValue)
{	
	Normalize();
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

bool Path::operator <(const Path& Other) const
{
	return (m_raw < Other.m_raw);
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
	size_t lastSeperatorOffset = m_raw.find_last_of(Seperator);
	size_t lastDotOffset = m_raw.find_last_of('.');

	if (lastDotOffset > lastSeperatorOffset && 
		lastDotOffset != std::string::npos)
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
	
	return m_raw;
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
			m_raw.c_str(),
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
				m_raw.substr(1).c_str(),
				Value.c_str());
		}
		else if (m_raw.size() > 1 && m_raw[1] == ':')
		{
			return Strings::Format("%s%s",
				m_raw.substr(2).c_str(),
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
		"objc",
		"cp",
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

bool Path::IsResourceFile() const
{
	static const char* extensions[] = {
		"strings",
		"nib",
		"xib",
		"storyboard",
		"icns",
		"rc",
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


bool Path::IsXamlFile() const
{
	static const char* extensions[] = {
		"xaml",
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

bool Path::IsImageFile() const
{
	static const char* extensions[] = {
		"png",
		"bmp",
		"ico",
		"gif",
		"jpeg",
		"jpg",
		"tif",
		"tiff",
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

bool Path::IsObjCFile() const
{
	static const char* extensions[] = {
		"m",
		"objc",
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

bool Path::IsObjCppFile() const
{
	static const char* extensions[] = {
		"mm",
		"objc",
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



bool Path::IsCFile() const
{
	static const char* extensions[] = {
		"c",
		"cc",
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

bool Path::IsCppFile() const
{
	static const char* extensions[] = {
		"cpp",
		"c++",
		"cxx",
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

bool Path::IsCSharpFile() const
{
	static const char* extensions[] = {
		"cs",
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
	std::string original = m_raw;

	if (m_raw.size() == 0)
	{
		return;
	}

	// Replace all environment variables.
	for(;;)
	{
		size_t startOffset = m_raw.find('%');
		if (startOffset != std::string::npos)
		{
			size_t endOffset = m_raw.find('%', startOffset + 1);
			if (endOffset != std::string::npos)
			{
				std::string tag = m_raw.substr(startOffset + 1, (endOffset - startOffset) - 1);
				std::string value = Platform::GetEnvironmentVariable(tag);

				m_raw = m_raw.replace(startOffset, endOffset + 1, value);
			}
			else
			{
				break;
			}

		}
		else
		{
			break;
		}
	}

	char correctSeperator = Seperator;
	char incorrectSeperator = '/';

	if (correctSeperator == incorrectSeperator)
	{
		incorrectSeperator = '\\';	
	}

	// Replace incorrect directory seperators.
	for(;;)
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
	for (;;)
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
	char seperatorSequence[3] = { Seperator, '.', '\0' };

	if (IsAbsolute() && m_raw.find(seperatorSequence) != std::string::npos)
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

				// If we are the first character, make sure to append the seperator so linux paths work.
				if (i == 0)
				{
					result = chr + result;
				}
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

	//printf("original=%s raw=%s\n", original.c_str(), m_raw.c_str());
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
	size_t minDirCount = MB_MIN(fragmentsDirCount, destFragmentsDirCount);

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
	if (paths.size() == 0)
	{
		result = "";
		return false;
	}

	size_t maxOffset = INT_MAX;
	for (Path& path : paths)
	{
		maxOffset = MB_MIN(maxOffset, path.ToString().size());
	}

	std::string matchPath = "";
	size_t matchOffset = 0; 

	for (;;)
	{
		// Find the next seperator.
		size_t offset = paths[0].ToString().find(Seperator, matchOffset);
		if (offset == std::string::npos)
		{
			offset = paths[0].ToString().size();
		}

		// The segment to match against the other paths is everything we haven't
		// matched so far.
		std::string main = paths[0].ToString().substr(0, offset);

		bool bMatching = true;

		// Check each path matches the current part we are checking.
		for (int i = 1; i < (int)paths.size(); i++)
		{
			std::string sub = paths[i].ToString().substr(0, offset);
			size_t subOffset = paths[i].ToString().find(Seperator, matchOffset);
			if (subOffset == std::string::npos)
			{
				subOffset = paths[i].ToString().size();
			}

			if (offset != subOffset || sub != main)
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
	if (commonPath.m_raw.size() + 1 > m_raw.size())
	{
		return m_raw;
	}
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
	
	//Log(LogSeverity::Verbose, "[MatchFilter_r] %s\n", base.ToString().c_str());

	// Nothing to match again? Game over.
	if (matches.size() == 0)
	{
		if (!base.IsDirectory())
		{
			result.push_back(base);
		}
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

			//Log(LogSeverity::Verbose, "[Contains] Dir:%s\n", path.c_str());
		}

		for (std::string& path : files)
		{
			PathMatchFragment frag;
			frag.full = path;
			frag.remaining = path;
			frag.valid = true;
			frag.isDirectory = false;
			potentialMatches.push_back(frag);

			//Log(LogSeverity::Verbose, "[Contains] File:%s\n", path.c_str());
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
				std::vector<Path> subDirs;

				MatchFilter_GetDirectories_r(base, subDirs);

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
					subDirs.push_back(base);
				}

				// Add a non-recursive match to check against for each directory.
				matchStack.insert(matchStack.begin(), "*");

				for (unsigned int i = 0; i < subDirs.size(); i++)
				{
					std::vector<Path> subResult =
						MatchFilter_r(subDirs[i], matchStack);

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
				if (frag.remaining.empty() && frag.valid/* && !frag.isDirectory*/)
				{
					result.push_back(base.AppendFragment(frag.full, true));
				}
			}
		}
	}
	
	return result;
}

bool SplitIntoMatchStack(
	std::string pathString, 
	std::vector<std::string>& matchStack, 
	bool bMergeSeperators = false)
{
	size_t offset = 0;

	// Split up into * ** and path fragments
	while (offset < pathString.size())
	{
		size_t seperatorIndex = pathString.find_first_of(Path::Seperator, offset);
		size_t matchIndex = pathString.find_first_of('*', offset);

		size_t splitIndex = matchIndex;
		if (!bMergeSeperators)
		{
			if (splitIndex == std::string::npos ||
				seperatorIndex < matchIndex)
			{
				splitIndex = seperatorIndex;
			}
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

					return false;
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

	return true;
}

void TrimMatchStackDownToFirstWildcard(
	std::vector<std::string>& matchStack,
	std::string& subValue)
{
	// Start at the first directory before a match value.
	unsigned int firstValidStartIndex = 0;
	std::string seperatorString(1, Path::Seperator);

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

	for (unsigned int i = 0; i <= firstValidStartIndex; i++)
	{
		subValue += matchStack[i];
	}

	matchStack.erase(
		matchStack.begin(),
		matchStack.begin() + (firstValidStartIndex + 1)
	);
}

std::vector<Path> Path::MatchFilter(const Path& path)
{
	//Time::TimedScope scope("Match Filter");

	std::vector<Path> result;

	// No match filters, early-out.
	if (path.ToString().find('*') == std::string::npos)
	{
		result.push_back(path);
		return result;
	}

	// Split into wildcards and fragments.
	std::vector<std::string> matchStack;
	if (!SplitIntoMatchStack(path.ToString(), matchStack))
	{
		result.push_back(path);
		return result;
	}

	/*Log(LogSeverity::Verbose, "=== MatchFilter(%s) ===\n", path.m_raw.c_str());
	for (auto str : matchStack)
	{
		Log(LogSeverity::Verbose, "[Stack] %s\n", str.c_str());
	}*/

	// If we only have one split, we are done.
	if (matchStack.size() == 1)
	{
		result.push_back(matchStack[0]);
	}
	else
	{
		std::string subValue = "";
		TrimMatchStackDownToFirstWildcard(matchStack, subValue);
		result = MatchFilter_r(subValue, matchStack);
	}

	return result;
}

Path Path::GetWorkingDirectory()
{
	char buffer[2048];
	char* result = getcwd(buffer, 2048);
	assert(result != nullptr);
	return result;
}

void Path::SetWorkingDirectory(const Path& other)
{
	int result = chdir(other.m_raw.c_str());
	assert(result == 0);
}

Path Path::GetExecutablePath()
{
	return g_executablePath;
}

void Path::SetExecutablePath(const Path& other)
{
	if (other.IsRelative())
	{
		g_executablePath = Path::GetWorkingDirectory() + other;
	}
	else
	{
		g_executablePath = other;
	}
}

bool MatchEatNeedle(const char*& remaining, const char*& match)
{
	while (true)
	{
		// No characters remaining in needle
		// or remaining buffer? Done.
		if (remaining[0] == '\0' &&
			match[0] == '\0')
		{
			return true;
		}

		// Ran out of match characters, Done.
		if (match[0] == '\0')
		{
			return true;
		}

		// Ran out of characters. No match.
		if (remaining[0] == '\0')
		{
			return false;
		}

		// No recursive matching please.
		if (remaining[0] == Path::Seperator)
		{
			return false;
		}

		// Check needle.
		char needleChr = *remaining;
		char matchChr = *match;

		if (needleChr == matchChr)
		{
			remaining++;
			match++;
		}
		else
		{
			return false;
		}
	}

	return false;
}

bool DoesMatchStartWith(const char* data, std::string& needle)
{
	for (unsigned int i = 0; i < needle.size(); i++)
	{
		if (data[i] == '\0')
		{
			return false;
		}
		if (data[i] != needle[i])
		{
			return false;
		}
	}
	return true;
}

bool Matches_r(
	std::vector<std::string>& matchStack,
	int matchStackIndex,
	const char* remaining,
	std::string& matched,
	std::string& matchedSegment)
{
	static std::string seperatorString(1, Path::Seperator);

	// Out of both stacks? We're done!
	if (matchStackIndex >= (int)matchStack.size() && remaining[0] == '\0')
	{
		return true;
	}

	// Out of matches, but still have path? No match.
	// Out of path, but still have matches? No match.
	else if (matchStackIndex >= (int)matchStack.size() || remaining[0] == '\0')
	{
		return false;
	}

	std::string match = matchStack[matchStackIndex++];
	
	// Non-recursive/recursive match.
	if (match == "*" || match == "**")
	{
		bool bIsRecursive = (match == "**");

		// Search forward until next match.
		if (matchStackIndex < (int)matchStack.size())
		{
			std::string& needle = matchStack[matchStackIndex++];

			while (true)
			{
				// Ran out of characters. No match.
				if (remaining[0] == '\0')
				{
					return false;
				}

				// Check if next part of data matches needle. If it does
				// we are done.
				if (DoesMatchStartWith(remaining, needle))
				{
					if (needle[0] == Path::Seperator)
					{
						matched += matchedSegment;
						matched += Path::Seperator;
						matchedSegment = "";
					}

					remaining += needle.size();
					break;
				}

				// Keep track of matched path.
				matchedSegment += remaining[0];
				if (remaining[0] == Path::Seperator)
				{
					matched += matchedSegment;
					matchedSegment = "";
				}

				// No recursive matching please. We have finished matching this
				// wildcard.
				if (remaining[0] == Path::Seperator && !bIsRecursive)
				{
					break;
				}

				remaining++;
			}
		}

		// Accept anything remaining in this directory.
		else
		{
			while (true)
			{
				// Got to end of data, we're done.
				if (remaining[0] == '\0')
				{
					return true;
				}

				// Keep track of matched path.
				matchedSegment += remaining[0];
				if (remaining[0] == Path::Seperator)
				{
					matched += matchedSegment;
					matchedSegment = "";
				}

				// Got to a new directory, no match.
				if (remaining[0] == Path::Seperator && !bIsRecursive)
				{
					return false;
				}

				remaining++;
			}
		}
	}

	// Directory split match.
	else if (match == seperatorString)
	{
		if (remaining[0] != Path::Seperator)
		{
			return false;
		}
		remaining++;
	}

	// Literal match.
	else 
	{
		const char* matchPtr = match.c_str();
		if (!MatchEatNeedle(remaining, matchPtr))
		{
			return false;
		}
	}

	return Matches_r(matchStack, matchStackIndex, remaining, matched, matchedSegment);
}

bool Path::Matches(const Path& other, Path* matched)
{
	// Split into wildcards and fragments.
	std::vector<std::string> matchStack;
	if (!SplitIntoMatchStack(other.ToString(), matchStack, true))
	{
		return false;
	}

	// No wildcards? Straight match.
	if (matchStack.size() == 1)
	{
		return (*this == other);
	}

	// Time to do matching shenanigans.
	else
	{
		std::string basePath = "";
		TrimMatchStackDownToFirstWildcard(matchStack, basePath);

		// Base paths match?
		if (m_raw.size() < basePath.size() || 
			m_raw.substr(0, basePath.size()) == basePath)
		{
			std::string remaining = (m_raw.size() < basePath.size() ? "" : m_raw.substr(basePath.size()));

			std::string matchedStr = "";
			std::string matchedSegmentStr = "";
			bool result = Matches_r(matchStack, 0, remaining.c_str(), matchedStr, matchedSegmentStr);

			if (matched != nullptr)
			{
				*matched = matchedStr;
			}

			return result;
		}

		// Nope not a match then.
		else
		{
			return false;
		}
	}

	return true;
}

bool Path::FindFile(const std::string& filename, Platform::Path& result)
{
	std::vector<Platform::Path> dirs;
	return FindFile(filename, result, dirs);
}

bool Path::FindFile(const std::string& filename, Platform::Path& result, std::vector<Platform::Path> additionalDirs)
{
	std::string pathEnv = Platform::GetEnvironmentVariable("PATH");
	
#if defined(MB_PLATFORM_WINDOWS)
	std::vector<std::string> pathDirs = Strings::Split(';', pathEnv);
#else
	std::vector<std::string> pathDirs = Strings::Split(':', pathEnv);
#endif

	for (Platform::Path& path : additionalDirs)
	{
		pathDirs.insert(pathDirs.begin(), path.ToString());
	}

	// Look for explicit match
	for (Platform::Path path : pathDirs)
	{
		Platform::Path location = path.AppendFragment(filename, true);
		if (location.Exists())
		{
			result = location;
			return true;
		}
	}

	// If no extension given, try and match any file with the same basename.
	if (filename.find('.') == std::string::npos)
	{
		for (Platform::Path path : pathDirs)
		{
			for (std::string& file : path.GetFiles())
			{
				Platform::Path location = path.AppendFragment(file, true);
				if (location.Exists() && location.GetBaseName() == filename)
				{
					result = location;
					return true;
				}

			}
		}
	}

	return false;
}

}; // namespace Platform
}; // namespace MicroBuild
