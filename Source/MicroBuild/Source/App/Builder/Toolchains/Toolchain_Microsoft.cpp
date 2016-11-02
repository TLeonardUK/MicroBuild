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
#include "App/Builder/Toolchains/Toolchain_Microsoft.h"

#include "Core/Platform/Platform.h"
#include "Core/Platform/Registry.h"
#include "Core/Platform/Process.h"

#include <sstream>

namespace MicroBuild {

Toolchain_Microsoft::Toolchain_Microsoft(ProjectFile& file)
	: Toolchain(file)
{
	m_bAvailable = FindToolchain();	
	m_bRequiresCompileStep = true;
	m_description = Strings::Format("Microsoft C++ Compiler (%s)", m_version.c_str());;
}

bool Toolchain_Microsoft::FindToolchain()
{
	switch (m_projectFile.Get_Project_Subsystem())
	{
	case EPlatformSubSystem::Native:
		break;
	case EPlatformSubSystem::WinRT:
		// Fallthrough - Not supported yet.
	default:
		return false;
	}

	switch (m_projectFile.Get_Build_PlatformToolset())
	{
	case EPlatformToolset::Default:
		// Fallthrough
	case EPlatformToolset::v140:
		{
			Platform::Path toolchainPath;
			Platform::Path vcInstallPath;
			Platform::Path windowsKitsRoot81Path;
			Platform::Path windowsKitsRoot10Path;
			
#if defined(MB_ARCHITECTURE_X64)
			if (!Platform::Registry::GetValue<Platform::Path>(Platform::ERegistryHive::LocalMachine, "Software/Wow6432Node/Microsoft/VisualStudio/14.0/Setup/VC", "ProductDir", vcInstallPath))
#else
			if (!Platform::Registry::GetValue<Platform::Path>(Platform::ERegistryHive::LocalMachine, "Software/Microsoft/VisualStudio/14.0/Setup/VC", "ProductDir", vcInstallPath))
#endif
			{
				return false;
			}
			
#if defined(MB_ARCHITECTURE_X64)
			if (!Platform::Registry::GetValue<Platform::Path>(Platform::ERegistryHive::LocalMachine, "Software/Wow6432Node/Microsoft/Windows Kits/Installed Roots", "KitsRoot81", windowsKitsRoot81Path))
#else
			if (!Platform::Registry::GetValue<Platform::Path>(Platform::ERegistryHive::LocalMachine, "Software/Microsoft/Windows Kits/Installed Roots", "KitsRoot81", windowsKitsRoot81Path))
#endif
			{
				return false;
			}

#if defined(MB_ARCHITECTURE_X64)
			if (!Platform::Registry::GetValue<Platform::Path>(Platform::ERegistryHive::LocalMachine, "Software/Wow6432Node/Microsoft/Windows Kits/Installed Roots", "KitsRoot10", windowsKitsRoot10Path))
#else
			if (!Platform::Registry::GetValue<Platform::Path>(Platform::ERegistryHive::LocalMachine, "Software/Microsoft/Windows Kits/Installed Roots", "KitsRoot10", windowsKitsRoot10Path))
#endif
			{
				return false;
			}
			
			std::vector<std::string> availableWindows10Kits = windowsKitsRoot10Path.AppendFragment("Include", true).GetDirectories();
			if (availableWindows10Kits.size() == 0)
			{
				return false;
			}

			std::sort(availableWindows10Kits.begin(), availableWindows10Kits.end()); // Sort alphabetically so last is most recent.
			std::string latestWindows10KitVersion = availableWindows10Kits[availableWindows10Kits.size() - 1];

			m_standardIncludePaths.push_back(vcInstallPath.AppendFragment("include", true));
			m_standardIncludePaths.push_back(vcInstallPath.AppendFragment("atlmfc/include", true));
			m_standardIncludePaths.push_back(windowsKitsRoot10Path.AppendFragment(Strings::Format("Include/%s/ucrt", latestWindows10KitVersion.c_str()), true));
			m_standardIncludePaths.push_back(windowsKitsRoot81Path.AppendFragment("Include/um", true));
			m_standardIncludePaths.push_back(windowsKitsRoot81Path.AppendFragment("Include/shared", true));
			m_standardIncludePaths.push_back(windowsKitsRoot81Path.AppendFragment("Include/winrt", true));
			
			switch (m_projectFile.Get_Target_Platform())
			{
			case EPlatform::x64: 
				{
					if (Platform::IsOperatingSystem64Bit())
					{
						toolchainPath = vcInstallPath.AppendFragment("bin/amd64", true);
					}
					else
					{
						toolchainPath = vcInstallPath.AppendFragment("bin/x86_amd64", true);
					}			
					m_standardLibraryPaths.push_back(vcInstallPath.AppendFragment("lib/amd64", true));
					m_standardLibraryPaths.push_back(vcInstallPath.AppendFragment("atlmfc/lib/amd64", true));
					m_standardLibraryPaths.push_back(windowsKitsRoot10Path.AppendFragment(Strings::Format("Lib/%s/ucrt/x64", latestWindows10KitVersion.c_str()), true));
					m_standardLibraryPaths.push_back(windowsKitsRoot81Path.AppendFragment("lib/winv6.3/um/x64", true));
					break;
				}
			case EPlatform::x86:
				{
					toolchainPath = vcInstallPath.AppendFragment("bin", true);
					m_standardLibraryPaths.push_back(vcInstallPath.AppendFragment("lib", true));
					m_standardLibraryPaths.push_back(vcInstallPath.AppendFragment("atlmfc/lib", true));
					m_standardLibraryPaths.push_back(windowsKitsRoot10Path.AppendFragment(Strings::Format("Lib/%s/ucrt/x86", latestWindows10KitVersion.c_str()), true));
					m_standardLibraryPaths.push_back(windowsKitsRoot81Path.AppendFragment("lib/winv6.3/um/x86", true));
					break;
				}
			case EPlatform::ARM:
			case EPlatform::ARM64:
				{
					if (Platform::IsOperatingSystem64Bit())
					{
						toolchainPath = vcInstallPath.AppendFragment("bin/amd64_arm", true);
					}
					else
					{
						toolchainPath = vcInstallPath.AppendFragment("bin/x86_arm", true);
					}
					m_standardLibraryPaths.push_back(vcInstallPath.AppendFragment("lib/arm", true));
					m_standardLibraryPaths.push_back(vcInstallPath.AppendFragment("atlmfc/lib/arm", true));
					m_standardLibraryPaths.push_back(windowsKitsRoot10Path.AppendFragment(Strings::Format("Lib/%s/ucrt/arm", latestWindows10KitVersion.c_str()), true));
					m_standardLibraryPaths.push_back(windowsKitsRoot81Path.AppendFragment("lib/winv6.3/um/arm", true));
					break;
				}
			}

			m_envVarBatchFilePath = vcInstallPath.AppendFragment("bin/vcvars32.bat", true);
			m_compilerPath = toolchainPath.AppendFragment("cl.exe", true);
			m_linkerPath = toolchainPath.AppendFragment("link.exe", true);
			m_archiverPath = toolchainPath.AppendFragment("lib.exe", true);
			m_version = "Toolset v140";

			break;
		}
	default:
		{
			return false;
		}
	}

	// Get all the visual studio environment variables, this is plain nasty, but theres no easier way to do this
	// that I can see :(.
	std::vector<std::string> args;
	args.push_back("/A");
	args.push_back("/C");
	args.push_back(Strings::Quoted(m_envVarBatchFilePath.ToString()));
	args.push_back("&&");
	args.push_back("set");

	Platform::Process process;
	if (!process.Open("cmd", Platform::Path::GetExecutablePath().GetDirectory(), args, true))
	{
		return false;
	}

	std::string output = process.ReadToEnd();
	std::vector<std::string> vars = Strings::Split('\n', output);

	for (auto& var : vars)
	{
		if (var[var.size() - 1] == '\r')
		{
			var = var.substr(0, var.size() - 1);
		}

		size_t equalOffset = var.find('=');
		if (equalOffset == std::string::npos)
		{
			continue;
		}

		std::string name = "";
		std::string value = "";
		Strings::SplitOnIndex(var, equalOffset, name, value);

		Platform::SetEnvironmentVariable(name, value);
	}
	
	if (!m_compilerPath.Exists() || 
		!m_linkerPath.Exists() || 
		!m_archiverPath.Exists())
	{
		return false;
	}

	return true;
}

void Toolchain_Microsoft::GetCompileArguments(std::vector<std::string>& args)
{
	switch (m_projectFile.Get_Build_OptimizationLevel())
	{
	default:
		// Fallthrough
	case EOptimizationLevel::None:
		// Fallthrough
	case EOptimizationLevel::Debug:	
		{
			args.push_back("/Od");
			break;
		}
	case EOptimizationLevel::Full:
		{
			args.push_back("/Ox");
			break;
		}
	case EOptimizationLevel::PreferSize:
		{
			args.push_back("/Os");
			break;
		}
	case EOptimizationLevel::PreferSpeed:
		{
			args.push_back("/Ot");
			break;
		}
	}

	switch (m_projectFile.Get_Build_CharacterSet())
	{
	default:
		// Fallthrough
	case ECharacterSet::Default:
		// Fallthrough
	case ECharacterSet::MBCS:
		{
			args.push_back("/D\"_MBCS\"");
			break;
		}
	case ECharacterSet::Unicode:
		{
			args.push_back("/D\"_UNICODE\"");
			args.push_back("/D\"UNICODE\"");
			break;
		}
	}

	switch (m_projectFile.Get_Build_WarningLevel())
	{
	default:
		// Fallthrough
	case EWarningLevel::Default:
		// Fallthrough
	case EWarningLevel::High:
		{
			args.push_back("/W4");
			break;
		}
	case EWarningLevel::None:
		{
			args.push_back("/W0");
			break;
		}
	case EWarningLevel::Low:
		{
			args.push_back("/W1");
			break;
		}
	case EWarningLevel::Medium:
		{
			args.push_back("/W2");
			break;
		}
	case EWarningLevel::Verbose:
		{
			args.push_back("/Wall");
			break;
		}
	}

	std::vector<std::string> customArgs = Strings::Crack(m_projectFile.Get_Build_CompilerArguments());
	args.insert(args.end(), customArgs.begin(), customArgs.end());

	for (auto& define : m_projectFile.Get_Defines_Define())
	{
		args.push_back(Strings::Format("/D%s", Strings::Quoted(define).c_str()));
	}

	for (auto& warning : m_projectFile.Get_DisabledWarnings_DisabledWarning())
	{
		args.push_back(Strings::Format("/wd%s", warning.c_str()));
	}

	for (auto& includeDir : m_projectFile.Get_SearchPaths_IncludeDirectory())
	{
		args.push_back(Strings::Format("/I%s", Strings::Quoted(includeDir.ToString()).c_str()));
	}

	for (auto& includeDir : m_standardIncludePaths)
	{
		args.push_back(Strings::Format("/I%s", Strings::Quoted(includeDir.ToString()).c_str()));
	}

	for (auto& forcedInclude : m_projectFile.Get_ForcedIncludes_ForcedInclude())
	{
		args.push_back(Strings::Format("/FI%s", Strings::Quoted(forcedInclude.ToString()).c_str()));
	}
	
	if (m_projectFile.Get_Flags_CompilerWarningsFatal())
	{
		args.push_back("/WX");	
	}

	if (m_projectFile.Get_Flags_RuntimeTypeInfo())
	{
		args.push_back("/GR");	
	}
	else
	{	
		args.push_back("/GR-");	
	}

	if (m_projectFile.Get_Flags_GenerateDebugInformation())
	{
		args.push_back("/Zi");	
	}
	
	if (m_projectFile.Get_Flags_LinkTimeOptimization())
	{
		args.push_back("/GL");	
	}

	if (m_projectFile.Get_Flags_Exceptions())
	{
		args.push_back("/EHa");	
	}

	// Disable startup blurb.
	args.push_back("/nologo");

	// Buffer security checks, turn them off if we are going for full opt.
	if (m_projectFile.Get_Build_OptimizationLevel() == EOptimizationLevel::Full)
	{
		args.push_back("/GS-");
	}
	else
	{
		args.push_back("/GS");
		args.push_back("/RTC1");
	}

	// Floating point precious.
	args.push_back("/fp:fast");

	// Use cdecl calling convention.
	args.push_back("/Gd");

	// Various microsoft specific extensions we need to control.
	args.push_back("/Zc:wchar_t"); // wchar_t as native type.
	args.push_back("/Zc:inline"); // remove unreferenced comdat
	args.push_back("/Zc:forScope"); // Force conformat for scopes.

	// No error reporting plz.	
	args.push_back("/errorReport:none");
	
	// Type of thing we are compiling and what runtime library to link.	
	if (m_projectFile.Get_Project_OutputType() == EOutputType::DynamicLib)
	{
		if (m_projectFile.Get_Build_OptimizationLevel() == EOptimizationLevel::Full)
		{
			args.push_back("/LD");
		}	
		else
		{
			args.push_back("/LDd");		
		}
	}
	
	if (m_projectFile.Get_Flags_StaticRuntime())
	{
		if (m_projectFile.Get_Build_OptimizationLevel() == EOptimizationLevel::Full)
		{
			args.push_back("/MT");
		}	
		else
		{
			args.push_back("/MTd");		
		}
	}
	else
	{
		if (m_projectFile.Get_Build_OptimizationLevel() == EOptimizationLevel::Full)
		{
			args.push_back("/MD");
		}	
		else
		{
			args.push_back("/MDd");		
		}
	}

	// Adding this argument will cause the include tree to be dumped out so we can figure out our dependencies.
	args.push_back("/showIncludes");
}

void Toolchain_Microsoft::GetPchCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args)
{
	GetCompileArguments(args);
	
	Platform::Path outputPdb = m_projectFile.Get_Project_IntermediateDirectory()
		.AppendFragment(Strings::Format("%s%s", m_projectFile.Get_Project_OutputName().c_str(), ".pdb"), true);

	Platform::Path pchPath = m_projectFile.Get_Project_IntermediateDirectory()
		.AppendFragment(m_projectFile.Get_Build_PrecompiledHeader().ChangeExtension("pch").GetFilename(), true);
	
	// Emit debug info to the appropriate PDB.
	args.push_back(Strings::Format("/Fd%s", Strings::Quoted(outputPdb.ToString()).c_str()));

	// Create PCH file in intermediate directory.
	args.push_back(Strings::Format("/Yc%s", Strings::Quoted(m_projectFile.Get_Build_PrecompiledHeader().GetFilename()).c_str()));
	args.push_back(Strings::Format("/Fp%s", Strings::Quoted(pchPath.ToString()).c_str()));
	
	// Set output folder for object file.
	args.push_back(Strings::Format("/Fo%s", Strings::Quoted(file.OutputPath.ToString()).c_str()));

	// Compile don't link.
	args.push_back("/c");

	// Now the actual file!
	args.push_back(m_projectFile.Get_Build_PrecompiledSource().ToString());
}

void Toolchain_Microsoft::GetSourceCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args)
{
	GetCompileArguments(args);

	Platform::Path outputPdb = m_projectFile.Get_Project_IntermediateDirectory()
		.AppendFragment(Strings::Format("%s%s", m_projectFile.Get_Project_OutputName().c_str(), ".pdb"), true);
	
	Platform::Path pchPath = m_projectFile.Get_Project_IntermediateDirectory()
		.AppendFragment(m_projectFile.Get_Build_PrecompiledHeader().ChangeExtension("pch").GetFilename(), true);

	std::string pchFilename = m_projectFile.Get_Build_PrecompiledHeader().GetFilename();

	// Emit debug info to the appropriate PDB.
	args.push_back(Strings::Format("/Fd%s", Strings::Quoted(outputPdb.ToString()).c_str()));
	args.push_back("/FS"); // Force syncronous file writes to PDB.

	// Use PCH file in intermediate directory.
	if (!m_projectFile.Get_Build_PrecompiledHeader().IsEmpty())
	{
		args.push_back(Strings::Format("/Yu%s", Strings::Quoted(pchFilename).c_str()));
		args.push_back(Strings::Format("/Fp%s", Strings::Quoted(pchPath.ToString()).c_str()));
	}

	// Set output folder for object file.
	args.push_back(Strings::Format("/Fo%s", Strings::Quoted(file.OutputPath.ToString()).c_str()));

	// Compile don't link.
	args.push_back("/c");

	// Now the actual file!
	args.push_back(file.SourcePath.ToString());
}

void Toolchain_Microsoft::GetLinkArguments(const std::vector<BuilderFileInfo>& sourceFiles, std::vector<std::string>& args)
{
	Platform::Path outputPath = m_projectFile.Get_Project_OutputDirectory()
		.AppendFragment(Strings::Format("%s%s", m_projectFile.Get_Project_OutputName().c_str(), m_projectFile.Get_Project_OutputExtension().c_str()), true);
	
	Platform::Path pchPath = m_projectFile.Get_Project_IntermediateDirectory()
		.AppendFragment(m_projectFile.Get_Build_PrecompiledHeader().ChangeExtension("pch").GetFilename(), true);
	
	Platform::Path pchObjectPath = m_projectFile.Get_Project_IntermediateDirectory()
		.AppendFragment(m_projectFile.Get_Build_PrecompiledHeader().ChangeExtension("o").GetFilename(), true);

	Platform::Path outputPdb = m_projectFile.Get_Project_IntermediateDirectory()
		.AppendFragment(Strings::Format("%s%s", m_projectFile.Get_Project_OutputName().c_str(), ".pdb"), true);
	
	args.push_back("/ERRORREPORT:NONE");
	args.push_back("/MANIFEST");
	args.push_back("/MANIFESTUAC:\"level='asInvoker' uiAccess='false'\"");
	args.push_back("/manifest:embed");
//	args.push_back("/ENTRY:\"mainCRTStartup\"");
	args.push_back("/DYNAMICBASE");
	args.push_back("/NXCOMPAT");

	args.push_back(Strings::Format("/OUT:%s", Strings::Quoted(outputPath.ToString()).c_str()));	

	if (m_projectFile.Get_Flags_GenerateDebugInformation())
	{
		args.push_back("/DEBUG");	
	}
	
	args.push_back("/NOLOGO");	
	
	if (m_projectFile.Get_Flags_LinkerWarningsFatal())
	{
		args.push_back("/WX");	
	}	

	switch (m_projectFile.Get_Target_Platform())
	{
	case EPlatform::x86:
		{		
			args.push_back("/MACHINE:X86");
			break;
		}
	case EPlatform::x64:
		{
			args.push_back("/MACHINE:X64");
			break;
		}
	case EPlatform::ARM64:
	case EPlatform::ARM:
		{
			args.push_back("/MACHINE:ARM");
			break;
		}
	default:
		{
			return;
		}
	}

	switch (m_projectFile.Get_Project_OutputType())
	{
	case EOutputType::ConsoleApp:
		{
			args.push_back("/SUBSYSTEM:CONSOLE");
			break;
		}
	case EOutputType::DynamicLib:
		{
			args.push_back("/DLL");
			args.push_back("/SUBSYSTEM:CONSOLE");
			break;
		}
	case EOutputType::Executable:
		{
			args.push_back("/SUBSYSTEM:WINDOWS");
			break;
		}
	default:
		{
			return;
		}
	}
	
	for (auto& includeDir : m_projectFile.Get_SearchPaths_LibraryDirectory())
	{
		args.push_back(Strings::Format("/LIBPATH:%s", Strings::Quoted(includeDir.ToString()).c_str()));
	}

	for (auto& includeDir : m_standardLibraryPaths)
	{
		args.push_back(Strings::Format("/LIBPATH:%s", Strings::Quoted(includeDir.ToString()).c_str()));
	}
	
	for (auto& warning : m_projectFile.Get_DisabledWarnings_DisabledWarning())
	{
		args.push_back(Strings::Format("/IGNORE:%s", warning.c_str()));
	}

	if (m_projectFile.Get_Flags_LinkTimeOptimization())
	{
		args.push_back("/LTCG");	
	}

	args.push_back(Strings::Format("/PDB:%s", Strings::Quoted(outputPdb.ToString()).c_str()));	
	
	std::vector<std::string> customArgs = Strings::Crack(m_projectFile.Get_Build_LinkerArguments());
	args.insert(args.end(), customArgs.begin(), customArgs.end());

	// Libraries to link.
	for (auto& library : m_projectFile.Get_Libraries_Library())
	{
		args.push_back(Strings::Quoted(library.ToString()));
	}

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

void Toolchain_Microsoft::GetArchiveArguments(const std::vector<BuilderFileInfo>& sourceFiles, std::vector<std::string>& args)
{
	Platform::Path outputPath = m_projectFile.Get_Project_OutputDirectory()
		.AppendFragment(Strings::Format("%s%s", m_projectFile.Get_Project_OutputName().c_str(), m_projectFile.Get_Project_OutputExtension().c_str()), true);
	
	Platform::Path pchPath = m_projectFile.Get_Project_IntermediateDirectory()
		.AppendFragment(m_projectFile.Get_Build_PrecompiledHeader().ChangeExtension("pch").GetFilename(), true);
	
	Platform::Path pchObjectPath = m_projectFile.Get_Project_IntermediateDirectory()
		.AppendFragment(m_projectFile.Get_Build_PrecompiledHeader().ChangeExtension("o").GetFilename(), true);

	Platform::Path outputPdb = m_projectFile.Get_Project_IntermediateDirectory()
		.AppendFragment(Strings::Format("%s%s", m_projectFile.Get_Project_OutputName().c_str(), ".pdb"), true);
		

	args.push_back(Strings::Format("/OUT:%s", Strings::Quoted(outputPath.ToString()).c_str()));	
	
	args.push_back("/NOLOGO");	
		
	switch (m_projectFile.Get_Target_Platform())
	{
	case EPlatform::x86:
		{		
			args.push_back("/MACHINE:X86");
			break;
		}
	case EPlatform::x64:
		{
			args.push_back("/MACHINE:X64");
			break;
		}
	case EPlatform::ARM64:
	case EPlatform::ARM:
		{
			args.push_back("/MACHINE:ARM");
			break;
		}
	default:
		{
			return;
		}
	}
	
	for (auto& warning : m_projectFile.Get_DisabledWarnings_DisabledWarning())
	{
		args.push_back(Strings::Format("/IGNORE:%s", warning.c_str()));
	}

	for (auto& includeDir : m_projectFile.Get_SearchPaths_LibraryDirectory())
	{
		args.push_back(Strings::Format("/LIBPATH:%s", Strings::Quoted(includeDir.ToString()).c_str()));
	}

	for (auto& includeDir : m_standardLibraryPaths)
	{
		args.push_back(Strings::Format("/LIBPATH:%s", Strings::Quoted(includeDir.ToString()).c_str()));
	}
	
	if (m_projectFile.Get_Flags_LinkTimeOptimization())
	{
		args.push_back("/LTCG");	
	}

	if (m_projectFile.Get_Flags_LinkerWarningsFatal())
	{
		args.push_back("/WX");	
	}	

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

void Toolchain_Microsoft::ExtractDependencies(const std::string& input, std::string& rawInput, std::vector<Platform::Path>& dependencies, bool bRemoveFirstLine)
{
	std::string startTag = "\r\nNote: including file:";
	std::string endTag = "\r\n";

	rawInput = input;

	while (true)
	{
		size_t startOffset = rawInput.find(startTag);
		if (startOffset == std::string::npos)
		{
			break;
		}
		size_t endOffset = rawInput.find(endTag, startOffset + startTag.size());
		if (endOffset == std::string::npos)
		{
			break;;
		}
		
		std::string extracted = rawInput.substr(startOffset + startTag.size(), endOffset - (startOffset + startTag.size()));
		extracted = Strings::Trim(extracted);

		size_t extractedSize =  (endOffset - startOffset);
		rawInput = rawInput.erase(startOffset, extractedSize);

		dependencies.push_back(extracted);
	}
	
	if (bRemoveFirstLine)
	{
		size_t endOffset = rawInput.find(endTag);
		if (endOffset != std::string::npos)
		{
			rawInput = rawInput.substr(endOffset + endTag.size());		
		}
	}
}

void Toolchain_Microsoft::UpdateDependencyManifest(BuilderFileInfo& fileInfo, std::vector<Platform::Path>& dependencies, BuilderFileInfo* pchFileInfo)
{	
	fileInfo.Dependencies.clear();
	
	for (auto& path : dependencies)
	{	
		BuilderDependencyInfo dependency;
		dependency.SourcePath = path;
		dependency.Hash = BuilderFileInfo::CalculateFileHash(dependency.SourcePath);

		bool bAlreadyExists = false;

		for (auto& dep : fileInfo.Dependencies)
		{
			if (dep.SourcePath == dependency.SourcePath)
			{
				dep.Hash = dependency.Hash;
				bAlreadyExists = true;
				break;
			}
		}
			
		if (!bAlreadyExists)
		{
			fileInfo.Dependencies.push_back(dependency);
		}
	}

	// If we have a PCH file, we also inherit all dependencies from it as well (VS doesn't emit include tags for things in the pch files -_-).
	if (pchFileInfo)
	{
		for (auto& pchDep : pchFileInfo->Dependencies)
		{
			bool bAlreadyExists = false;

			for (auto& dep : fileInfo.Dependencies)
			{
				if (dep.SourcePath == pchDep.SourcePath)
				{
					bAlreadyExists = true;
					break;
				}
			}
			
			if (!bAlreadyExists)
			{
				fileInfo.Dependencies.push_back(pchDep);
			}
		}
	}

	fileInfo.StoreManifest();
}

Platform::Path Toolchain_Microsoft::FindLibraryPath(const Platform::Path& lib)
{
	for (auto& path : m_projectFile.Get_SearchPaths_LibraryDirectory())
	{
		Platform::Path newPath = path.AppendFragment(lib.ToString().c_str(), true);
		if (newPath.Exists())
		{
			return newPath;
		}
	}

	for (auto& path : m_standardLibraryPaths)
	{
		Platform::Path newPath = path.AppendFragment(lib.ToString().c_str(), true);
		if (newPath.Exists())
		{
			return newPath;
		}
	}

	return lib;
}

bool Toolchain_Microsoft::OpenBatchProcess(Platform::Process& process, BuilderFileInfo& fileInfo, const Platform::Path& exe, std::vector<std::string>& args) 
{
	MB_UNUSED_PARAMETER(fileInfo);

	/*
	Platform::Path batchPath = m_projectFile.Get_Project_IntermediateDirectory()
		.AppendFragment(Strings::Format("Compile_%s.bat", fileInfo.OutputPath.GetFilename().c_str()), true);

	std::stringstream stream;
	stream << "@echo off\n";	
	stream << "CALL " << Strings::Quoted(m_envVarBatchFilePath.ToString()) << "\n";	
	stream << "CALL " << Strings::Quoted(exe.ToString());	
	for (auto& arg : args)
	{
		stream << " " << arg;
	}
	stream << "\n";
	stream << "if %ERRORLEVEL% NEQ 0 (" << "\n";
  	stream << "    exit /b %ERRORLEVEL%" << "\n";
	stream << ")" << "\n";

	if (!Strings::WriteFile(batchPath, stream.str()))
	{
		Log(LogSeverity::Fatal, "Failed to write to batch file '%s'.\n", batchPath.ToString().c_str());
		return false;
	}
	
	if (!process.Open(batchPath, Platform::Path::GetExecutablePath().GetDirectory(), std::vector<std::string>(), true))
	{
		return false;
	}
	*/
	
	if (!process.Open(exe, Platform::Path::GetExecutablePath().GetDirectory(), args, true))
	{
		return false;
	}

	return true;
}

bool Toolchain_Microsoft::CompilePch(BuilderFileInfo& fileInfo) 
{
	std::vector<std::string> arguments;
	GetPchCompileArguments(fileInfo, arguments);

	Platform::Process process;
	if (!OpenBatchProcess(process, fileInfo, m_compilerPath, arguments))
	{
		return false;
	}

	std::string output = process.ReadToEnd();
	
	std::vector<Platform::Path> dependencies;
	std::string rawOutput;
	ExtractDependencies(output, rawOutput, dependencies, true);
	
	printf("%s", rawOutput.c_str());

	if (process.GetExitCode() != 0)
	{
		return false;
	}

	UpdateDependencyManifest(fileInfo, dependencies);

	return true;
}

bool Toolchain_Microsoft::Compile(BuilderFileInfo& fileInfo, BuilderFileInfo& pchFileInfo)
{
	std::vector<std::string> arguments;
	GetSourceCompileArguments(fileInfo, arguments);
	
	Platform::Process process;
	if (!OpenBatchProcess(process, fileInfo, m_compilerPath, arguments))
	{
		return false;
	}
	
	std::string output = process.ReadToEnd();
	
	std::vector<Platform::Path> dependencies;
	std::string rawOutput;
	ExtractDependencies(output, rawOutput, dependencies, true);
	
	printf("%s", rawOutput.c_str());

	if (process.GetExitCode() != 0)
	{
		return false;
	}
	
	UpdateDependencyManifest(fileInfo, dependencies, &pchFileInfo);

	return true;
}

bool Toolchain_Microsoft::Archive(std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile)
{
	std::vector<std::string> arguments;
	GetArchiveArguments(files, arguments);

	Platform::Process process;
	if (!OpenBatchProcess(process, outputFile, m_archiverPath, arguments))
	{
		return false;
	}
	
	std::string output = process.ReadToEnd();
	printf("%s", output.c_str());

	if (process.GetExitCode() != 0)
	{
		return false;
	}

	outputFile.Dependencies.clear();

	// Object files to link.
	for (auto& sourceFile : files)
	{
		BuilderDependencyInfo info;
		info.SourcePath = sourceFile.OutputPath;
		info.Hash = BuilderFileInfo::CalculateFileHash(info.SourcePath);
		outputFile.Dependencies.push_back(info);
	}
	
	// Link PCH.
	if (!m_projectFile.Get_Build_PrecompiledHeader().IsEmpty())
	{
		Platform::Path pchObjectPath = m_projectFile.Get_Project_IntermediateDirectory()
			.AppendFragment(m_projectFile.Get_Build_PrecompiledHeader().ChangeExtension("o").GetFilename(), true);

		BuilderDependencyInfo info;
		info.SourcePath = pchObjectPath;
		info.Hash = BuilderFileInfo::CalculateFileHash(info.SourcePath);
		outputFile.Dependencies.push_back(info);
	}

	// Copy PDB to output.	
	Platform::Path intPdb = m_projectFile.Get_Project_IntermediateDirectory()
		.AppendFragment(Strings::Format("%s%s", m_projectFile.Get_Project_OutputName().c_str(), ".pdb"), true);
	Platform::Path outPdb = m_projectFile.Get_Project_OutputDirectory()
		.AppendFragment(Strings::Format("%s%s", m_projectFile.Get_Project_OutputName().c_str(), ".pdb"), true);
		
	if (!intPdb.Copy(outPdb))
	{
		Log(LogSeverity::Fatal, "Failed to copy pdb file to '%s'.", outPdb.ToString().c_str());
		return false;
	}

	outputFile.StoreManifest();

	return true;
}

bool Toolchain_Microsoft::Link(std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile)
{
	std::vector<std::string> arguments;
	GetLinkArguments(files, arguments);

	Platform::Process process;
	if (!OpenBatchProcess(process, outputFile, m_linkerPath, arguments))
	{
		return false;
	}
	
	std::string output = process.ReadToEnd();
	printf("%s", output.c_str());

	if (process.GetExitCode() != 0)
	{
		return false;
	}
	
	outputFile.Dependencies.clear();

	// Libraries to link.
	for (auto& library : m_projectFile.Get_Libraries_Library())
	{
		Platform::Path fullPath = library;
		if (fullPath.IsRelative())
		{
			fullPath = FindLibraryPath(library);
		}

		// Can't do dependency tracking on this library as we don't know where it is!
		if (fullPath.IsAbsolute())
		{
			BuilderDependencyInfo info;
			info.SourcePath = fullPath;
			info.Hash = BuilderFileInfo::CalculateFileHash(fullPath);
			outputFile.Dependencies.push_back(info);
		}
		else
		{
			Log(LogSeverity::Warning, "Could not find library '%s', dependency building will not function correctly.\n", fullPath.ToString().c_str());
		}
	}

	// Object files to link.
	for (auto& sourceFile : files)
	{
		BuilderDependencyInfo info;
		info.SourcePath = sourceFile.OutputPath;
		info.Hash = BuilderFileInfo::CalculateFileHash(info.SourcePath);
		outputFile.Dependencies.push_back(info);
	}

	// Link PCH.
	if (!m_projectFile.Get_Build_PrecompiledHeader().IsEmpty())
	{
		Platform::Path pchObjectPath = m_projectFile.Get_Project_IntermediateDirectory()
			.AppendFragment(m_projectFile.Get_Build_PrecompiledHeader().ChangeExtension("o").GetFilename(), true);

		BuilderDependencyInfo info;
		info.SourcePath = pchObjectPath;
		info.Hash = BuilderFileInfo::CalculateFileHash(info.SourcePath);
		outputFile.Dependencies.push_back(info);
	}
	
	// Copy PDB to output.	
	Platform::Path intPdb = m_projectFile.Get_Project_IntermediateDirectory()
		.AppendFragment(Strings::Format("%s%s", m_projectFile.Get_Project_OutputName().c_str(), ".pdb"), true);
	Platform::Path outPdb = m_projectFile.Get_Project_OutputDirectory()
		.AppendFragment(Strings::Format("%s%s", m_projectFile.Get_Project_OutputName().c_str(), ".pdb"), true);
		
	if (!intPdb.Copy(outPdb))
	{
		Log(LogSeverity::Fatal, "Failed to copy pdb file to '%s'.", outPdb.ToString().c_str());
		return false;
	}

	outputFile.StoreManifest();

	return true;
}

}; // namespace MicroBuild