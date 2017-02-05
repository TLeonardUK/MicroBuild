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
#include "App/Builder/Toolchains/Toolchain.h"

#include "App/Builder/Tasks/ArchiveTask.h"
#include "App/Builder/Tasks/CompilePchTask.h"
#include "App/Builder/Tasks/CompileTask.h"
#include "App/Builder/Tasks/CompileVersionInfoTask.h"
#include "App/Builder/Tasks/LinkTask.h"
#include "App/Builder/Tasks/ShellCommandTask.h"

#include "Core/Platform/Process.h"

namespace MicroBuild {

Toolchain::Toolchain(ProjectFile& file, uint64_t configurationHash)
	: m_bAvailable(false)
	, m_bRequiresCompileStep(true)
	, m_bRequiresVersionInfo(false)
	, m_bGeneratesPchObject(true)
	, m_description("")
	, m_projectFile(file)
	, m_configurationHash(configurationHash)
{
	MB_UNUSED_PARAMETER(file);
}

void Toolchain::SetProjectInfo(ProjectFile& file, uint64_t configurationHash)
{
	m_configurationHash = configurationHash;
	m_projectFile = file;
}

bool Toolchain::IsAvailable()
{
	return m_bAvailable;
}

std::string Toolchain::GetDescription()
{
	return m_description;
}

bool Toolchain::RequiresCompileStep()
{
	return m_bRequiresCompileStep;
}

bool Toolchain::RequiresVersionInfo()
{
	if (m_bRequiresVersionInfo && 
		!m_projectFile.Get_ProductInfo_Name().empty())
	{
		if (m_projectFile.Get_Project_OutputType() == EOutputType::ConsoleApp || 
			m_projectFile.Get_Project_OutputType() == EOutputType::Executable)
		{
			return true;
		}
	}	
	return false;
}

std::vector<std::shared_ptr<BuildTask>> Toolchain::GetTasks(std::vector<BuilderFileInfo>& files, uint64_t configurationHash, BuilderFileInfo& outputFile, VersionNumberInfo& versionInfo)
{
	std::vector<std::shared_ptr<BuildTask>> tasks;

	// TODO: This function is fairly gross, this needs to be structured better. Especially how we handle one-of linkable 
	//		 files like the version info.

	// Skip all individual builds if we are not multi-pass.
	if (RequiresCompileStep())
	{
		// Do we have a precompiled header? If so compile that first.
		Platform::Path precompiledSourcePath = m_projectFile.Get_Build_PrecompiledSource();
	
		BuilderFileInfo dummyFile; 
		BuilderFileInfo precompiledSourceFile; 
		bool bPrecompiledSourceFileFound = false;
		
		for (auto iter = files.begin(); iter != files.end(); iter++)
		{
			BuilderFileInfo& file = *iter;
			if (file.SourcePath == precompiledSourcePath)
			{
				// Remove precompiled source file from the list as we want to compile it seperately.
				bPrecompiledSourceFileFound = true;
				precompiledSourceFile = file;
				files.erase(iter);
				break;
			}
		}

		if (bPrecompiledSourceFileFound)
		{
			if (precompiledSourceFile.bOutOfDate)
			{
				std::shared_ptr<CompilePchTask> task = std::make_shared<CompilePchTask>(this, m_projectFile, precompiledSourceFile);
				tasks.push_back(task);
			}
		}

		// General build tasks for each translation unit.
		for (auto& file : files)
		{
			if (file.bOutOfDate)
			{
				std::shared_ptr<CompileTask> task = std::make_shared<CompileTask>(this, m_projectFile, file, precompiledSourceFile);
				tasks.push_back(task);
			}
		}
	}

	// Do we need to generate a version info?
	BuilderFileInfo versionInfoFile;

	if (RequiresVersionInfo())
	{
		// check out of date.
		versionInfoFile.SourcePath			= "";
		versionInfoFile.OutputPath			= m_projectFile.Get_Project_IntermediateDirectory().AppendFragment(Strings::Format("%s_VersionInfo.generated.o", m_projectFile.Get_Project_Name().c_str()), true);
		versionInfoFile.ManifestPath		= m_projectFile.Get_Project_IntermediateDirectory().AppendFragment(Strings::Format("%s_VersionInfo.generated.manifest", m_projectFile.Get_Project_Name().c_str()), true);
		versionInfoFile.Hash				= 0;
		versionInfoFile.bOutOfDate			= BuilderFileInfo::CheckOutOfDate(versionInfoFile, Strings::Hash64(CastToString(versionInfo.TotalChangelists), configurationHash), false);

		if (versionInfoFile.bOutOfDate)
		{
			std::shared_ptr<CompileVersionInfoTask> task = std::make_shared<CompileVersionInfoTask>(this, m_projectFile, versionInfoFile, versionInfo);
			tasks.push_back(task);
		}
	}	
	
	for (auto& command : m_projectFile.Get_PreLinkCommands_Command())
	{
		std::shared_ptr<ShellCommandTask> task = std::make_shared<ShellCommandTask>(BuildStage::PreLinkUser, command);
		tasks.push_back(task);
	}

	// Generate a final link task for all the object files.
	if (m_projectFile.Get_Project_OutputType() == EOutputType::StaticLib)
	{
		std::shared_ptr<ArchiveTask> archiveTask = std::make_shared<ArchiveTask>(files, this, m_projectFile, outputFile);
		tasks.push_back(archiveTask);
	}
	else
	{
		std::shared_ptr<LinkTask> linkTask = std::make_shared<LinkTask>(files, this, m_projectFile, outputFile);
		tasks.push_back(linkTask);
	}

	// Queue any postbuild commands.
	for (auto& command : m_projectFile.Get_PostBuildCommands_Command())
	{
		std::shared_ptr<ShellCommandTask> task = std::make_shared<ShellCommandTask>(BuildStage::PostBuildUser, command);
		tasks.push_back(task);
	}

	return tasks;
}

void Toolchain::UpdateDependencyManifest(BuilderFileInfo& fileInfo, std::vector<Platform::Path>& dependencies, std::vector<BuilderFileInfo*> inherits)
{	
	fileInfo.Dependencies.clear();
	
	for (auto& path : dependencies)
	{	
		BuilderDependencyInfo dependency;
		dependency.SourcePath = path;
		dependency.Hash = BuilderFileInfo::CalculateFileHash(dependency.SourcePath, m_configurationHash);

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

	// Inherit dependencies.
	for (auto& info : inherits)
	{
		for (auto& pchDep : info->Dependencies)
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

Platform::Path Toolchain::FindLibraryPath(const Platform::Path& lib)
{
	if (lib.IsAbsolute())
	{
		return lib;
	}

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

void Toolchain::PrintMessages(BuilderFileInfo& file)
{
	for (auto msg : file.Messages)
	{
		std::string typeString = "";

		switch (msg.Type)
		{
		case EBuilderFileMessageType::Info:
			{
				typeString = "message";
				break;
			}
		case EBuilderFileMessageType::Warning:
			{
				typeString = "warning";
				break;
			}
		case EBuilderFileMessageType::Error:
			{
				typeString = "error";
				break;
			}
		}

		std::string message;
		message += msg.Origin.ToString();
		if (msg.Line != 0 || msg.Column != 0)
		{
			message += "(";
			message += CastToString(msg.Line);
			message += ",";
			message += CastToString(msg.Column); 
			message += ")";
		}
		message += ": ";
		message += typeString;
		if (msg.Identifier.size() > 0)
		{
				message += " ";
				message += msg.Identifier;
		}
		message += ": ";
		message += msg.Text.c_str();

		Log(LogSeverity::SilentInfo, "%s\n", message.c_str());
		//printf("%s\n", message.c_str());
	}
}

bool Toolchain::ParseOutput(BuilderFileInfo& file, std::string& output)
{
	// Nothing to do here.

	MB_UNUSED_PARAMETER(file);
	MB_UNUSED_PARAMETER(output);

	return true;
}

void Toolchain::GetBaseCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args)
{
	MB_UNUSED_PARAMETER(file);
	MB_UNUSED_PARAMETER(args);
}
	
void Toolchain::GetPchCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args)
{
	MB_UNUSED_PARAMETER(file);
	MB_UNUSED_PARAMETER(args);
}

void Toolchain::GetSourceCompileArguments(const BuilderFileInfo& file, std::vector<std::string>& args)
{
	MB_UNUSED_PARAMETER(file);
	MB_UNUSED_PARAMETER(args);
}
	
void Toolchain::GetLinkArguments(const std::vector<BuilderFileInfo>& sourceFiles, std::vector<std::string>& args)
{
	MB_UNUSED_PARAMETER(sourceFiles);
	MB_UNUSED_PARAMETER(args);
}

void Toolchain::GetArchiveArguments(const std::vector<BuilderFileInfo>& sourceFiles, std::vector<std::string>& args)
{
	MB_UNUSED_PARAMETER(sourceFiles);
	MB_UNUSED_PARAMETER(args);
}

bool Toolchain::CompilePch(BuilderFileInfo& fileInfo) 
{
	std::vector<std::string> arguments;
	GetBaseCompileArguments(fileInfo, arguments);
	GetPchCompileArguments(fileInfo, arguments);

	Platform::Process process;
	if (!process.Open(m_compilerPath, m_compilerPath.GetDirectory(), arguments, true))
	{
		return false;
	}

	std::string output = process.ReadToEnd();
	if (!ParseOutput(fileInfo, output))
	{
		return false;
	}
	if (LogGetVerbose())
	{
		printf("%s", output.c_str());
	}
	PrintMessages(fileInfo);
	if (fileInfo.ErrorCount > 0 || (fileInfo.WarningCount > 0 && m_projectFile.Get_Flags_CompilerWarningsFatal()))
	{
		return false;
	}
	if (process.GetExitCode() != 0)
	{
		return false;
	}

	std::vector<BuilderFileInfo*> inheritsFromFiles;
	UpdateDependencyManifest(fileInfo, fileInfo.OutputDependencyPaths, inheritsFromFiles);

	return true;
}

bool Toolchain::Compile(BuilderFileInfo& fileInfo, BuilderFileInfo& pchFileInfo)
{
	std::vector<std::string> arguments;
	GetBaseCompileArguments(fileInfo, arguments);
	GetSourceCompileArguments(fileInfo, arguments);
//	arguments.push_back("-v");
	
	Platform::Process process;	
	if (!process.Open(m_compilerPath, m_compilerPath.GetDirectory(), arguments, true))
	{
		return false;
	}
	
	std::string output = process.ReadToEnd();
	if (!ParseOutput(fileInfo, output))
	{
		return false;
	}
	if (LogGetVerbose())
	{
		printf("%s", output.c_str());
	}
	PrintMessages(fileInfo);
	if (fileInfo.ErrorCount > 0 || (fileInfo.WarningCount > 0 && m_projectFile.Get_Flags_CompilerWarningsFatal()))
	{
		return false;
	}
	if (process.GetExitCode() != 0)
	{
		return false;
	}

	std::vector<BuilderFileInfo*> inheritsFromFiles;
	inheritsFromFiles.push_back(&pchFileInfo);
	UpdateDependencyManifest(fileInfo, fileInfo.OutputDependencyPaths, inheritsFromFiles);

	return true;
}


bool Toolchain::CompileVersionInfo(BuilderFileInfo& fileInfo, VersionNumberInfo versionInfo)
{
	MB_UNUSED_PARAMETER(fileInfo);
	MB_UNUSED_PARAMETER(versionInfo);

	// Not implemented in most toolchains, mainly for windows icons/version-info.
	return false;
}

bool Toolchain::Archive(std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile)
{
	std::vector<std::string> arguments;
	GetArchiveArguments(files, arguments);

	Platform::Process process;
	Platform::Path responseFilePath = outputFile.ManifestPath.AppendFragment(".rsp", false);
	if (!OpenResponseFileProcess(process, responseFilePath, m_archiverPath, m_archiverPath.GetDirectory(), arguments, true))
	{
		return false;
	}
	
	outputFile.Dependencies.clear();
	
	std::string output = process.ReadToEnd();
	if (!ParseOutput(outputFile, output))
	{
		return false;
	}
	if (LogGetVerbose())
	{
		printf("%s", output.c_str());
	}
	PrintMessages(outputFile);
	if (outputFile.ErrorCount > 0 || (outputFile.WarningCount > 0 && m_projectFile.Get_Flags_LinkerWarningsFatal()))
	{
		return false;
	}
	if (process.GetExitCode() != 0)
	{
		return false;
	}

	if (m_bRequiresCompileStep)
	{
		// Object files to link.
		for (auto& sourceFile : files)
		{
			BuilderDependencyInfo info;
			info.SourcePath = sourceFile.OutputPath;
			info.Hash = BuilderFileInfo::CalculateFileHash(info.SourcePath, m_configurationHash);
			outputFile.Dependencies.push_back(info);
		}
	
		// Link PCH.
		if (!m_projectFile.Get_Build_PrecompiledHeader().IsEmpty())
		{
			Platform::Path pchObjectPath = m_projectFile.Get_Project_IntermediateDirectory()
				.AppendFragment(m_projectFile.Get_Build_PrecompiledHeader().ChangeExtension("o").GetFilename(), true);

			BuilderDependencyInfo info;
			info.SourcePath = pchObjectPath;
			info.Hash = BuilderFileInfo::CalculateFileHash(info.SourcePath, m_configurationHash);
			outputFile.Dependencies.push_back(info);
		}
	}

	// Depndent on every input source file if no compile step.
	else
	{
		for (auto& sourceFile : files)
		{
			BuilderDependencyInfo info;
			info.SourcePath = sourceFile.SourcePath;
			info.Hash = BuilderFileInfo::CalculateFileHash(info.SourcePath, m_configurationHash);
			outputFile.Dependencies.push_back(info);
		}	
	}

	outputFile.StoreManifest();

	return true;
}

void Toolchain::UpdateLinkDependencies(std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile)
{
	outputFile.Dependencies.clear();

	// Libraries to link.
	for (auto& library : m_projectFile.Get_Libraries_Library())
	{
		Platform::Path fullPath = FindLibraryPath(library);

		// Can't do dependency tracking on this library as we don't know where it is!
		if (fullPath.IsAbsolute())
		{
			BuilderDependencyInfo info;
			info.SourcePath = fullPath;
			info.Hash = BuilderFileInfo::CalculateFileHash(fullPath, m_configurationHash);
			outputFile.Dependencies.push_back(info);
		}
		else if (library.IsAbsolute())
		{
			Log(LogSeverity::Warning, "Could not find library '%s', dependency building will not function correctly.\n", fullPath.ToString().c_str());
		}
	}

	// Object files to link.
	if (m_bRequiresCompileStep)
	{
		for (auto& sourceFile : files)
		{
			BuilderDependencyInfo info;
			info.SourcePath = sourceFile.OutputPath;
			info.Hash = BuilderFileInfo::CalculateFileHash(info.SourcePath, m_configurationHash);
			outputFile.Dependencies.push_back(info);
		}

		// Link PCH.
		if (!m_projectFile.Get_Build_PrecompiledHeader().IsEmpty())
		{
			Platform::Path pchObjectPath = m_projectFile.Get_Project_IntermediateDirectory()
				.AppendFragment(m_projectFile.Get_Build_PrecompiledHeader().ChangeExtension("o").GetFilename(), true);

			BuilderDependencyInfo info;
			info.SourcePath = pchObjectPath;
			info.Hash = BuilderFileInfo::CalculateFileHash(info.SourcePath, m_configurationHash);
			outputFile.Dependencies.push_back(info);
		}

		// Link version info as well if required.
		if (RequiresVersionInfo())
		{
			Platform::Path versionInfoPath = m_projectFile.Get_Project_IntermediateDirectory()
				.AppendFragment(Strings::Format("%s_VersionInfo.generated.o", m_projectFile.Get_Project_Name().c_str()), true);

			BuilderDependencyInfo info;
			info.SourcePath = versionInfoPath;
			info.Hash = BuilderFileInfo::CalculateFileHash(info.SourcePath, m_configurationHash);
			outputFile.Dependencies.push_back(info);
		}
	}

	// Depndent on every input source file if no compile step.
	else
	{
		for (auto& sourceFile : files)
		{
			BuilderDependencyInfo info;
			info.SourcePath = sourceFile.SourcePath;
			info.Hash = BuilderFileInfo::CalculateFileHash(info.SourcePath, m_configurationHash);
			outputFile.Dependencies.push_back(info);
		}	
	}	
}

bool Toolchain::Link(std::vector<BuilderFileInfo>& files, BuilderFileInfo& outputFile)
{
	std::vector<std::string> arguments;
	GetLinkArguments(files, arguments);

//	for (size_t i = 0; i < arguments.size(); i++)
//	{
//		Log(LogSeverity::Warning, "[%i] %s\n", i, arguments[i].c_str());
//	}	

	Platform::Process process;
	Platform::Path responseFilePath = outputFile.ManifestPath.AppendFragment(".rsp", false);
	if (!OpenResponseFileProcess(process, responseFilePath, m_linkerPath, m_linkerPath.GetDirectory(), arguments, true))
	{
		return false;
	}
	
	std::string output = process.ReadToEnd();
	if (!ParseOutput(outputFile, output))
	{
		return false;
	}
	if (LogGetVerbose())
	{
		printf("%s", output.c_str());
	}
	PrintMessages(outputFile);
	if (outputFile.ErrorCount > 0 || (outputFile.WarningCount > 0 && m_projectFile.Get_Flags_LinkerWarningsFatal()))
	{
		return false;
	}
	if (process.GetExitCode() != 0)
	{
		return false;
	}
	
	UpdateLinkDependencies(files, outputFile);
	outputFile.StoreManifest();

	return true;
}

bool Toolchain::OpenResponseFileProcess(Platform::Process& process, const Platform::Path& responseFilePath, const Platform::Path& exePath, const Platform::Path& workingDir, const std::vector<std::string>& arguments, bool bRedirectStdout)
{
	std::string data = Strings::Join(arguments, "\n");

	if (!Strings::WriteFile(responseFilePath, data))
	{
		return false;
	}

	return process.Open(exePath, workingDir, { "@" + responseFilePath.ToString() }, bRedirectStdout);
}

Platform::Path Toolchain::GetOutputPath()
{
	return m_projectFile.Get_Project_OutputDirectory()
		.AppendFragment(Strings::Format("%s%s", m_projectFile.Get_Project_OutputName().c_str(), m_projectFile.Get_Project_OutputExtension().c_str()), true);
}

Platform::Path Toolchain::GetPchPath()
{
	return m_projectFile.Get_Project_IntermediateDirectory()
		.AppendFragment(m_projectFile.Get_Build_PrecompiledHeader().AppendFragment(".gch", false).GetFilename(), true);
}

Platform::Path Toolchain::GetPchObjectPath()
{
	if (m_bGeneratesPchObject)
	{
		return m_projectFile.Get_Project_IntermediateDirectory()
			.AppendFragment(m_projectFile.Get_Build_PrecompiledHeader().ChangeExtension("o").GetFilename(), true);
	}
	else
	{
		return "";
	}
}

Platform::Path Toolchain::GetVersionInfoObjectPath()
{
	return m_projectFile.Get_Project_IntermediateDirectory()
		.AppendFragment(Strings::Format("%s_VersionInfo.generated.o", m_projectFile.Get_Project_Name().c_str()), true);
}

Platform::Path Toolchain::GetPdbPath()
{
	return m_projectFile.Get_Project_IntermediateDirectory()
		.AppendFragment(Strings::Format("%s%s", m_projectFile.Get_Project_OutputName().c_str(), ".pdb"), true);
}

Platform::Path Toolchain::GetOutputPdbPath()
{
	return m_projectFile.Get_Project_OutputDirectory()
		.AppendFragment(Strings::Format("%s%s", m_projectFile.Get_Project_OutputName().c_str(), ".pdb"), true);
}

}; // namespace MicroBuild
