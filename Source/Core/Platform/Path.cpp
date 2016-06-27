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
	int MinDirCount = min(
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

	int DirsToGoUp = m_directories.size() - MatchingDirs;
	for (int i = 0; i < DirsToGoUp; i++)
	{
		Result.m_directories.push_back("..");
	}

	int DirsToAdd = Destination.m_directories.size() - MatchingDirs;
	for (int i = 0; i < DirsToAdd; i++)
	{
		Result.m_directories.push_back(
			Destination.m_directories[MatchingDirs + i]);
	}

	Result.UpdateCachedString();

	return Result;
}

}; // namespace Platform
}; // namespace MicroBuild