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
#include "Core/Config/ConfigFile.h"
#include "Core/Config/ConfigTokenizer.h"
#include "Core/Platform/Path.h"
#include "Core/Helpers/Strings.h"
#include "Core/Helpers/Time.h"

namespace MicroBuild {

ConfigFile::ConfigFile()
{
}

ConfigFile::~ConfigFile()
{
}

bool ConfigFile::EndOfTokens()
{
}

void ConfigFile::NextToken()
{
}

void ConfigFile::ExpectToken(TokenType type)
{
}

void ConfigFile::ParseStatement()
{
}

bool ConfigFile::Parse(const Platform::Path& path)
{
	m_path = path;
	m_token_index = 0;

	// Break the file down into tokens.
	ConfigTokenizer tokenizer;
	if (!tokenizer.Tokenize(path))
	{
		return false;
	}

	// Parse tokens into a key/value representation.
	while (!EndOfTokens())
	{
		ParseStatement();
	}

	// Check our required version is valid if it is provided.

	return true;
}

}; // namespace MicroBuild