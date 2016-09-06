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
// Workspace
// ---------------------------------------------------------------------------
START_ARRAY_OPTION(
	std::string,
	Workspace,
	Project,
	"List of all projects contained in generated workspace."
)
END_ARRAY_OPTION()

START_ARRAY_OPTION(
	std::string,
	Workspace,
	Configuration,
	"List of all build configurations contained in generated workspace."
)
END_ARRAY_OPTION()

START_ARRAY_OPTION(
	EPlatform,
	Workspace,
	Platform,
	"List of all build platforms contained in generated workspace."
)
END_ARRAY_OPTION()

// ---------------------------------------------------------------------------
// Clean
// ---------------------------------------------------------------------------
START_ARRAY_OPTION(
	Platform::Path,
	Clean,
	File,
	"Files that should be cleaned when rebuild or clean commands are executed. "
	"Can contain wildcards (*) and recursive wildcards (**)."
)
OPTION_RULE_EXPAND_PATH_WILDCARDS(true)
END_ARRAY_OPTION()

// ---------------------------------------------------------------------------

START_ARRAY_OPTION(
	Platform::Path,
	Clean,
	Directory,
	"Directory that should be cleaned when rebuild or clean commands are executed. "
)
END_ARRAY_OPTION()

// ---------------------------------------------------------------------------
// Database
// ---------------------------------------------------------------------------

START_OPTION(
	Platform::Path,
	Database,
	Directory,
	"Automatically defined to the directory the database file is contained in."
)
END_OPTION()

// ---------------------------------------------------------------------------

START_OPTION(
	Platform::Path,
	Database,
	File,
	"Automatically defined to the path to the database file."
)
END_OPTION()
