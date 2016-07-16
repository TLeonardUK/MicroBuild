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
#include "Core/Commands/CommandPathArgument.h"
#include "Core/Platform/Platform.h"
#include "Core/Platform/Path.h"

namespace MicroBuild {

CommandPathArgument::CommandPathArgument()
	: CommandArgument("")
	, m_expectsDirectory(false)
	, m_expectsExisting(false)
{
}

CommandPathArgument::~CommandPathArgument()
{
}

bool CommandPathArgument::GetExpectsDirectory()
{
	return m_expectsDirectory;
}

void CommandPathArgument::SetExpectsDirectory(bool value)
{
	m_expectsDirectory = value;
}

bool CommandPathArgument::GetExpectsExisting()
{
	return m_expectsExisting;
}

void CommandPathArgument::SetExpectsExisting(bool value)
{
	m_expectsExisting = value;
}

bool CommandPathArgument::ValidateAndSet(std::string value)
{
	Platform::Path basePath(value);

	if (basePath.IsRelative())
	{
		basePath = Platform::Path::GetWorkingDirectory().AppendFragment(value, true);
	}

	if (m_expectsExisting)
	{
		if (!basePath.Exists())
		{
			Log(LogSeverity::Fatal,
				"Path '%s', for argument '%s', does not exists.\n",
				value.c_str(), GetName().c_str());

			return false;
		}
	}

	if (m_expectsDirectory)
	{
		if (!basePath.IsDirectory())
		{
			Log(LogSeverity::Fatal,
				"Path '%s', for argument '%s', is not a directory.\n",
				value.c_str(), GetName().c_str());

			return false;
		}

	}

	*m_output = basePath;

	return true;
}

}; // namespace MicroBuild