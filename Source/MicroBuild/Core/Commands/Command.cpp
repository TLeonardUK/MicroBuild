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
#include "Core/Commands/Command.h"
#include "Core/Commands/CommandArgument.h"
#include "Core/Helpers/Strings.h"

namespace MicroBuild {

Command::Command()
{
}

Command::~Command()
{
}

void Command::SetName(std::string name)
{
	m_name = name;
}

std::string Command::GetName()
{
	return m_name;
}

void Command::SetShortName(std::string name)
{
	m_shortName = name;
}

std::string Command::GetShortName()
{
	return m_shortName;
}

void Command::SetDescription(std::string name)
{
	m_description = name;
}

std::string Command::GetDescription()
{
	return m_description;
}

std::string Command::GetExampleString()
{
	std::string result = m_name;
	if (!m_shortName.empty())
	{
		result += "|";
		result += "-";
		result += m_shortName;
	}

	// Add positional arguments.
	for (CommandArgumentBase* arg : m_arguments)
	{
		if (arg->GetPositional())
		{
			result += " " + arg->GetExampleString();
		}
	}

	// Add named arguments.
	for (CommandArgumentBase* arg : m_arguments)
	{
		if (!arg->GetPositional())
		{
			result += " " + arg->GetExampleString();
		}
	}

	return result;
}

void Command::RegisterArgument(CommandArgumentBase* argument)
{
	m_arguments.push_back(argument);
}

CommandArgumentBase* Command::FindArgument(const char* name)
{
	for (CommandArgumentBase* arg : m_arguments)
	{
		if (Strings::CaseInsensitiveEquals(arg->GetName(), name) ||
			Strings::CaseInsensitiveEquals(arg->GetShortName(), name))
		{
			return arg;
		}
	}
	return nullptr;
}

CommandArgumentBase* Command::FindPositionalArgument(int index)
{
	int positionIndex = 0;
	for (CommandArgumentBase* arg : m_arguments)
	{
		if (arg->GetPositional())
		{
			if (positionIndex == index)
			{
				return arg;
			}
			positionIndex++;
		}
	}
	return nullptr;
}

}; // namespace MicroBuild
