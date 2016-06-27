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

#include <algorithm>

namespace MicroBuild {
namespace Strings {

bool CaseInsensitiveEquals(const std::string& a, const std::string& b)
{
	return ToLowercase(a) == ToLowercase(b);
}

std::string ToLowercase(const std::string& input)
{
	std::string result = input;
	std::transform(result.begin(), result.end(), result.begin(), ::tolower);
	return result;
}

std::string Join(int argc, char* argv[], std::string glue)
{
	size_t glueSize = glue.length();
	size_t totalSize = 0;

	for (int i = 0; i < argc; i++)
	{
		totalSize += strlen(argv[i]);
		totalSize += glueSize;
	}

	std::string result;
	result.reserve(totalSize);

	for (int i = 0; i < argc; i++)
	{
		if (i != 0)
		{
			result += glue;
		}
		result += argv[i];
	}

	return result;
}

std::vector<std::string> Crack(std::string value, char seperator)
{
	std::vector<std::string> result;
	std::string segment = "";
	bool inQuotes = false;

	for (size_t i = 0; i < value.length(); i++)
	{
		char chr = value[i];

		// End of value.
		if (chr == seperator && !inQuotes)
		{
			if (segment.length() > 0)
			{
				result.push_back(segment);
			}
			segment.clear();
			continue;
		}
		
		// Quote value start.
		else if (chr == '"')
		{
			inQuotes = !inQuotes;
			continue;
		}
		
		// Quote escape.
		else if (chr == '\\' && inQuotes)
		{
			if (i + 1 < value.length())
			{
				if (value[i + 1] == '"')
				{
					i++;
					continue;
				}
			}
		}

		segment += chr;
	}

	return result;
}

void SplitOnIndex(const std::string& input, size_t index,
	std::string& leftOut, std::string& rightOut)
{
	// These temporaries are intentional as its common to do things like this with this function:
	// Consumable.Split(10, Consumable, Consumed)

	std::string leftValue = input.substr(0, index);
	std::string rightValue = input.substr(index + 1);

	leftOut = leftValue;
	rightOut = rightValue;
}

bool ReadFile(const Platform::Path& path, std::string& output)
{
	FILE* file = fopen(path.ToString().c_str(), "r");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		long length = ftell(file);
		fseek(file, 0, SEEK_SET);

		std::vector<char> data;
		data.resize(length);

		int ret = fread(data.data(), 1, length, file);
		data.resize(ret);

		fclose(file);

		output = std::string(data.data(), data.size());

		return true;
	}

	return false;
}

bool WriteFile(const Platform::Path& path, const std::string& data)
{
	FILE* file = fopen(path.ToString().c_str(), "w");
	if (file)
	{
		int ret = fwrite(data.c_str(), 1, data.size(), file);
		if (ret != data.size())
		{
			return false;
		}

		fclose(file);

		return true;
	}

	return false;
}

std::string Format(std::string format, ...)
{
	va_list args;
	va_start(args, format);
	std::string result = FormatVa(format, args);
	va_end(args);

	return result;
}

std::string FormatVa(std::string format, va_list args)
{
	const int STACK_BUFFER_SIZE = 512;

	char stackBuffer[STACK_BUFFER_SIZE];
	char* buffer = stackBuffer;

	int bytesRequired = vsnprintf(buffer, STACK_BUFFER_SIZE, format.c_str(), args);
	if (bytesRequired >= STACK_BUFFER_SIZE - 1)
	{
		buffer = new char[bytesRequired + 1];
		vsnprintf(buffer, bytesRequired + 1, format.c_str(), args);
	}

	std::string result = buffer;

	if (buffer != stackBuffer)
	{
		delete[] buffer;
	}

	return result;
}

}; // namespace Strings
}; // namespace MicroBuild