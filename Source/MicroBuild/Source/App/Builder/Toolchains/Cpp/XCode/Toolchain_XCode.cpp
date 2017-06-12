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

// TODO: ios sdk etc support.
// TODO: Framework support
// TODO: -arch support
// TODO: -mios-simulation-version-min=?? etc support
// TODO: Update GCC ParseMessageOutput to support libtool errors (error: libtool: message)

#include "PCH.h"
#include "App/Builder/Toolchains/Cpp/XCode/Toolchain_XCode.h"
#include "Core/Platform/Process.h"

namespace MicroBuild {
	
Toolchain_XCode::Toolchain_XCode(ProjectFile& file, uint64_t configurationHash)
	: Toolchain_Clang(file, configurationHash)
{
	m_useStartEndGroup = false;
}

bool Toolchain_XCode::Init() 
{
	m_bAvailable = FindToolchain();
	m_bRequiresCompileStep = true;
	m_description = Strings::Format("XCode Clang (%s)", m_version.c_str());	
	return m_bAvailable;
}

bool Toolchain_XCode::FindXCodeExe(const std::string& exeName, Platform::Path& output)
{	
	Platform::Process process;

	std::vector<std::string> args;
	args.push_back("-find");
	args.push_back(exeName);

	if (!process.Open("/usr/bin/xcodebuild", "/usr/bin", args, true))
	{
		return false;
	}

	output = Strings::Trim(process.ReadToEnd());
	return output.Exists();
}

bool Toolchain_XCode::FindToolchain()
{
	if (!FindXCodeExe("clang++", m_compilerPath))
	{
		Log(LogSeverity::Warning, "Could not find clang++ in xcode toolchain.\n");
		return false;
	}
	if (!FindXCodeExe("libtool", m_archiverPath))
	{
		Log(LogSeverity::Warning, "Could not find ar in xcode toolchain.\n");
		return false;
	}

	m_linkerPath = m_compilerPath;

	Platform::Process process;

	std::vector<std::string> args;
	args.push_back("--version");
	if (!process.Open(m_compilerPath, m_compilerPath.GetDirectory(), args, true))
	{
		Log(LogSeverity::Warning, "Could not execute clang++ to gain version number.\n");
		return false;
	}

	m_version = "Unknown Version";

	std::vector<std::string> lines = Strings::Split('\n', process.ReadToEnd());
	if (lines.size() > 0)
	{
		std::string versionLine = lines[0];
		std::vector<std::string> split = Strings::Split(' ', versionLine);
		if (split.size() > 4)
		{
			m_version = split[3];
		}
	}	

	// Find the base include and library paths for the toolchain.
	Platform::Path baseUsrPath = m_compilerPath.GetDirectory().GetDirectory();
	m_standardIncludePaths.push_back(baseUsrPath.AppendFragment("include", true));
	m_standardLibraryPaths.push_back(baseUsrPath.AppendFragment("lib", true));

	// Find the platform sdk specific include and library paths.
	Platform::Path baseDeveloperPath = baseUsrPath.GetDirectory().GetDirectory().GetDirectory();
	Platform::Path basePlatformsPath = baseDeveloperPath.AppendFragment("Platforms", true);

	Platform::Path platformPath = basePlatformsPath.AppendFragment("MacOSX.platform", true);
	Platform::Path sdkPath = platformPath.AppendFragment("Developer/SDKs/MacOSX.sdk", true);
	m_standardIncludePaths.push_back(sdkPath.AppendFragment("usr/include", true));
	m_standardLibraryPaths.push_back(sdkPath.AppendFragment("usr/lib", true));

	m_isysRoot = sdkPath;

	return true;
}

void Toolchain_XCode::GetBaseCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args)
{
	Toolchain_Gcc::GetBaseCompileArguments(file, args);

	args.push_back("-isysroot");
	args.push_back(m_isysRoot.ToString());
}

void Toolchain_XCode::GetArchiveArguments(const std::vector<BuilderFileInfo>& sourceFiles, std::vector<std::string>& args)
{
	Platform::Path outputPath = GetOutputPath();
	Platform::Path pchPath = GetPchPath();
	Platform::Path pchObjectPath = GetPchObjectPath();

	args.push_back("-static");
	args.push_back("-o");
	args.push_back(Strings::Quoted(outputPath.ToString()).c_str());
	args.push_back("-filelist");
	args.push_back("@");

	// Object files to link.
	for (auto& sourceFile : sourceFiles)
	{
		args.push_back(sourceFile.OutputPath.ToString());
	}

	// Link PCH.
	if (!pchObjectPath.IsEmpty() && !m_projectFile.Get_Build_PrecompiledHeader().IsEmpty())
	{
		args.push_back(pchObjectPath.ToString());
	}
}

}; // namespace MicroBuild