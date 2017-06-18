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
#include "App/Builder/Toolchains/Cpp/Gcc/Toolchain_GccOutputParser.h"
#include "Core/Platform/Process.h"

namespace MicroBuild {
	
Toolchain_Gcc::Toolchain_Gcc(ProjectFile& file, uint64_t configurationHash)
	: Toolchain(file, configurationHash)		
#if defined(MB_PLATFORM_WINDOWS)
	, m_microsoftToolchain(file, m_configurationHash, true)
#endif
{
	m_useStartEndGroup = true;
	m_bGeneratesPchObject = false;
}

bool Toolchain_Gcc::Init() 
{
	m_bAvailable = FindToolchain();
	m_bRequiresCompileStep = true;
	m_bRequiresVersionInfo = true;
	m_description = Strings::Format("GCC (%s)", m_version.c_str());	
	return m_bAvailable;
}

bool Toolchain_Gcc::FindToolchain()
{
	std::vector<Platform::Path> additionalDirs;
	
#if defined(MB_PLATFORM_WINDOWS)
	if (!InitMicrosoftToolchain())
	{
		return false;
	}
#endif

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

	if (!Platform::Path::FindFile("windres", m_windowsResourceCompilerPath, additionalDirs))
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
	
#if defined(MB_PLATFORM_WINDOWS)

bool Toolchain_Gcc::InitMicrosoftToolchain()
{
	if (!m_microsoftToolchain.Init() || 
		!m_microsoftToolchain.IsAvailable())
	{
		Log(LogSeverity::Fatal, "Failed to find msvc toolchain to use as backend for clang.");
		return false;
	}	

	return true;
}

#endif

void Toolchain_Gcc::GetBaseCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args)
{
	// Windows is always PIC, this is redundent.
#ifndef MB_PLATFORM_WINDOWS
	args.push_back("-fPIC");
#endif

	switch (m_projectFile.Get_Build_OptimizationLevel())
	{
	default:
		// Fallthrough
	case EOptimizationLevel::None:
		// Fallthrough
	case EOptimizationLevel::Debug:	
		{
			// -Og is the correct one to use here, but rather aggrevatingly
			// clang doesn't support it (Bug #20765), so for the time being we're
			// just going to use O1, which is trivially different.
			args.push_back("-O1");
			//args.push_back("-Og");
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
		args.push_back("-include");
		args.push_back(Strings::Quoted(forcedInclude.ToString()));
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
	
	// Older versions of clang error on -no-lto flags during compile, this is fixed in:
	// http://llvm.org/viewvc/llvm-project?view=revision&revision=225100
	// But for the time being, lets just supress the warning.
	args.push_back("-Qunused-arguments");
	
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
	case EPlatform::x86:
		{
			args.push_back("-m32");
			break;
		}
	default:
		{
			// Do we want to do anything here?
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
			if (file.SourcePath.IsCppFile())
			{
				args.push_back("-std=c++11");
			}
			break;
		}
	case ELanguageVersion::Cpp_98:
		{
			if (file.SourcePath.IsCppFile())
			{
				args.push_back("-std=c++98");
			}
			break;
		}
	case ELanguageVersion::Cpp_14:
		{
			if (file.SourcePath.IsCppFile())
			{
				args.push_back("-std=c++14");
			}
			break;
		}
	default:
		{
			break;
		}
	}	
	
	switch (m_projectFile.Get_Project_StandardLibrary())
	{
	case EStandardLibrary::Default:
		{
			break;
		}
	case EStandardLibrary::LibCpp:
		{
			args.push_back("-stdlib=libc++");
			break;
		}
	case EStandardLibrary::LibStdCpp:
		{
			args.push_back("-stdlib=libstdc++");
			break;
		}
	}
	
	// Dumps out dependencies to a file.
	args.push_back("-MD");
	args.push_back("-MP");
}

void Toolchain_Gcc::GetPchCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args) 
{
	MB_UNUSED_PARAMETER(file);

	Platform::Path pchPath = GetPchPath();
	
	args.push_back("-x");
	args.push_back("c++-header");

	args.push_back("-o");
	args.push_back(Strings::Quoted(pchPath.ToString()));
	
	args.push_back("-c");
	args.push_back(Strings::Quoted(m_projectFile.Get_Build_PrecompiledSource().ToString()));
}

void Toolchain_Gcc::GetSourceCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args) 
{
	// Windows is always PIC, this is redundent.
#ifndef MB_PLATFORM_WINDOWS
	args.push_back("-fPIC");
#endif

	// Include our generated pch before anything else.
	if (!m_projectFile.Get_Build_PrecompiledHeader().IsEmpty())
	{
		Platform::Path pchPath = GetPchPath();

		args.push_back(Strings::Format("-I%s", Strings::Quoted(pchPath.GetDirectory().ToString()).c_str()));

		//args.push_back("-include");
		//args.push_back(Strings::Quoted(pchPath.ToString()));
	}

	// Force the language based on the extension - not stricly required, but
	// silences to clang warnings.
	args.push_back("-x");
	if (file.SourcePath.IsCFile())
	{
		args.push_back("c");
	}
	else if (file.SourcePath.IsCppFile())
	{
		args.push_back("c++");
	}
	else if (file.SourcePath.IsObjCFile())
	{
		args.push_back("objective-c");
	}
	else if (file.SourcePath.IsObjCppFile())
	{
		args.push_back("objective-c++");
	}

	args.push_back("-o");
	args.push_back(Strings::Quoted(file.OutputPath.ToString()));
	
	args.push_back("-c");
	args.push_back(Strings::Quoted(file.SourcePath.ToString()));
}

void Toolchain_Gcc::GetLinkArguments(const std::vector<BuilderFileInfo>& sourceFiles, std::vector<std::string>& args) 
{
	Platform::Path outputPath = GetOutputPath();
	
	if (m_projectFile.Get_Project_OutputType() == EOutputType::DynamicLib)
	{
		args.push_back("-shared");
	}

	args.push_back("-o");
	args.push_back(Strings::Quoted(outputPath.ToString()));
		
	// Object files to link.
	for (auto& sourceFile : sourceFiles)
	{
		args.push_back(Strings::Quoted(sourceFile.OutputPath.ToString()));
	}
	
#if defined(MB_PLATFORM_WINDOWS)

	// Link Version Info
	if (RequiresVersionInfo())
	{
		args.push_back(Strings::Quoted(m_microsoftToolchain.GetVersionInfoObjectPath().ToString()));
	}

#endif

	// Flags.
	switch (m_projectFile.Get_Build_OptimizationLevel())
	{
	default:
		// Fallthrough
	case EOptimizationLevel::None:
		// Fallthrough
	case EOptimizationLevel::Debug:	
		{
			// -Og is the correct one to use here, but rather aggrevatingly
			// clang doesn't support it (Bug #20765), so for the time being we're
			// just going to use O1, which is trivially different.
			args.push_back("-O1");
			//args.push_back("-Og");
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
	
	// Older versions of clang error on -no-lto flags during compile, this is fixed in:
	// http://llvm.org/viewvc/llvm-project?view=revision&revision=225100
	// But for the time being, lets just supress the warning.
	args.push_back("-Qunused-arguments");
	
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
#if !defined(MB_PLATFORM_WINDOWS)
			if (Platform::Path("/usr/lib64").Exists())
			{
				args.push_back("-L/usr/lib64");
			}
#endif
			break;
		}
	case EPlatform::x86:
		{
			args.push_back("-m32");
#if !defined(MB_PLATFORM_WINDOWS)
			if (Platform::Path("/usr/lib32").Exists())
			{
				args.push_back("-L/usr/lib32");
			}
#endif
			break;
		}
	default:
		{
			// Do we want to do anything here?
			break;
		}
	}
	
	switch (m_projectFile.Get_Project_StandardLibrary())
	{
	case EStandardLibrary::Default:
		{
			break;
		}
	case EStandardLibrary::LibCpp:
		{
			args.push_back("-stdlib=libc++");
			break;
		}
	case EStandardLibrary::LibStdCpp:
		{
			args.push_back("-stdlib=libstdc++");
			break;
		}
	}

	if (m_useStartEndGroup)
	{
		args.push_back("-Wl,--start-group");
	}

	// Libraries.
	for (auto& library : m_projectFile.Get_Libraries_Library())
	{
		if (library.IsRelative())
		{
			args.push_back(Strings::Format("-l%s", Strings::Quoted(library.ToString()).c_str()));
		}
		else
		{
			args.push_back(Strings::Quoted(library.ToString()));	
		}
	}

	if (m_useStartEndGroup)
	{
		args.push_back("-Wl,--end-group");
	}

#if defined(MB_PLATFORM_LINUX)
	// Allow shared libraries to be loaded from the binary folder.
	args.push_back(" -Wl,-R,'$$ORIGIN'");
#endif

}

void Toolchain_Gcc::GetArchiveArguments(const std::vector<BuilderFileInfo>& sourceFiles, std::vector<std::string>& args) 
{
	Platform::Path outputPath = GetOutputPath();	
	Platform::Path pchPath = GetPchPath();	
	Platform::Path pchObjectPath = GetPchObjectPath();

	args.push_back("rcs");	
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

bool Toolchain_Gcc::ParseDependencyFile(BuilderFileInfo& file, std::string& input)
{	
	MB_UNUSED_PARAMETER(input);

	Platform::Path depsFile = file.OutputPath.ChangeExtension("d");
	if (depsFile.Exists())
	{
		std::string rawDeps;
		if (!Strings::ReadFile(depsFile, rawDeps))
		{
			return false;
		}

		rawDeps = Strings::Replace(rawDeps, "\\\n", " ");

		std::vector<std::string> lines = Strings::Split('\n', rawDeps);
		for (std::string& line : lines)
		{
			std::vector<std::string> lineSplit = Strings::Split(' ', line, true, true);

			for (auto& dep : lineSplit)
			{
				std::string depStripped = Strings::Trim(dep);
				if (depStripped[depStripped.size() - 1] == ':')
				{
					depStripped = depStripped.substr(0, depStripped.size() - 1);
				}
				file.OutputDependencyPaths.push_back(depStripped);
			}
		}
	}

	return true;
}

bool Toolchain_Gcc::ParseMessageOutput(BuilderFileInfo& file, std::string& input)
{
	std::vector<ToolchainOutputMessage> messages;

	Toolchain_GccOutputParser parser;
	parser.ExtractMessages(input, messages);

	for (ToolchainOutputMessage& message : messages)
	{
		// Supress the 
		// file: xxx has no symbols
		// warning, its pointless, but there is no easy way to supress it :(.
		if (message.Text.find("has no symbols") == std::string::npos)
		{
			file.AddMessage(message);
		}
	}

	return true;
}

bool Toolchain_Gcc::ParseOutput(BuilderFileInfo& file, std::string& input)
{
	if (!Toolchain::ParseOutput(file, input))
	{
		return false;
	}
	if (!ParseDependencyFile(file, input))
	{
		return false;
	}
	if (!ParseMessageOutput(file, input))
	{
		return false;
	}
	return true;
}

#if defined(MB_PLATFORM_WINDOWS)

void Toolchain_Gcc::GetCompileVersionInfoAction(BuildAction& action, BuilderFileInfo& fileInfo, VersionNumberInfo versionInfo)
{	
	 MB_UNUSED_PARAMETER(fileInfo);
	 MB_UNUSED_PARAMETER(versionInfo);
	 
	Platform::Path iconPath = fileInfo.OutputPath.ChangeExtension("ico");
	Platform::Path rcScriptPath = fileInfo.OutputPath.ChangeExtension("rc");

	if (!m_microsoftToolchain.CreateVersionInfoScript(iconPath, rcScriptPath, versionInfo))
	{
		return;
	}

	// Call mingw resource compiler to build the version info script into an object file.
	action.Arguments.push_back(rcScriptPath.ToString());
	action.Arguments.push_back("-O");
	action.Arguments.push_back("coff");
	action.Arguments.push_back("-o");
	action.Arguments.push_back(fileInfo.OutputPath.ToString());
	
	action.Tool = m_windowsResourceCompilerPath;
	action.WorkingDirectory = m_windowsResourceCompilerPath.GetDirectory();
	action.FileInfo = fileInfo;


	action.PostProcessDelegate = [this](BuildAction& action) -> bool
	{
		printf("%s", action.Output.c_str());
		if (action.ExitCode != 0)
		{
			return false;
		}

		std::vector<BuilderFileInfo*> inheritsFromFiles;

		std::vector<Platform::Path> dependencies;
		for (auto& path : m_projectFile.Get_ProductInfo_Icon())
		{
			dependencies.push_back(path);
		}

		UpdateDependencyManifest(action.FileInfo, dependencies, inheritsFromFiles);

		return true;
	};
}

#elif defined(MB_PLATFORM_LINUX)

void Toolchain_Gcc::GetCompileVersionInfoAction(BuildAction& action, BuilderFileInfo& fileInfo, VersionNumberInfo versionInfo) 
{	
	// todo: Generate desktop entry object.	
	return false;
}

#elif defined(MB_PLATFORM_MACOS)

void Toolchain_Gcc::GetCompileVersionInfoAction(BuildAction& action, BuilderFileInfo& fileInfo, VersionNumberInfo versionInfo) 
{	
	// todo: Generate iconset and embed it.
	return false;
}

#endif

}; // namespace MicroBuild
