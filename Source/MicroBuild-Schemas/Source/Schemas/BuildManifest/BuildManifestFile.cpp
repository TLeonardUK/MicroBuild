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
#include "Schemas/BuildManifest/BuildManifestFile.h"
#include "Core/Helpers/TextStream.h"

namespace MicroBuild {

BuildManifestFile::BuildManifestFile(
	const Platform::Path& outputPath
)
	: m_outputPath(outputPath)
{
	Set_Database_Directory(m_outputPath.GetDirectory());
	Set_Database_File(m_outputPath);
}

BuildManifestFile::~BuildManifestFile()
{
}

void BuildManifestFile::Resolve()
{
	BaseConfigFile::Resolve();
}

bool BuildManifestFile::Write()
{
	return Serialize(m_outputPath);
}

bool BuildManifestFile::Read()
{
	std::vector<Platform::Path> includePaths;
	includePaths.push_back(m_outputPath.GetDirectory());

	if (Parse(m_outputPath, includePaths))
	{
		Resolve();
		
		if (Validate())
		{
			return true;
		}
	}

	return false;
}

#define SCHEMA_FILE "Schemas/BuildManifest/BuildManifestSchema.inc"
#define SCHEMA_CLASS BuildManifestFile
#include "Schemas/Config/SchemaImpl.h"
#undef SCHEMA_FILE
#undef SCHEMA_CLASS

}; // namespace MicroBuild