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
#include "App/Builder/Toolchains/Cpp/NintendoWiiU/Toolchain_NintendoWiiU.h"
#include "App/Builder/Toolchains/Cpp/Nintendo3ds/Toolchain_Nintendo3ds.h"
#include "App/Builder/Toolchains/Cpp/NintendoWiiU/Toolchain_NintendoWiiUOutputParser.h"
#include "Core/Platform/Process.h"
#include "Core/Platform/Platform.h"
#include "Core/Helpers/Strings.h"

namespace MicroBuild {

Toolchain_NintendoWiiU::Toolchain_NintendoWiiU(ProjectFile& file, uint64_t configurationHash)
	: Toolchain(file, configurationHash)
{
}

bool Toolchain_NintendoWiiU::Init() 
{
	m_bAvailable = FindToolchain();
	m_bRequiresCompileStep = true;
	m_description = Strings::Format("Nintendo WiiU (SDK %s)", m_version.c_str());	
	return m_bAvailable;
}

bool Toolchain_NintendoWiiU::FindToolchain()
{
	// Nintendo's SDK's are just gross, stuff thrown all over the place, sigh.

	m_sdkPath		= Platform::GetEnvironmentVariable("CAFE_ROOT");
	m_toolchainPath = Platform::GetEnvironmentVariable("GHS_ROOT");
	
	if (m_sdkPath.IsEmpty())
	{
		Log(LogSeverity::Fatal, "CAFE_ROOT environment variable not defined, unable to locate toolchain.");
		return false;
	}
	if (!m_sdkPath.Exists())
	{
		Log(LogSeverity::Fatal, "CAFE_ROOT environment variable pointing to non-existing folder.");
		return false;
	}
	if (m_toolchainPath.IsEmpty())
	{
		Log(LogSeverity::Fatal, "GHS_ROOT environment variable not defined, unable to locate toolchain.");
		return false;
	}
	if (!m_toolchainPath.Exists())
	{
		Log(LogSeverity::Fatal, "GHS_ROOT environment variable pointing to non-existing folder.");
		return false;
	}

	m_compilerPath	= m_toolchainPath.AppendFragment("cxppc.exe", true);
	m_linkerPath	= m_toolchainPath.AppendFragment("cxppc.exe", true);
	m_archiverPath	= m_toolchainPath.AppendFragment("cxppc.exe", true);	

	if (Platform::IsOperatingSystem64Bit())
	{
		m_makeRplPath	= m_sdkPath.AppendFragment("system/bin/tool/makerpl64.exe", true);
		m_prepRplPath	= m_sdkPath.AppendFragment("system/bin/tool/preprpl64.exe", true);
	}
	else
	{
		m_makeRplPath	= m_sdkPath.AppendFragment("system/bin/tool/makerpl32.exe", true);
		m_prepRplPath	= m_sdkPath.AppendFragment("system/bin/tool/preprpl32.exe", true);
	}
	
	m_rplExportAllPath = m_sdkPath.AppendFragment("system/bin/tool/rplexportall.exe", true);

	if (!m_compilerPath.Exists() ||
		!m_linkerPath.Exists()	 ||
		!m_archiverPath.Exists() ||
		!m_makeRplPath.Exists()  ||
		!m_prepRplPath.Exists()  ||
		!m_rplExportAllPath.Exists())
	{
		return false;
	}

	// This is just ridiculous, there doesn't appear to be a nice way to get the SDK version
	// so we're just going to extract it from one of the files we know is always there.
	std::string versionFileData;
	if (!Strings::ReadFile(m_sdkPath.AppendFragment("system/include/sdk_ver.h", true), versionFileData))
	{
		return false;
	}

	std::string nnVersionMajor;
	std::string nnVersionMinor;
	std::string nnVersionMicro;
	std::string nnVersionRelStep;
	std::string nnVersionPatch;

	std::vector<std::string> lines = Strings::Split('\n', versionFileData);
	for (auto& line : lines)
	{
		std::string prefix = "#define ";
		
		line = Strings::Trim(line);
		line = Strings::Replace(Strings::Replace(line, "\t", " "), "  ", " ");

		if (line.size() > prefix.size() && line.substr(0, prefix.size()) == prefix)
		{
			std::vector<std::string> split = Strings::Crack(line);
			if (split.size() >= 3)
			{
				std::string name = Strings::Trim(split[1]);
				std::string value = Strings::Trim(split[2]);

				if (name == "CAFE_OS_SDK_VERSION_STRING")
				{
					m_version = Strings::Replace(value, "\"", "");
				}
			}
		}
	}

	m_standardIncludePaths.push_back(m_sdkPath.AppendFragment("system/include", true));

	if (m_projectFile.Get_Build_OptimizationLevel() == EOptimizationLevel::Debug)
	{
		m_standardLibraryPaths.push_back(m_sdkPath.AppendFragment("system/lib/ghs/cafe/DEBUG", true));
	}
	else
	{
		m_standardLibraryPaths.push_back(m_sdkPath.AppendFragment("system/lib/ghs/cafe/NDEBUG", true));
	}

	return true;
}

Platform::Path Toolchain_NintendoWiiU::GetElfPath()
{
	return GetOutputPath().ChangeExtension("elf");
}

Platform::Path Toolchain_NintendoWiiU::GetMapPath()
{
	return GetOutputPath().ChangeExtension("map");
}

Platform::Path Toolchain_NintendoWiiU::GetRplExportObjectPath()
{
	return m_projectFile.Get_Project_IntermediateDirectory()
		.AppendFragment(Strings::Format("__%s_rpl_exports.o", m_projectFile.Get_Project_Name().c_str()), true);
}

Platform::Path Toolchain_NintendoWiiU::GetRplExportDefPath()
{
	return m_projectFile.Get_Project_IntermediateDirectory()
		.AppendFragment(Strings::Format("__%s_rpl_exports.def", m_projectFile.Get_Project_Name().c_str()), true);
}

Platform::Path Toolchain_NintendoWiiU::GetRplExportLibraryPath()
{
	return GetOutputPath().ChangeExtension("a");
}

void Toolchain_NintendoWiiU::GetBaseCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args)
{
	MB_UNUSED_PARAMETER(file);
	MB_UNUSED_PARAMETER(args);

	// Default arguments.
	args.push_back("-DNDEV=1");
	args.push_back("-DCAFE=1");
	args.push_back("-DPLATFORM=CAFE");
	args.push_back("-DEPPC");
	args.push_back("-cpu=espresso");
	args.push_back("-sda=none");

	args.push_back("--no_wrap_diagnostics");
	args.push_back("--g++");	

	switch (m_projectFile.Get_Build_OptimizationLevel())
	{
	default:
		// Fallthrough
	case EOptimizationLevel::None:
		// Fallthrough
	case EOptimizationLevel::Debug:	
		{
			args.push_back("-D_DEBUG=1");
			args.push_back("-Odebug");
			break;
		}
	case EOptimizationLevel::PreferSize:
		{
			args.push_back("-DNDEBUG=1");
			args.push_back("-Ospace");
			break;
		}
	case EOptimizationLevel::PreferSpeed:
		{
			args.push_back("-DNDEBUG=1");
			args.push_back("-Ospeed");
			break;
		}
	case EOptimizationLevel::Full:
		{
			args.push_back("-DNDEBUG=1");
			args.push_back("-Ospeed");
			break;
		}
	}

	switch (m_projectFile.Get_Build_WarningLevel())
	{
	default:
		// Fallthrough
	case EWarningLevel::Default:
		// Fallthrough
	case EWarningLevel::None:
		// Fallthrough
	case EWarningLevel::Low:
		// Fallthrough
	case EWarningLevel::High:
		// Fallthrough
	case EWarningLevel::Medium:
		{
			// Nothing but verbose does anything else.
			break;
		}
	case EWarningLevel::Verbose:
		{
			args.push_back("--remarks");
			break;
		}
	}

	std::vector<std::string> customArgs = Strings::Crack(m_projectFile.Get_Build_CompilerArguments());
	args.insert(args.end(), customArgs.begin(), customArgs.end());

	for (auto& define : m_projectFile.Get_Defines_Define())
	{
		args.push_back(Strings::Format("-D%s", Strings::Quoted(define).c_str()));
	}

	for (auto& warning : m_projectFile.Get_DisabledWarnings_DisabledWarning())
	{
		args.push_back(Strings::Format("-Wno-%s", warning.c_str()));
	}

	for (auto& includeDir : m_projectFile.Get_SearchPaths_IncludeDirectory())
	{
		args.push_back(Strings::Format("-I%s", Strings::Quoted(includeDir.ToString()).c_str()));
	}

	for (auto& includeDir : m_standardIncludePaths)
	{
		args.push_back(Strings::Format("-I%s", Strings::Quoted(includeDir.ToString()).c_str()));
	}

	for (auto& forcedInclude : m_projectFile.Get_ForcedIncludes_ForcedInclude())
	{
		args.push_back(Strings::Format("-include %s", Strings::Quoted(forcedInclude.ToString()).c_str()));
	}

	if (m_projectFile.Get_Flags_CompilerWarningsFatal())
	{
		args.push_back("--quit_after_warnings");	
	}

	if (m_projectFile.Get_Flags_GenerateDebugInformation())
	{ 
		args.push_back("-G");	
	}
	
	if (m_projectFile.Get_Flags_Exceptions())
	{
		args.push_back("--exceptions");	
	}
	else
	{
		args.push_back("--no_exceptions");
	}
	
	// Prohibited from use by Nintendo for some reason.
	if (m_projectFile.Get_Flags_LinkTimeOptimization())
	{
		args.push_back("-Owholeprogram");	
	}

	// Dumps out dependencies to a file.
	args.push_back("-MMD");
}

void Toolchain_NintendoWiiU::GetPchCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args) 
{
	MB_UNUSED_PARAMETER(file);

	Platform::Path pchPath = GetPchPath();
	
	args.push_back("--create_pch=" + Strings::Quoted(pchPath.ToString()));
	args.push_back(Strings::Quoted(m_projectFile.Get_Build_PrecompiledSource().ToString()));
}

void Toolchain_NintendoWiiU::GetSourceCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args) 
{
	// Include our generated pch before anything else.
	if (!m_projectFile.Get_Build_PrecompiledHeader().IsEmpty())
	{
		Platform::Path pchPath = GetPchPath();
		args.push_back(Strings::Format("-include %s", Strings::Quoted(pchPath.ToString()).c_str()));
	}

	args.push_back("-c");

	args.push_back("-o");
	args.push_back(Strings::Quoted(file.OutputPath.ToString()));

	args.push_back(Strings::Quoted(file.SourcePath.ToString()));
}

void Toolchain_NintendoWiiU::GetLinkArguments(const std::vector<BuilderFileInfo>& sourceFiles, std::vector<std::string>& args) 
{
	MB_UNUSED_PARAMETER(sourceFiles);
	MB_UNUSED_PARAMETER(args);

	Platform::Path outputPath = GetElfPath();
	Platform::Path mapPath = GetMapPath();
	Platform::Path pchObjectPath = GetPchObjectPath();
	
	args.push_back("-o");
	args.push_back(Strings::Quoted(outputPath.ToString()));

	args.push_back("--no_wrap_diagnostics");

	args.push_back("-lnk=\"-nosegments_always_executable\"");

	std::vector<std::string> customArgs = Strings::Crack(m_projectFile.Get_Build_LinkerArguments());	
	for (auto& arg : customArgs)
	{
		args.push_back(Strings::Format("-lnk=%s", Strings::Quoted(arg).c_str()));
	}

	switch (m_projectFile.Get_Build_OptimizationLevel())
	{
	default:
		// Fallthrough
	case EOptimizationLevel::None:
		// Fallthrough
	case EOptimizationLevel::Debug:	
		{
			args.push_back("-Odebug");
			break;
		}
	case EOptimizationLevel::PreferSize:
		{
			args.push_back("-Ospace");
			break;
		}
	case EOptimizationLevel::PreferSpeed:
		{
			args.push_back("-Ospeed");
			break;
		}
	case EOptimizationLevel::Full:
		{
			args.push_back("-Ospeed");
			break;
		}
	}

	switch (m_projectFile.Get_Build_WarningLevel())
	{
	default:
		// Fallthrough
	case EWarningLevel::Default:
		// Fallthrough
	case EWarningLevel::None:
		// Fallthrough
	case EWarningLevel::Low:
		// Fallthrough
	case EWarningLevel::High:
		// Fallthrough
	case EWarningLevel::Medium:
		{
			// Nothing but verbose does anything else.
			break;
		}
	case EWarningLevel::Verbose:
		{
			args.push_back("--remarks");
			break;
		}
	}

	args.push_back("-delete");
	
	if (m_projectFile.Get_Flags_CompilerWarningsFatal())
	{
		args.push_back("--quit_after_warnings");	
	}

	if (m_projectFile.Get_Flags_GenerateDebugInformation())
	{ 
		args.push_back("-G");	
	}
	
	args.push_back(Strings::Format("-map=%s", Strings::Quoted(mapPath.ToString()).c_str()));
	
	args.push_back("-Mn");	
	args.push_back("-Mu");	
	
	if (m_projectFile.Get_Project_OutputType() == EOutputType::DynamicLib)
	{
		args.push_back(Strings::Quoted(m_sdkPath.AppendFragment("system/include/cafe/eppc.Cafe.rpl.ld", true).ToString()));
	}
	else
	{
		args.push_back(Strings::Quoted(m_sdkPath.AppendFragment("system/include/cafe/eppc.Cafe.ld", true).ToString()));
	}

	args.push_back("-cpu=espresso");
	args.push_back("-sda=none");	
	args.push_back("--g++");	
	
	if (m_projectFile.Get_Project_OutputType() == EOutputType::DynamicLib)
	{
		args.push_back("-e");
		args.push_back(Strings::Quoted("__rpl_crt"));
	}
	else
	{
		args.push_back("-e");
		args.push_back(Strings::Quoted("_start"));
	}

	args.push_back("-nostartfiles");
	args.push_back("--link_once_templates");

	args.push_back("-relprog_cafe");

	if (m_projectFile.Get_Flags_Exceptions())
	{
		args.push_back("--exceptions");	
	}
	else
	{
		args.push_back("--no_exceptions");
	}	
	
	for (auto& warning : m_projectFile.Get_DisabledWarnings_DisabledWarning())
	{
		args.push_back(Strings::Format("-Wno-%s", warning.c_str()));
	}

	for (auto& includeDir : m_projectFile.Get_SearchPaths_LibraryDirectory())
	{
		args.push_back(Strings::Format("-L %s", Strings::Quoted(includeDir.ToString()).c_str()));
	}

	for (auto& includeDir : m_standardLibraryPaths)
	{
		args.push_back(Strings::Format("-L %s", Strings::Quoted(includeDir.ToString()).c_str()));
	}
	
	if (m_projectFile.Get_Flags_CompilerWarningsFatal())
	{
		args.push_back("-Werror");	
	}
		
	// Object files to link.
	for (auto& sourceFile : sourceFiles)
	{
		args.push_back(Strings::Quoted(sourceFile.OutputPath.ToString()));
	}
	
	// Link PCH.
	if (!pchObjectPath.IsEmpty() && !m_projectFile.Get_Build_PrecompiledHeader().IsEmpty())
	{
		args.push_back(Strings::Quoted(pchObjectPath.ToString()));
	}
	
	// Add RPL export file.
	if (m_projectFile.Get_Project_OutputType() == EOutputType::DynamicLib)
	{
		args.push_back(Strings::Quoted(GetRplExportObjectPath().ToString()));

		// Dynamic libs require linking against the rpl crt.
		args.push_back(FindLibraryPath("rpl.a").ToString());
	}
	
	// Libraries.
	for (auto& library : m_projectFile.Get_Libraries_Library())
	{
		Platform::Path libPath = library;
		if (libPath.IsRelative())
		{
			libPath = FindLibraryPath(libPath);
		}
		args.push_back(Strings::Quoted(libPath.ToString()));
	}
	
	// Always required, or we won't compile, might as well force import it here.
	args.push_back(FindLibraryPath("coredyn.a").ToString());
}

void Toolchain_NintendoWiiU::GetArchiveArguments(const std::vector<BuilderFileInfo>& sourceFiles, std::vector<std::string>& args) 
{	
	MB_UNUSED_PARAMETER(sourceFiles);
	MB_UNUSED_PARAMETER(args);

	Platform::Path outputPath = GetOutputPath();	
	Platform::Path pchPath = GetPchPath();
	Platform::Path pchObjectPath = GetPchObjectPath();

	args.push_back("--no_wrap_diagnostics");

	args.push_back("-merge_archive");	
	if (m_projectFile.Get_Flags_GenerateDebugInformation())
	{ 
		args.push_back("-G");	
	}

	args.push_back("-o");	
	args.push_back(Strings::Quoted(outputPath.ToString()).c_str());	
	
	// Object files to link.
	for (auto& sourceFile : sourceFiles)
	{
		args.push_back(Strings::Quoted(sourceFile.OutputPath.ToString()));
	}
	
	// Link PCH.
	if (!pchObjectPath.IsEmpty() && !m_projectFile.Get_Build_PrecompiledHeader().IsEmpty())
	{
		args.push_back(Strings::Quoted(pchObjectPath.ToString()));
	}
}

bool Toolchain_NintendoWiiU::ParseMessageOutput(BuilderFileInfo& file, std::string& input)
{
	MB_UNUSED_PARAMETER(file);

	std::vector<ToolchainOutputMessage> messages;

	Toolchain_NintendoWiiUOutputParser parser;
	parser.ExtractMessages(input, messages);

	for (ToolchainOutputMessage& message : messages)
	{
		file.AddMessage(message);
	}

	return true;
}

bool Toolchain_NintendoWiiU::ParseOutput(BuilderFileInfo& file, std::string& input)
{
	if (!Toolchain::ParseOutput(file, input))
	{
		return false;
	}
	if (!Toolchain_Gcc::ParseDependencyFile(file, input))
	{
		return false;
	}
	if (!ParseMessageOutput(file, input))
	{
		return false;
	}
	return true;
}

bool Toolchain_NintendoWiiU::RplExportAll(std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile, const Platform::Path& outputDefFile)
{
	MB_UNUSED_PARAMETER(files);
	MB_UNUSED_PARAMETER(outputFile);
	MB_UNUSED_PARAMETER(outputDefFile);
	
	Platform::Path pchObjectPath = GetPchObjectPath();

	std::vector<std::string> arguments;

	// Object files to link.
	for (auto& sourceFile : files)
	{
		arguments.push_back(Strings::Quoted(sourceFile.OutputPath.ToString()));
	}
	
	// Link PCH.
	if (!pchObjectPath.IsEmpty() && !m_projectFile.Get_Build_PrecompiledHeader().IsEmpty())
	{
		arguments.push_back(Strings::Quoted(pchObjectPath.ToString()));
	}

	// Run the app and hope for the best!
	Platform::Process process;
	Platform::Path responseFilePath = outputFile.OutputPath.AppendFragment(".rsp", false);
	if (!OpenResponseFileProcess(process, responseFilePath, m_rplExportAllPath, m_rplExportAllPath.GetDirectory(), arguments, true))
	{
		return false;
	}
	
	std::string output = process.ReadToEnd();
	if (process.GetExitCode() != 0)
	{
		printf("%s", output.c_str());
		return false;
	}	

	// Split output by lines and remove functions that are not allowed to be exported.
	std::vector<std::string> blacklist;
	blacklist.push_back("main");
	blacklist.push_back("rpl_entry");

	std::vector<std::string> lines = Strings::Split('\n', output);
	std::string filteredOutput = "";
	for (auto& line : lines)
	{
		bool bIsValid = true;

		for (auto& blackEntry : blacklist)
		{
			if (line.size() >= blackEntry.size() && line.substr(0, blackEntry.size() + 1) == blackEntry + " ")
			{
				bIsValid = false;
				break;
			}
		}

		if (bIsValid)
		{
			filteredOutput += line + "\n";
		}
	}

	// Write output to file.
	if (!Strings::WriteFile(outputDefFile, filteredOutput))
	{
		return false;
	}

	return true;
}

bool Toolchain_NintendoWiiU::PrepRPL(std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile)
{
	MB_UNUSED_PARAMETER(files);
	MB_UNUSED_PARAMETER(outputFile);
	
	Platform::Path elfPath = GetElfPath();
	Platform::Path rplObjectPath = GetRplExportObjectPath();
	Platform::Path rplLibraryPath = GetRplExportLibraryPath();
	Platform::Path pchObjectPath = GetPchObjectPath();
	Platform::Path rplExportDefLibraryPath = GetRplExportDefPath();
	
	if (!RplExportAll(files, outputFile, rplExportDefLibraryPath))
	{
		return false;
	}

	std::vector<std::string> arguments;

	arguments.push_back("-o");
	arguments.push_back(Strings::Quoted(rplObjectPath.ToString()));

	arguments.push_back("-x");
	arguments.push_back(Strings::Quoted(rplExportDefLibraryPath.ToString()));

	arguments.push_back("-l");
	arguments.push_back(Strings::Quoted(rplLibraryPath.ToString()));

	arguments.push_back("-q");

	arguments.push_back("-r");	
	arguments.push_back(Strings::Quoted(GetOutputPath().GetFilename()));	
	
	// Object files to link.
	for (auto& sourceFile : files)
	{
		arguments.push_back(Strings::Quoted(sourceFile.OutputPath.ToString()));
	}
	
	// Link PCH.
	if (!pchObjectPath.IsEmpty() && !m_projectFile.Get_Build_PrecompiledHeader().IsEmpty())
	{
		arguments.push_back(Strings::Quoted(pchObjectPath.ToString()));
	}

	// Run the app and hope for the best!
	Platform::Process process;
	Platform::Path responseFilePath = outputFile.OutputPath.AppendFragment(".rsp", false);
	if (!OpenResponseFileProcess(process, responseFilePath, m_prepRplPath, m_prepRplPath.GetDirectory(), arguments, true))
	{
		return false;
	}
	
	std::string output = process.ReadToEnd();

	if (process.GetExitCode() != 0)
	{
		printf("%s", output.c_str());
		return false;
	}	

	return true;
}

bool Toolchain_NintendoWiiU::MakeRPL(std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile)
{	
	MB_UNUSED_PARAMETER(files);
	MB_UNUSED_PARAMETER(outputFile);

	Platform::Path elfPath = GetElfPath();

	std::vector<std::string> arguments;

	arguments.push_back(Strings::Quoted(elfPath.ToString()));

	arguments.push_back("-dbg_source_root");
	arguments.push_back(Strings::Quoted(m_sdkPath.AppendFragment("system", true).ToString()));
		
	if (m_projectFile.Get_Project_OutputType() != EOutputType::DynamicLib)
	{
		arguments.push_back("-f");
	}

	arguments.push_back("-nolib");
	
	if (m_projectFile.Get_Build_OptimizationLevel() == EOptimizationLevel::Debug)
	{
		arguments.push_back("-t BUILD_TYPE=DEBUG");	
	}
	else
	{
		arguments.push_back("-t BUILD_TYPE=NDEBUG");	
	}

	if (m_projectFile.Get_Project_OutputType() == EOutputType::DynamicLib)
	{
		arguments.push_back("-padall 10");
	}
	else
	{
		arguments.push_back("-s");
	}

	// Run the app and hope for the best!
	Platform::Process process;
	if (!process.Open(m_makeRplPath, m_makeRplPath.GetDirectory(), arguments, true))
	{
		return false;
	}
	
	std::string output = process.ReadToEnd();

	if (process.GetExitCode() != 0)
	{
		printf("%s", output.c_str());
		return false;
	}	

	return true;
}
	
bool Toolchain_NintendoWiiU::Link(std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile) 
{
	// Welcome to the world of superfluous build steps ...

	// Step one - If we are building a dynamic library (rpl), we need to generate an export
	// object file from our def file.
	if (m_projectFile.Get_Project_OutputType() == EOutputType::DynamicLib)
	{
		if (!PrepRPL(files, outputFile))
		{
			return false;
		}
	}
	
	// Step two - Call linker to generate elf.	
	std::vector<std::string> arguments;
	GetLinkArguments(files, arguments);

	Platform::Process process;
	Platform::Path responseFilePath = outputFile.ManifestPath.AppendFragment(".rsp", false);
	if (!OpenResponseFileProcess(process, responseFilePath, m_linkerPath, m_linkerPath.GetDirectory(), arguments, true))
	{
		return false;
	}
	
	std::string output = process.ReadToEnd();
	printf("%s", output.c_str());

	if (process.GetExitCode() != 0)
	{
		return false;
	}
	
	// Step three - Call MakeRPL to generate our .rpx/.rpl output file (depending if we are making exe or dynamic lib).	
	if (!MakeRPL(files, outputFile))
	{
		return false;
	}

	// Finally update dependencies
	UpdateLinkDependencies(files, outputFile);
	outputFile.StoreManifest();

	return true;
}


}; // namespace MicroBuild
