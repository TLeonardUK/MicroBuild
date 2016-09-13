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

namespace MicroBuild {

// Defines an argument that has to be set to one of multiple pre-defined 
// options.
class CommandComboArgument 
	: public CommandArgument<std::string>
{
public:
	CommandComboArgument();
	virtual ~CommandComboArgument();

	// Gets/Sets the available options that the argument can be set to.
	std::vector<std::string> GetOptions();
	void SetOptions(std::vector<std::string> value);

	virtual std::string ToString() override;

protected:
	virtual bool ValidateAndSet(std::string value) override;


private:
	std::vector<std::string> m_options;

};

}; // namespace MicroBuild