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
#include "Core/Helpers/BinaryStream.h"

#include <sstream>

namespace MicroBuild {

// This class is a super-simple implementation of a zip file writer, it takes
// in various directories and paths and compresses them together.
class ZipFile
{
public:
	ZipFile(const ZipFile& node) = delete;
	ZipFile& operator=(const ZipFile&) = delete;

	ZipFile();
	~ZipFile();

	bool Open(const Platform::Path& path);
	void Close();

	bool AddDirectory(const Platform::Path& source, const Platform::Path& destination);
	bool AddFile(const Platform::Path& source, const Platform::Path& destination);
	
private:
	struct ZipFileBlock
	{
		uint32_t crc32;
		uint32_t offset;
		uint32_t fileSize;
		std::string name;

		Platform::Path source;
		Platform::Path destination;
	};

	std::vector<ZipFileBlock> m_blocks;
	BinaryStream m_stream;

};

}; // namespace MicroBuild