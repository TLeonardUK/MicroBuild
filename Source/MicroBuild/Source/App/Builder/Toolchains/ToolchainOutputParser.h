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

#include <regex>

namespace MicroBuild {

// Defines the type of information a single capture group in a parser regex
// is extracting.
enum class EToolchainCaptureType
{
	Origin,				// Filename/Program
	Type,				// Error type. Should be one of: fatal, error, warning, message, info. One of these is mandatory, we don't parse lines without this.
	Identifier,			// Unique identifier for the error, compiler specific (eg. C1234 for msbuild).
	Line,
	Column,
	Message
};

// Type of toolchain message output.
enum class EToolchainOutputMessageType
{
	Info,
	Warning,
	Error
};

// Stores information on a message extracted from toolchain output.
struct ToolchainOutputMessage
{
	// File or tool that this message originated from.
	Platform::Path				Origin;

	// Line number this messaged originated on.
	int							Line;

	// Column number this messaged originated on.
	int							Column;

	// Internal identifier of this message (per-toolchain), eg. C4001 for msbuild
	std::string					Identifier;

	// Type of message - Error/Warning/etc.
	EToolchainOutputMessageType	Type;

	// The main text of this message.
    std::string					Text;

    // Extended multi-line error message.
    std::vector<std::string>    Details;
};

// This class is used as a generic parser to extract warning/message/error's from build output.
// Most toolchains will create a derived version of this class that registers the appropriate error format regex's in its constructor.
// The toolchain then passes its tool output into one of the functions to extract meaningful information out.
class ToolchainOutputParser
{
private:
	struct MatchType
	{
		std::regex Pattern;
		std::vector<EToolchainCaptureType> CaptureTypes;
	};
	std::vector<MatchType> m_matchTypes;

protected:

	// Checks if extracting the given input results in regex captures matching those passed in. Asserts on failure.
	void TestOutput(const std::string& input, const std::vector<std::string>& expectedCaptures);

public:

    ToolchainOutputParser();

	// Registers a new regex used to extract messages.
	// The capture types define what information each capture group in the regex is extracting (filename/line-number/etc).
	void RegisterOutput(const std::string& regex, const std::vector<EToolchainCaptureType>& captureTypes);

	// Attempts to extract any messages in the output. Output is written into the given array.
	void ExtractMessages(const std::string& input, std::vector<ToolchainOutputMessage>& extractedOutput);

	// Can be overridden in derived parses to perform validity tests, handy for debugging.
	virtual void Test();

}; 

}; // namespace MicroBuild
