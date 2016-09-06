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
#include "App/Ides/MSBuild/Versions/VisualStudio_2015.h"

namespace MicroBuild {

Ide_VisualStudio_2015::Ide_VisualStudio_2015()
{
	SetShortName("vs2015");
	SetHeaderShortName("Visual Studio 2015");
	SetHeaderVersion("12.0");
	SetDefaultToolset(EPlatformToolset::v140);
	SetDefaultToolsetString("14.0");
	SetMSBuildVersion(MSBuildVersion::Version12);
}

Platform::Path Ide_VisualStudio_2015::GetMSBuildLocation()
{
	Platform::Path path = "%ProgramFiles%/MSBuild/14.0/bin/msbuild.exe";
	if (!path.Exists())
	{
		path = "%ProgramFiles(x86)%/MSBuild/14.0/bin/msbuild.exe";
		if (!path.Exists())
		{
			Log(LogSeverity::Warning, 
				"Cannot find explicit msbuild executable.");
			path = "msbuild";
		}
	}
	return path;
}

}; // namespace MicroBuild