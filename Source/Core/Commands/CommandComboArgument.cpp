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

#include "PCH.h"
#include "Core/Commands/CommandComboArgument.h"
#include "Core/Helpers/Strings.h"


namespace MicroBuild {

CommandComboArgument::CommandComboArgument()
	: CommandArgument("")
{
}

CommandComboArgument::~CommandComboArgument()
{
}

std::vector<std::string> CommandComboArgument::GetOptions()
{
	return m_options;
}

void CommandComboArgument::SetOptions(std::vector<std::string> value)
{
	m_options = value;
}

bool CommandComboArgument::ValidateAndSet(std::string value)
{
	bool bExists = false;

	for (std::string& opt : m_options)
	{
		if (Strings::CaseInsensitiveEquals(opt, value))
		{
			bExists = true;
			break;
		}
	}

	if (bExists)
	{
		return true;
	}
	else
	{
		Log(LogSeverity::Fatal,
			"Invalid value '%s' for argument '%s'.\n",
			value.c_str(), GetName().c_str());

		*m_output = value;

		return false;
	}
}

std::string CommandComboArgument::ToString()
{
	std::string result = "(";

	for (std::string& opt : m_options)
	{
		if (result.size() > 1)
		{
			result += "|";
		}
		result += opt;
	}

	result += ")";

	return result;
}

}; // namespace MicroBuild