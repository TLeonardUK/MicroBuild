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

// This class wraps an json node that can be serialized into a string.
class JsonNode
{
public:
	JsonNode(const JsonNode& node) = delete;
	JsonNode& operator=(const JsonNode&) = delete;

	JsonNode();
	~JsonNode();

	JsonNode& Node(const char* name, ...);
	JsonNode& Value(const char* value, ...);
	JsonNode& Value(bool value);
	JsonNode& Value(int value);
	JsonNode& Value(float value);

	std::string ToString(int indentLevel = 0);

protected:
	bool IsValueNode();

private:
	std::string m_name;
	std::string m_value;

	std::vector<JsonNode*> m_children;

};

}; // namespace MicroBuild