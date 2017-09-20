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
#include "App/Builder/Toolchains/Cpp/NintendoWiiU/Toolchain_NintendoWiiUOutputParser.h"

namespace MicroBuild {

Toolchain_NintendoWiiUOutputParser::Toolchain_NintendoWiiUOutputParser()
    : ToolchainOutputParser()
{
	// "Path\To\File", line 34: Error:  #20: identifier "zyx" is undefined
	RegisterOutput(
		R"(^\"(.*)\", line (\d+): .*(Fatal|Error|Warning|Info|Message|fatal|error|warning|info|message).*\#(\d+): (.*)$)",
		{
			EToolchainCaptureType::Origin,
			EToolchainCaptureType::Line,
			EToolchainCaptureType::Type,
			EToolchainCaptureType::Identifier,
			EToolchainCaptureType::Message,
		}
	);

	// "Path\To\File": Error:  #20: identifier "zyx" is undefined
	RegisterOutput(
		R"(^\"(.*)\": .*(Fatal|Error|Warning|Info|Message|fatal|error|warning|info|message).*\#(\d+): (.*)$)",
		{
			EToolchainCaptureType::Origin,
			EToolchainCaptureType::Type,
			EToolchainCaptureType::Identifier,
			EToolchainCaptureType::Message,
		}
	);
	
	// Path\To\File, line 34: Error:  #20: identifier "zyx" is undefined
	RegisterOutput(
		R"(^(.*), line (\d+): .*(Fatal|Error|Warning|Info|Message|fatal|error|warning|info|message).*\#(\d+): (.*)$)",
		{
			EToolchainCaptureType::Origin,
			EToolchainCaptureType::Line,
			EToolchainCaptureType::Type,
			EToolchainCaptureType::Identifier,
			EToolchainCaptureType::Message,
		}
	);

	// Path\To\File: Error:  #20: identifier "zyx" is undefined
	RegisterOutput(
		R"(^(.*): .*(Fatal|Error|Warning|Info|Message|fatal|error|warning|info|message).*\#(\d+): (.*)$)",
		{
			EToolchainCaptureType::Origin,
			EToolchainCaptureType::Type,
			EToolchainCaptureType::Identifier,
			EToolchainCaptureType::Message,
		}
	);
}

void Toolchain_NintendoWiiUOutputParser::Test()
{
	// "Path\To\File", line 34: Error:  #20: identifier "zyx" is undefined
	TestOutput(
		R"("Path\To\File", line 34: Error:  #20: identifier "zyx" is undefined)",
		{
			"Path\\To\\File",
			"34",
			"Error",
			"20",
			"identifier \"zyx\" is undefined"
		}
	);

	// "Path\To\File": Error:  #20: identifier "zyx" is undefined
	TestOutput(
		R"("Path\To\File": Error:  #20: identifier "zyx" is undefined)",
		{
			"Path\\To\\File",
			"Error",
			"20",
			"identifier \"zyx\" is undefined"
		}
	);

	// Path\To\File, line 34: Error:  #20: identifier "zyx" is undefined
	TestOutput(
		R"(Path\To\File, line 34: Error:  #20: identifier "zyx" is undefined)",
		{
			"Path\\To\\File",
			"34",
			"Error",
			"20",
			"identifier \"zyx\" is undefined"
		}
	);

	// Path\To\File: Error:  #20: identifier "zyx" is undefined
	TestOutput(
		R"(Path\To\File: Error:  #20: identifier "zyx" is undefined)",
		{
			"Path\\To\\File",
			"Error",
			"20",
			"identifier \"zyx\" is undefined"
		}
	);
}

}; // namespace MicroBuild
