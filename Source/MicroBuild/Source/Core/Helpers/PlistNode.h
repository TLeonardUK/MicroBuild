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

// This class wraps an plist node that can be serialized into a string.
class PlistNode
{
public:
	PlistNode(const PlistNode& node) = delete;
	PlistNode& operator=(const PlistNode&) = delete;

	PlistNode();
	~PlistNode();

	PlistNode& Dict(const char* name, ...);
	PlistNode& Array(const char* name, ...);
	PlistNode& Node(const char* name, ...);
	PlistNode& Value(const char* value, ...);
	PlistNode& Value(bool value);
	PlistNode& Value(int value);
	PlistNode& Value(float value);

	std::string ToString(int indentLevel = 0, bool bAppendHeader = true);

protected:
	bool IsValueNode();

private:
	std::string m_name;
	std::string m_value;
    
    bool m_valueSet;

    bool m_isArraySet;
    bool m_isArray;

	std::vector<PlistNode*> m_children;

};

}; // namespace MicroBuild