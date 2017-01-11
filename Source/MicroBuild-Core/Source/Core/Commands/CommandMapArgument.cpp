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
#include "Core/Commands/CommandMapArgument.h"
#include "Core/Helpers/Strings.h"

namespace MicroBuild {

CommandMapArgument::CommandMapArgument()
	: CommandArgument()
{
}

CommandMapArgument::~CommandMapArgument()
{
}

bool CommandMapArgument::ValidateAndSet(std::string value)
{
	std::string key;
	std::string val;

	size_t offset = value.find_first_of('=');
	if (offset != std::string::npos)
	{
		Strings::SplitOnIndex(value, offset, key, val);
		(*m_output)[key] = val;

		return true;
	}

	return false;
}

}; // namespace MicroBuild
