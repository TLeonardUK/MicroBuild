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
#include "App/Ides/MSBuild/Versions/VisualStudio_2017.h"

namespace MicroBuild {

Ide_VisualStudio_2017::Ide_VisualStudio_2017()
{
	SetShortName("vs2017");
	SetHeaderShortName("Visual Studio 2017");
	SetHeaderVersion("12.0");
	SetDefaultToolset(EPlatformToolset::MSBuild_v141);
	SetDefaultToolsetString("15.0");
	SetMSBuildVersion(MSBuildVersion::Version12);
}

Platform::Path Ide_VisualStudio_2017::GetVS2017InstallPath()
{ 
	// There does not appear to be any easy way to do this, so we are going to be searching the default
	// installation paths. It might be an idea to try parsing the files in:
	//	%ProgramData%\Microsoft\VisualStudio\Packages\_Instance
	// and extracting the install paths, but that is unsupported.

	Platform::Path rootDir = "%ProgramFiles%/Microsoft Visual Studio/2017/";
	if (!rootDir.Exists())
	{
		rootDir = "%ProgramFiles(x86)%/Microsoft Visual Studio/2017/";
	}

	std::string folders[] = { "Enterprise", "Professional", "Community" };
	for (auto folder : folders)
	{
		Platform::Path installDir = rootDir.AppendFragment(folder, true);
		if (installDir.Exists())
		{
			return installDir;
		}
	}

	return rootDir;
}

Platform::Path Ide_VisualStudio_2017::GetMSBuildLocation()
{
	return GetVS2017InstallPath().AppendFragment("MSBuild/15.0/Bin/msbuild.exe", true);
}

}; // namespace MicroBuild