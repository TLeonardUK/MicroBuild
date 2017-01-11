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
#include "App/Packager/Packagers/Steamworks/Steamworks_Packager.h"
#include "Core/Platform/Process.h"
#include "Core/Helpers/ZipFile.h"
#include "Core/Helpers/VdfNode.h"
#include "Core/Helpers/TextStream.h"

#include "Core/Platform/Platform.h"

namespace MicroBuild {

Steamworks_Packager::Steamworks_Packager()
{
	SetShortName("Steamworks");
}

bool Steamworks_Packager::Package(
	ProjectFile& projectFile,
	const Platform::Path& packageDirectory)
{
	MB_UNUSED_PARAMETER(projectFile);
	MB_UNUSED_PARAMETER(packageDirectory);

	// Try and find the steamworks installation directory.
	Platform::Path steamworksRoot = Platform::GetEnvironmentVariable("STEAMWORKS_ROOT");
	if (!steamworksRoot.Exists())
	{
		Log(LogSeverity::Warning, "Failed to package for steam. Unable to find steamworks installation folder.\n");
		Log(LogSeverity::Warning, "Make sure you set the environment variable STEAMWORKS_ROOT to the root folder of your steamworks install.\n");
		return false;
	}

	// Find the content-builder tool.
#if defined(MB_PLATFORM_WINDOWS)
	Platform::Path contentBuilderExe = steamworksRoot.AppendFragment("tools/ContentBuilder/builder/steamcmd.exe", true);
#elif defined(MB_PLATFORM_LINUX)
	Platform::Path contentBuilderExe = steamworksRoot.AppendFragment("tools/ContentBuilder/builder_linux/linux32/steamcmd", true);
#elif defined(MB_PLATFORM_MACOS)
	Platform::Path contentBuilderExe = steamworksRoot.AppendFragment("tools/ContentBuilder/builder_osx/osx32/steamcmd", true);
#endif

	if (!contentBuilderExe.Exists())
	{
		Log(LogSeverity::Warning, "Unable to find content-builder executable.\n");
		return false;
	}

	// Create our packaging directory.
	if (!packageDirectory.Exists() && !packageDirectory.CreateAsDirectory())
	{
		Log(LogSeverity::Warning, "Failed to create directory for package generation.\n");
		return false;
	}

	// Create directory structure.
	Platform::Path outputDirectory = packageDirectory.AppendFragment("output", true);
	Platform::Path contentDirectory = packageDirectory.AppendFragment("content", true);
	Platform::Path scriptsDirectory = packageDirectory.AppendFragment("scripts", true);

	if (!outputDirectory.Exists() && !outputDirectory.CreateAsDirectory())
	{
		Log(LogSeverity::Warning, "Failed to create output directory for package generation.\n");
		return false;
	}
	
	if (!contentDirectory.Exists() && !contentDirectory.CreateAsDirectory())
	{
		Log(LogSeverity::Warning, "Failed to create content directory for package generation.\n");
		return false;
	}

	if (!scriptsDirectory.Exists() && !scriptsDirectory.CreateAsDirectory())
	{
		Log(LogSeverity::Warning, "Failed to create script directory for package generation.\n");
		return false;
	}

	// Copy all files specified into the content directory.
	if (!CopyPackageFilesToFolder(projectFile, contentDirectory))
	{
		Log(LogSeverity::Warning, "Failed to copy files to package content directory.\n");
		return false;
	}

	// Generate depot vdf script.
	int depotId = projectFile.Get_Steamworks_DepotId();
	Platform::Path depotVdf = scriptsDirectory.AppendFragment(Strings::Format("%i_depot.vdf", depotId), true);
	if (!GenerateDepotVdf(projectFile, contentDirectory, scriptsDirectory, depotVdf))
	{
		Log(LogSeverity::Warning, "Failed to generate depot VDF script for packaging.\n");
		return false;
	}

	// Generate app vdf script.
	int appId = projectFile.Get_Steamworks_AppId();
	Platform::Path appVdf = scriptsDirectory.AppendFragment(Strings::Format("%i_app.vdf", appId), true);
	if (!GenerateAppVdf(projectFile, outputDirectory, contentDirectory, scriptsDirectory, depotVdf, appVdf))
	{
		Log(LogSeverity::Warning, "Failed to generate app VDF script for packaging.\n");
		return false;
	}

	// Run steamcmd!
	std::vector<std::string> arguments;
	arguments.push_back("+login");
	arguments.push_back(projectFile.Get_Steamworks_Username());
	arguments.push_back(projectFile.Get_Steamworks_Password());
	arguments.push_back("+run_app_build_http");
	arguments.push_back("-preview");
	arguments.push_back(appVdf.ToString());
	arguments.push_back("+quit");

	Platform::Process process;
	if (!process.Open(contentBuilderExe, packageDirectory.GetDirectory(), arguments, false))
	{
		Log(LogSeverity::Warning, "Failed to run SteamCMD to generate package.\n");
		return false;
	}

	process.Wait();

	int exitCode = process.GetExitCode();
	if (exitCode != 0)
	{
		Log(LogSeverity::Warning, "SteamCMD returned failure (exit code %i).\n", exitCode);
		return false;
	}

	return true;
}

bool Steamworks_Packager::GenerateDepotVdf(
	ProjectFile& projectFile, 
	Platform::Path contentPath, 
	Platform::Path scriptsPath, 
	Platform::Path depotVdfPath)
{
	VdfNode root;
	VdfNode& depotRoot = root.Node("DepotBuildConfig");

	depotRoot.Node("DepotID").Value("%i", projectFile.Get_Steamworks_DepotId());

	VdfNode& fileMappingRoot = depotRoot.Node("FileMapping");
	fileMappingRoot.Node("LocalPath").Value("%s/*", scriptsPath.RelativeTo(contentPath).ToString().c_str());
	fileMappingRoot.Node("DepotPath").Value("./");
	fileMappingRoot.Node("recursive").Value("1");

	// Dump vdf to file.
	TextStream stream;
	stream.Write("%s", depotRoot.ToString().c_str());
	if (!stream.WriteToFile(depotVdfPath))
	{
		return false;
	}

	return true;
}

bool Steamworks_Packager::GenerateAppVdf(
	ProjectFile& projectFile, 
	Platform::Path outputPath, 
	Platform::Path contentPath, 
	Platform::Path scriptsPath, 
	Platform::Path depotVdfPath, 
	Platform::Path appVdfPath)
{
	VdfNode root;
	VdfNode& appbuildRoot = root.Node("appbuild");

	appbuildRoot.Node("appid").Value("%i", projectFile.Get_Steamworks_AppId());
	appbuildRoot.Node("desc").Value("%s", projectFile.Get_Steamworks_BuildDescription().c_str());
	appbuildRoot.Node("buildoutput").Value("%s", scriptsPath.RelativeTo(outputPath).ToString().c_str());
	appbuildRoot.Node("contentroot").Value("%s", scriptsPath.RelativeTo(contentPath).ToString().c_str());
	appbuildRoot.Node("setlive").Value("0");
	appbuildRoot.Node("preview").Value("1");

	VdfNode& depotsRoot = appbuildRoot.Node("depots");
	depotsRoot.Node("%i", projectFile.Get_Steamworks_DepotId()).Value("%s", depotVdfPath.GetFilename().c_str());

	// Dump vdf to file.
	TextStream stream;
	stream.Write("%s", appbuildRoot.ToString().c_str());
	if (!stream.WriteToFile(appVdfPath))
	{
		return false;
	}

	return true;
}

Platform::Path Steamworks_Packager::GetContentDirectory(
	const Platform::Path& packageDirectory
)
{
	return packageDirectory.AppendFragment("content", true);
}

}; // namespace MicroBuild