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

#include "App/Builder/Toolchains/ToolchainOutputParser.h"

#include "Core/Helpers/StringConverter.h"

#include <cassert>

namespace MicroBuild {

void ToolchainOutputParser::RegisterOutput(const std::string& regex, const std::vector<EToolchainCaptureType>& captureTypes)
{
	MatchType type;

	try
	{
		type.Pattern = std::regex(regex, std::regex_constants::ECMAScript | std::regex_constants::icase);
	}
	catch (const std::regex_error& e) 
	{
		Log(LogSeverity::Fatal, "Regex error (code %i) caught (%s) for pattern: %s", e.code(), e.what(), regex.c_str());
		assert(false);
	}

	type.CaptureTypes = captureTypes;
	m_matchTypes.push_back(type);
}

void ToolchainOutputParser::ExtractMessages(const std::string& input, std::vector<ToolchainOutputMessage>& extractedOutput)
{
	// Iterate through each line and see if its matches anything.
	size_t startOffset = 0;
	while (startOffset < input.size())
	{
		// Find the end of this line.
		size_t endOffset = input.find("\n", startOffset);
		if (endOffset == std::string::npos)
		{
			// Everything from the last newline to the end of the input.
			endOffset = input.size();
		}

		// Extract line to parse.
		std::string line = input.substr(startOffset, endOffset - startOffset);

		// Strip off the \r of \r\n combos.
		if (line[line.size() - 1] == '\r')
		{
			line.resize(line.size() - 1);
		}

		// Check if the line contains any tags we are intrested in. Allows us to early out
		// of doing the expensive regex matching.
		if (line.find("error")		!= std::string::npos ||
			line.find("fatal")		!= std::string::npos ||
			line.find("warning")	!= std::string::npos ||
			line.find("info")		!= std::string::npos ||
			line.find("message")	!= std::string::npos ||
			line.find("note")		!= std::string::npos ||
			line.find("fatal error") != std::string::npos ||
			line.find("Error")		!= std::string::npos ||
			line.find("Fatal")		!= std::string::npos ||
			line.find("Warning")	!= std::string::npos ||
			line.find("Info")		!= std::string::npos ||
			line.find("Message")	!= std::string::npos ||
			line.find("Note")		!= std::string::npos ||
			line.find("Fatal Error") != std::string::npos)
		{
			if (line.find("Note: including file:") == std::string::npos)
			{
				for (MatchType& matchType : m_matchTypes)
				{
					std::smatch match;
					if (std::regex_search(line, match, matchType.Pattern))
					{
						ToolchainOutputMessage message;
						message.Origin = "";
						message.Line = 1;
						message.Column = 1;
						message.Identifier = "";
						message.Type = EToolchainOutputMessageType::Info;
						message.Text = "";

						for (size_t i = 1; i < match.size() && i - 1 < matchType.CaptureTypes.size(); i++)
						{
							std::string captureValue = match[i];

							switch (matchType.CaptureTypes[i - 1])
							{
							case EToolchainCaptureType::Origin:
							{
								message.Origin = captureValue;
								break;
							}
							case EToolchainCaptureType::Type:
							{
								if (captureValue == "fatal" || captureValue == "error" || captureValue == "fatal error" ||
									captureValue == "Fatal" || captureValue == "Error" || captureValue == "Fatal Error")
								{
									message.Type = EToolchainOutputMessageType::Error;
								}
								else if (captureValue == "warning" ||
									captureValue == "Warning")
								{
									message.Type = EToolchainOutputMessageType::Warning;
								}
								else if (captureValue == "info" || captureValue == "message" || captureValue == "note" ||
									captureValue == "Info" || captureValue == "Message" || captureValue == "Note")
								{
									message.Type = EToolchainOutputMessageType::Info;
								}
								else
								{
									// We should never get here unless someone wrongly sets up the regex pattern.
									assert(false);
								}
								break;
							}
							case EToolchainCaptureType::Identifier:
							{
								message.Identifier = captureValue;
								break;
							}
							case EToolchainCaptureType::Line:
							{
								message.Line = CastFromString<int>(captureValue);
								break;
							}
							case EToolchainCaptureType::Column:
							{
								message.Column = CastFromString<int>(captureValue);
								break;
							}
							case EToolchainCaptureType::Message:
							{
								message.Text = captureValue;
								break;
							}
							default:
							{
								// We should never get here unless someone fucked up.
								assert(false);
								break;
							}
							}
						}

						extractedOutput.push_back(message);
						break;
					}
				}
			}
		}

		startOffset = endOffset + 1;
	}
}

void ToolchainOutputParser::Test()
{
}

void ToolchainOutputParser::TestOutput(const std::string& input, const std::vector<std::string>& expectedCaptures)
{
	for (MatchType& matchType : m_matchTypes)
	{
		std::smatch match;
		if (std::regex_search(input, match, matchType.Pattern))
		{
			if (match.size() != expectedCaptures.size() + 1)
			{
				assert(false);
			}

			for (size_t i = 0; i < expectedCaptures.size(); i++)
			{
				if (match[i + 1] != expectedCaptures[i])
				{
					assert(false);
				}
			}

			return;
		}
	}

	assert(false);
}

}; // namespace MicroBuild
