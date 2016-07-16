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

#include "PCH.h"
#include "Core/Helpers/Strings.h"
#include "Core/Helpers/BinaryStream.h"

#include <algorithm>

namespace MicroBuild {

uint32_t BinaryStream::s_crc32Table[256];
bool BinaryStream::s_crc32TableInit = false;

BinaryStream::BinaryStream()
	: m_file(nullptr)
{
}

BinaryStream::~BinaryStream()
{
	if (m_file != nullptr)
	{
		Close();
	}
}

bool BinaryStream::Open(const Platform::Path& path, bool bWrite)
{
	if (m_file != nullptr)
	{
		Close();
	}

	Platform::Path dir = path.GetDirectory();
	if (!dir.Exists())
	{
		if (!dir.CreateAsDirectory())
		{
			return false;
		}
	}

	if (bWrite)
	{
		m_file = fopen(path.ToString().c_str(), "wb");
	}
	else
	{
		m_file = fopen(path.ToString().c_str(), "rb");
	}

	return (m_file != nullptr);
}

void BinaryStream::Close()
{
	if (m_file != nullptr)
	{
		fclose(m_file);
		m_file = nullptr;
	}
}

void BinaryStream::WriteBuffer(const char* buffer, size_t bufferLength)
{
	fwrite(buffer, 1, bufferLength, m_file);
}

void BinaryStream::ReadBuffer(char* buffer, size_t bufferLength)
{
	fread(buffer, 1, bufferLength, m_file);
}

uint32_t BinaryStream::Crc32()
{
	if (!s_crc32TableInit)
	{
		uint32_t polynomial = 0xEDB88320;;
		uint32_t crc32 = 0;
		for (int i = 0; i < k_Crc32TableSize; i++)
		{
			crc32 = i;
			for (int j = 8; j > 0; j--)
			{
				if ((crc32 & 1) == 1)
				{
					crc32 = (crc32 >> 1) ^ polynomial;
				}
				else
				{
					crc32 >>= 1;
				}
			}
			s_crc32Table[i] = crc32;
		}
		s_crc32TableInit = true;
	}

	Seek(0);
	uint32_t crc32 = 0xFFFFFFFF;;
	uint32_t len = Length();

	while (Offset() < len)
	{
		uint8_t chr = Read<uint8_t>();
		crc32 = ((crc32) >> 8) ^ s_crc32Table[(chr) ^ ((crc32) & 0x000000FF)];
	}

	return ~crc32;	
}

uint32_t BinaryStream::Length()
{
	uint32_t offset = Offset();

	fseek(m_file, 0, SEEK_END);	
	uint32_t length = Offset();
	
	fseek(m_file, offset, SEEK_SET);

	return length;
}

uint32_t BinaryStream::Offset()
{
	return static_cast<uint32_t>(ftell(m_file));
}

void BinaryStream::Seek(uint32_t offset)
{
	fseek(m_file, offset, SEEK_SET);
}

void BinaryStream::CopyTo(BinaryStream& other)
{
	char buffer[k_ChunkSize];

	uint32_t length = Length();
	uint32_t offset = Offset();
	uint32_t bytesLeft = length - offset;

	while (true)
	{
		uint32_t chunkSize = bytesLeft;

		if (chunkSize > k_ChunkSize)
		{
			chunkSize = k_ChunkSize;
		}
		if (chunkSize <= 0)
		{
			break;
		}

		ReadBuffer(buffer, chunkSize);
		other.WriteBuffer(buffer, chunkSize);

		bytesLeft -= chunkSize;
	}
}

}; // namespace MicroBuild