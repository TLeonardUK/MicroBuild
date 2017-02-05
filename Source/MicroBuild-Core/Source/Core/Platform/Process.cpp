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

}; // namespace Platform
}; // namespace MicroBuild
