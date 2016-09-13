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

class CommandLineParser;
class CommandArgumentBase;

// Defines a command that can be dispatched by the CommandLineParser. This
// class is abstract and should be inherted to provide functionality.
class Command
{
public:
	Command();
	virtual ~Command();

	// Gets the name of this command, this is the verb you use to trigger
	// this command to run by command line.
	std::string GetName();

	// Gets the short name of this command, this is a shortened version of
	// the main name used as faster shorthand.
	std::string GetShortName();

	// Gets the description of this command, this is whats shown when using
	// the help command.
	std::string GetDescription();

	// Gets an example string for this command showing its use.
	std::string GetExampleString();

	// Find argument by given name. Comparison is done case insensitive.
	CommandArgumentBase* FindArgument(const char* name);

	// Find an argument given its position index rather than name.
	CommandArgumentBase* FindPositionalArgument(int index);

protected:
	friend class CommandLineParser;

	// Adds a argument that this command can take. Unnamed arguments are 
	// considered to be positional-dependent.
	// Ownership is passed to the command and will be disposed off when
	// destroyed.
	void RegisterArgument(CommandArgumentBase* argument);

	// Sets this commands name.
	void SetName(std::string name);

	// Sets this commands short name.
	void SetShortName(std::string name);

	// Sets this commands description.
	void SetDescription(std::string name);

	// Performs the actual logic of this command, arguments and options
	// should have been correctly validated and set before this is called.
	virtual bool Invoke(CommandLineParser* parser) = 0;

private:
	std::string m_name;
	std::string m_shortName;
	std::string m_description;
	std::vector<CommandArgumentBase*> m_arguments;
	
};

}; // namespace MicroBuild