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
	
std::map<uint64_t, std::time_t> BuilderFileInfo::m_modifiedTimeCache;
std::map<uint64_t, bool> BuilderFileInfo::m_fileExistsCache;
std::mutex BuilderFileInfo::m_fileCacheLock;

BuilderFileInfo::BuilderFileInfo()
	: bOutOfDate(false)
	, Hash(0)
	, ErrorCount(0)
	, WarningCount(0)
	, InfoCount(0)
{
}

void BuilderFileInfo::AddMessage(const ToolchainOutputMessage& message)
{
	if (message.Type == EToolchainOutputMessageType::Error)
	{
		ErrorCount++;
	}
	else if (message.Type == EToolchainOutputMessageType::Warning)
	{
		WarningCount++;
	}
	else if (message.Type == EToolchainOutputMessageType::Info)
	{
		InfoCount++;
	}
	Messages.push_back(message);
}

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

std::time_t BuilderFileInfo::GetCachedModifiedTime(const Platform::Path& path)
{
	std::string extension = path.GetExtension();

	// We only permit caching of source-code file-types, as other file-types may be updated during the build.
	if (path.IsIncludeFile() || 
		path.IsSourceFile() ||
		path.IsResourceFile() ||
		path.IsXamlFile() ||
		path.IsImageFile() ||
		extension == "")
	{
		std::lock_guard<std::mutex> lock(m_fileCacheLock);

		uint64_t key = Strings::Hash64(path.ToString());

		auto iter = m_modifiedTimeCache.find(key);
		if (iter != m_modifiedTimeCache.end())
		{
			return iter->second;
		}

		std::time_t time = path.GetModifiedTime();
		
		m_modifiedTimeCache[key] = time;

		return time;
	}
	else
	{
		return path.GetModifiedTime();
	}
}

bool BuilderFileInfo::GetCachedPathExists(const Platform::Path& path)
{
	std::lock_guard<std::mutex> lock(m_fileCacheLock);

	uint64_t key = Strings::Hash64(path.ToString());
		
	auto iter = m_fileExistsCache.find(key);
	if (iter != m_fileExistsCache.end())
	{
		return iter->second;
	}

	bool bState = path.Exists();

	m_fileExistsCache[key] = bState;

	return bState;
}

uint64_t BuilderFileInfo::CalculateFileHash(const Platform::Path& path, uint64_t configurationHash)
{
	configurationHash = Strings::Hash64(Strings::Format("%llu", GetCachedModifiedTime(path)), configurationHash);
	configurationHash = Strings::Hash64(path.ToString(), configurationHash);	
	return configurationHash;
}

bool BuilderFileInfo::CheckOutOfDate(BuilderFileInfo& info, uint64_t configurationHash, bool bNoIntermediateFiles)
{
	if (!bNoIntermediateFiles && 
		(!GetCachedPathExists(info.ManifestPath) ||
		 !GetCachedPathExists(info.OutputPath)))
	{
		info.bOutOfDate = true;
		if (!GetCachedPathExists(info.ManifestPath))
		{
			Log(LogSeverity::Verbose, "[%s] Out of date because manifest path is non-existant.\n", info.SourcePath.GetFilename().c_str());
		}
		else if (!GetCachedPathExists(info.OutputPath))
		{
			Log(LogSeverity::Verbose, "[%s] Out of date because output path is non-existant.\n", info.SourcePath.GetFilename().c_str());
		}
	}
	else
	{
		uint64_t currentHash = info.Hash;

		if (!bNoIntermediateFiles && !info.LoadManifest())
		{
			info.bOutOfDate = true;
			Log(LogSeverity::Verbose, "[%s] Out of date because could not load manifest.\n", info.SourcePath.ToString().c_str());
		}
		else if (!info.SourcePath.IsEmpty() && info.Hash != currentHash)
		{
			Log(LogSeverity::Verbose, "[%s] Out of date because file hash was different.\n", info.SourcePath.ToString().c_str());
			info.bOutOfDate = true;
			info.Hash = currentHash;
		}
		else
		{
			for (const BuilderDependencyInfo& dependencyInfo : info.Dependencies)
			{
				uint64_t dependencyHash = CalculateFileHash(dependencyInfo.SourcePath, configurationHash);

				if (!GetCachedPathExists(dependencyInfo.SourcePath) ||
					 dependencyInfo.Hash != dependencyHash)
				{
					Log(LogSeverity::Verbose, "[%s] Out of date because dependency was out of date (hash=%i exists=%i): %s.\n",
						info.SourcePath.GetFilename().c_str(),
						dependencyInfo.Hash != dependencyHash,
						GetCachedPathExists(dependencyInfo.SourcePath),
						 dependencyInfo.SourcePath.ToString().c_str()
					);
				
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
	Platform::Path outputDirectory,
	uint64_t configurationHash,
	bool bNoIntermediateFiles
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
		info.Hash					= CalculateFileHash(info.SourcePath, configurationHash);

		Platform::Path baseDirectory = info.OutputPath.GetDirectory();
		//if (!baseDirectory.Exists())
		if (!GetCachedPathExists(baseDirectory))
		{
			baseDirectory.CreateAsDirectory();
		}

		info.bOutOfDate = CheckOutOfDate(info, configurationHash, bNoIntermediateFiles);

		result.push_back(info);
	}

	return result;
}

}; // namespace MicroBuild
