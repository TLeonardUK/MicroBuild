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
#include "App/Packager/PackagerType.h"
#include "Core/Platform/Process.h"
#include "Core/Helpers/ZipFile.h"
#include "App/Ides/IdeHelper.h"

#include <utility>
#include <map>

namespace MicroBuild {

PackagerType::PackagerType()
	: m_shortName("")
{
}

PackagerType::~PackagerType()
{
}

std::string PackagerType::GetShortName()
{
	return m_shortName;
}

void PackagerType::SetShortName(const std::string& value)
{
	m_shortName = value;
}

bool PackagerType::CopyPackageFilesToFolder(
	ProjectFile& projectFile,
	const Platform::Path& contentDirectory
)
{
	MB_UNUSED_PARAMETER(contentDirectory);

	std::map<Platform::Path, Platform::Path> fileMap;

	std::vector<std::pair<std::string, std::string>> packageMap = projectFile.Get_PackageFiles();
	for (auto& pair : packageMap)
	{
		std::vector<Platform::Path> files = Platform::Path::MatchFilter(pair.first);
		
		size_t wildcardOffset = pair.first.find('*');
		Platform::Path common;
		if (wildcardOffset != std::string::npos)
		{
			common = pair.first.substr(0, wildcardOffset);			
		}
		else
		{
			common = Platform::Path(pair.first).GetDirectory();
		}

		Platform::Path baseOutputPath = pair.second;
		for (auto& file : files)
		{
			Platform::Path uncommonPath = file.GetUncommonPath(common);
			Platform::Path outputPath = baseOutputPath.AppendFragment(uncommonPath.ToString(), true);

			if (!file.Exists())
			{
				Log(LogSeverity::Warning, "Package files does not exist: %s\n", file.ToString().c_str());
				return false;
			}

			fileMap[file] = outputPath;
		}
	}
	
	for (auto& pair : fileMap)
	{
		if (!pair.first.Copy(pair.second))
		{
			return false;
		}
	}

	return true;
}

Platform::Path PackagerType::GetContentDirectory(
	const Platform::Path& packageDirectory
)
{
	return packageDirectory;
}

}; // namespace MicroBuild