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

#include "PCH.h"
#include "Core/Platform/Path.h"

#ifdef MB_PLATFORM_WINDOWS

#include <Windows.h>

namespace MicroBuild {
namespace Platform {

char Path::Seperator = '\\';

std::vector<Path> Path::GetFiles() const
{
	std::vector<Path> Result;

	WIN32_FIND_DATAA Data;
	HANDLE Handle;

	std::string Pattern = m_cachedString + Seperator + "*";

	Handle = FindFirstFileA(Pattern.c_str(), &Data);
	if (Handle != INVALID_HANDLE_VALUE)
	{
		do
		{
			std::string Filename = Data.cFileName;
			if (Filename != "." && Filename != "..")
			{
				Path FullPath = AppendFragment(Filename);

				if (FullPath.IsFile())
				{
					Result.push_back(Filename);
				}
			}
		} while (FindNextFileA(Handle, &Data) != 0);

		FindClose(Handle);
	}

	return Result;
}

std::vector<Path> Path::GetDirectories() const
{
	std::vector<Path> Result;

	WIN32_FIND_DATAA Data;
	HANDLE Handle;

	std::string Pattern = m_cachedString + Seperator + "*";

	Handle = FindFirstFileA(Pattern.c_str(), &Data);
	if (Handle != INVALID_HANDLE_VALUE)
	{
		do
		{
			std::string Filename = Data.cFileName;
			if (Filename != "." && Filename != "..")
			{
				Path FullPath = AppendFragment(Filename);

				if (FullPath.IsDirectory())
				{
					Result.push_back(Filename);
				}
			}
		} while (FindNextFileA(Handle, &Data) != 0);

		FindClose(Handle);
	}

	return Result;
}

bool Path::IsFile() const
{
	DWORD flags = GetFileAttributesA(m_cachedString.c_str());

	if (flags == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}

	if ((flags & FILE_ATTRIBUTE_DIRECTORY) != 0)
	{
		return false;
	}

	return true;
}

bool Path::IsDirectory() const
{
	DWORD flags = GetFileAttributesA(m_cachedString.c_str());

	if (flags == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}

	if ((flags & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		return false;
	}

	return true;
}

bool Path::CreateAsDirectory() const
{
	Path ParentDir = GetDirectory();

	if (!ParentDir.IsEmpty() && !ParentDir.IsDirectory())
	{
		ParentDir.CreateAsDirectory();
	}

	BOOL Ret = CreateDirectoryA(m_cachedString.c_str(), NULL);
	return (Ret != 0);
}

bool Path::CreateAsFile() const
{
	Path ParentDir = GetDirectory();

	if (!ParentDir.IsEmpty() && !ParentDir.IsDirectory())
	{
		ParentDir.CreateAsDirectory();
	}

	HANDLE Ret = CreateFileA(
		m_cachedString.c_str(),
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (Ret == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	else
	{
		CloseHandle(Ret);
		return true;
	}
}

bool Path::Exists() const
{
	DWORD flags = GetFileAttributesA(m_cachedString.c_str());
	return !(flags == INVALID_FILE_ATTRIBUTES);
}

bool Path::Copy(const Path& Destination) const
{
	if (IsFile())
	{
		BOOL Ret = CopyFileA(m_cachedString.c_str(), 
			Destination.m_cachedString.c_str(), false);
		if (Ret == 0)
		{
			return false;
		}
	}
	else
	{
		std::vector<Path> SubDirs = GetDirectories();
		std::vector<Path> SubFiles = GetFiles();

		// If directory dosen't exist, get creating.
		if (!Destination.Exists())
		{
			if (!Destination.CreateAsDirectory())
			{
				return false;
			}
		}

		// Copy all sub-directories.
		for (Path& path : SubDirs)
		{
			if (!path.Copy(path.ChangeDirectory(Destination)))
			{
				return false;
			}
		}

		// Copy all sub files.
		for (Path& path : SubFiles)
		{
			if (!path.Copy(path.ChangeDirectory(Destination)))
			{
				return false;
			}
		}
	}

	return true;
}

bool Path::Delete() const
{
	if (IsFile())
	{
		BOOL Ret = DeleteFileA(m_cachedString.c_str());
		return (Ret != 0);
	}
	else if (IsDirectory())
	{
		std::vector<Path> SubDirs = GetDirectories();
		std::vector<Path> SubFiles = GetFiles();

		// Delete all sub directories.
		for (Path& path : SubDirs)
		{
			Path FullPath = *this + path;
			FullPath.Delete();
		}

		// Delete all sub files.
		for (Path& path : SubFiles)
		{
			Path FullPath = *this + path;
			FullPath.Delete();
		}

		// Delete the actual folder.
		BOOL Ret = RemoveDirectoryA(m_cachedString.c_str());
		return (Ret != 0);
	}

	return false;
}

std::time_t Path::GetModifiedTime() const
{
	WIN32_FILE_ATTRIBUTE_DATA Attributes;
	BOOL Result = GetFileAttributesExA(m_cachedString.c_str(), 
		GetFileExInfoStandard, &Attributes);
	if (!Result)
	{
		return 0ULL;
	}
	else
	{
		ULARGE_INTEGER ull;
		ull.LowPart = Attributes.ftLastWriteTime.dwLowDateTime;
		ull.HighPart = Attributes.ftLastWriteTime.dwHighDateTime;
		return ull.QuadPart / 10000000ULL - 11644473600ULL;
	}
}

}; // namespace Platform
}; // namespace MicroBuild

#endif // MB_PLATFORM_WINDOWS