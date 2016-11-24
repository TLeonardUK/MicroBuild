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
#include "App/Builder/Toolchains/Cpp/AndroidNdk/Toolchain_AndroidNdk.h"
#include "Core/Platform/Process.h"

namespace MicroBuild {

Toolchain_AndroidNdk::Toolchain_AndroidNdk(ProjectFile& file, uint64_t configurationHash)
	: Toolchain_Clang(file, configurationHash)
{
}

bool Toolchain_AndroidNdk::Init() 
{
	m_bAvailable = FindToolchain();
	m_bRequiresCompileStep = true;
	m_description = Strings::Format("Android NDK (%s)", m_version.c_str());	
	return m_bAvailable;
}

bool Toolchain_AndroidNdk::FindToolchain()
{
	Platform::Path basePath = Platform::GetEnvironmentVariable("NDK_ROOT");
	if (basePath.IsEmpty())
	{
		Log(LogSeverity::Fatal, "NDK_ROOT environment variable not defined, unable to locate toolchain.\n");
		return false;
	}
	if (!basePath.Exists())
	{
		Log(LogSeverity::Fatal, "NDK_ROOT environment variable pointing to non-existing folder.\n");
		return false;
	}
	
	// Get a list of all platform folders.
	Platform::Path platformFolder = basePath.AppendFragment("platforms", true);
	std::vector<std::string> platformFolders = platformFolder.GetDirectories();

	if (platformFolders.size() == 0)
	{
		Log(LogSeverity::Fatal, "Failed to find platforms list from NDK.\n");
		return false;
	}

	// Get versions.
	std::vector<int> apiLevels;
	for (auto& folder : platformFolders)
	{
		std::string prefix = "android-";
		if (folder.size() > prefix.size())
		{
			apiLevels.push_back(CastFromString<int>(folder.substr(prefix.size())));
		}
	}
	std::sort(apiLevels.begin(), apiLevels.end());

	// Get folder name for android SDK we want.
	if (m_projectFile.Get_Build_PlatformToolset() == EPlatformToolset::Default)
	{
		std::string apiLevelStr = CastToString(apiLevels[apiLevels.size() - 1]);
		m_platformFolder = platformFolder.AppendFragment("android-" + apiLevelStr, true);
	}
	else
	{
		std::string platformFolderName = CastToString(m_projectFile.Get_Build_PlatformToolset());
		platformFolderName = Strings::Replace(platformFolderName, "_", "-");
		platformFolderName = Strings::Replace(platformFolderName, "Android", "android");
		m_platformFolder = platformFolder.AppendFragment(platformFolderName, true);
	}

	// Check folder exists.	
	if (!m_platformFolder.Exists())
	{
		Log(LogSeverity::Fatal, "Could not find installed api level at: %s\n", m_platformFolder.ToString().c_str());
		return false;
	}

	// Grab version number.
	Platform::Path versionFile = basePath.AppendFragment("source.properties", true);

	m_version = "Unknown version";

	if (versionFile.Exists())
	{
		std::string versionData = "";
		std::string versionKey = "Pkg.Revision";

		if (Strings::ReadFile(versionFile, versionData))
		{
			std::vector<std::string> lines = Strings::Split('\n', versionData);
			for (auto& line : lines)
			{
				if (line.size() > versionKey.size() && line.substr(0, versionKey.size()) == versionKey)
				{
					size_t equalIndex = line.find("=");
					if (equalIndex != std::string::npos)
					{
						m_version = Strings::Trim(line.substr(equalIndex + 1));
						break;
					}
				}
			}
		}
	}

	m_version = m_version + ", targeting " + m_platformFolder.GetFilename();

	// Find toolchain folder.
	std::string gccPlatformPrefix = "";
	std::string archName = "";

	switch (m_projectFile.Get_Target_Platform())
	{	
	case EPlatform::Android_ARM:
		{	
			gccPlatformPrefix = "arm-linux-androideabi-";
			archName = "arm";
			break;
		}
	case EPlatform::Android_ARM64:
		{	
			gccPlatformPrefix = "aarch64-linux-android-";
			archName = "arm64";
			break;
		}
	case EPlatform::Android_x86:
		{	
			gccPlatformPrefix = "a86-linux-android-";
			archName = "x86";
			break;
		}
	case EPlatform::Android_x64:
		{	
			gccPlatformPrefix = "x86_64-linux-android-";
			archName = "x86_64";
			break;
		}
	case EPlatform::Android_MIPS:
		{	
			gccPlatformPrefix = "mipsel-linux-android-";
			archName = "mips";
			break;
		}
	case EPlatform::Android_MIPS64:
		{	
			gccPlatformPrefix = "mips64el-linux-android-";
			archName = "mips64";
			break;
		}
	default:
		{
			assert(false);
			return false;
		}
	}
	
	m_sysRoot = m_platformFolder.AppendFragment("arch-" + archName, true);

	m_standardIncludePaths.push_back(m_sysRoot.AppendFragment("usr/include", true));
	m_standardLibraryPaths.push_back(m_sysRoot.AppendFragment("usr/lib", true));

	Platform::Path matchFilter = 
		basePath
		.AppendFragment("toolchains", true)
		.AppendFragment(gccPlatformPrefix + "*", true);

	std::vector<Platform::Path> matchingFolders = 
		Platform::Path::MatchFilter(matchFilter);
	
	std::vector<float> gccVersions;
	for (auto& folder : matchingFolders)
	{
		std::string filename = folder.GetFilename();
		if (filename.size() > gccPlatformPrefix.size())
		{
			gccVersions.push_back(CastFromString<float>(filename.substr(gccPlatformPrefix.size())));
		}
	}
	std::sort(gccVersions.begin(), gccVersions.end());

	// Check folder exists.	
	if (gccVersions.size() == 0)
	{
		Log(LogSeverity::Fatal, "Could not find valid NDK toolset for platform at: %s\n", m_platformFolder.ToString().c_str());
		return false;
	}

	Platform::Path toolchainBaseFolder = 
		basePath
		.AppendFragment("toolchains", true)
		.AppendFragment(Strings::Format("%s%.1f", gccPlatformPrefix.c_str(), gccVersions[gccVersions.size() - 1]), true);
	
#if defined(MB_PLATFORM_WINDOWS)
	#define PLATFORM_NAME "windows"
#elif defined(MB_PLATFORM_LINUX)
	#define PLATFORM_NAME "linux"
#elif defined(MB_PLATFORM_MACOS)
	#define PLATFORM_NAME "darwin"
#endif

	m_toolchainFolder = toolchainBaseFolder.AppendFragment("prebuilt/" PLATFORM_NAME "-x86_64", true);	
	if (!m_toolchainFolder.Exists())
	{
		m_toolchainFolder = toolchainBaseFolder.AppendFragment("prebuilt/" PLATFORM_NAME "-x86", true);
	}
	if (!m_toolchainFolder.Exists())
	{
		Log(LogSeverity::Fatal, "Could not find valid prebuilt NDK for current platform at: %s\n", m_platformFolder.ToString().c_str());
		return false;
	}

#undef PLATFORM_NAME

	Platform::Path binFolder = m_toolchainFolder.AppendFragment("bin", true);

#if defined(MB_PLATFORM_WINDOWS)
	m_compilerPath = binFolder.AppendFragment(gccPlatformPrefix + "g++.exe", true);
	m_archiverPath = binFolder.AppendFragment(gccPlatformPrefix + "ar.exe", true);
	m_linkerPath = binFolder.AppendFragment(gccPlatformPrefix + "g++.exe", true);
#else
	m_compilerPath = binFolder.AppendFragment(gccPlatformPrefix + "g++", true);
	m_archiverPath = binFolder.AppendFragment(gccPlatformPrefix + "ar", true);
	m_linkerPath = binFolder.AppendFragment(gccPlatformPrefix + "g++", true);
#endif

	if (!m_compilerPath.Exists() ||
		!m_archiverPath.Exists() ||
		!m_linkerPath.Exists())
	{
		return false;
	}
	
	return true;
}

void Toolchain_AndroidNdk::GetBaseCompileArguments(std::vector<std::string>& args) 
{
	Toolchain_Clang::GetBaseCompileArguments(args);

	switch (m_projectFile.Get_Target_Platform())
	{
		case EPlatform::Android_ARM:
			{
				args.push_back("-march=armv7-a"); 
				break;
			}
		case EPlatform::Android_ARM64:
			{
				args.push_back("-march=armv8-a"); 
				break;
			}
		case EPlatform::Android_x86:
			{
				args.push_back("-march=x86"); 
				break;
			}
		case EPlatform::Android_x64:
			{
				args.push_back("-march=x86_64"); 
				break;
			}
		case EPlatform::Android_MIPS:
			{
				args.push_back("-march=mips"); 
				break;
			}
		case EPlatform::Android_MIPS64:
			{
				args.push_back("-march=mips64"); 
				break;
			}
		default:
			{
				assert(false);
				break;
			}
	}

	args.push_back("-mfloat-abi=softfp");
	args.push_back("-mfpu=vfpv3-d16");

	args.push_back(Strings::Format("--sysroot=%s" ,Strings::Quoted(m_sysRoot.ToString()).c_str()));
}

void Toolchain_AndroidNdk::GetLinkArguments(const std::vector<BuilderFileInfo>& sourceFiles, std::vector<std::string>& args) 
{
	Toolchain_Clang::GetLinkArguments(sourceFiles, args);
	
	switch (m_projectFile.Get_Target_Platform())
	{
		case EPlatform::Android_ARM:
			{
				args.push_back("-march=armv7-a"); 
				break;
			}
		case EPlatform::Android_ARM64:
			{
				args.push_back("-march=armv8-a"); 
				break;
			}
		case EPlatform::Android_x86:
			{
				args.push_back("-march=x86"); 
				break;
			}
		case EPlatform::Android_x64:
			{
				args.push_back("-march=x86_64"); 
				break;
			}
		case EPlatform::Android_MIPS:
			{
				args.push_back("-march=mips"); 
				break;
			}
		case EPlatform::Android_MIPS64:
			{
				args.push_back("-march=mips64"); 
				break;
			}
		default:
			{
				assert(false);
				break;
			}
	}

	args.push_back("-Wl,--fix-cortex-a8");
	
	args.push_back(Strings::Format("--sysroot=%s" ,Strings::Quoted(m_sysRoot.ToString()).c_str()));
}

}; // namespace MicroBuild
