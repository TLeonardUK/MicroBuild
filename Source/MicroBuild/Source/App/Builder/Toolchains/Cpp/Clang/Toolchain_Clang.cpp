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
#include "App/Builder/Toolchains/Cpp/Clang/Toolchain_Clang.h"
#include "App/Builder/Toolchains/Cpp/Microsoft/Toolchain_Microsoft.h"
#include "Core/Platform/Process.h"

namespace MicroBuild {

Toolchain_Clang::Toolchain_Clang(ProjectFile& file, uint64_t configurationHash)
	: Toolchain_Gcc(file, configurationHash)
{
}

bool Toolchain_Clang::Init() 
{
	m_bAvailable = FindToolchain();
	m_bRequiresCompileStep = true;
	m_bRequiresVersionInfo = true;
	m_description = Strings::Format("Clang (%s)", m_version.c_str());	
	return m_bAvailable;
}

bool Toolchain_Clang::FindToolchain()
{
	std::vector<Platform::Path> additionalDirs;

#if defined(MB_PLATFORM_WINDOWS)
	additionalDirs.push_back("C:/Program Files/LLVM/bin");
#endif
	
#if defined(MB_PLATFORM_WINDOWS)
	if (!InitMicrosoftToolchain())
	{
		return false;
	}
#endif

	if (!Platform::Path::FindFile("clang++", m_compilerPath, additionalDirs))
	{
		return false;
	}

	if (!Platform::Path::FindFile("llvm-ar", m_archiverPath, additionalDirs))
	{
		return false;
	}
	
	m_linkerPath = m_compilerPath;

	Platform::Process process;

	std::vector<std::string> args;
	args.push_back("--version");
	if (!process.Open(m_compilerPath, m_compilerPath.GetDirectory(), args, true))
	{
		return false;
	}

	m_version = "Unknown Version";

	std::string data = process.ReadToEnd();
//	Log(LogSeverity::Warning, "Data=%s\n", data.c_str());

	std::vector<std::string> lines = Strings::Split('\n', data);
	if (lines.size() > 0)
	{
		std::string versionLine = lines[0];
		std::vector<std::string> split = Strings::Split(' ', versionLine);
		if (split.size() >= 3)
		{
			m_version = split[2];
		}
	}	

	return true;
}

bool Toolchain_Clang::CompileVersionInfo(BuilderFileInfo& fileInfo) 
{	
	 MB_UNUSED_PARAMETER(fileInfo);

#if defined(MB_PLATFORM_WINDOWS)
	
	// Use microsoft toolchain to generate the version info file which we can link in to the exe.
	if (!m_microsoftToolchain.CompileVersionInfo(fileInfo))
	{
		return false;
	}

#elif defined(MB_PLATFORM_LINUX)

	// We could generate a desktop entry file here, but this
	// seems more something to do during packaging as it adds files
	// rather than modifies existing executables.

#elif defined(MB_PLATFORM_MACOS)

	// On MacOS we want to generate a plist/iconset and embed
	// it within the application package. This is better done during packaging
	// rather than during compilation.

#endif

	return true;
}

}; // namespace MicroBuild
