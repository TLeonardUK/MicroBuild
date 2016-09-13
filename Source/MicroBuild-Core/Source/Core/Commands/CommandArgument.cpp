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
#include "Core/Commands/CommandArgument.h"

namespace MicroBuild {

CommandArgumentBase::CommandArgumentBase()
	: m_name("")
	, m_shortName("")
	, m_description("")
	, m_required(false)
	, m_positional(false)
{
}

CommandArgumentBase::~CommandArgumentBase()
{
}

std::string CommandArgumentBase::GetName()
{
	return m_name;
}

void CommandArgumentBase::SetName(std::string value)
{
	m_name = value;
}

std::string CommandArgumentBase::GetShortName()
{
	return m_shortName;
}

void CommandArgumentBase::SetShortName(std::string value)
{
	m_shortName = value;
}

std::string CommandArgumentBase::GetDescription()
{
	return m_description;
}

void CommandArgumentBase::SetDescription(std::string value)
{
	m_description = value;
}

bool CommandArgumentBase::GetRequired()
{
	return m_required;
}

void CommandArgumentBase::SetRequired(bool value)
{
	m_required = value;
}

bool CommandArgumentBase::GetPositional()
{
	return m_positional;
}

void CommandArgumentBase::SetPositional(bool value)
{
	m_positional = value;
}

std::string CommandArgumentBase::ToString()
{
	return GetName();
}

std::string CommandArgumentBase::GetExampleString(bool bWithDescription)
{
	std::string result = "";

	if (bWithDescription)
	{
		if (!m_positional)
		{
			result += "-" + GetShortName();
			result += " | ";
			result += "--" + GetName();
		}
		else
		{
			result += "-" + GetShortName();
			result += "=" + ToString();
			result += " | ";
			result += "--" + GetName();
			result += "=" + ToString();
		}

		while (result.size() < ExampleStringPadding)
		{
			result.push_back(' ');
		}

		result += GetDescription();
	}
	else
	{
		if (m_required)
		{
			result += "<";
		}
		else
		{
			result += "[";
		}

		if (m_positional)
		{
			result += ToString().c_str();
		}
		else
		{
			result += "-";
			result += GetShortName().c_str();
			result += "=";
			result += ToString().c_str();
		}

		if (m_required)
		{
			result += ">";
		}
		else
		{
			result += "]";
		}
	}

	return result;
}


}; // namespace MicroBuild
