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
#include "Core/Helpers/XmlNode.h"

#include <algorithm>

namespace MicroBuild {

XmlAttribute::XmlAttribute()
	: m_name("")
	, m_value("")
{
}

XmlAttribute::~XmlAttribute()
{
}

XmlNode::XmlNode()
	: m_name("")
{
}

XmlNode::~XmlNode()
{
	for (XmlNode* node : m_children)
	{
		delete node;
	}
	for (XmlAttribute* node : m_attributes)
	{
		delete node;
	}

	m_children.clear();
	m_attributes.clear();
}

XmlNode& XmlNode::Node(const char* name)
{
	XmlNode* node = new XmlNode();
	node->m_name = name;
	
	m_children.push_back(node);

	return *node;
}

XmlNode& XmlNode::Value(const char* value, ...)
{
	va_list list;
	va_start(list, value);
	m_value = Strings::FormatVa(value, list);
	va_end(list);

	return *this;
}

XmlNode& XmlNode::Value(bool value)
{
	return Value(value ? "true" : "false");
}

XmlNode& XmlNode::Value(int value)
{
	return Value("%i", value);
}

XmlNode& XmlNode::Value(float value)
{
	return Value("%f", value);
}

XmlNode& XmlNode::Attribute(const char* name, const char* value, ...)
{
	XmlAttribute* node = new XmlAttribute();
	node->m_name = name;

	va_list list;
	va_start(list, value);
	node->m_value = Strings::FormatVa(value, list);
	va_end(list);

	m_attributes.push_back(node);

	return *this;
}

std::string XmlNode::ToString(int indentLevel)
{
	std::stringstream stream;

	if (!m_name.empty())
	{
		if (indentLevel > 0)
		{
			stream << std::string(indentLevel, '\t');
		}

		stream << "<";
		stream << m_name;

		for (XmlAttribute* node : m_attributes)
		{
			stream << " ";
			stream << node->m_name;
			stream << "=";
			stream << "\"";
			//stream << Strings::Escaped(node->m_value);
			stream << Strings::Replace(node->m_value, "\"", "&quot;");
			stream << "\"";
		}

		if (m_children.size() == 0 && m_value.empty())
		{
			if (m_name[0] == '?')
			{
				stream << "?>";
			}
			else
			{
				stream << "/>";
			}
		}
		else
		{
			stream << ">";
		}
		
		if (m_value.empty())
		{
			stream << "\n";
		}
	}

	if (m_value.empty())
	{
		for (XmlNode* node : m_children)
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
	}
	else
	{
		stream << m_value;
	}
		
	if (!m_name.empty())
	{
		if (m_children.size() > 0 || !m_value.empty())
		{
			if (indentLevel > 0 && m_value.empty())
			{
				stream << std::string(indentLevel, '\t');
			}
			stream << "</";
			stream << m_name;
			stream << ">";
			stream << "\n";
		}
	}

	return stream.str();
}

}; // namespace MicroBuild
