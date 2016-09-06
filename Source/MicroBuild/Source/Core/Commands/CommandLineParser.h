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

#pragma once

namespace MicroBuild {

class Command;
class CommandArgumentBase;

// Describes a command and its argument that has been parsed from the command
// line and is ready for dispatch.
struct PendingCommand
{
	Command* command;
	std::vector<std::pair<CommandArgumentBase*, std::string> > arguments;
};

// Parses commands passed on the command line, validates their input and
// dispatches the commands to the correct classes.
class CommandLineParser
{
public:
	CommandLineParser(
		const char* appName, 
		const char* appDescription, 
		const char* appCopyright);

	~CommandLineParser();

	// Parses raw input and queues commands to be dispatched with 
	// DispatchCommands. Returns true if successfully parsed.
	bool Parse(int argc, char* argv[]);
	
	// Returns true if the parser has commands queued to dispatch, this will
	// always be false until Parse is called with valid input.
	bool HasCommands();

	// Dispatches queued commands and returns true if they all report success.
	bool DispatchCommands();

	// Prints the command line help for this application.
	void PrintHelp();

	// Prints the license header for this application.
	void PrintLicense();

	// Registers a command that can be dispatched by this parser. Ownership
	// is passed to the parser which will delete the command on destruction.
	void RegisterCommand(Command* command);

protected:

	// Finds a registered command given its name.
	Command* FindCommand(const char* name);

private:
	const char* m_appName;
	const char* m_appDescription;
	const char* m_appCopyright;

	std::vector<PendingCommand> m_pendingCommands;
	std::vector<Command*> m_commands;

};

}; // namespace MicroBuild