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
#include "App/Builder/Toolchains/Cpp/Gcc/Toolchain_GccOutputParser.h"

namespace MicroBuild {

Toolchain_GccOutputParser::Toolchain_GccOutputParser()
    : ToolchainOutputParser()
{
	// MyFile.cpp:100:100: error: variable or field 'f' declared void
	RegisterOutput(
		R"(^(.*):(\d+):(\d+): (fatal|error|warning|info|message|note): (.*)$)",
		{
			EToolchainCaptureType::Origin,
			EToolchainCaptureType::Line,
			EToolchainCaptureType::Column,
			EToolchainCaptureType::Type,
			EToolchainCaptureType::Message,
		}
	);
		
	// MyFile.cpp:100: variable or field 'f' declared void
	RegisterOutput(
		R"(^(.*):(\d+): (.*)$)",
		{
			EToolchainCaptureType::Origin,
			EToolchainCaptureType::Line,
			EToolchainCaptureType::Message,
		}
	);

	// MyFile.cpp: error: variable or field 'f' declared void
	RegisterOutput(
		R"(^(.*): (fatal|error|warning|info|message|note): (.*)$)",
		{
			EToolchainCaptureType::Origin,
			EToolchainCaptureType::Type,
			EToolchainCaptureType::Message,
		}
	);

	// tool: warning: xxxx
	RegisterOutput(
		R"(^(.*): (fatal|error|warning|info|message|note|file): (.*)$)",
		{
			EToolchainCaptureType::Origin,
			EToolchainCaptureType::Type,
			EToolchainCaptureType::Message,
		}
	);

	// warning: tool: xxxx
	RegisterOutput(
		R"(^(fatal|error|warning|info|message|note|file): (.*): (.*)$)",
		{
			EToolchainCaptureType::Type,
			EToolchainCaptureType::Origin,
			EToolchainCaptureType::Message,
		}
	);
}

void Toolchain_GccOutputParser::Test()
{
	// MyFile.cpp:100:100: error: variable or field 'f' declared void
	TestOutput(
		R"(MyFile.cpp:100:100: error: variable or field 'f' declared void)",
		{
			"MyFile.cpp",
			"100",
			"100",
			"error",
			"variable or field 'f' declared void"
		}
	);
	
	// MyFile.cpp:100: variable or field 'f' declared void
	TestOutput(
		R"(MyFile.cpp:100: variable or field 'f' declared void)",
		{
			"MyFile.cpp",
			"100",
			"variable or field 'f' declared void"
		}
	);

	// MyFile.cpp: error: variable or field 'f' declared void
	TestOutput(
		R"(MyFile.cpp: error: variable or field 'f' declared void)",
		{
			"MyFile.cpp",
			"error",
			"variable or field 'f' declared void"
		}
	);

	// tool: warning: xxxx
	TestOutput(
		R"(tool: warning: xxxx)",
		{
			"tool",
			"warning",
			"xxxx"
		}
	);

	// warning: tool: xxxx
	TestOutput(
		R"(warning: tool: xxxx)",
		{
			"warning",
			"tool",
			"xxxx"
		}
	);	
}

}; // namespace MicroBuild
