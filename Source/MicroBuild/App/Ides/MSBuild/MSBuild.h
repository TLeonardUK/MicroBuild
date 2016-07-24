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

#pragma once

#include "App/Ides/IdeType.h"
#include "Core/Templates/TemplateEvaluator.h"

namespace MicroBuild {

// MSBuild file format version to use.
enum class MSBuildVersion
{
	Version12, // Visual Studio 2015

	COUNT
};

// Simple class that stores which msbuild groups each file goes in.
struct MSBuildFile
{
	std::string Path;
	std::string TypeId;
};
struct MSBuildFileGroup
{
	std::vector<MSBuildFile> Files;
};


// Base class for all Microsoft Build types (visual studio).
class Ide_MSBuild 
	: public IdeType
{
public:

	Ide_MSBuild();
	~Ide_MSBuild();

	// Sets the msbuild version to use, this determines what format to 
	// generate for solution and project files.
	void SetMSBuildVersion(MSBuildVersion version);

	// Various default values derived classes should set.
	void SetHeaderShortName(const std::string& value);
	void SetHeaderVersion(const std::string& value);
	void SetDefaultToolset(EPlatformToolset toolset);
	void SetDefaultToolsetString(const std::string& value);

	// Generates a basic msbuild solution file that links to the given
	// project files.
	bool GenerateSolutionFile(
		DatabaseFile& databaseFile,
		WorkspaceFile& workspaceFile,
		std::vector<ProjectFile>& projectFiles,
		BuildWorkspaceMatrix& buildMatrix
	);

	// Generates a basic csharp project file.
	bool Generate_Csproj(
		DatabaseFile& databaseFile,
		WorkspaceFile& workspaceFile,
		ProjectFile& projectFiles,
		BuildProjectMatrix& buildMatrix
		);

	// Generates a basic msbuild project file.
	bool Generate_Vcxproj(
		DatabaseFile& databaseFile,
		WorkspaceFile& workspaceFile,
		ProjectFile& projectFiles,
		BuildProjectMatrix& buildMatrix
	);

	// Generates a basic msbuild project filters file.
	bool Generate_Vcxproj_Filters(
		DatabaseFile& databaseFile,
		WorkspaceFile& workspaceFile,
		ProjectFile& projectFiles,
		BuildProjectMatrix& buildMatrix,
		std::vector<MSBuildFileGroup>& groups
	);

	virtual bool Generate(
		DatabaseFile& databaseFile,
		WorkspaceFile& workspaceFile,
		std::vector<ProjectFile>& projectFiles) override;

	virtual bool Clean(
		WorkspaceFile& workspaceFile) override;

	virtual bool Build(
		WorkspaceFile& workspaceFile,
		bool bRebuild,
		const std::string& configuration,
		const std::string& platform) override;

protected:

	// Generates an output file using the given template and evaluator.
	bool GenerateTemplateFile(
		WorkspaceFile& workspaceFile,
		Platform::Path& directory,
		Platform::Path& location,
		const char* templateData,
		TemplateEvaluator& evaluator);

	// Converts a platform enum into an msbuild platform id.
	std::string GetPlatformID(EPlatform platform);

	// Converts a platform enum into a dotnet target id.
	std::string GetPlatformDotNetTarget(EPlatform platform);

	// Finds the correct project type guid based on language.
	std::string GetProjectTypeGuid(ELanguage language);

	// Finds a project in an array based on its name.
	ProjectFile* GetProjectByName(
		std::vector<ProjectFile>& projectFiles,
		const std::string& name);

	// Gets the location of the msbuild exe.
	virtual Platform::Path GetMSBuildLocation() = 0;

private:
	MSBuildVersion m_msBuildVersion;
	std::string m_headerShortName;
	std::string m_headerVersion;
	EPlatformToolset m_defaultToolset;
	std::string m_defaultToolsetString;

};

}; // namespace MicroBuild