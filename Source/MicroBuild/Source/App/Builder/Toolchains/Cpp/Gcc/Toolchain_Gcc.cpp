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
#include "App/Builder/Toolchains/Cpp/Gcc/Toolchain_Gcc.h"
#include "Core/Platform/Process.h"

namespace MicroBuild {
	
Toolchain_Gcc::Toolchain_Gcc(ProjectFile& file, uint64_t configurationHash)
	: Toolchain(file, configurationHash)
{
}

bool Toolchain_Gcc::Init() 
{
	m_bAvailable = FindToolchain();
	m_bRequiresCompileStep = true;
	m_description = Strings::Format("GCC (%s)", m_version.c_str());	
	return m_bAvailable;
}

bool Toolchain_Gcc::FindToolchain()
{
	std::vector<Platform::Path> additionalDirs;

	std::string compilerName = "g++";
	std::string archiverName = "ar";

#if defined(MB_PLATFORM_WINDOWS)
	// Just because it's not on the path var by default.
	additionalDirs.push_back("C:/MinGW/bin");

	if (m_projectFile.Get_Target_Platform() == EPlatform::x64)
	{
		compilerName = "x86_64-w64-mingw32-g++";
		archiverName = "x86_64-w64-mingw32-gcc-ar";
	}
	else
	{
		compilerName = "mingw32-g++";
		archiverName = "mingw32-gcc-ar";
	}
#else
	additionalDirs.push_back("/usr/bin");
#endif
	
	if (!Platform::Path::FindFile(compilerName, m_compilerPath, additionalDirs))
	{
		return false;
	}

	if (!Platform::Path::FindFile(archiverName, m_archiverPath, additionalDirs))
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
		size_t lastSpace = versionLine.find_last_of(' ');
		if (lastSpace != std::string::npos)
		{
			m_version = versionLine.substr(lastSpace + 1);
		}
	}	

	return true;
}

void Toolchain_Gcc::GetBaseCompileArguments(std::vector<std::string>& args)
{
	switch (m_projectFile.Get_Build_OptimizationLevel())
	{
	default:
		// Fallthrough
	case EOptimizationLevel::None:
		// Fallthrough
	case EOptimizationLevel::Debug:	
		{
			args.push_back("-Og");
			break;
		}
	case EOptimizationLevel::PreferSize:
		{
			args.push_back("-Os");
			break;
		}
	case EOptimizationLevel::PreferSpeed:
		{
			args.push_back("-Ofast");
			break;
		}
	case EOptimizationLevel::Full:
		{
			args.push_back("-O3");
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
	case EWarningLevel::Medium:
			// All of these just use the default warning level in gcc.
		break;
	case EWarningLevel::High:
		{
			args.push_back("-Wall");
			break;
		}
	case EWarningLevel::Verbose:
		{
			args.push_back("-Wextra");
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
		args.push_back(Strings::Format("-include I%s", Strings::Quoted(forcedInclude.ToString()).c_str()));
	}
	
	if (m_projectFile.Get_Flags_CompilerWarningsFatal())
	{
		args.push_back("-Werror");	
	}

	if (m_projectFile.Get_Flags_RuntimeTypeInfo())
	{
		args.push_back("-frtti");	
	}
	else
	{	
		args.push_back("-fno-rtti");	
	}

	if (m_projectFile.Get_Flags_GenerateDebugInformation())
	{
		args.push_back("-g");	
	}
	
	if (m_projectFile.Get_Flags_LinkTimeOptimization())
	{
		args.push_back("-flto");	
	}
	else
	{
		args.push_back("-fno-lto");	
	}

	if (m_projectFile.Get_Flags_Exceptions())
	{
		args.push_back("-fexceptions");	
	}
	else
	{
		args.push_back("-fno-exceptions");
	}

	if (m_projectFile.Get_Flags_StaticRuntime())
	{
		args.push_back("-static");
	}
	
	switch (m_projectFile.Get_Target_Platform())
	{
	case EPlatform::ARM64:
		// Fallthrough
	case EPlatform::x64:
		{
			args.push_back("-m64");
			break;
		}
	default:
		{
			args.push_back("-m32");
			break;
		}
	}

	switch (m_projectFile.Get_Project_LanguageVersion())
	{
	case ELanguageVersion::Default:
		{
			break;
		}
	case ELanguageVersion::Cpp_11:
		{
			args.push_back("-std=c++0x");
			break;
		}
	case ELanguageVersion::Cpp_98:
		{
			args.push_back("-std=c++98");
			break;
		}
	case ELanguageVersion::Cpp_14:
		{
			args.push_back("-std=c++14");
			break;
		}
	}	

	if (m_projectFile.Get_Project_OutputType() == EOutputType::DynamicLib)
	{
		args.push_back("-fPIC");
	}
	
	// Dumps out dependencies to a file.
	args.push_back("-MD");
	args.push_back("-MP");
}

void Toolchain_Gcc::GetPchCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args) 
{
	MB_UNUSED_PARAMETER(file);

	Platform::Path pchPath = m_projectFile.Get_Project_IntermediateDirectory()
		.AppendFragment(m_projectFile.Get_Build_PrecompiledHeader().ChangeExtension("pch").GetFilename(), true);
	
	args.push_back("-x");
	args.push_back("c++-header");

	args.push_back("-o");
	args.push_back(Strings::Quoted(pchPath.ToString()));
	
	args.push_back("-c");
	args.push_back(Strings::Quoted(m_projectFile.Get_Build_PrecompiledSource().ToString()));
}

void Toolchain_Gcc::GetSourceCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args) 
{
	// Include our generated pch before anything else.
	if (!m_projectFile.Get_Build_PrecompiledHeader().IsEmpty())
	{
		Platform::Path pchPath = m_projectFile.Get_Project_IntermediateDirectory()
			.AppendFragment(m_projectFile.Get_Build_PrecompiledHeader().ChangeExtension("pch").GetFilename(), true);
	
		args.push_back(Strings::Format("-include %s", Strings::Quoted(pchPath.ToString()).c_str()));
	}
	
	args.push_back("-o");
	args.push_back(Strings::Quoted(file.OutputPath.ToString()));
	
	args.push_back("-c");
	args.push_back(Strings::Quoted(file.SourcePath.ToString()));
}

void Toolchain_Gcc::GetLinkArguments(const std::vector<BuilderFileInfo>& sourceFiles, std::vector<std::string>& args) 
{
	Platform::Path outputPath = m_projectFile.Get_Project_OutputDirectory()
		.AppendFragment(Strings::Format("%s%s", m_projectFile.Get_Project_OutputName().c_str(), m_projectFile.Get_Project_OutputExtension().c_str()), true);
	
	args.push_back("-o");
	args.push_back(Strings::Quoted(outputPath.ToString()));
		
	// Object files to link.
	for (auto& sourceFile : sourceFiles)
	{
		args.push_back(Strings::Quoted(sourceFile.OutputPath.ToString()));
	}

	// Flags.
	switch (m_projectFile.Get_Build_OptimizationLevel())
	{
	default:
		// Fallthrough
	case EOptimizationLevel::None:
		// Fallthrough
	case EOptimizationLevel::Debug:	
		{
			args.push_back("-Og");
			break;
		}
	case EOptimizationLevel::PreferSize:
		{
			args.push_back("-Os");
			break;
		}
	case EOptimizationLevel::PreferSpeed:
		{
			args.push_back("-Ofast");
			break;
		}
	case EOptimizationLevel::Full:
		{
			args.push_back("-O3");
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
	case EWarningLevel::Medium:
			// All of these just use the default warning level in gcc.
		break;
	case EWarningLevel::High:
		{
			args.push_back("-Wall");
			break;
		}
	case EWarningLevel::Verbose:
		{
			args.push_back("-Wextra");
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
	
	if (m_projectFile.Get_Flags_LinkerWarningsFatal())
	{
		args.push_back("-Werror");	
	}

	if (m_projectFile.Get_Flags_RuntimeTypeInfo())
	{
		args.push_back("-frtti");	
	}
	else
	{	
		args.push_back("-fno-rtti");	
	}

	if (m_projectFile.Get_Flags_GenerateDebugInformation())
	{
		args.push_back("-g");	
	}
	
	if (m_projectFile.Get_Flags_LinkTimeOptimization())
	{
		args.push_back("-flto");	
	}
	else
	{
		args.push_back("-fno-lto");	
	}

	if (m_projectFile.Get_Flags_Exceptions())
	{
		args.push_back("-fexceptions");	
	}
	else
	{
		args.push_back("-fno-exceptions");
	}

	if (m_projectFile.Get_Flags_StaticRuntime())
	{
		args.push_back("-static");
	}
	
	switch (m_projectFile.Get_Target_Platform())
	{
	case EPlatform::ARM64:
		// Fallthrough
	case EPlatform::x64:
		{
			args.push_back("-m64");
			args.push_back("-L/usr/lib64");
			break;
		}
	default:
		{
			args.push_back("-m32");
			args.push_back("-L/usr/lib32");
			break;
		}
	}

	if (m_projectFile.Get_Project_OutputType() == EOutputType::DynamicLib)
	{
		args.push_back("-fPIC");
		args.push_back("-shared");
	}
	
	// Libraries.
	for (auto& library : m_projectFile.Get_Libraries_Library())
	{
		args.push_back(Strings::Quoted(library.ToString()));
	}
}

void Toolchain_Gcc::GetArchiveArguments(const std::vector<BuilderFileInfo>& sourceFiles, std::vector<std::string>& args) 
{
	Platform::Path outputPath = m_projectFile.Get_Project_OutputDirectory()
		.AppendFragment(Strings::Format("%s%s", m_projectFile.Get_Project_OutputName().c_str(), m_projectFile.Get_Project_OutputExtension().c_str()), true);
	
	Platform::Path pchPath = m_projectFile.Get_Project_IntermediateDirectory()
		.AppendFragment(m_projectFile.Get_Build_PrecompiledHeader().ChangeExtension("pch").GetFilename(), true);
	
	Platform::Path pchObjectPath = m_projectFile.Get_Project_IntermediateDirectory()
		.AppendFragment(m_projectFile.Get_Build_PrecompiledHeader().ChangeExtension("o").GetFilename(), true);

	args.push_back("rcs");	
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

void Toolchain_Gcc::ExtractDependencies(const BuilderFileInfo& file, const std::string& input, std::string& rawInput, std::vector<Platform::Path>& dependencies)
{
	MB_UNUSED_PARAMETER(input);
	MB_UNUSED_PARAMETER(rawInput);
	
	// We read the .d dependency file we forced gcc to dump out earlier during the compile
	// to extract dependencies.

	rawInput = input;

	Platform::Path depsFile = file.OutputPath.ChangeExtension("d");
	if (depsFile.Exists())
	{
		std::string rawDeps;
		if (!Strings::ReadFile(depsFile, rawDeps))
		{
			return;
		}

		std::vector<std::string> lines = Strings::Split('\n', rawDeps);
		for (std::string& line : lines)
		{
			if (line.size() > 2)
			{
				if (line.substr(line.size() - 2, 2) == " \\")
				{
					line = line.substr(0, line.size() - 2);
				}

				if (line[line.size() - 1] != ':')
				{
					dependencies.push_back(Strings::Replace(Strings::Trim(line), "\\ ", " "));
				}
			}
		}
	}
}

}; // namespace MicroBuild