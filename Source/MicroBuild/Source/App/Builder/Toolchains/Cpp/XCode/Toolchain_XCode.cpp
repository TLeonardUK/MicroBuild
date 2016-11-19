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
#include "App/Builder/Toolchains/Cpp/XCode/Toolchain_XCode.h"
#include "Core/Platform/Process.h"

namespace MicroBuild {
	
Toolchain_XCode::Toolchain_XCode(ProjectFile& file, uint64_t configurationHash)
	: Toolchain_Clang(file, configurationHash)
{
}

bool Toolchain_XCode::Init() 
{
	m_bAvailable = FindToolchain();
	m_bRequiresCompileStep = true;
	m_description = Strings::Format("XCode Clang (%s)", m_version.c_str());	
	return m_bAvailable;
}

bool Toolchain_XCode::FindToolchain()
{
	std::vector<Platform::Path> additionalDirs;

	additionalDirs.push_back("/Applications/Xcode.app/Contents/Developer/usr/bin");

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

	std::vector<std::string> lines = Strings::Split('\n', process.ReadToEnd());
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
}; // namespace MicroBuild