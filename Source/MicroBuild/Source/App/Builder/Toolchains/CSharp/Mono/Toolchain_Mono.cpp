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
#include "App/Builder/Toolchains/CSharp/Mono/Toolchain_Mono.h"

#include "Core/Platform/Platform.h"
#include "Core/Platform/Registry.h"
#include "Core/Platform/Process.h"

namespace MicroBuild {
	
Toolchain_Mono::Toolchain_Mono(ProjectFile& file, uint64_t configurationHash)
	: Toolchain(file, configurationHash)
{
	m_bAvailable = FindToolchain();
	m_bRequiresCompileStep = false;
	m_description = Strings::Format("Mono C# Compiler (%s)", m_version.c_str());
}

bool Toolchain_Mono::Init() 
{
	m_bAvailable = FindToolchain();
	m_bRequiresCompileStep = false;
	m_description = Strings::Format("Mono C# Compiler (%s)", m_version.c_str());
	return m_bAvailable;
}

bool Toolchain_Mono::FindToolchain()
{
	m_version = Strings::Trim(m_version);
	
	std::vector<Platform::Path> additionalDirs;

#if defined(MB_PLATFORM_WINDOWS)
#if defined(MB_ARCHITECTURE_X64)
	additionalDirs.push_back("C:/Program Files (x86)/Mono/bin");
#else
	additionalDirs.push_back("C:/Program Files/Mono");
#endif
#else
	additionalDirs.push_back("/usr/bin");
#endif

	
#if defined(MB_PLATFORM_WINDOWS)
	if (!Platform::Path::FindFile("mcs.bat", m_compilerPath, additionalDirs))
#else
	if (!Platform::Path::FindFile("mcs", m_compilerPath, additionalDirs))
#endif
	{
		return false;
	}

	m_archiverPath = m_compilerPath;
	m_linkerPath = m_compilerPath;

	if (!m_compilerPath.Exists() ||
		!m_archiverPath.Exists() ||
		!m_linkerPath.Exists())
	{
		return false;
	}

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
		size_t lastSpace = versionLine.find_last_of(' ');
		if (lastSpace != std::string::npos)
		{
			m_version = versionLine.substr(lastSpace + 1);
		}
	}	
	
	return true;
}

void Toolchain_Mono::GetLinkArguments(const std::vector<BuilderFileInfo>& files, std::vector<std::string>& args)
{
	Platform::Path outputPath = m_projectFile.Get_Project_OutputDirectory()
		.AppendFragment(Strings::Format("%s%s", m_projectFile.Get_Project_OutputName().c_str(), m_projectFile.Get_Project_OutputExtension().c_str()), true);
	
	Platform::Path outputPdb = m_projectFile.Get_Project_IntermediateDirectory()
		.AppendFragment(Strings::Format("%s%s", m_projectFile.Get_Project_OutputName().c_str(), ".pdb"), true);
	
	args.push_back(Strings::Format("/out:%s", Strings::Quoted(outputPath.ToString()).c_str()));

	switch (m_projectFile.Get_Build_OptimizationLevel())
	{
	default:
		// Fallthrough
	case EOptimizationLevel::None:
		// Fallthrough
	case EOptimizationLevel::Debug:	
		{
			args.push_back("/optimize-");
			break;
		}
	case EOptimizationLevel::Full:
		// Fallthrough
	case EOptimizationLevel::PreferSize:
		// Fallthrough
	case EOptimizationLevel::PreferSpeed:
		{
			args.push_back("/optimize+");
			break;
		}
	}

	switch (m_projectFile.Get_Build_WarningLevel())
	{
	default:
		// Fallthrough
	case EWarningLevel::Default:
		// Fallthrough
	case EWarningLevel::Verbose:
		{
			args.push_back("/warn:4");
			break;
		}
	case EWarningLevel::High:
		{
			args.push_back("/warn:3");
			break;
		}
	case EWarningLevel::None:
		{
			args.push_back("/warn:0");
			break;
		}
	case EWarningLevel::Low:
		{
			args.push_back("/warn:1");
			break;
		}
	case EWarningLevel::Medium:
		{
			args.push_back("/warn:2");
			break;
		}
	}
	
	if (m_projectFile.Get_Flags_CompilerWarningsFatal())
	{
		args.push_back("/warnaserror");	
	}
	
	for (auto& warning : m_projectFile.Get_DisabledWarnings_DisabledWarning())
	{
		args.push_back(Strings::Format("/nowarn:%s", warning.c_str()));
	}

	// Disable some useless messages.

	// This always occurs when building for specific platforms and is supressed by default.
	args.push_back("/nowarn:1607"); // Referenced assembly '' targets a different processor
									
	// Add compiler and linker args, seeing as csc does both.
	std::vector<std::string> customArgs = Strings::Crack(m_projectFile.Get_Build_CompilerArguments());
	args.insert(args.end(), customArgs.begin(), customArgs.end());
	
	customArgs = Strings::Crack(m_projectFile.Get_Build_LinkerArguments());
	args.insert(args.end(), customArgs.begin(), customArgs.end());

	for (auto& define : m_projectFile.Get_Defines_Define())
	{
		args.push_back(Strings::Format("/define:%s", Strings::Quoted(define).c_str()));
	}	
	
	for (auto& includeDir : m_projectFile.Get_SearchPaths_LibraryDirectory())
	{
		args.push_back(Strings::Format("/lib:%s", Strings::Quoted(includeDir.ToString()).c_str()));
	}

	for (auto file : m_projectFile.Get_References_Reference())
	{
		if (file.IsAbsolute())
		{
			args.push_back(Strings::Format("/lib:%s", Strings::Quoted(file.GetDirectory().ToString()).c_str()));
		}
	}
	
	// Libraries to link.
	for (auto& library : m_projectFile.Get_Libraries_Library())
	{
		Platform::Path fullPath = FindLibraryPath(library);
		args.push_back(Strings::Format("/reference:%s", Strings::Quoted(fullPath.GetFilename()).c_str()));	
	}

	// Disable startup blurb.
	args.push_back("/nologo");
	
	// General flags.	
	if (m_projectFile.Get_Flags_GenerateDebugInformation())
	{
		args.push_back("/debug+");	
	}
	else
	{
		args.push_back("/debug-");	
	}
	
	if (m_projectFile.Get_Flags_AllowUnsafeCode())
	{
		args.push_back("/unsafe+");
	}
	else
	{
		args.push_back("/unsafe-");
	}

	// Platform	
	switch (m_projectFile.Get_Target_Platform())
	{
	case EPlatform::AnyCPU:
		{		
			if (m_projectFile.Get_Flags_Prefer32Bit())
			{
				args.push_back("/platform:anycpu");
			}
			else
			{
				args.push_back("/platform:anycpu32bitpreferred");
			}
			break;
		}
	case EPlatform::x86:
		{
			args.push_back("/platform:x86");
			break;
		}
	case EPlatform::x64:
		{
			args.push_back("/platform:x64");
			break;
		}
	case EPlatform::ARM64:
	case EPlatform::ARM:
		{
			args.push_back("/platform:arm");
			break;
		}
	default:
		{
			assert(false);
			break;
		}
	}
	
	switch (m_projectFile.Get_Project_OutputType())
	{
	case EOutputType::ConsoleApp:
		{
			args.push_back("/target:exe");
			break;
		}
	case EOutputType::DynamicLib:
		{
			args.push_back("/target:library");
			break;
		}
	case EOutputType::Executable:
		{
			args.push_back("/target:winexe");
			break;
		}
	default:
		{
			assert(false);
			break;
		}
	}
	
	switch (m_projectFile.Get_Project_LanguageVersion())
	{
	case ELanguageVersion::CSharp_1_0:
		{
			args.push_back("/langversion:ISO-1");
			break;
		}
	case ELanguageVersion::CSharp_2_0:
		{
			args.push_back("/langversion:ISO-2");
			break;
		}
	case ELanguageVersion::CSharp_3_0:
		{
			args.push_back("/langversion:3");
			break;
		}
	case ELanguageVersion::CSharp_4_0:
		{
			args.push_back("/langversion:4");
			break;
		}
	case ELanguageVersion::CSharp_5_0:
		{
			args.push_back("/langversion:5");
			break;
		}
	case ELanguageVersion::CSharp_6_0:
		{
			args.push_back("/langversion:6");
			break;
		}
	default:
		{
			assert(false);
			break;
		}
	}

	// And finally the actual files we are compiling.
	for (auto& file : files)
	{
		args.push_back(file.SourcePath.ToString());
	}
}

void Toolchain_Mono::GetArchiveAction(BuildAction& action, std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile)
{
	Toolchain::GetArchiveAction(action, files, outputFile);

	std::function<bool(BuildAction& Action)> PreviousDelegate = action.PostProcessDelegate;

	action.PostProcessDelegate = [this, PreviousDelegate](BuildAction& action) -> bool
	{
		if (PreviousDelegate != nullptr)
		{
			if (!PreviousDelegate(action))
			{
				return false;
			}
		}

		// Copy PDB to output.	
		Platform::Path intPdb = m_projectFile.Get_Project_IntermediateDirectory()
			.AppendFragment(Strings::Format("%s%s%s", m_projectFile.Get_Project_OutputName().c_str(), m_projectFile.Get_Project_OutputExtension().c_str(), ".mdb"), true);
		Platform::Path outPdb = m_projectFile.Get_Project_OutputDirectory()
			.AppendFragment(Strings::Format("%s%s%s", m_projectFile.Get_Project_OutputName().c_str(), m_projectFile.Get_Project_OutputExtension().c_str(), ".mdb"), true);

		if (!intPdb.Copy(outPdb))
		{
			Log(LogSeverity::Fatal, "Failed to copy pdb file to '%s'.", outPdb.ToString().c_str());
			return false;
		}

		return true;
	};
}

void Toolchain_Mono::GetLinkAction(BuildAction& action, std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile)
{
	Toolchain::GetLinkAction(action, files, outputFile);

	std::function<bool(BuildAction& Action)> PreviousDelegate = action.PostProcessDelegate;

	action.PostProcessDelegate = [this, PreviousDelegate](BuildAction& action) -> bool
	{
		if (PreviousDelegate != nullptr)
		{
			if (!PreviousDelegate(action))
			{
				return false;
			}
		}

		// Copy PDB to output.	
		Platform::Path intPdb = m_projectFile.Get_Project_IntermediateDirectory()
			.AppendFragment(Strings::Format("%s%s%s", m_projectFile.Get_Project_OutputName().c_str(), m_projectFile.Get_Project_OutputExtension().c_str(), ".mdb"), true);
		Platform::Path outPdb = m_projectFile.Get_Project_OutputDirectory()
			.AppendFragment(Strings::Format("%s%s%s", m_projectFile.Get_Project_OutputName().c_str(), m_projectFile.Get_Project_OutputExtension().c_str(), ".mdb"), true);

		if (!intPdb.Copy(outPdb))
		{
			Log(LogSeverity::Fatal, "Failed to copy pdb file to '%s'.", outPdb.ToString().c_str());
			return false;
		}

		return true;
	};
}

}; // namespace MicroBuild
