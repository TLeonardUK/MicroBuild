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
#include "Core/Platform/Platform.h"
#include "App/Builder/Toolchains/Cpp/Emscripten/Toolchain_Emscripten.h"

namespace MicroBuild {
	
Toolchain_Emscripten::Toolchain_Emscripten(ProjectFile& file, uint64_t configurationHash)
	: Toolchain_Gcc(file, configurationHash)
{
}

bool Toolchain_Emscripten::Init() 
{
	m_bAvailable = FindToolchain();
	m_bRequiresCompileStep = true;
	m_description = Strings::Format("Emscripten (%s)", m_version.c_str());;
	return m_bAvailable;
}

bool Toolchain_Emscripten::FindToolchain()
{
	Platform::Path basePath = Platform::GetEnvironmentVariable("EMSCRIPTEN");
	if (!basePath.Exists())
	{
		return false;
	}
	
	Platform::Path versionPath = basePath.AppendFragment("emscripten-version.txt", true);
	if (!versionPath.Exists() || !Strings::ReadFile(versionPath, m_version))
	{
		return false;
	}

	m_version = Strings::Trim(m_version);

#if defined(MB_PLATFORM_WINDOWS)
	m_compilerPath = basePath.AppendFragment("em++.bat", true);
	m_archiverPath = basePath.AppendFragment("emar.bat", true);
#else
	m_compilerPath = basePath.AppendFragment("em++", true);
	m_archiverPath = basePath.AppendFragment("emar", true);
#endif
	m_linkerPath = m_compilerPath;

	if (!m_compilerPath.Exists() ||
		!m_archiverPath.Exists() ||
		!m_linkerPath.Exists())
	{
		return false;
	}
	
	return true;
}

}; // namespace MicroBuild