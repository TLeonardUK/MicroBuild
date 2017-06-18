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
#include "App/Builder/Accelerators/Sndbs/Accelerator_Sndbs.h"

#include "App/Builder/Tasks/BuildTask.h"

#include "Core/Helpers/Strings.h"
#include "Core/Helpers/JsonNode.h"

#include "Core/Platform/Process.h"
#include "Core/Platform/Platform.h"

#include <cstdlib>
#include <cassert>

namespace MicroBuild {

Accelerator_Sndbs::Accelerator_Sndbs()
{
}

bool Accelerator_Sndbs::Init()
{
	Platform::Path sceRootDir = Platform::GetEnvironmentVariable("SCE_ROOT_DIR");
	if (!sceRootDir.Exists())
	{
		return false;
	}

	Platform::Path sndbsRoot = sceRootDir.AppendFragment("Common/SN-DBS", true);
	if (!sndbsRoot.Exists())
	{
		return false;
	}

	m_dbsBuildPath = sndbsRoot.AppendFragment("bin/dbsbuild.exe", true);
	if (!m_dbsBuildPath.Exists())
	{
		return false;
	}

	Platform::Process process;

	std::vector<std::string> args;
	args.push_back("--version");
	if (!process.Open(m_dbsBuildPath, m_dbsBuildPath.GetDirectory(), args, true))
	{
		return false;
	}

	std::string output = process.ReadToEnd();
	std::vector<std::string> split = Strings::Split(' ', output);
	if (split.size() > 2)
	{
		m_description = Strings::Format("SN-DBS (Version %s)", split[1].c_str());
	}
	else
	{
		m_description = "SN-DBS (Unknown version)";

	}

	m_bAvailable = true;

	return true;
}

bool Accelerator_Sndbs::RunActions(Toolchain* toolchain, BuildTask* baseTask, ProjectFile& projectFile, std::vector<BuildAction>& actions)
{
	MB_UNUSED_PARAMETER(toolchain);

	// Write all action command lines to bat script.
	Platform::Path scriptPath = projectFile.Get_Project_IntermediateDirectory()
		.AppendFragment(Strings::Format("%s.sndbs.json", projectFile.Get_Project_Name().c_str()), true);

	std::string deliminator = Strings::Format("[sndbs-output-%s] job=", Strings::Guid({ projectFile.Get_Project_Name() }).c_str());
	std::string dbsOutputPrefix = "dbsbuild:";

	JsonNode root;
	JsonNode& jobsRoot = root.Node("jobs");

	int jobIndex = 0;
	for (BuildAction& action : actions)
	{
		// Add command.
		std::string command = Strings::Format("\"%s\" %s", action.Tool.ToString().c_str(), Strings::Join(action.Arguments, " ").c_str());
		JsonNode& commandRoot = jobsRoot.Node("job%i", jobIndex);
		commandRoot.Node("command").Value("%s", Strings::EscapeSlashes(command).c_str());
		commandRoot.Node("echo").Value("%s%i", deliminator.c_str(), jobIndex);

		jobIndex++;
	}

	std::string scriptData = root.ToString(); 

	// Write file.
	if (!Strings::WriteFile(scriptPath, scriptData))
	{
		Log(LogSeverity::Fatal, "Failed to write script file '%s'.", scriptPath.ToString().c_str());
		return false;
	}

	// Run script.
	Platform::Process process;

	std::vector<std::string> buildArguments;
	//buildArguments.push_back("-v");
	buildArguments.push_back("-q");
	buildArguments.push_back("-p");
	buildArguments.push_back(projectFile.Get_Project_Name());
	buildArguments.push_back("-s");
	buildArguments.push_back(scriptPath.ToString());

	if (!process.Open(m_dbsBuildPath, scriptPath.GetDirectory(), buildArguments, true))
	{
		Log(LogSeverity::Fatal, "Failed to execute sndbs command.");
		return false;
	}

	// Read output progressively so we can keep our output up to date.
	std::string currentActionOutput = "";
	int currentOutputIndex = -1;

	bool bCompletedSuccessfully = true;

	auto consumeCurrentInput = [&]()
	{
		if (currentOutputIndex >= 0)
		{
			BuildAction& currentAction = actions[currentOutputIndex];
			currentAction.Output = currentActionOutput;
			currentAction.ExitCode = 0;

			currentActionOutput = "";
			currentOutputIndex++;

			if (currentAction.PostProcessDelegate)
			{
				if (!currentAction.PostProcessDelegate(currentAction))
				{
					bCompletedSuccessfully = false;

					// note: don't break, we still want to get the output from the other files first.
				}
			}
		}
	};

	while (!process.AtEnd())
	{
		std::string line = process.ReadLine();
		if (line.size() >= deliminator.size() && line.substr(0, deliminator.size()) == deliminator)
		{
			consumeCurrentInput();
			
			currentOutputIndex = CastFromString<int>(line.substr(deliminator.size()));

			// Try and show progress by printing out the action who's output we recieved last.
			// This only shows progress after the work has been done but this seems preferable to just dumping
			// out the status messages of every task up front :S.
			if (currentOutputIndex >= 0)
			{
				BuildAction& action = actions[currentOutputIndex];
				if (!action.StatusMessage.empty())
				{
					baseTask->TaskLog(LogSeverity::SilentInfo, currentOutputIndex + 1, "%s", action.StatusMessage.c_str());
				}
			}
		}
		else 
		{
			// Check if error matches an sn-dbs format error.
			if (line.size() >= dbsOutputPrefix.size() && line.substr(0, dbsOutputPrefix.size()) == dbsOutputPrefix)
			{
				// Not sure what we should do with these messages? Should we add them as build errors? Or just print them out?
				Log(LogSeverity::Warning, "%s\n", line.c_str());
			}

			// Otherwise its just output.
			else
			{
				currentActionOutput += line + "\n";
			}
		}
	}

	if (currentActionOutput.size() > 0)
	{
		consumeCurrentInput();
	}

	return bCompletedSuccessfully;
}

}; // namespace MicroBuild
