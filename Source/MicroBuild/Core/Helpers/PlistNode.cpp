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
#include "Core/Helpers/Strings.h"
#include "Core/Helpers/PlistNode.h"

#include <algorithm>

namespace MicroBuild {

PlistNode::PlistNode()
	: m_name("")
{
}

PlistNode::~PlistNode()
{
	for (PlistNode* node : m_children)
	{
		delete node;
	}
	m_children.clear();
}

PlistNode& PlistNode::Node(const char* name, ...)
{
	PlistNode* node = new PlistNode();

	va_list list;
	va_start(list, name);
	node->m_name = Strings::FormatVa(name, list);
	va_end(list);

	m_children.push_back(node);

	return *node;
}

PlistNode& PlistNode::Value(const char* value, ...)
{
	va_list list;
	va_start(list, value);
	m_value = Strings::FormatVa(value, list);
	va_end(list);

	return *this;
}

PlistNode& PlistNode::Value(bool value)
{
	return Value(value ? "true" : "false");
}

PlistNode& PlistNode::Value(int value)
{
	return Value("%i", value);
}

PlistNode& PlistNode::Value(float value)
{
	return Value("%f", value);
}

bool PlistNode::IsValueNode()
{
	return m_children.empty() && m_name.empty();
}

std::string PlistNode::ToString(int indentLevel, bool bAppendHeader)
{
	std::stringstream stream;

	if (bAppendHeader)
	{
		stream << "// !$*UTF8*$!\n";
	}

	if (indentLevel > 0)
	{
		stream << std::string(indentLevel, '\t');
	}

	if (IsValueNode())
	{
		stream << Strings::Quoted(m_value);
	}
	else
	{
		if (!m_name.empty())
		{
			stream << m_name << " = ";
		}

		bool bArray = false;

		for (PlistNode* node : m_children)
		{
			if (node->IsValueNode())
			{
				bArray = true;
			}
		}

		if (!m_value.empty())
		{
			stream << m_value << "\n";
		}
		else
		{
			if (bArray)
			{
				stream << "(";
			}
			else
			{
				stream << "{";
			}

			if (!m_children.empty())
			{
				stream << "\n";

				for (size_t i = 0; i < m_children.size(); i++)
				{
					PlistNode* node = m_children[i];

					if (!node->m_name.empty())
					{
						stream << node->ToString(indentLevel + 1, false);
					}

					stream << ";\n";
				}			

				if (indentLevel > 0)
				{
					stream << std::string(indentLevel, '\t');
				}
			}

			if (bArray)
			{
				stream << ")";
			}
			else
			{
				stream << "}";
			}
		}
	}

	return stream.str();
}

}; // namespace MicroBuild
