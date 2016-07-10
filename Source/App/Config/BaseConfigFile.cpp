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
#include "App/Config/BaseConfigFile.h"
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
#elif defined(MB_PLATFORM_LINUX)
	Set_Host_Platform(EHostPlatform::Linux);
#elif defined(MB_PLATFORM_MACOS)
	Set_Host_Platform(EHostPlatform::MacOS);
#else
	#error Unimplemented platform.
#endif

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
		if (version > MB_VERSION)
		{
			ValidateError(
				"Requires version %.2f of MicroBuild to use.",
				version);

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

#define SCHEMA_FILE "App/Config/BaseSchema.inc"
#define SCHEMA_CLASS BaseConfigFile
#define SCHEMA_IS_BASE 1
#include "App/Config/SchemaImpl.h"
#undef SCHEMA_FILE
#undef SCHEMA_CLASS
#undef SCHEMA_IS_BASE

}; // namespace MicroBuild