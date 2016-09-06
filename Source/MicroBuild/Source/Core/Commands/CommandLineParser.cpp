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
#include "Core/Commands/CommandLineParser.h"
#include "Core/Commands/CommandArgument.h"
#include "Core/Commands/Command.h"
#include "Core/Helpers/Strings.h"

namespace MicroBuild {

CommandLineParser::CommandLineParser(
	const char* appName,
	const char* appDescription,
	const char* appCopyright)
	: m_appName(appName)
	, m_appDescription(appDescription)
	, m_appCopyright(appCopyright)
{
}

CommandLineParser::~CommandLineParser()
{
	for (Command* cmd : m_commands)
	{
		delete cmd;
	}
	m_commands.clear();
}

bool CommandLineParser::Parse(int argc, char* argv[])
{
	PrintLicense();

	if (argc > 1)
	{
		// Parse each command individually.
		for (int i = 1; i < argc; i++)
		{
			const char* commandName = argv[i];

			// Skip the - and -- at the start of the command name.
			if (strcmp(commandName, "-") == 0)
			{
				commandName++;
			}
			if (strcmp(commandName, "-") == 0)
			{
				commandName++;
			}

			PendingCommand pendingCommand;
			pendingCommand.command = FindCommand(commandName);

			if (pendingCommand.command == nullptr)
			{
				Log(LogSeverity::Fatal, "Unknown command '%s'\n", commandName);
				return false;
			}

			// Eat up any arguments.
			i++;

			int argPlacementIndex = 0;
			while (i < argc)
			{
				std::string argumentName = argv[i];
				std::string argumentValue;

				// Hard-coded options, bleh.
				if (argumentName == "-v" ||
					argumentName == "--verbose")
				{
					LogSetVerbose(true);
					i++;
					continue;
				}

				// Command seperator.
				if (argumentName == ";")
				{
					break;
				}
				else
				{
					// Check if there is an equals sign somewhere.
					size_t equal_offset = argumentName.find('=');
					if (equal_offset != std::string::npos)
					{
						argumentValue = argumentName.substr(equal_offset + 1);
						argumentName = argumentName.substr(0, equal_offset);
					}
				}

				// If no value is provided, see if its placement or not.
				if (argumentValue.empty())
				{
					if (argumentName[0] != '-')
					{
						argumentValue = argumentName;
						argumentName.clear();
					}
					else
					{
						argumentValue = "1";
					}
				}

				// Strip off the "-" or "--" from the name if required.
				if (argumentName.size() >= 2 && 
					argumentName.substr(0, 2) == "--")
				{
					argumentName = argumentName.substr(2);
				}
				else if (argumentName.size() >= 1 &&
						 argumentName.substr(0, 1) == "-")
				{
					argumentName = argumentName.substr(1);
				}

				// Argument exists?
				CommandArgumentBase* argument = nullptr;

				if (!argumentName.empty())
				{
					argument = pendingCommand.command->FindArgument(
						argumentName.c_str());
				}
				else
				{
					argument = pendingCommand.command->FindPositionalArgument(
						argPlacementIndex);
				}

				if (argument == nullptr)
				{
					if (argumentName.empty())
					{
						Log(LogSeverity::Fatal, 
							"More unnamed arguments than expected provided "
							"for command '%s'\n",
							commandName);
					}
					else
					{
						Log(LogSeverity::Fatal,
							"Unknown argument '%s' for command '%s'\n",
							argumentName.c_str(), commandName);
					}

					return false;
				}

				// Store for later.
				std::pair<CommandArgumentBase*, std::string> argValue;
				argValue.first = argument;
				argValue.second = argumentValue;
				pendingCommand.arguments.push_back(argValue);
				
				argPlacementIndex++;
				i++;
			}

			// Check we have all required arguments.
			for (CommandArgumentBase* base : 
				pendingCommand.command->m_arguments)
			{
				if (base->GetRequired())
				{
					bool bExists = false;

					for (std::pair<CommandArgumentBase*, std::string>& arg : 
						pendingCommand.arguments)
					{
						if (arg.first == base)
						{
							bExists = true;
							break;
						}
					}

					if (!bExists)
					{
						Log(LogSeverity::Fatal,
							"Required argument '%s' for command '%s' not provided.\n",
							base->GetName().c_str(), commandName);
						return false;
					}
				}
			}

			// Add the command to the start.
			m_pendingCommands.push_back(pendingCommand);
		}
	}

	return true;
}

bool CommandLineParser::HasCommands()
{
	return m_pendingCommands.size() > 0;
}

bool CommandLineParser::DispatchCommands()
{
	for (PendingCommand& cmd : m_pendingCommands)
	{
		for (CommandArgumentBase* arg : cmd.command->m_arguments)
		{
			arg->SetDefault();
		}

		for (std::pair<CommandArgumentBase*, std::string>& arg : cmd.arguments)
		{
			if (!arg.first->ValidateAndSet(arg.second))
			{
				return false;
			}
		}

		if (!cmd.command->Invoke(this))
		{
			return false;
		}
	}
	return true;
}

void CommandLineParser::RegisterCommand(Command* command)
{
	m_commands.push_back(command);
}

Command* CommandLineParser::FindCommand(const char* name)
{
	for (Command* cmd : m_commands)
	{
		if (Strings::CaseInsensitiveEquals(cmd->GetName(), name) ||
			Strings::CaseInsensitiveEquals(cmd->GetShortName(), name))
		{
			return cmd;
		}
	}
	return nullptr;
}

void CommandLineParser::PrintLicense()
{
	Log(LogSeverity::Info, "%s\n", m_appName);
	Log(LogSeverity::Info, "%s\n", m_appCopyright);
	Log(LogSeverity::Info, "This program comes with ABSOLUTELY NO WARRANTY.\n");
	Log(LogSeverity::Info, "This is free software, and you are welcome to redistribute it\n");
	Log(LogSeverity::Info, "under certain conditions; read LICENSE file for details.\n");
	Log(LogSeverity::Info, "\n");
}

void CommandLineParser::PrintHelp()
{
	Log(LogSeverity::Info, "Usage:\n");

	for (Command* Cmd : m_commands)
	{
		Log(LogSeverity::Info, "\t%s %s\n", 
			m_appName, 
			Cmd->GetExampleString().c_str());
		Log(LogSeverity::Info, "\t%s\n\n",
			Cmd->GetDescription().c_str());
	}

	Log(LogSeverity::Info, "Options:\n");

	std::vector<std::string> shownArgs;

	for (Command* Cmd : m_commands)
	{
		for (CommandArgumentBase* Arg : Cmd->m_arguments)
		{
			if (!Arg->GetPositional())
			{
				if (std::find(shownArgs.begin(), shownArgs.end(), Arg->GetName())
					== shownArgs.end())
				{
					Log(LogSeverity::Info, "\t%s\n", 
						Arg->GetExampleString(true).c_str());

					shownArgs.push_back(Arg->GetName());
				}
			}
		}		
	}

	std::string verboseExampleString = "-v | --verbose";
	while (verboseExampleString.size() < CommandArgumentBase::ExampleStringPadding)
	{
		verboseExampleString.push_back(' ');
	}

	Log(LogSeverity::Info, "\t%s\n",
		verboseExampleString.c_str());
}

}; // namespace MicroBuild
