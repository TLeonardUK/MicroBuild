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
#include "App/Ides/MSBuild/MSBuild_Ids.h"

namespace MicroBuild {
namespace MSBuild {

const char* k_GuiUapProject			= "{A5A43C5B-DE2A-4C0C-9213-0A381AF9435A}";
const char* k_GuidCSharpProject		= "{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}";
const char* k_GuidCppProject		= "{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}";
const char* k_GuidSolutionFolder	= "{2150E333-8FDC-42A3-9474-1A3956D46DE8}";

std::string GetPlatformID(EPlatform platform)
{
	switch (platform)
	{
		case EPlatform::x86:
		{
			return "Win32";
		}
		case EPlatform::x64:
		{
			return "x64";
		}
		case EPlatform::AnyCPU:
		{
			return "Any CPU";
		}
		case EPlatform::ARM:
		{
			return "ARM";
		}
		case EPlatform::ARM64:
		{
			return "ARM64";
		}
		/*
		case EPlatform::PS4:
		{
			return "PS4";
		}
		case EPlatform::XboxOne:
		{
			return "Durango";
		}
		case EPlatform::WiiU:
		{
			return "CAFE2";
		}
		case EPlatform::Nintendo3DS:
		{
			return "CTR";
		}
		*/
	}
	return "";
}

std::string GetPlatformDotNetTarget(EPlatform platform)
{
	switch (platform)
	{
		case EPlatform::AnyCPU:
		{
			return "AnyCPU";
		}
		default:
		{
			return GetPlatformDotNetTarget(platform);
		}
	}
}

std::string GetProjectTypeGuid(ELanguage language)
{
	switch (language)
	{
	case ELanguage::CSharp:
		return k_GuidCSharpProject;
	case ELanguage::Cpp:
		return k_GuidCppProject;
	default:
		return "";
	}
}

}; // namespace MSBuild
}; // namespace MicroBuild