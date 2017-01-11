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

#include "Schemas/Workspace/WorkspaceFile.h"
#include "Schemas/Project/ProjectFile.h"

#include "Core/Platform/Path.h"
#include "Core/Helpers/Strings.h"
#include "Schemas/Config/BaseConfigFile.h"

#include <functional>

namespace MicroBuild {

// Base class for all packager types.
class PackagerType
{
public:

	PackagerType();
	virtual ~PackagerType();

	// Gets the short-named use on the command line and in config files
	// to refer to this IDE.
	std::string GetShortName();

	// Takes a fully resolved workspace file and builds and packages it
	// into the given directory.
	virtual bool Package(
		ProjectFile& projectFile,
		const Platform::Path& packageDirectory) = 0;

	// Gets the directory that package content is built in. Its what 
	// Target.PackageDirectory is set to.
	virtual Platform::Path GetContentDirectory(
		const Platform::Path& packageDirectory
	);

protected:

	void SetShortName(const std::string& value);

	bool CopyPackageFilesToFolder(
		ProjectFile& projectFile,
		const Platform::Path& contentDirectory
	);

private:

	std::string m_shortName;
	
};

}; // namespace MicroBuild