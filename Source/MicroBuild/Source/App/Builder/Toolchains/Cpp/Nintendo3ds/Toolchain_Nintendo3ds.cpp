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
#include "App/Builder/Toolchains/Cpp/Nintendo3ds/Toolchain_Nintendo3ds.h"
#include "App/Builder/Toolchains/Cpp/Gcc/Toolchain_Gcc.h"
#include "Core/Platform/Process.h"
#include "Core/Platform/Platform.h"
#include "Core/Helpers/Strings.h"

// TODO: on packaging we need to generate a rom file, and associated banner file.

namespace MicroBuild {

Toolchain_Nintendo3ds::Toolchain_Nintendo3ds(ProjectFile& file, uint64_t configurationHash)
	: Toolchain(file, configurationHash)
{
}

bool Toolchain_Nintendo3ds::Init() 
{
	m_bAvailable = FindToolchain();
	m_bRequiresCompileStep = true;
	m_description = Strings::Format("Nintendo 3DS (SDK %s)", m_version.c_str());	
	return m_bAvailable;
}

bool Toolchain_Nintendo3ds::FindToolchain()
{
	// Nintendo's SDK's are just gross, stuff thrown all over the place, sigh.

	m_sdkPath		= Platform::GetEnvironmentVariable("CTRSDK_ROOT");
	m_toolchainPath = Platform::GetEnvironmentVariable("ARMCC5BIN");
	
	if (m_sdkPath.IsEmpty())
	{
		Log(LogSeverity::Fatal, "CTRSDK_ROOT environment variable not defined, unable to locate toolchain.");
		return false;
	}
	if (!m_sdkPath.Exists())
	{
		Log(LogSeverity::Fatal, "CTRSDK_ROOT environment variable pointing to non-existing folder.");
		return false;
	}
	if (m_toolchainPath.IsEmpty())
	{
		Log(LogSeverity::Fatal, "ARMCC5BIN environment variable not defined, unable to locate toolchain.");
		return false;
	}
	if (!m_toolchainPath.Exists())
	{
		Log(LogSeverity::Fatal, "ARMCC5BIN environment variable pointing to non-existing folder.");
		return false;
	}

	m_compilerPath		= m_toolchainPath.AppendFragment("armcc.exe", true);
	m_linkerPath		= m_toolchainPath.AppendFragment("armlink.exe", true);
	m_archiverPath		= m_toolchainPath.AppendFragment("armar.exe", true);
	m_makeRomPath		= m_sdkPath.AppendFragment("tools/CommandLineTools/ctr_makerom32.exe", true);
	m_makeBannerPath	= m_sdkPath.AppendFragment("tools/CommandLineTools/ctr_makebanner32.exe", true);

	if (!m_compilerPath.Exists() ||
		!m_linkerPath.Exists()	 ||
		!m_archiverPath.Exists() ||
		!m_makeRomPath.Exists()	 ||
		!m_makeBannerPath.Exists())
	{
		return false;
	}
	
	// This is just ridiculous, there doesn't appear to be a nice way to get the SDK version
	// so we're just going to extract it from one of the files we know is always there.
	std::string versionFileData;
	if (!Strings::ReadFile(m_sdkPath.AppendFragment("include/nn/version.h", true), versionFileData))
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
			std::vector<std::string> split = Strings::Split(' ', line);
			if (split.size() >= 3)
			{
				std::string name = Strings::Trim(split[1]);
				std::string value = Strings::Trim(split[2]);

				if (name == "NN_VERSION_MAJOR")
				{
					nnVersionMajor = value;
				}
				else if (name == "NN_VERSION_MINOR")
				{
					nnVersionMinor = value;
				}
				else if (name == "NN_VERSION_MICRO")
				{
					nnVersionMicro = value;
				}
				else if (name == "NN_VERSION_RELSTEP")
				{
					nnVersionRelStep = value;
				}
				else if (name == "NN_VERSION_PATCH")
				{
					nnVersionPatch = Strings::Replace(value, "\"", "");
				}
			}
		}
	}

	m_version = Strings::Format("CTR_SDK-%s_%s_%s_%s_%s",
		nnVersionMajor.c_str(),
		nnVersionMinor.c_str(),
		nnVersionMicro.c_str(),
		nnVersionRelStep.c_str(),
		nnVersionPatch.c_str()
	);

	return true;
}

void Toolchain_Nintendo3ds::GetBaseCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args)
{
	MB_UNUSED_PARAMETER(file);
	MB_UNUSED_PARAMETER(args);

	args.push_back("--no_wrap_diagnostics");

	// CPU definitions for 3ds.
	args.push_back("--apcs=/interwork");
	args.push_back("--cpu=MPCore");

	// Must be defined, none-optional.
	args.push_back("-DNN_COMPILER_OPTION_ARM");
	args.push_back("-DNN_COMPILER_RVCT");
	args.push_back("-DNN_DEBUFFER_KMC_PARTNER");
	args.push_back("-DNN_HARDWARE_CTR");
	args.push_back("-DNN_HARDWARE_CTR_TS");
	args.push_back("-DNN_PLATFORM_CTR");
	args.push_back("-DNN_PROCESSOR_ARM");
	args.push_back("-DNN_PROCESSOR_ARM11MPCORE");
	args.push_back("-DNN_PROCESSOR_ARM_V6");
	args.push_back("-DNN_PROCESSOR_ARM_VFP_V2");
	args.push_back("-DNN_SYSTEM_PROCESS=1");

	// Compiler to use, we hardcode this to 5 for the time being.
	args.push_back("-DNN_COMPILER_RVCT_VERSION_MAJOR=5");

	// Disable various SDK verbose logging.
	args.push_back("-DNN_SWITCH_DISABLE_ASSERT_WARNING=1");
	args.push_back("-DNN_SWITCH_DISABLE_ASSERT_WARNING_FOR_SDK=1");
	args.push_back("-DNN_SWITCH_DISABLE_DEBUG_PRINT=1");
	args.push_back("-DNN_SWITCH_DISABLE_DEBUG_PRINT_FOR_SDK=1");

	// SDK include directory.
	args.push_back("-I" + Strings::Quoted(m_sdkPath.AppendFragment("include", true).ToString()));
	
	// Options
	args.push_back("--fpmode=fast");
	args.push_back("--signed_chars");
	args.push_back("--cpp");
	args.push_back("--force_new_nothrow");
	args.push_back("--dwarf3");

	switch (m_projectFile.Get_Build_OptimizationLevel())
	{
	default:
		// Fallthrough
	case EOptimizationLevel::None:
		// Fallthrough
	case EOptimizationLevel::Debug:	
		{
			args.push_back("-O0");
			args.push_back("-DNN_EFFORT_SMALL");
			break;
		}
	case EOptimizationLevel::PreferSize:
		{
			args.push_back("-Ospace");
			args.push_back("-DNN_EFFORT_FAST");
			break;
		}
	case EOptimizationLevel::PreferSpeed:
		{
			args.push_back("-Otime");
			args.push_back("-DNN_EFFORT_FAST");
			break;
		}
	case EOptimizationLevel::Full:
		{
			args.push_back("-O3");
			args.push_back("-DNN_EFFORT_FAST");
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
		args.push_back(Strings::Format("-preinclude=%s", Strings::Quoted(forcedInclude.ToString()).c_str()));
	}

	if (m_projectFile.Get_Flags_CompilerWarningsFatal())
	{
		args.push_back("-Werror");	
	}

	if (m_projectFile.Get_Flags_RuntimeTypeInfo())
	{	
		args.push_back("--rtti");
	}
	else
	{	
		args.push_back("--no_rtti");
	}

	if (m_projectFile.Get_Flags_GenerateDebugInformation())
	{ 
		args.push_back("--debug");	
	}
	
	if (m_projectFile.Get_Flags_Exceptions())
	{
		args.push_back("--exceptions");	
	}
	else
	{
		args.push_back("--no_exceptions");
	}
	
	/*
	// Prohibited from use by Nintendo for some reason.
	if (m_projectFile.Get_Flags_LinkTimeOptimization())
	{
		args.push_back("--ltcg");	
	}
	else
	{
		args.push_back("--no-ltcg");	
	}
	*/

	// Dumps out dependencies to a file.
	args.push_back("--md");
}

void Toolchain_Nintendo3ds::GetPchCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args) 
{
	// ArmCC is to stupid to create the dependency file itself, so lets make it now ..			
	Platform::Path depsFile = file.OutputPath.ChangeExtension("d");
	depsFile.CreateAsFile();
	args.push_back(Strings::Format("--depend=%s", Strings::Quoted(depsFile.ToString()).c_str()));

	Platform::Path pchPath = GetPchPath();
	
	args.push_back("--create_pch=" + Strings::Quoted(pchPath.ToString()));
	args.push_back(Strings::Quoted(m_projectFile.Get_Build_PrecompiledSource().ToString()));
}

void Toolchain_Nintendo3ds::GetSourceCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args) 
{
	// ArmCC is to stupid to create the dependency file itself, so lets make it now ..	
	Platform::Path depsFile = file.OutputPath.ChangeExtension("d");
	depsFile.CreateAsFile();
	args.push_back(Strings::Format("--depend=%s", Strings::Quoted(depsFile.ToString()).c_str()));

	// Include our generated pch before anything else.
	if (!m_projectFile.Get_Build_PrecompiledHeader().IsEmpty())
	{
		Platform::Path pchPath = GetPchPath();
		args.push_back(Strings::Format("-preinclude=%s", Strings::Quoted(pchPath.ToString()).c_str()));
	}

	args.push_back("-c");

	args.push_back("-o");
	args.push_back(Strings::Quoted(file.OutputPath.ToString()));

	args.push_back(Strings::Quoted(file.SourcePath.ToString()));
}

void Toolchain_Nintendo3ds::GetLinkArguments(const std::vector<BuilderFileInfo>& sourceFiles, std::vector<std::string>& args) 
{
	MB_UNUSED_PARAMETER(sourceFiles);
	MB_UNUSED_PARAMETER(args);

	Platform::Path outputPath = GetOutputPath();

	args.push_back("--no_wrap_diagnostics");

	// Required arguments
	args.push_back("--datacompressor=off");
	args.push_back("--keep=nnMain");
	args.push_back("--scatter=" + Strings::Quoted(m_sdkPath.AppendFragment("resources/specfiles/linker/CTR.Process.MPCore.ldscript", true).ToString()));

	args.push_back("--be8");
	args.push_back("--cpu=MPCore");
	args.push_back("--entry=__ctr_start");
	args.push_back("--legacyalign");
	args.push_back("--library_type=standardlib");
	args.push_back("--ref_cpp_init");
	args.push_back("--scanlib");
	args.push_back("--startup=__ctr_start");

	args.push_back("--largeregions");
	args.push_back("--no_eager_load_debug");
	args.push_back("--strict");
	
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

	std::vector<std::string> customArgs = Strings::Crack(m_projectFile.Get_Build_LinkerArguments());	args.insert(args.end(), customArgs.begin(), customArgs.end());

	for (auto& warning : m_projectFile.Get_DisabledWarnings_DisabledWarning())
	{
		args.push_back(Strings::Format("-Wno-%s", warning.c_str()));
	}

	for (auto& includeDir : m_projectFile.Get_SearchPaths_LibraryDirectory())
	{
		args.push_back(Strings::Format("-I%s", Strings::Quoted(includeDir.ToString()).c_str()));
	}

	for (auto& includeDir : m_standardLibraryPaths)
	{
		args.push_back(Strings::Format("-I%s", Strings::Quoted(includeDir.ToString()).c_str()));
	}
	
	if (m_projectFile.Get_Flags_CompilerWarningsFatal())
	{
		args.push_back("-Werror");	
	}

	if (m_projectFile.Get_Flags_GenerateDebugInformation())
	{ 
		args.push_back("--debug");	
	}
	
	if (m_projectFile.Get_Flags_Exceptions())
	{
		args.push_back("--exceptions");	
	}
	else
	{
		args.push_back("--no_exceptions");
	}
	
	/*
	// Prohibited from use by Nintendo for some reason.
	if (m_projectFile.Get_Flags_LinkTimeOptimization())
	{
		args.push_back("--ltcg");	
	}
	else
	{
		args.push_back("--no-ltcg");	
	}
	*/

	args.push_back("-o");
	args.push_back(Strings::Quoted(outputPath.ToString()));
		
	// Object files to link.
	for (auto& sourceFile : sourceFiles)
	{
		args.push_back(Strings::Quoted(sourceFile.OutputPath.ToString()));
	}

	// Libraries.
	for (auto& library : m_projectFile.Get_Libraries_Library())
	{
		args.push_back(Strings::Quoted(library.ToString()));
	}
}

void Toolchain_Nintendo3ds::GetArchiveArguments(const std::vector<BuilderFileInfo>& sourceFiles, std::vector<std::string>& args) 
{	
	MB_UNUSED_PARAMETER(sourceFiles);
	MB_UNUSED_PARAMETER(args);

	Platform::Path outputPath = GetOutputPath();	
	Platform::Path pchPath = GetPchPath();
	Platform::Path pchObjectPath = GetPchObjectPath();

	args.push_back("--no_wrap_diagnostics");

	args.push_back("--create");	
	args.push_back(Strings::Quoted(outputPath.ToString()).c_str());	
	
	// Object files to link.
	for (auto& sourceFile : sourceFiles)
	{
		args.push_back(Strings::Quoted(sourceFile.OutputPath.ToString()));
	}
	
	// Link PCH.
	if (!m_projectFile.Get_Build_PrecompiledHeader().IsEmpty())
	{
		args.push_back(Strings::Quoted(pchObjectPath.ToString()));
	}
}

bool Toolchain_Nintendo3ds::ParseMessageOutput(BuilderFileInfo& file, std::string& input)
{
	MB_UNUSED_PARAMETER(file);

	// Attempts to extract messages in the following formats:
	// Rather ugly ...

	// "D:\Git\Ludo\Tools\MicroBuild\Tests\Projects\Cpp_Exe\Project\Source\File.cpp", line 34: Error:  #20: identifier "zyx" is undefined

	size_t startOffset = 0;
	while (startOffset < input.size())
	{
		size_t endOffset = input.find("\r\n", startOffset);
		if (endOffset == std::string::npos)
		{
			break;
		}

		std::string line = input.substr(startOffset, endOffset - startOffset);

		size_t colonIndex = line.find(':', 3 /* Skip drive colon */);
		if (colonIndex != std::string::npos)
		{
			std::string origin;
			std::string message;
			Strings::SplitOnIndex(line, colonIndex, origin, message);

			colonIndex = message.find(':');
			if (colonIndex != std::string::npos)
			{
				std::string errorType;
				Strings::SplitOnIndex(message, colonIndex, errorType, message);

				message = Strings::Trim(message);
				errorType = Strings::Trim(errorType);

				colonIndex = message.find(':');
				if (colonIndex != std::string::npos)
				{
					std::string errorIdentifier;
					Strings::SplitOnIndex(message, colonIndex, errorIdentifier, message);

					message = Strings::Trim(message);
					errorIdentifier = Strings::Trim(errorIdentifier);

					if (errorIdentifier[0] == '#')
					{
						BuilderFileMessage fileMessage;
						fileMessage.Identifier = errorIdentifier;
						fileMessage.Text = message;

						errorType = Strings::ToLowercase(errorType);
						if (errorType == "error" ||
							errorType == "fatal")
						{
							fileMessage.Type = EBuilderFileMessageType::Error;
						}
						else if (errorType == "warning")
						{
							fileMessage.Type = EBuilderFileMessageType::Warning;
						}
						else if (errorType == "info" ||
								 errorType == "message")
						{
							fileMessage.Type = EBuilderFileMessageType::Info;
						}

						// Try and extract line/colum information from origin.
						if (origin[0] == '"')
						{
							size_t closeQuoteOffset = origin.find_last_of('"');
							std::string lineText = Strings::Trim(origin.substr(closeQuoteOffset + 1));
							origin = Strings::StripQuotes(origin.substr(0, closeQuoteOffset + 1));
					
							if (lineText.substr(0, 6) == ", line")
							{
								fileMessage.Line = CastFromString<int>(lineText.substr(6));
								fileMessage.Column = 1;
							}
						}

						fileMessage.Origin = origin;
						file.AddMessage(fileMessage);
					}
				}
			}
		}

		startOffset = endOffset + 2;
	}

	return true;
}

bool Toolchain_Nintendo3ds::ParseOutput(BuilderFileInfo& file, std::string& input)
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

}; // namespace MicroBuild
