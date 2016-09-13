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
class XmlAttribute
{
public:
	XmlAttribute();
	~XmlAttribute();

private:
	friend class XmlNode;

	std::string m_name;
	std::string m_value;

};

// This class wraps an xml node that can be serialized into a string.
class XmlNode
{
public:
	XmlNode(const XmlNode& node) = delete;
	XmlNode& operator=(const XmlNode&) = delete;

	XmlNode();
	~XmlNode();

	XmlNode& Node(const char* name);
	XmlNode& Value(const char* value, ...);
	XmlNode& Value(bool value);
	XmlNode& Value(int value);
	XmlNode& Value(float value);
	XmlNode& Attribute(const char* name, const char* value, ...);

	std::string ToString(int indentLevel = 0);

private:
	std::string m_name;
	std::string m_value;

	std::vector<XmlNode*> m_children;
	std::vector<XmlAttribute*> m_attributes;

};

}; // namespace MicroBuild