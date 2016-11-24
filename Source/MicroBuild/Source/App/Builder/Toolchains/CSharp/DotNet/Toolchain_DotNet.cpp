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
#include "App/Builder/Toolchains/CSharp/DotNet/Toolchain_DotNet.h"

#include "Core/Platform/Platform.h"
#include "Core/Platform/Registry.h"
#include "Core/Platform/Process.h"

namespace MicroBuild {
	
Toolchain_DotNet::Toolchain_DotNet(ProjectFile& file, uint64_t configurationHash)
	: Toolchain(file, configurationHash)
{
	m_bAvailable = FindToolchain();
	m_bRequiresCompileStep = false;
	m_description = Strings::Format("Microsoft C# Compiler (%s)", m_version.c_str());
}

bool Toolchain_DotNet::Init() 
{
	m_bAvailable = FindToolchain();
	m_bRequiresCompileStep = false;
	m_description = Strings::Format("Microsoft C# Compiler (%s)", m_version.c_str());
	return m_bAvailable;
}

bool Toolchain_DotNet::FindToolchain()
{
	std::map<EPlatformToolset, DotNetFramework> availableFrameworks = GetAvailableFrameworks();	
	if (availableFrameworks.size() <= 0)
	{
		return false;
	}

	EPlatformToolset latestFrameworkAvailable = EPlatformToolset::Default;	
	for (auto& framework : availableFrameworks)
	{
		if (framework.first > latestFrameworkAvailable)
		{
			latestFrameworkAvailable = framework.first;
		}
	}

	EPlatformToolset targetFramework = m_projectFile.Get_Build_PlatformToolset();
	if (targetFramework == EPlatformToolset::Default)
	{
		targetFramework = latestFrameworkAvailable;
	}

	switch (targetFramework)
	{
	case EPlatformToolset::DotNet_2_0:
		// Fallthrough
	case EPlatformToolset::DotNet_3_0:
		// Fallthrough
	case EPlatformToolset::DotNet_3_5:
		// Fallthrough
	case EPlatformToolset::DotNet_4_0:	
		// Fallthrough
		// Fallthrough
	case EPlatformToolset::DotNet_4_5:
		// Fallthrough
	case EPlatformToolset::DotNet_4_5_1:
		// Fallthrough
	case EPlatformToolset::DotNet_4_5_2:
		// Fallthrough
	case EPlatformToolset::DotNet_4_6:
		{	
			if (availableFrameworks.count(targetFramework) <= 0)
			{
				return false;
			}
			m_compilerPath = availableFrameworks[targetFramework].FrameworkPath.AppendFragment("csc.exe", true);
			m_version = Strings::Replace(Strings::Replace(CastToString(targetFramework), "DotNet_", ".NET Framework "), "_", ".");
			break;
		}
	default:
		{
			return false;
		}
	}

	m_archiverPath = m_compilerPath;
	m_linkerPath = m_compilerPath;

	// Figure out the core and framework assemblies location.
	m_standardLibraryPaths.push_back(m_compilerPath.GetDirectory());
	m_standardLibraryPaths.push_back(availableFrameworks[targetFramework].AssemblyPath);

	if (!m_compilerPath.Exists())
	{
		return false;
	}

	return true;
}

std::map<EPlatformToolset, DotNetFramework> Toolchain_DotNet::GetAvailableFrameworks()
{	
	std::map<EPlatformToolset, DotNetFramework> result;
	
#if defined(MB_ARCHITECTURE_X64)
	Platform::Path programFilesPath = "C:/Program Files (x86)";
#else
	Platform::Path programFilesPath = "C:/Program Files";
#endif

	// Version 2.0
	if (Platform::Registry::KeyExists(Platform::ERegistryHive::LocalMachine, "Software/Microsoft/NET Framework Setup/NDP/v2.0.50727"))
	{
#if defined(MB_ARCHITECTURE_X64)
		Platform::Path InstallPath = "C:/Windows/Microsoft.NET/Framework64/v2.0.50727/";
#else
		Platform::Path InstallPath = "C:/Windows/Microsoft.NET/Framework/v2.0.50727/";
#endif

		if (InstallPath.Exists())
		{
			DotNetFramework framework;
			framework.FrameworkPath = InstallPath;
			framework.AssemblyPath = ""; // No seperate reference assembly folder in 2.0.
			
			result.insert(std::pair<EPlatformToolset, DotNetFramework>(EPlatformToolset::DotNet_2_0, framework));
		}
	}
	
	// Version 3.0
	if (Platform::Registry::KeyExists(Platform::ERegistryHive::LocalMachine, "Software/Microsoft/NET Framework Setup/NDP/v3.0"))
	{
		// Should this be based on what we are compiling? Not on what we are compiled as?
#if defined(MB_ARCHITECTURE_X64)
		Platform::Path InstallPath = "C:/Windows/Microsoft.NET/Framework64/v3.0/";
#else
		Platform::Path InstallPath = "C:/Windows/Microsoft.NET/Framework/v3.0/";
#endif

		if (InstallPath.Exists())
		{
			DotNetFramework framework;
			framework.FrameworkPath = InstallPath;
			
			if (Platform::Registry::GetValue<Platform::Path>(Platform::ERegistryHive::LocalMachine, "Software/Microsoft/.NETFramework/AssemblyFolders/v3.0", "All Assemblies In", framework.AssemblyPath))
			{
				if (framework.AssemblyPath.Exists())
				{
					result.insert(std::pair<EPlatformToolset, DotNetFramework>(EPlatformToolset::DotNet_3_0, framework));
				}
			}
		}
	}
	
	// Version 3.5
	if (Platform::Registry::KeyExists(Platform::ERegistryHive::LocalMachine, "Software/Microsoft/NET Framework Setup/NDP/v3.5"))
	{
		Platform::Path InstallPath;

		if (Platform::Registry::GetValue<Platform::Path>(Platform::ERegistryHive::LocalMachine, "Software/Microsoft/NET Framework Setup/NDP/v3.5", "InstallPath", InstallPath))
		{
			if (InstallPath.Exists())
			{
				DotNetFramework framework;
				framework.FrameworkPath = InstallPath;
			
				if (Platform::Registry::GetValue<Platform::Path>(Platform::ERegistryHive::LocalMachine, "Software/Microsoft/.NETFramework/AssemblyFolders/v3.5", "All Assemblies In", framework.AssemblyPath))
				{
					if (framework.AssemblyPath.Exists())
					{
						result.insert(std::pair<EPlatformToolset, DotNetFramework>(EPlatformToolset::DotNet_3_5, framework));
					}
				}
			}
		}
	}

	// Version >= 4.0
	if (Platform::Registry::KeyExists(Platform::ERegistryHive::LocalMachine, "Software/Microsoft/NET Framework Setup/NDP/v4/Full"))
	{
		Platform::Path InstallPath;

		if (Platform::Registry::GetValue<Platform::Path>(Platform::ERegistryHive::LocalMachine, "Software/Microsoft/NET Framework Setup/NDP/v4/Full", "InstallPath", InstallPath))
		{
			if (InstallPath.Exists())
			{			
				DotNetFramework framework;
				framework.FrameworkPath = InstallPath;
				framework.AssemblyPath = programFilesPath.AppendFragment("Reference Assemblies/Microsoft/Framework/.NETFramework/v4.0", true);
				if (framework.AssemblyPath.Exists())
				{
					result.insert(std::pair<EPlatformToolset, DotNetFramework>(EPlatformToolset::DotNet_4_0, framework));
				}

				int Release = 0;
				if (Platform::Registry::GetValue<int>(Platform::ERegistryHive::LocalMachine, "Software/Microsoft/NET Framework Setup/NDP/v4/Full", "Release", Release))
				{
					// Versions above 4.0 are in-place updates, so we just check the current release
					// tag to work out if they are available.

					// Version 4.5
					if (Release >= 378389)
					{
						framework.AssemblyPath = programFilesPath.AppendFragment("Reference Assemblies/Microsoft/Framework/.NETFramework/v4.5", true);
						if (framework.AssemblyPath.Exists())
						{
							result.insert(std::pair<EPlatformToolset, DotNetFramework>(EPlatformToolset::DotNet_4_5, framework));
						}
					}
					// Version 4.5.1
					if (Release >= 378675)
					{
						framework.AssemblyPath = programFilesPath.AppendFragment("Reference Assemblies/Microsoft/Framework/.NETFramework/v4.5.1", true);
						if (framework.AssemblyPath.Exists())
						{
							result.insert(std::pair<EPlatformToolset, DotNetFramework>(EPlatformToolset::DotNet_4_5_1, framework));
						}
					}
					// Version 4.5.2
					if (Release >= 379893)
					{
						framework.AssemblyPath = programFilesPath.AppendFragment("Reference Assemblies/Microsoft/Framework/.NETFramework/v4.5.2", true);
						if (framework.AssemblyPath.Exists())
						{
							result.insert(std::pair<EPlatformToolset, DotNetFramework>(EPlatformToolset::DotNet_4_5_2, framework));
						}
					}
					// Version 4.6
					if (Release >= 393295)
					{
						framework.AssemblyPath = programFilesPath.AppendFragment("Reference Assemblies/Microsoft/Framework/.NETFramework/v4.6", true);
						if (framework.AssemblyPath.Exists())
						{
							result.insert(std::pair<EPlatformToolset, DotNetFramework>(EPlatformToolset::DotNet_4_6, framework));
						}
					}
				}
			}
		}
	}
	
	return result;
}

void Toolchain_DotNet::GetLinkArguments(const std::vector<BuilderFileInfo>& files, std::vector<std::string>& args)
{
	Platform::Path outputPath = m_projectFile.Get_Project_OutputDirectory()
		.AppendFragment(Strings::Format("%s%s", m_projectFile.Get_Project_OutputName().c_str(), m_projectFile.Get_Project_OutputExtension().c_str()), true);
	
	Platform::Path outputPdb = m_projectFile.Get_Project_IntermediateDirectory()
		.AppendFragment(Strings::Format("%s%s", m_projectFile.Get_Project_OutputName().c_str(), ".pdb"), true);
	
	args.push_back(Strings::Format("/out:%s", Strings::Quoted(outputPath.ToString()).c_str()));
	args.push_back(Strings::Format("/pdb:%s", Strings::Quoted(outputPdb.ToString()).c_str()));

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

bool Toolchain_DotNet::Archive(std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile)
{
	if (!Toolchain::Archive(files, outputFile))
	{
		return false;
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

	return true;
}

bool Toolchain_DotNet::Link(std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile)
{
	if (!Toolchain::Link(files, outputFile))
	{
		return false;
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

	return true;
}

}; // namespace MicroBuild
