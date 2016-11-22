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
#include "Schemas/Config/BaseConfigFile.h"
#include "Core/Helpers/Strings.h"

namespace MicroBuild {

BaseConfigFile::BaseConfigFile()
{
}

BaseConfigFile::~BaseConfigFile()
{
}

void BaseConfigFile::Resolve()
{
	// Host settings.
#if defined(MB_PLATFORM_WINDOWS)
	Set_Host_Platform(EHostPlatform::Windows);
	Set_Host_ExeExtension(".exe");
	Set_Host_StaticLibExtension(".lib");
	Set_Host_DynamicLibExtension(".dll");

#elif defined(MB_PLATFORM_LINUX)
	Set_Host_Platform(EHostPlatform::Linux);
	Set_Host_ExeExtension("");
	Set_Host_StaticLibExtension(".a");
	Set_Host_DynamicLibExtension(".so");

#elif defined(MB_PLATFORM_MACOS)
	Set_Host_Platform(EHostPlatform::MacOS);
	Set_Host_ExeExtension("");
	Set_Host_StaticLibExtension(".a");
	Set_Host_DynamicLibExtension(".dylib");

#else
	#error Unimplemented platform.

#endif

	// Target settings.
	switch (Get_Target_Platform())
	{
	// Web platforms
	case EPlatform::HTML5:
	{
		Set_Target_ExeExtension(".js");
		Set_Target_StaticLibExtension(".bc");
		Set_Target_DynamicLibExtension(".bc");
		break;
	}

	// Console platforms		
	case EPlatform::Nintendo3DS:
	{
		Set_Target_ExeExtension(".axf");
		Set_Target_StaticLibExtension(".a");
		Set_Target_DynamicLibExtension(".a");
		break;
	}
	
	case EPlatform::NintendoWiiU:
	{
		Set_Target_ExeExtension(".rpx");
		Set_Target_StaticLibExtension(".a");
		Set_Target_DynamicLibExtension(".rpl");
		break;
	}
	
	// Mobile platforms
	case EPlatform::Android_ARM:
	case EPlatform::Android_ARM64:
	case EPlatform::Android_x86:
	case EPlatform::Android_x64:
	case EPlatform::Android_MIPS:
	case EPlatform::Android_MIPS64:
	{
		Set_Target_ExeExtension("");
		Set_Target_StaticLibExtension(".a");
		Set_Target_DynamicLibExtension(".so");
		break;
	}

    // Desktop Platforms
	case EPlatform::x86:
	case EPlatform::x64:
	case EPlatform::ARM:
	case EPlatform::ARM64:
	case EPlatform::WinRT_x86:
	case EPlatform::WinRT_x64:
	case EPlatform::WinRT_ARM:
	case EPlatform::WinRT_ARM64:
		// Fallthrough

	// Mobile platforms
	case EPlatform::iOS:
		// Fallthrough

	case EPlatform::PS3:
	case EPlatform::PS4:
	case EPlatform::PSVita:
	case EPlatform::Xbox360:	
	case EPlatform::XboxOne:
		// Fallthrough

    // DotNet Specific
	case EPlatform::AnyCPU:
		// Fallthrough

    // MacOS Bundles
    case EPlatform::Native:
    case EPlatform::Universal86:
    case EPlatform::Universal64:
	case EPlatform::Universal:
	default:		
		{
			Set_Target_ExeExtension(Get_Host_ExeExtension());
			Set_Target_StaticLibExtension(Get_Host_StaticLibExtension());
			Set_Target_DynamicLibExtension(Get_Host_DynamicLibExtension());
			break;
		}
	}

	// Store and use a base timestamp.
	static time_t s_baseTime;
	static bool s_timeSet = false;
	if (!s_timeSet)
	{
		s_timeSet = true;
		time(&s_baseTime);
	}

	struct tm* now = localtime(&s_baseTime);
	char buffer[64];
	strftime(buffer, sizeof(buffer), "%d%m%Y%H%M", now);
	std::string bufferVal = buffer;

	Set_Target_Timestamp(bufferVal);

	ConfigFile::Resolve();
}

Platform::Path BaseConfigFile::ResolvePath(Platform::Path& value) const
{
	if (value.IsAbsolute())
	{
		return value;
	}
	else
	{
		return GetPath() + value;
	}
}

void BaseConfigFile::ValidateError(const char* format, ...) const
{
	va_list list;
	va_start(list, format);

	std::string message = Strings::FormatVa(format, list);

	va_end(list);

	std::string value = Strings::Format("%s: %s\n",
		GetPath().ToString().c_str(),
		message.c_str());

	Log(LogSeverity::Fatal, "%s", value.c_str());
}

bool BaseConfigFile::ValidateVersion(std::vector<std::string>& values) const
{
	if (values.size() >= 1)
	{
		float version = (float)atof(values[0].c_str());
		if ((version - MB_VERSION) > 0.001f)
		{
			ValidateError(
				"Requires version %.2f of MicroBuild to use (current version is %.2f).",
				version,
				MB_VERSION
			);

			return false;
		}
		else
		{
			return true;
		}
	}

	return true;
}

bool BaseConfigFile::ExpandPaths(
	std::vector<std::string>& values, bool bCanCache) 
{
	std::vector<std::string> original = values;
	values.clear();

	for (std::string& val : original)
	{
		if (!ExpandPath(val, values, bCanCache))
		{
			return false;
		}
	}

	return true;
}

bool BaseConfigFile::ExpandPath(Platform::Path path, 
	std::vector<std::string>& results, 
	bool bCanCache) 
{
	if (bCanCache)
	{
		for (CachedExpandedPaths& cache : m_cachedExpandedPaths)
		{
			if (cache.path == path)
			{
				results.insert(
					results.end(), 
					cache.expanded.begin(), 
					cache.expanded.end()
				);
				return true;
			}
		}
	}

	Platform::Path resolved = ResolvePath(path);
	if (resolved.IsRelative())
	{
		ValidateError(
			"Path '%s' does not resolve to an absolute path. All paths must "
			"be absolute. Use tokens to expand relative paths to absolute.",
			path.ToString().c_str());

		return false;
	}

	std::vector<Platform::Path> matches =
		Platform::Path::MatchFilter(resolved);

	for (Platform::Path& match : matches)
	{
		results.push_back(match.ToString());
	}

	if (bCanCache)
	{
		m_cachedExpandedPaths.resize(m_cachedExpandedPaths.size() + 1);
		
		CachedExpandedPaths& cache = *m_cachedExpandedPaths.rbegin();
		cache.expanded.insert(
			cache.expanded.begin(),
			results.end() - matches.size(),
			results.end()
		);
		cache.path = path;
	}

	return true;
}

bool BaseConfigFile::ValidateOptions(
	std::vector<std::string>& values,
	std::vector<std::string>& options,
	std::string& group,
	std::string& key) const
{
	for (std::string& value : values)
	{
		bool bExists = false;;

		for (std::string& opt : options)
		{
			if (Strings::CaseInsensitiveEquals(value, opt))
			{
				bExists = true;
			}
		}

		if (!bExists)
		{
			ValidateError(
				"Value '%s' is not a value option for %s.%s.",
				value.c_str(),
				group.c_str(),
				key.c_str()
			);
			return false;
		}
	}

	return true;
}

#define SCHEMA_FILE "Schemas/Config/BaseSchema.inc"
#define SCHEMA_CLASS BaseConfigFile
#define SCHEMA_IS_BASE 1
#include "Schemas/Config/SchemaImpl.h"
#undef SCHEMA_FILE
#undef SCHEMA_CLASS
#undef SCHEMA_IS_BASE

}; // namespace MicroBuild