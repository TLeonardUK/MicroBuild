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

#include "PCH.h"
#include "Core/Helpers/Strings.h"
#include "Core/Helpers/VbNode.h"

#include <algorithm>

namespace MicroBuild {

VbAttribute::VbAttribute()
	: m_value("")
	, m_quoted(false)
{
}

VbAttribute::~VbAttribute()
{
}

VbNode::VbNode()
	: m_name("")
	, m_text(false)
{
}

VbNode::~VbNode()
{
	for (VbNode* node : m_children)
	{
		delete node;
	}
	for (VbAttribute* node : m_attributes)
	{
		delete node;
	}
	for (VbAttribute* node : m_values)
	{
		delete node;
	}

	m_children.clear();
	m_attributes.clear();
	m_values.clear();
}

VbNode& VbNode::Node(const char* format, ...)
{
	VbNode* node = new VbNode();
	node->m_text = false;

	va_list list;
	va_start(list, format);
	node->m_name = Strings::FormatVa(format, list);
	va_end(list);

	m_children.push_back(node);

	return *node;
}

VbNode& VbNode::Single(const char* format, ...)
{
	VbNode* node = new VbNode();
	node->m_text = true;

	va_list list;
	va_start(list, format);
	node->m_name = Strings::FormatVa(format, list);
	va_end(list);

	m_children.push_back(node);

	return *node;
}

VbNode& VbNode::Attribute(bool bQuoted, const char* format, ...)
{
	VbAttribute* node = new VbAttribute();
	node->m_quoted = bQuoted;

	va_list list;
	va_start(list, format);
	node->m_value = Strings::FormatVa(format, list);
	va_end(list);

	m_attributes.push_back(node);

	return *this;
}

VbNode& VbNode::Value(bool bQuoted, const char* format, ...)
{
	VbAttribute* node = new VbAttribute();
	node->m_quoted = bQuoted;

	va_list list;
	va_start(list, format);
	node->m_value = Strings::FormatVa(format, list);
	va_end(list);

	m_values.push_back(node);

	return *this;
}

std::string VbNode::ToString(int indentLevel)
{
	std::stringstream stream;

	if (!m_name.empty())
	{
		stream << std::string(indentLevel, '\t');
		stream << m_name;

		if (m_attributes.size() > 0)
		{
			stream << "(";

			for (unsigned int i = 0; i < m_attributes.size(); i++)
			{
				VbAttribute* attr = m_attributes[i];
				if (i != 0)
				{
					stream << ", ";
				}

				if (attr->m_quoted)
				{
					stream << "\"";
				}
				stream << Strings::Escaped(attr->m_value);
				if (attr->m_quoted)
				{
					stream << "\"";
				}
			}

			stream << ")";
		}

		if (m_values.size() > 0)
		{
			stream << " = ";

			for (unsigned int i = 0; i < m_values.size(); i++)
			{
				VbAttribute* attr = m_values[i];
				if (i != 0)
				{
					stream << ", ";
				}

				if (attr->m_quoted)
				{
					stream << "\"";
				}
				stream << Strings::Escaped(attr->m_value);
				if (attr->m_quoted)
				{
					stream << "\"";
				}
			}
		}

		stream << "\n";
	}
	else if (m_text)
	{
		stream << "\n";
	}

	for (VbNode* node : m_children)
	{
		if (m_name.empty())
		{
			stream << node->ToString(indentLevel);
		}
		else
		{
			stream << node->ToString(indentLevel + 1);
		}
	}

	if (!m_name.empty() && !m_text)
	{
		stream << std::string(indentLevel, '\t');
		stream << "End" << m_name << "\n";
	}

	return stream.str();
}

}; // namespace MicroBuild