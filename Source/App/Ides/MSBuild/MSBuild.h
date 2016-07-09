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

// Base class for all Microsoft Build types (visual studio).
class Ide_MSBuild 
	: public IdeType
{
public:

	// Used to define the platform/configuration matrix that defines what
	// projects we build in what configurations.
	struct MSBuildProjectPair
	{
		std::string config;
		EPlatform platform;
		bool shouldBuild;
	};
	typedef std::vector<MSBuildProjectPair> MSBuildProjectMatrix;
	typedef std::vector<MSBuildProjectMatrix> MSBuildWorkspaceMatrix;

	Ide_MSBuild();
	~Ide_MSBuild();

	// Sets the msbuild version to use, this determines what format to 
	// generate for solution and project files.
	void SetMSBuildVersion(MSBuildVersion version);

	// Generates a basic msbuild solution file that links to the given
	// project files.
	bool GenerateSolutionFile(
		WorkspaceFile& workspaceFile,
		std::vector<ProjectFile>& projectFiles,
		MSBuildWorkspaceMatrix& buildMatrix
	);

	// Generates a basic msbuild project file.
	bool GenerateProjectFile(
		WorkspaceFile& workspaceFile,
		ProjectFile& projectFiles,
		MSBuildProjectMatrix& buildMatrix
	);

	virtual bool Generate(
		WorkspaceFile& workspaceFile,
		std::vector<ProjectFile>& projectFiles) override;

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

	// Converts a project name into a GUID.
	std::string GetProjectGuid(
		const std::string& workspaceName, 
		const std::string& projectName);

	// Converts a filter name into a GUID.
	std::string GetFilterGuid(
		const std::string& workspaceName,
		const std::string& projectName,
		const std::string& filter);

	// Finds the correct project type guid based on language.
	std::string GetProjectTypeGuid(ELanguage language);

	// Finds a project in an array based on its name.
	ProjectFile* GetProjectByName(
		std::vector<ProjectFile>& projectFiles,
		const std::string& name);

private:
	MSBuildVersion m_msBuildVersion;

};

}; // namespace MicroBuild