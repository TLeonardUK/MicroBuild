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
#include "Core/Platform/Process.h"
#include "Core/Helpers/Time.h"

#include <cstdlib>
#include <sstream>
#include <cstring>

namespace MicroBuild {
namespace Platform {

std::string GetEnvironmentVariable(const std::string& tag)
{
	const char* ptr = std::getenv(tag.c_str());
	if (ptr == nullptr)
	{
		return "";
	}
	else
	{
		return ptr;
	}
}

std::string Process::ReadToEnd(bool bPrintOutput)
{
	//Time::TimedScope scope("ReadToEnd:%s", false);

	//MB_UNUSED_PARAMETER(bPrintOutput);

	std::stringstream stream;

	const int bufferSize = 128;
	std::vector<char> buffer;
	buffer.resize(bufferSize);

	while (!AtEnd())
	{
		size_t bytesToRead = bufferSize - 1;
		size_t readBytes = Read(buffer.data(), bytesToRead);
		buffer[readBytes] = '\0';

		if (bPrintOutput)
		{
			Log(LogSeverity::Info, "%s", buffer.data());
		}

		stream << buffer.data();

		if (readBytes == 0)
		{
			break;
		}
	}

	Wait();

	return stream.str();
}

std::string Process::ReadLine()
{
	std::string stream;
	stream.reserve(128);

	while (!AtEnd())
	{
		char buffer = '\0'; 
		size_t readBytes = Read(&buffer, 1);
		if (readBytes == 0)
		{
			break;
		}

		if (buffer != '\n')
		{
			stream.push_back(buffer);
		}
		else
		{
			break;
		}
	}

	// If the result has a \r on the end, remove it to normalize line endings.
	if (stream[stream.size() - 1] == '\r')
	{
		stream.resize(stream.size() - 1);
	}

	return stream;
}

size_t Process::Write(void* buffer, uint64_t bufferLength)
{
	return Internal_Write(buffer, bufferLength);
}

size_t Process::Read(void* buffer, uint64_t bufferLength)
{
	uint8_t* ptr = (uint8_t*)buffer;
	uint64_t leftToRead = bufferLength;

	while (leftToRead > 0 && !AtEnd())
	{
		// Try and fulfill from buffer.
		if (m_readBuffer.size() > 0)
		{
			size_t amountToRead = MB_MIN(m_readBuffer.size(), leftToRead);
			memcpy(ptr, m_readBuffer.data(), amountToRead);

			if (amountToRead >= m_readBuffer.size())
			{
				m_readBuffer.clear();
			}
			else
			{
				size_t finalSize = m_readBuffer.size() - amountToRead;
				
				memcpy(m_readBuffer.data(), m_readBuffer.data() + amountToRead, finalSize);

				m_readBuffer.resize(finalSize);
			}

			leftToRead -= amountToRead;
			ptr += amountToRead;
		}

		// If more is required, read the next chunk.
		if (leftToRead > 0 && m_readBuffer.size() <= 0)
		{
			m_readBuffer.resize(BufferChunkSize);
			size_t totalRead = Internal_Read(m_readBuffer.data(), BufferChunkSize);
			m_readBuffer.resize(totalRead);
		}
	}

	return (bufferLength - leftToRead);
}

uint64_t Process::BytesLeft()
{
	return Internal_BytesLeft() + m_readBuffer.size();
}

bool Process::AtEnd()
{
	// Early out if we have anything in the read buffer.
	return m_readBuffer.size() <= 0 && !IsRunning() && BytesLeft() <= 0;
}

}; // namespace Platform
}; // namespace MicroBuild
