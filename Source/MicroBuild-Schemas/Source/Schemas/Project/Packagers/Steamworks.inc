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
// Steamworks
// ---------------------------------------------------------------------------
START_OPTION(
	int,
	Steamworks,
	AppId,
	"Application ID of project on steam."
)
END_OPTION()

START_OPTION(
	int,
	Steamworks,
	DepotId,
	"Depot ID to generated packaged project for."
)
END_OPTION()

START_OPTION(
	std::string,
	Steamworks,
	Branch,
	"Steam branch to upload build to."
)
END_OPTION()

START_OPTION(
	bool,
	Steamworks,
	SetLive,
	"If true this build will be uploaded and made live on the previously specified branch."
)
END_OPTION()

START_OPTION(
	std::string,
	Steamworks,
	BuildDescription,
	"Description of build uploaded to steam."
)
OPTION_RULE_DEFAULT(std::string("Application Update"))
END_OPTION()

START_OPTION(
	std::string,
	Steamworks,
	Username,
	"Username for account to upload package builds with."
)
END_OPTION()

START_OPTION(
	std::string,
	Steamworks,
	Password,
	"Password for account to upload package builds with."
)
END_OPTION()

START_OPTION(
	std::string,
	Steamworks,
	GuardCode,
	"Code for account if protected by Steam Guard."
)
END_OPTION()
