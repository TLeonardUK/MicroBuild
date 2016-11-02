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

#include "App/Builder/BuilderFileInfo.h"

#include "Core/Config/ConfigFile.h"

namespace MicroBuild {

bool BuilderFileInfo::LoadManifest()
{
	ConfigFile file;

	std::vector<Platform::Path> includePaths;
	includePaths.push_back(ManifestPath.GetDirectory());

	if (!file.Parse(ManifestPath, includePaths))
	{
		return false;
	}

	file.Resolve();

	Hash = file.GetCastedValue<uint64_t>("Manifest", "Hash", 0);

	Dependencies.clear();

	std::vector<std::pair<std::string, std::string>> pairs = file.GetPairs("Dependencies");
	for (auto& pair : pairs)
	{
		BuilderDependencyInfo dependency;
		dependency.SourcePath = pair.second;
		if (!StringCast<std::string, uint64_t>(pair.first, dependency.Hash))
		{
			continue;
		}
		Dependencies.push_back(dependency);
	}

	return true;
}

bool BuilderFileInfo::StoreManifest()
{
	ConfigFile file;
	file.SetOrAddValue("Manifest", "Hash", CastToString(Hash));

	for (BuilderDependencyInfo& dependency : Dependencies)
	{
		file.SetOrAddValue("Dependencies", CastToString(dependency.Hash), dependency.SourcePath.ToString());
	}

	return file.Serialize(ManifestPath);
}

uint64_t BuilderFileInfo::CalculateFileHash(const Platform::Path& path)
{
	uint64_t hash = 0;

	hash = path.GetModifiedTime();
	hash = Strings::Hash64(path.ToString(), hash);
	
	return hash;
}

bool BuilderFileInfo::CheckOutOfDate(BuilderFileInfo& info)
{
	if (!info.ManifestPath.Exists() ||
		!info.OutputPath.Exists())
	{
		info.bOutOfDate = true;
	}
	else
	{
		uint64_t currentHash = info.Hash;

		if (!info.LoadManifest())
		{
			info.bOutOfDate = true;
		}
		else if (!info.SourcePath.IsEmpty() && info.Hash != currentHash)
		{
			info.bOutOfDate = true;
			info.Hash = currentHash;
		}
		else
		{
			for (const BuilderDependencyInfo& dependencyInfo : info.Dependencies)
			{
				uint64_t dependencyHash = CalculateFileHash(dependencyInfo.SourcePath);

				if (!dependencyInfo.SourcePath.Exists() ||
					 dependencyInfo.Hash != dependencyHash)
				{
					info.bOutOfDate = true;
					break;
				}
			}
		}
	}

	return info.bOutOfDate;
}

std::vector<BuilderFileInfo> BuilderFileInfo::GetMultipleFileInfos(
	const std::vector<Platform::Path>& paths,
	Platform::Path rootDirectory,
	Platform::Path outputDirectory
)
{
	std::vector<BuilderFileInfo> result;

	for (const Platform::Path& path : paths)
	{
		BuilderFileInfo info;
		info.SourcePath = path;

		Platform::Path relativePath = rootDirectory.RelativeTo(path);
		assert(relativePath.IsRelative());

		info.OutputPath				= outputDirectory.AppendFragment(path.ChangeExtension("o").GetFilename(), true);
		info.ManifestPath			= info.OutputPath.ChangeExtension("build.manifest");
		info.bOutOfDate				= false;
		info.Hash					= CalculateFileHash(info.SourcePath);

		Platform::Path baseDirectory = info.OutputPath.GetDirectory();
		if (!baseDirectory.Exists())
		{
			baseDirectory.CreateAsDirectory();
		}

		info.bOutOfDate = CheckOutOfDate(info);

		result.push_back(info);
	}

	return result;
}

}; // namespace MicroBuild