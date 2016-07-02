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

// The base class for the templated CommandArgument.
class CommandArgumentBase
{
public:
	CommandArgumentBase();
	virtual ~CommandArgumentBase();

	// Gets/Sets the long name of the command. The long name can be used on
	// the command line like "-name=x"
	std::string GetName();
	void SetName(std::string value);

	// Gets/Sets the short name of the command. The short name is just a 
	// short hand version of the long name (eg. -v instead of -version) 
	std::string GetShortName();
	void SetShortName(std::string value);

	// Gets/Sets the description of this command, as show in the help command.
	std::string GetDescription();
	void SetDescription(std::string value);

	// Gets/Sets if this argument is required and the command will fail if 
	// its not provided.
	bool GetRequired();
	void SetRequired(bool value);

	// Gets/Sets if this argument is not referenced by its name (eg. "-name=x")
	// but is instead based on its position in the command line string. 
	// (eg. "command <x> <y>")
	bool GetPositional();
	void SetPositional(bool value);

	// Gets an example string for this command showing its use.
	virtual std::string GetExampleString(bool bWithDescription = false);

	// Gets a string representation of this argument, typically this is the
	// same as GetName except in situations like multiple-option arguments.
	virtual std::string ToString();

	enum 
	{
		// How much padding the printed example string should have between
		// the command and the description.
		ExampleStringPadding = 25
	};

protected:
	friend class CommandLineParser;

	// Validates the given value. If the value is valid the argument output
	// is set to it, otherwise the function returns false.
	virtual bool ValidateAndSet(std::string value) = 0;

	// Sets the argument output to the default value.
	virtual void SetDefault() = 0;

	std::string m_name;
	std::string m_shortName;
	std::string m_description;

	bool m_required;
	bool m_positional;

};

// Defines an argument a command takes. This class is the base class for the
// different argument types that perform different types of validation.
// The DataType template type defines what kind of data this command argument
// stores internally.
template <typename DataType>
class CommandArgument 
	: public CommandArgumentBase
{
public:
	CommandArgument(DataType defaultValue)
		: CommandArgumentBase()
		, m_output(nullptr)
		, m_default(defaultValue)
	{
	}

	virtual ~CommandArgument()
	{
	}

	// Gets/Sets the default value if not defined on the command line.
	DataType GetDefault()
	{
		return m_default;
	}

	void SetDefault(DataType value)
	{
		m_default = value;
	}

	// Gets/Sets the pointer to where the arguments final value is stored.
	DataType* GetOutput()
	{
		return m_output;
	}

	void SetOutput(DataType* value)
	{
		m_output = value;
	}

protected:

	virtual void SetDefault() override
	{
		*m_output = m_default;
	}

	DataType m_default;
	DataType* m_output;

};

}; // namespace MicroBuild