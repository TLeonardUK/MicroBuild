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
#include <inttypes.h>

namespace MicroBuild {
namespace Strings {

bool CaseInsensitiveEquals(const std::string& a, const std::string& b)
{
	if (a.size() != b.size())
	{
		return false;
	}
	for (unsigned int i = 0; i < a.size(); i++)
	{
		if (::tolower(a[i]) != ::tolower(b[i]))
		{
			return false;
		}
	}
	return true;
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

std::string Join(std::vector<std::string>& needles, std::string glue)
{
	std::string result = "";
	for (std::string& res : needles)
	{
		if (!result.empty())
		{
			result += glue;
		}
		result += res;
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

		size_t ret = fread(data.data(), 1, length, file);
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
		size_t ret = fwrite(data.c_str(), 1, data.size(), file);
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

unsigned int Hash(const std::string& value, unsigned int start)
{
	unsigned int hash = start;
	const char* Value = value.c_str();

	for (; *Value; ++Value)
	{
		hash += *Value;
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}

uint64_t Hash64(const std::string& value, uint64_t start)
{
	uint64_t hash = start;
	const char* Value = value.c_str();

	for (; *Value; ++Value)
	{
		hash += *Value;
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}

std::string Trim(const std::string& input)
{
	if (input.size() == 0)
	{
		return "";
	}

	size_t startIndex = 0;
	while (startIndex < input.size())
	{
		if (input[startIndex] == ' ' || input[startIndex] == '\t')
		{
			startIndex++;
		}
		else
		{
			break;
		}
	}

	size_t endIndex = input.size() - 1;
	while (endIndex >= startIndex)
	{
		if (input[endIndex] == ' ' || input[endIndex] == '\t')
		{
			endIndex--;
		}
		else
		{
			break;
		}
	}

	return input.substr(startIndex, (endIndex - startIndex) + 1);
}

std::vector<std::string> Split(char seperator, const std::string& value)
{
	std::vector<std::string> result;

	size_t offset = 0;
	while (offset < value.size())
	{
		size_t endOffset = value.find(seperator, offset);
		if (endOffset != std::string::npos)
		{
			std::string frag = value.substr(offset, endOffset - offset);
			result.push_back(Trim(frag));

			offset = endOffset + 1;
		}
		else
		{
			std::string frag = value.substr(offset);
			result.push_back(Trim(frag));
			break;
		}
	}

	return result;
}

std::string Guid(const std::vector<std::string>& values)
{
	uint64_t hash = 0;

	for (const std::string& res : values)
	{
		hash = Hash64(res, hash);
	}

	char buffer[64];
	sprintf(buffer, "%012" PRIu64, hash);

	std::string result = buffer;
	if (result.size() > 12)
	{
		result.resize(12);
	}

	return Strings::Format("{00000000-0000-0000-0000-%s}", result.c_str());
}

std::string Escaped(const std::string& input)
{
	std::string result;
	result.reserve(input.size());
	for (char chr : input)
	{
		if (chr == '"')
		{
			result.push_back('\\');
		}
		result.push_back(chr);
	}
	return result;
}

}; // namespace Strings
}; // namespace MicroBuild