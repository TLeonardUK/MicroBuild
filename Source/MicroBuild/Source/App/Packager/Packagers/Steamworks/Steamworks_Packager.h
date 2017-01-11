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

#pragma once

#include "App/Packager/PackagerType.h"

namespace MicroBuild {

// Takes a workspace and packages it in a format suitable for steam, and uploads
// it to the given depots.
class Steamworks_Packager
	: public PackagerType
{
public:

	Steamworks_Packager();

	virtual bool Package(
		ProjectFile& projectFile,
		const Platform::Path& packageDirectory) override;

	virtual Platform::Path GetContentDirectory(
		const Platform::Path& packageDirectory
	) override;

private:

	bool GenerateDepotVdf(
		ProjectFile& projectFile, 
		Platform::Path contentPath, 
		Platform::Path scriptsPath, 
		Platform::Path depotVdfPath);

	bool GenerateAppVdf(
		ProjectFile& projectFile, 
		Platform::Path outputPath, 
		Platform::Path contentPath, 
		Platform::Path scriptsPath, 
		Platform::Path depotVdfPath, 
		Platform::Path appVdfPath);

};

}; // namespace MicroBuild