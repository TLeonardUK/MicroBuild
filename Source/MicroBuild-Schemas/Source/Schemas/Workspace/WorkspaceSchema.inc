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
OPTION_RULE_ABSOLUTE_PATH()
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

START_OPTION(
	bool,
	Workspace,
	UseInternalBuildTool,
	"If true all projects will invoke the internal build tool in microbuild "
	"rather than the built-in build tool the target IDE uses. This is mainly used "
	"by plugins that need extra control over how the project is built. "
)
OPTION_RULE_DEFAULT(false)
END_OPTION()

// ---------------------------------------------------------------------------

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
OPTION_RULE_EXPAND_PATH_WILDCARDS(true)
END_ARRAY_OPTION()

// ---------------------------------------------------------------------------
// WorkspaceFiles
// ---------------------------------------------------------------------------
START_ARRAY_OPTION(
	Platform::Path,
	WorkspaceFiles,
	File,
	"Location of files to include in workspace. Can contain wildcards (*) "
	"and recursive wildcards (**)."
)
OPTION_RULE_EXPAND_PATH_WILDCARDS(true)
END_ARRAY_OPTION()

// ---------------------------------------------------------------------------
// Virtual Paths
// ---------------------------------------------------------------------------
START_KEY_VALUE_ARRAY_OPTION(
	WorkspaceVirtualPaths,
	"Defines a key-value pair, with the value being a path pattern to match "
	"workspace files against, and the key being a virtual path that the file "
	"will be placed under in the output workspace file. In the virtual path * "
	"will be expanded to the matched workspace path, explicitly provided path "
	"segments in the path pattern will be exluded from the expansion."
)
END_KEY_VALUE_ARRAY_OPTION()