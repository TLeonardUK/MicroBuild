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
// Workspace
// ---------------------------------------------------------------------------
START_OPTION(
	std::string,
	Workspace,
	Name,
	"Name of workspace, used for output file and workspace descriptions."
)
OPTION_RULE_REQUIRED()
END_OPTION()

// ---------------------------------------------------------------------------

START_OPTION(
	Platform::Path,
	Workspace,
	Location,
	"Output folder that workspace files are generated in."
)
OPTION_RULE_REQUIRED()
END_OPTION()

// ---------------------------------------------------------------------------

START_OPTION(
	std::string,
	Workspace,
	StartProject,
	"The name of the project that should be built on startup."
)
END_OPTION()

// ---------------------------------------------------------------------------

START_OPTION(
	Platform::Path,
	Workspace,
	Directory,
	"Automatically defined to the directory the workspace file is contained in."
)
END_OPTION()

// ---------------------------------------------------------------------------

START_OPTION(
	Platform::Path,
	Workspace,
	File,
	"Automatically defined to the path to the workspace file."
)
END_OPTION()

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------
START_ARRAY_OPTION(
	std::string,
	Configurations,
	Configuration,
	"Configurations that should be generated for this workspace."
)
OPTION_RULE_REQUIRED()
END_ARRAY_OPTION()

// ---------------------------------------------------------------------------
// Platforms
// ---------------------------------------------------------------------------
START_ARRAY_OPTION(
	EPlatform,
	Platforms,
	Platform,
	"Platforms that should be generated for this workspace."
)
OPTION_RULE_REQUIRED()
END_ARRAY_OPTION()

// ---------------------------------------------------------------------------
// Projects
// ---------------------------------------------------------------------------
START_ARRAY_OPTION(
	Platform::Path,
	Projects,
	Project,
	"Location of projects to include in workspace. Can contain wildcards (*) and recursive wildcards (**)."
)
OPTION_RULE_REQUIRED()
OPTION_RULE_EXPAND_PATH_WILDCARDS()
END_ARRAY_OPTION()
