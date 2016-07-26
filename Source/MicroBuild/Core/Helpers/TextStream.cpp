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
#include "Core/Helpers/TextStream.h"

#include <algorithm>

namespace MicroBuild {

TextStream::TextStream()
	: m_indentLevel(0)
{
}

TextStream::~TextStream()
{
}

void TextStream::Indent()
{
	m_indentLevel++;
}

void TextStream::Undent()
{
	m_indentLevel--;
	assert(m_indentLevel >= 0);
}

void TextStream::Write(const char* format, ...)
{
	va_list list;
	va_start(list, format);
	std::string formatted = Strings::FormatVa(format, list);
	va_end(list);

	for (int i = 0; i < m_indentLevel; i++)
	{
		m_stream << "\t";
	}

	m_stream << formatted;
}

void TextStream::WriteLine(const char* format, ...)
{
	va_list list;
	va_start(list, format);
	std::string formatted = Strings::FormatVa(format, list);
	va_end(list);

	for (int i = 0; i < m_indentLevel; i++)
	{
		m_stream << "\t";
	}

	m_stream << formatted;
	m_stream << "\n";
}

bool TextStream::WriteToFile(Platform::Path& path)
{
	std::string result = m_stream.str();
	return Strings::WriteFile(path, result);
}

}; // namespace MicroBuild
