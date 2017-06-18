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
#include "App/Builder/Toolchains/Cpp/Microsoft/Toolchain_MicrosoftOutputParser.h"

namespace MicroBuild {

Toolchain_MicrosoftOutputParser::Toolchain_MicrosoftOutputParser()
{	
	// https://blogs.msdn.microsoft.com/msbuild/2006/11/02/msbuild-visual-studio-aware-error-messages-and-message-formats/

	// origin(Line): [subcategory] fatal/error/warning/message ErrorNumber: Text
	RegisterOutput(
		R"(^(.*)\((\d+)\): .*(fatal|error|warning|message) (\w+): (.*)$)", 
		{ 
			EToolchainCaptureType::Origin, 
			EToolchainCaptureType::Line,
			EToolchainCaptureType::Type,
			EToolchainCaptureType::Identifier,
			EToolchainCaptureType::Message,
		}
	);

	// origin(Line,Col): [subcategory] fatal/error/warning/message ErrorNumber: Text
	RegisterOutput(
		R"(^(.*)\((\d+)\,(\d+)\): .*(fatal|error|warning|message) (\w+): (.*)$)",
		{
			EToolchainCaptureType::Origin,
			EToolchainCaptureType::Line,
			EToolchainCaptureType::Column,
			EToolchainCaptureType::Type,
			EToolchainCaptureType::Identifier,
			EToolchainCaptureType::Message,
		}
	);

	// origin(Line-Line): [subcategory] fatal/error/warning/message ErrorNumber: Text
	// origin(Line-Col): [subcategory] fatal/error/warning/message ErrorNumber: Text
	RegisterOutput(
		R"(^(.*)\((\d+)\-\d+\): .*(fatal|error|warning|message) (\w+): (.*)$)",
		{
			EToolchainCaptureType::Origin,
			EToolchainCaptureType::Line,
			EToolchainCaptureType::Type,
			EToolchainCaptureType::Identifier,
			EToolchainCaptureType::Message,
		}
	);

	// origin(Line,Col-Col): [subcategory] fatal/error/warning/message ErrorNumber: Text
	RegisterOutput(
		R"(^(.*)\((\d+)\,(\d+)-\d+\): .*(fatal|error|warning|message) (\w+): (.*)$)",
		{
			EToolchainCaptureType::Origin,
			EToolchainCaptureType::Line,
			EToolchainCaptureType::Column,
			EToolchainCaptureType::Type,
			EToolchainCaptureType::Identifier,
			EToolchainCaptureType::Message,
		}
	);

	// origin(Line,Col,Line,Col): [subcategory] fatal/error/warning/message ErrorNumber: Text
	RegisterOutput(
		R"(^(.*)\((\d+)\,(\d+),\d+,\d+\): .*(fatal|error|warning|message) (\w+): (.*)$)",
		{
			EToolchainCaptureType::Origin,
			EToolchainCaptureType::Line,
			EToolchainCaptureType::Column,
			EToolchainCaptureType::Type,
			EToolchainCaptureType::Identifier,
			EToolchainCaptureType::Message,
		}
	);

	// origin: fatal/error/warning/message ErrorNumber: Text
	RegisterOutput(
		R"(^(.*)\s+: .*(fatal error|fatal|error|warning|message) (\w+): (.*)$)",
		{
			EToolchainCaptureType::Origin,
			EToolchainCaptureType::Type,
			EToolchainCaptureType::Identifier,
			EToolchainCaptureType::Message,
		}
	);
}

void Toolchain_MicrosoftOutputParser::Test()
{
	// origin(Line): [subcategory] fatal/error/warning/message ErrorNumber: Text
	TestOutput(
		R"(Path\To\File.c(100): warning C4267: Error description)",
		{ 
			"Path\\To\\File.c",
			"100",
			"warning",
			"C4267",
			"Error description" 
		}
	);
	TestOutput(
		R"(Path\To\File.c(100): command line warning C4267: Error description)",
		{
			"Path\\To\\File.c",
			"100",
			"warning",
			"C4267",
			"Error description"
		}
	);

	// origin(Line,Col): [subcategory] fatal/error/warning/message ErrorNumber: Text
	TestOutput(
		R"(Path\To\File.c(100,200): warning C4267: Error description)",
		{
			"Path\\To\\File.c",
			"100",
			"200",
			"warning",
			"C4267",
			"Error description"
		}
	);
	TestOutput(
		R"(Path\To\File.c(100,200): command line warning C4267: Error description)",
		{
			"Path\\To\\File.c",
			"100",
			"200",
			"warning",
			"C4267",
			"Error description"
		}
	);

	// origin(Line-Line): [subcategory] fatal/error/warning/message ErrorNumber: Text
	// origin(Line-Col): [subcategory] fatal/error/warning/message ErrorNumber: Text
	TestOutput(
		R"(Path\To\File.c(100-200): warning C4267: Error description)",
		{
			"Path\\To\\File.c",
			"100",
			"warning",
			"C4267",
			"Error description"
		}
	);
	TestOutput(
		R"(Path\To\File.c(100-200): command line warning C4267: Error description)",
		{
			"Path\\To\\File.c",
			"100",
			"warning",
			"C4267",
			"Error description"
		}
	);

	// origin(Line,Col-Col): [subcategory] fatal/error/warning/message ErrorNumber: Text
	TestOutput(
		R"(Path\To\File.c(100,200-300): warning C4267: Error description)",
		{
			"Path\\To\\File.c",
			"100",
			"200",
			"warning",
			"C4267",
			"Error description"
		}
	);
	TestOutput(
		R"(Path\To\File.c(100,200-300): command line warning C4267: Error description)",
		{
			"Path\\To\\File.c",
			"100",
			"200",
			"warning",
			"C4267",
			"Error description"
		}
	);

	// origin(Line,Col,Line,Col): [subcategory] fatal/error/warning/message ErrorNumber: Text
	TestOutput(
		R"(Path\To\File.c(100,200,300,400): warning C4267: Error description)",
		{
			"Path\\To\\File.c",
			"100",
			"200",
			"warning",
			"C4267",
			"Error description"
		}
	);
	TestOutput(
		R"(Path\To\File.c(100,200,300,400): command line warning C4267: Error description)",
		{
			"Path\\To\\File.c",
			"100",
			"200",
			"warning",
			"C4267",
			"Error description"
		}
	);

	// origin: fatal/error/warning/message ErrorNumber: Text
	TestOutput(
		R"(LINK : fatal error LNK1104: Error description)",
		{
			"LINK",
			"fatal error",
			"LNK1104",
			"Error description"
		}
	);
	TestOutput(
		R"(LINK: fatal error LNK1104: Error description)",
		{
			"LINK",
			"fatal error",
			"LNK1104",
			"Error description"
		}
	);
}

}; // namespace MicroBuild
