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
	ENUM_KEY(Windows_x86)
	ENUM_KEY(Windows_x64)
	ENUM_KEY(Windows_AnyCPU)

	ENUM_KEY(Linux_x86)
	ENUM_KEY(Linux_x64)

	ENUM_KEY(MacOS_x86)
	ENUM_KEY(MacOS_x64)
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

