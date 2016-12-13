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

#ifdef MB_PLATFORM_LINUX

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>

namespace MicroBuild {
namespace Platform {

char Path::Seperator = '/';

std::vector<std::string> Path::GetFiles() const
{
	std::vector<std::string> result;
	
	DIR* handle = opendir(m_raw.c_str());
	if (handle == nullptr)
	{	
		//Log(LogSeverity::Warning, "Failed to enumerate directory: %s\n", m_raw.c_str());
		return result;
	}

	//Log(LogSeverity::Verbose, "GetFiles(%s)\n", m_raw.c_str());

	while (true)
	{
		struct dirent* entry = readdir(handle);
		if (entry == nullptr)
		{	
			break;
		}		

		//Log(LogSeverity::Verbose, "\tdirent: name=%s type=%i\n", entry->d_name, entry->d_type);

		struct stat attr;
		int res = stat(AppendFragment(entry->d_name, true).ToString().c_str(), &attr);

		// note: don't use d_type, its unsupported on older kernels.

		if (res == 0 && S_ISREG(attr.st_mode))
		{
			std::string value = entry->d_name;
			result.push_back(value);
		}
	}

	closedir(handle);
	return result;
}

std::vector<std::string> Path::GetDirectories() const
{
	std::vector<std::string> result;
	
	DIR* handle = opendir(m_raw.c_str());
	if (handle == nullptr)
	{
		//Log(LogSeverity::Warning, "Failed to enumerate directory: %s\n", m_raw.c_str());
		return result;
	}

	//Log(LogSeverity::Verbose, "GetDirectories(%s)\n", m_raw.c_str());

	while (true)
	{
		struct dirent* entry = readdir(handle);
		if (entry == nullptr)
		{	
			break;
		}

		//Log(LogSeverity::Verbose, "\tdirent: name=%s type=%i\n", entry->d_name, entry->d_type);

		struct stat attr;
		int res = stat(AppendFragment(entry->d_name, true).ToString().c_str(), &attr);

		// note: don't use d_type, its unsupported on older kernels.

		if (res == 0 && S_ISDIR(attr.st_mode))
		{
			std::string value = entry->d_name;
			if (value != "." && value != "..")
			{
				result.push_back(value);
			}
		}
	}

	closedir(handle);
	return result;
}

bool Path::Copy(const Path& Destination) const
{
	if (IsFile())
	{
		Path DestDirectory = Destination.GetDirectory();

		// If directory dosen't exist, get creating.
		if (!DestDirectory.Exists())
		{
			if (!DestDirectory.CreateAsDirectory())
			{
				return false;
			}
		}

		std::ifstream source(m_raw.c_str(), std::ios::binary);
		std::ofstream dest(Destination.m_raw.c_str(), std::ios::binary);
		if (!source.good() || !dest.good())
		{
			return false;
		}

	    	dest << source.rdbuf();

		source.close();
		dest.close();
	}
	else
	{
		std::vector<std::string> SubDirs = GetDirectories();
		std::vector<std::string> SubFiles = GetFiles();

		// If directory dosen't exist, get creating.
		if (!Destination.Exists())
		{
			if (!Destination.CreateAsDirectory())
			{
				return false;
			}
		}

		// Copy all sub-directories.
		for (std::string& filename : SubDirs)
		{
			Path path = (*this).AppendFragment(filename, true);
			if (!path.Copy(path.ChangeDirectory(Destination)))
			{
				return false;
			}
		}

		// Copy all sub files.
		for (std::string& filename : SubFiles)
		{
			Path path = (*this).AppendFragment(filename, true);
			if (!path.Copy(path.ChangeDirectory(Destination)))
			{
				return false;
			}
		}
	}

	return true;
}

bool Path::IsFile() const
{
	struct stat attr;
	int result = stat(m_raw.c_str(), &attr);
	return (result == 0 && S_ISREG(attr.st_mode));
}

bool Path::IsDirectory() const
{
	struct stat attr;
	int result = stat(m_raw.c_str(), &attr);
	return (result == 0 && S_ISDIR(attr.st_mode));
}

bool Path::CreateAsDirectory() const
{
	int result = mkdir(m_raw.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	return (result == 0);
}

bool Path::CreateAsFile() const
{
	int fd = open(
		m_raw.c_str(), 
		O_CREAT | O_RDWR,  
		S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH
	);
	if (fd <= 0)
	{
		return false;
	}
	close(fd);
	return true;
}

bool Path::Exists() const
{
	struct stat attr;
	int result = stat(m_raw.c_str(), &attr);
	return (result == 0);
}

bool Path::Delete() const
{
	int result = unlink(m_raw.c_str());
	return (result == 0);
}

std::time_t Path::GetModifiedTime() const
{
	struct stat attr;
	int result = stat(m_raw.c_str(), &attr);
	if (result == 0)
	{
		return attr.st_mtime;
	}
	else
	{
		return std::time_t(0);
	}
}

}; // namespace Platform
}; // namespace MicroBuild

#endif // MB_PLATFORM_LINUX
