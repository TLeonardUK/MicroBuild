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

#include "Core/Commands/CommandArgument.h"
#include "Core/Platform/Path.h"

namespace MicroBuild {

// Defines an argument that has to be set to a path value and is validated
// to make sure its a valid path.
class CommandPathArgument
	: public CommandArgument<Platform::Path>
{
public:
	CommandPathArgument();
	virtual ~CommandPathArgument();

	// Gets/Sets if the path expected should be a path to a directory rather 
	// than a file.
	bool GetExpectsDirectory();
	void SetExpectsDirectory(bool value);

	// Gets/Sets if the path should already exist.
	bool GetExpectsExisting();
	void SetExpectsExisting(bool value);

protected:
	virtual bool ValidateAndSet(std::string value) override;

private:
	bool m_expectsDirectory;
	bool m_expectsExisting;

};

}; // namespace MicroBuild