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
#include "Core/Helpers/JsonNode.h"

#include <algorithm>

namespace MicroBuild {

JsonNode::JsonNode()
	: m_name("")
{
}

JsonNode::~JsonNode()
{
	for (JsonNode* node : m_children)
	{
		delete node;
	}
	m_children.clear();
}

JsonNode& JsonNode::Node(const char* name, ...)
{
	JsonNode* node = new JsonNode();

	va_list list;
	va_start(list, name);
	node->m_name = Strings::FormatVa(name, list);
	va_end(list);

	m_children.push_back(node);

	return *node;
}

JsonNode& JsonNode::Value(const char* value, ...)
{
	va_list list;
	va_start(list, value);
	m_value = Strings::FormatVa(value, list);
	va_end(list);

	return *this;
}

JsonNode& JsonNode::Value(bool value)
{
	return Value(value ? "true" : "false");
}

JsonNode& JsonNode::Value(int value)
{
	return Value("%i", value);
}

JsonNode& JsonNode::Value(float value)
{
	return Value("%f", value);
}

bool JsonNode::IsValueNode()
{
	return m_children.empty() && m_name.empty();
}

std::string JsonNode::ToString(int indentLevel)
{
	std::stringstream stream;

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
			stream << Strings::Quoted(m_name) << ": ";
		}

		bool bArray = false;

		for (JsonNode* node : m_children)
		{
			if (node->IsValueNode())
			{
				bArray = true;
			}
		}

		if (!m_value.empty())
		{
			stream << Strings::Quoted(m_value);
		}
		else
		{
			if (bArray)
			{
				stream << "[";
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
					JsonNode* node = m_children[i];

					if (!node->m_name.empty())
					{
						stream << node->ToString(indentLevel + 1);
					}

					if (i < m_children.size() - 1)
					{
						stream << ",";
					}
					stream << "\n";
				}			

				if (indentLevel > 0)
				{
					stream << std::string(indentLevel, '\t');
				}
			}

			if (bArray)
			{
				stream << "]";
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
