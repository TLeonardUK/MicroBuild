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
#include "Schemas/Database/DatabaseFile.h"
#include "Core/Helpers/TextStream.h"

namespace MicroBuild {

DatabaseFile::DatabaseFile(
	const Platform::Path& outputPath, 
	const std::string& targetIde
)
	: m_outputPath(outputPath)
{
	Set_Target_IDE(targetIde);
	Set_Database_Directory(m_outputPath.GetDirectory());
	Set_Database_File(m_outputPath);
}

DatabaseFile::~DatabaseFile()
{
}

void DatabaseFile::Resolve()
{
	BaseConfigFile::Resolve();
}

bool DatabaseFile::Write()
{
	return Serialize(m_outputPath);
}

bool DatabaseFile::Read()
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

bool DatabaseFile::Clean(
	WorkspaceFile& workspaceFile,
	bool bDeleteProjectFiles
)
{
	//std::vector<Platform::Path> files = Get_Clean_File();
	//std::vector<Platform::Path> dirs = Get_Clean_Directory();

	MB_UNUSED_PARAMETER(workspaceFile);
	MB_UNUSED_PARAMETER(bDeleteProjectFiles);

	// todo: fix
	return true;

	/*
	// Ask IDE we originally tided up to clean up any build artifacts.
	IdeType* type = app->GetIdeByShortName(Get_Target_IDE());
	if (type == nullptr)
	{
		Log(LogSeverity::Fatal,
			"Could not find original target ide for workspace '%s'.\n",
			Get_Target_IDE().c_str());

		return false;
	}
	else
	{
		if (!type->Clean(workspaceFile, *this))
		{
			// This seems like it shouldn't be an error incase a user takes
			// an msbuild project file and tries to clean it on mac or something
			// similar?
			Log(LogSeverity::Warning,
				"Original target ide failed to clean files, ignoring.\n",
				Get_Target_IDE().c_str());

			return false;
		}
	}

	if (bDeleteProjectFiles)
	{
		// Remove files we created when we generated workspace.
		for (auto path : files)
		{
			if (path.Exists())
			{
				if (!path.Delete())
				{
					Log(LogSeverity::Fatal,
						"Failed to delete file '%s'.\n",
						path.ToString().c_str());

					return false;
				}
			}
		}

		// Remove this database file.
		if (m_outputPath.Exists())
		{
			if (!m_outputPath.Delete())
			{
				Log(LogSeverity::Fatal,
					"Failed to delete file '%s'.\n",
					m_outputPath.ToString().c_str());

				return false;
			}
		}

		// Remove directory structures we created when generating workspace.
		for (auto path : dirs)
		{
			if (path.Exists())
			{
				if (!path.Delete())
				{
					Log(LogSeverity::Fatal,
						"Failed to delete directory '%s'.\n",
						path.ToString().c_str());

					return false;
				}
			}
		}
	}
	
	return true;
	*/
}

bool DatabaseFile::StoreFile(
	WorkspaceFile& workspaceFile,
	Platform::Path& location,
	const char* data
)
{
	// Dump evaluation result out to file.
	TextStream stream;
	stream.Write("%s", data);
    
    Platform::Path directory = location.GetDirectory();
    
	// Ensure location directory is created.
	if (!directory.Exists())
	{
		// Create each directory incrementally.
		std::vector<Platform::Path> toCreate;
        
        Log(LogSeverity::Info, "Directory doesn't exist: %s\n", directory.ToString().c_str());

		Platform::Path base = directory;
		while (!base.Exists())
		{
			toCreate.push_back(base);
			base = base.GetDirectory();
		}

		for (auto iter = toCreate.rbegin(); iter != toCreate.rend(); iter++)
		{
            auto directoryPath = *iter;
            
			if (!directoryPath.CreateAsDirectory())
			{
				workspaceFile.ValidateError(
					"Failed to create directory '%s'.\n",
					directoryPath.ToString().c_str());

				return false;
			}
			else
			{
				Platform::Path relativePath =
					Get_Database_Directory().RelativeTo(directoryPath);

				SetOrAddValue(
					"Clean",
					"Directory",
					Strings::Format("$(Database.Directory)/%s", relativePath.ToString().c_str()),
					false);
			}
		}
	}

	// Write the solution file to the location.
	if (!stream.WriteToFile(location))
	{
		workspaceFile.ValidateError(
			"Failed to write to file '%s'.\n",
			location.ToString().c_str());

		return false;
	}
	else
	{
		Platform::Path relativePath = 
			Get_Database_Directory().RelativeTo(location);
		 
		SetOrAddValue(
			"Clean", 
			"File", 
			Strings::Format("$(Database.Directory)/%s", relativePath.ToString().c_str()), 
			false);
	}

	return true;
}

#define SCHEMA_FILE "Schemas/Database/DatabaseSchema.inc"
#define SCHEMA_CLASS DatabaseFile
#include "Schemas/Config/SchemaImpl.h"
#undef SCHEMA_FILE
#undef SCHEMA_CLASS

}; // namespace MicroBuild