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

#include <sstream>

namespace MicroBuild {

// This class provides a nice wrapper interface for reading and writing
// binary data to files.
class BinaryStream
{
public:
	BinaryStream();
	~BinaryStream();

	bool Open(const Platform::Path& path, bool bWrite);
	void Close();

	void WriteBuffer(const char* buffer, size_t bufferLength);
	void ReadBuffer(char* buffer, size_t bufferLength);

	template <typename UnderlyingType, typename BaseType>
	void Write(BaseType value)
	{
		UnderlyingType baseType = (UnderlyingType)value;
		WriteBuffer(reinterpret_cast<const char*>(&baseType), sizeof(UnderlyingType));
	}

	template <typename UnderlyingType>
	UnderlyingType Read()
	{
		UnderlyingType value;
		ReadBuffer(reinterpret_cast<char*>(&value), sizeof(UnderlyingType));
		return value;
	}

	uint32_t Crc32();
	uint32_t Length();
	uint32_t Offset();
	void	 Seek(uint32_t offset);

	void CopyTo(BinaryStream& other);

private:
	enum
	{
		k_ChunkSize = 1024 * 8,
		k_Crc32TableSize = 256
	};


	static uint32_t s_crc32Table[k_Crc32TableSize];
	static bool s_crc32TableInit;

	FILE* m_file;

};

}; // namespace MicroBuild