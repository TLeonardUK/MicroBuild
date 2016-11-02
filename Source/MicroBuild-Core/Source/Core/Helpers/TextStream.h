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

// This class provides a nice wrapper interface for reading and writing
// text data to files/string output.
class TextStream
{
public:
	TextStream(bool bUseSpacesForIndents = false);
	~TextStream();

	void Indent();
	void Undent();

	void Write(const char* format, ...);
	void WriteRaw(const char* format, ...);
	void WriteNewLine();
	void WriteLine(const char* format, ...);

	bool WriteToFile(Platform::Path& path, bool bOnlyWriteIfDifferent = false);

	std::string ToString();

private:
	int m_indentLevel;
	bool m_bUseSpacesForIndents;
	std::stringstream m_stream;

};

}; // namespace MicroBuild