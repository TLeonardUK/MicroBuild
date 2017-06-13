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

#include "App/Builder/Toolchains/ToolchainOutputParser.h"

#include <regex>

namespace MicroBuild {

// Output parser for all nintendo wiiu toolchain tools.
class Toolchain_NintendoWiiUOutputParser
	: public ToolchainOutputParser
{
protected:

public:
	Toolchain_NintendoWiiUOutputParser();

	// Can be overridden in derived parses to perform validity tests, handy for debugging.
	virtual void Test() override;

}; 

}; // namespace MicroBuild
