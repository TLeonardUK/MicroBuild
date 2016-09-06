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

#include "App/Plugin/PluginInterface.h"

namespace MicroBuild {

// Version 1 of the plugin interfaces.
class PluginInterface1 : public IPluginInterface
{
public:
	// Gets or sets the name of the plugin.
	virtual void SetName(const std::string& value) = 0;
	virtual std::string GetName() const = 0;

	// Gets or sets the description of the plugin.
	virtual void SetDescription(const std::string& value) = 0;
	virtual std::string GetDescription() const = 0;

private:
	std::string m_name;
	std::string m_description;

};

}; // namespace MicroBuild