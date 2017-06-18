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
#include "App/Builder/Accelerators/IncrediBuild/Accelerator_IncrediBuild.h"

#include "App/Builder/Toolchains/Cpp/Microsoft/Toolchain_Microsoft.h"

#include "App/Builder/Tasks/BuildTask.h"

#include "Core/Helpers/Strings.h"
#include "Core/Helpers/XmlNode.h"

#include "Core/Platform/Process.h"
#include "Core/Platform/Platform.h"

namespace MicroBuild {

Accelerator_IncrediBuild::Accelerator_IncrediBuild()
{
}

bool Accelerator_IncrediBuild::Init()
{
	if (!Platform::Path::FindFile("xgConsole.exe", m_xgConsolePath))
	{
		return false;
	}

	Platform::Process process;

	std::vector<std::string> args;
	if (!process.Open(m_xgConsolePath, m_xgConsolePath.GetDirectory(), args, true))
	{
		return false;
	}

	std::string output = process.ReadToEnd();
	std::vector<std::string> split = Strings::Split('\n', output);
	if (split.size() > 2)
	{
		m_description = Strings::Format("IncrediBuild (Version %s)", split[2].c_str());
	}
	else
	{
		m_description = "IncrediBuild (Unknown version)";

	}

	m_bAvailable = true;

	return true;
}

bool Accelerator_IncrediBuild::RunActions(Toolchain* toolchain, BuildTask* baseTask, ProjectFile& projectFile, std::vector<BuildAction>& actions)
{
	// Write all action command lines to bat script.
	Platform::Path scriptPath = projectFile.Get_Project_IntermediateDirectory()
		.AppendFragment(Strings::Format("%s.xge.xml", projectFile.Get_Project_Name().c_str()), true);

	std::string deliminator = Strings::Format("[xge-output-%s] job=", Strings::Guid({ projectFile.Get_Project_Name() }).c_str());
	std::string projectOutputPrefix = "--------------------Project:";
	std::string finishedOutputPrefix = "---------------------- Done ----------------------";

	XmlNode root;

	XmlNode& buildSetNode = root.Node("BuildSet");
	buildSetNode.Attribute("FormatVersion", "1");

	XmlNode& environmentsNode = buildSetNode.Node("Environments");

	XmlNode& defaultEnvironmentNode = environmentsNode.Node("Environment");
	defaultEnvironmentNode.Attribute("Name", "Default");

	XmlNode& toolsNode = defaultEnvironmentNode.Node("Tools");

	/*XmlNode& varsNode =*/ defaultEnvironmentNode.Node("Variables");

	int jobIndex = 0;
	for (BuildAction& action : actions)
	{
		XmlNode& toolNode = toolsNode.Node("Tool");
		toolNode.Attribute("Name", "Tool%i", jobIndex);
		toolNode.Attribute("AllowRemote", "true");
		toolNode.Attribute("OutputPrefix", (deliminator + "%i").c_str(), jobIndex); 
		toolNode.Attribute("Params", Strings::Join(action.Arguments, " ").c_str());
		toolNode.Attribute("Path", action.Tool.ToString().c_str());
		toolNode.Attribute("SkipIfProjectFailed", "true");
		if (dynamic_cast<Toolchain_Microsoft*>(toolchain) != nullptr)
		{
			toolNode.Attribute("VCCompiler", "true");
		}
		jobIndex++;
	}
 
	XmlNode& projectNode = buildSetNode.Node("Project");
	projectNode.Attribute("Name", "Default");
	projectNode.Attribute("Env", "Default");

	jobIndex = 0;
	for (BuildAction& action : actions)
	{
		XmlNode& toolNode = projectNode.Node("Task");
		toolNode.Attribute("SourceFile", ""); 
		toolNode.Attribute("Caption", action.StatusMessage.c_str()); 
		toolNode.Attribute("Name", "Action%i", jobIndex);
		toolNode.Attribute("Tool", "Tool%i", jobIndex);
		toolNode.Attribute("WorkingDir", action.WorkingDirectory.ToString().c_str()); 
		toolNode.Attribute("SkipIfProjectFailed", "true");
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
	buildArguments.push_back(scriptPath.ToString());
	buildArguments.push_back("/Rebuild");
	buildArguments.push_back("/NoWait");
	//buildArguments.push_back("/StopOnErrors");
	buildArguments.push_back("/NoLogo");
	//buildArguments.push_back("/Silent");
	//buildArguments.push_back("/ShowTime");

	if (!process.Open(m_xgConsolePath, scriptPath.GetDirectory(), buildArguments, true))
	{
		Log(LogSeverity::Fatal, "Failed to execute incredibuild command.");
		return false;
	}

	// Read output progressively so we can keep our output up to date.
	bool bCompletedSuccessfully = true;

	std::string currentActionOutput = "";
	int currentOutputIndex = -1;

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
			// Ignore XGE output that we don't care about.
			if (line.size() >= projectOutputPrefix.size() && line.substr(0, projectOutputPrefix.size()) == projectOutputPrefix)
			{
				// Ignore
			}

			// Found finished tag?
			else if (line.size() >= finishedOutputPrefix.size() && line.substr(0, finishedOutputPrefix.size()) == finishedOutputPrefix)
			{
				break;
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
