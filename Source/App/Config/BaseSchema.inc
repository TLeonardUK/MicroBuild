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

// ---------------------------------------------------------------------------
// General use enums.
// ---------------------------------------------------------------------------
START_ENUM(EHostArchitecture)
	ENUM_KEY(x64)
	ENUM_KEY(x86)
END_ENUM()

START_ENUM(EHostPlatform)
	ENUM_KEY(Windows)
	ENUM_KEY(Linux)
	ENUM_KEY(MacOS)
END_ENUM()

START_ENUM(EPlatform)
	ENUM_KEY(x86)
	ENUM_KEY(x64)
	ENUM_KEY(ARM)
	ENUM_KEY(ARM64)
	ENUM_KEY(AnyCPU)

	ENUM_KEY(iOS)
	ENUM_KEY(Android)
	ENUM_KEY(Emscripten)
	
	ENUM_KEY(PS4)
	ENUM_KEY(XboxOne)
	
	ENUM_KEY(WiiU)
	ENUM_KEY(Nintendo3DS)
END_ENUM()

START_ENUM(ELanguage)
	ENUM_KEY(Cpp)
	ENUM_KEY(CSharp)
END_ENUM()

START_ENUM(ELanguageVersion)
	ENUM_KEY(Default)

	// C++
	ENUM_KEY(Cpp_14)
	ENUM_KEY(Cpp_11)
	ENUM_KEY(Cpp_98)

	// C#
	ENUM_KEY(CSharp_1_0)
	ENUM_KEY(CSharp_2_0)
	ENUM_KEY(CSharp_3_0)
	ENUM_KEY(CSharp_4_0)
	ENUM_KEY(CSharp_5_0)
	ENUM_KEY(CSharp_6_0)
END_ENUM()

// ---------------------------------------------------------------------------
// MicroBuild
// ---------------------------------------------------------------------------
START_OPTION(
	float,
	MicroBuild,
	RequiredVersion,
	"Required version of MicroBuild to parse this file."
)
OPTION_RULE_DEFAULT(MB_VERSION)
OPTION_RULE_VALIDATOR(ValidateVersion)
END_OPTION()

// ---------------------------------------------------------------------------
// Host
// ---------------------------------------------------------------------------
START_OPTION(
	EHostPlatform,
	Host,
	Platform,
	"Automatically defined to the name of the platform of the host."
)
END_OPTION()

// ---------------------------------------------------------------------------

START_OPTION(
	EHostArchitecture,
	Host,
	Architecture,
	"Automatically defined to the architecture of the host."
)
END_OPTION()

// ---------------------------------------------------------------------------
// Target
// ---------------------------------------------------------------------------
START_OPTION(
	std::string,
	Target,
	Configuration,
	"Automatically defined to the configuration of project file currently being generated."
)
END_OPTION()

// ---------------------------------------------------------------------------

START_OPTION(
	EPlatform,
	Target,
	Platform,
	"Automatically defined to the platform of project file currently being generated."
)
END_OPTION()

// ---------------------------------------------------------------------------

START_OPTION(
	std::string,
	Target,
	IDE,
	"Automatically defined to the ide the project file are currently being generated for."
)
END_OPTION()

// ---------------------------------------------------------------------------

START_OPTION(
	Platform::Path,
	Target,
	PackageDirectory,
	"Automatically defined to the directory the package is being built in."
)
END_OPTION()

// ---------------------------------------------------------------------------
// Package
// ---------------------------------------------------------------------------
START_ARRAY_OPTION(
	std::string,
	Package,
	Command,
	"Defines an individual command to run while packaging. When packaging all "
	"project commands are ran first, then the workspace commands ran after."
)
OPTION_RULE_ORDER_IMPORTANT()
END_ARRAY_OPTION()