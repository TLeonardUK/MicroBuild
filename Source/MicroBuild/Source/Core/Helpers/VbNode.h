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
// This class wraps an xml attribute attached to a node.
class VbAttribute
{
public:
	VbAttribute();
	~VbAttribute();

private:
	friend class VbNode;

	std::string m_value;
	bool m_quoted;

};

// This class wraps an vb-script style node. It is the basis for the format
// of visual studio solution files.
class VbNode
{
public:
	VbNode(const VbNode& node) = delete;
	VbNode& operator=(const VbNode&) = delete;

	VbNode();
	~VbNode();

	VbNode& Node(const char* format, ...);
	VbNode& Single(const char* format, ...);
	VbNode& Attribute(bool bQuoted, const char* format, ...);
	VbNode& Value(bool bQuoted, const char* format, ...);

	std::string ToString(int indentLevel = 0);

private:
	std::string m_name;
	bool m_text;

	std::vector<VbNode*> m_children;
	std::vector<VbAttribute*> m_attributes;
	std::vector<VbAttribute*> m_values;

};

}; // namespace MicroBuild