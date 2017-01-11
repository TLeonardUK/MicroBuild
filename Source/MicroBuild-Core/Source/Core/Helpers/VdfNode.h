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

#include "Core/Platform/Path.h"

#include <sstream>

namespace MicroBuild {

// This class wraps an vdf-script node that can be serialized into a string.
class VdfNode
{
public:
	VdfNode(const VdfNode& node) = delete;
	VdfNode& operator=(const VdfNode&) = delete;

	VdfNode();
	~VdfNode();

	VdfNode& Node(const char* name, ...);
	VdfNode& Value(const char* value, ...);
	VdfNode& Value(bool value);
	VdfNode& Value(int value);
	VdfNode& Value(float value);

	std::string ToString(int indentLevel = 0);

protected:
	bool IsValueNode();

private:
	std::string m_name;
	std::string m_value;

	std::vector<VdfNode*> m_children;

};

}; // namespace MicroBuild