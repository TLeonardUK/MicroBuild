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
// Packager
// ---------------------------------------------------------------------------
START_OPTION(
	bool,
	Packager,
	RequiresBuild,
	"If set to true making this packager requires building the project."
)
OPTION_RULE_DEFAULT(true)
END_OPTION()

START_OPTION(
	bool,
	Packager,
	BuildChangelog,
	"If set to true, when the project is packaged a changelog is created from "
	"source control history and saved in the file path referenced by ChangelogFile. "
	"The changelog is made by tallying all Text after the [ChangeLog] tag in SCM commit "
	"messages. Once the changelog is built, and differs from the last changelog, "
	"it is checked into SCM. "
)
OPTION_RULE_DEFAULT(false)
END_OPTION()

START_OPTION(
	Platform::Path,
	Packager,
	ChangelogFile,
	"File path to save changelog to if BuildChangelog is set to true."
)
END_OPTION()

START_ARRAY_OPTION(
	std::string,
	Packager,
	BuildProject,
	"Defines a project to build when this project is packaged. If none are defined "
	"the the current project is built on its own."
)
END_ARRAY_OPTION()

// ---------------------------------------------------------------------------
// PrePackageCommands
// ---------------------------------------------------------------------------
START_ARRAY_OPTION(
	std::string,
	PrePackageCommands,
	Command,
	"Commands to run before packaging."
)
END_ARRAY_OPTION()

START_ARRAY_OPTION(
	std::string,
	PostPackageCommands,
	Command,
	"Commands to run after packaging."
)
END_ARRAY_OPTION()

// ---------------------------------------------------------------------------
// Packager Files
// ---------------------------------------------------------------------------
START_KEY_VALUE_ARRAY_OPTION(
	PackageFiles,
	"Defines a key-value pair, with the value being a path pattern to match "
	"project files against, and the key being a target path in the packager "
	"folder they will bbe placed under in the output project file. In the virtual path * "
	"will be expanded to the matched project path, explicitly provided path "
	"segments in the path pattern will be exluded from the expansion."
)
END_KEY_VALUE_ARRAY_OPTION()
