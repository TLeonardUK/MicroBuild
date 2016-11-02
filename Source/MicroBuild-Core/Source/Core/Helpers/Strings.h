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

namespace MicroBuild {
namespace Strings {

// Does a case-inesitive equality comparison between two strings.
bool CaseInsensitiveEquals(const std::string& a, const std::string& b);

// Converts an std::string to lowercase.
std::string ToLowercase(const std::string& input);

// Joins the given string list with the given glue value inbetween each
// part.
std::string Join(int argc, char* argv[], std::string glue);
std::string Join(const std::vector<std::string>& needles, std::string glue);

// Cracks the given string into multiple values. This works like the command 
// line syntax - eg, you can quote values to prevent them being cracked.
std::vector<std::string> Crack(std::string value, char seperator = ' ', bool bKeepQuotes = false);

// Splits a string into a left and right fragment at the given character 
// position.
void SplitOnIndex(const std::string& input, size_t index, 
	std::string& leftOut, std::string& rightOut);

// Returns the last index of any of the needles in the input string.
size_t LastIndexOfAny(const std::string& input, 
	std::vector<std::string>& needles);

// Returns the first index of any of the needles in the input string.
size_t IndexOfAny(const std::string& input,
	std::vector<std::string>& needles);

// Reads a file from the given path into a string.
bool ReadFile(const Platform::Path& path, std::string& data);

// Writes a string to the given path.
bool WriteFile(const Platform::Path& path, const std::string& data);

// Formats the string with the given format and arguments.
std::string Format(std::string format, ...);

// Formats the string with the given format and arguments.
std::string FormatVa(std::string format, va_list list);

// Hashes a string. Not particularly collision tolerant, but should be fine
// for our purposes.
unsigned int Hash(const std::string& value, unsigned int start = 0);
uint64_t Hash64(const std::string& value, uint64_t start = 0);

// Trims whitespace from the start and end of a value.
std::string Trim(const std::string& input);

// Splits the string into fragments deliminated by the given seperator.
std::vector<std::string> Split(char seperator, const std::string& value);

// Generates a guid given the key data.
std::string Guid(const std::vector<std::string>& values);

// Generates a uuid of the given length from the given the key data.
std::string Uuid(int length, const std::vector<std::string>& values);

// Escapes any quotes in the string.
std::string Escaped(const std::string& input, bool bEscapeSequences = false);

// Escapes any quotes in the string and then surrounds with quotes.
std::string Quoted(const std::string& input, bool bEscapeSequences = false);

// Escapes any quotes in the string.
std::string SpacesEscaped(const std::string& input);

// Resmoves quote at start and end of string if it contains them.
std::string StripQuotes(const std::string& input);

// Checks if a string matches the given pattern that can contain several
// wildcards.
bool IsMatch(const std::string& input, const std::string& pattern);

// Replaces a string withing a string.
std::string Replace(const std::string& context, const std::string& from, const std::string& to);

}; // namespace StringHelper
}; // namespace MicroBuild