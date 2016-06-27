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

namespace MicroBuild {

enum class TokenType
{
	Unknown,

	// Sections
	Group,
	Literal,
	Value,
	String,

	// Operators
	Assignment,
	Equal,
	NotEqual,
	Less,
	Greater,
	LessEqual,
	GreaterEqual,
	Not,
	And,
	Or,

	// Symbols
	Open_Brace,
	Close_Brace,
	Open_Parent,
	Close_Parent,

	// Keywords
	Include,
	If,
	Else,

	COUNT
};

extern const char* TokenTypeLiteral[(int)TokenType::COUNT];

struct Token
{
	TokenType Type;
	std::string Literal;
	Platform::Path File;
	int Line;
	int Column;

	Token(int column, const Platform::Path& path, int line, TokenType type, const std::string& lit)
		: Type(type)
		, Literal(lit)
		, File(path)
		, Line(line)
		, Column(column)
	{
	}
};

class ConfigTokenizer
{
public:
	ConfigTokenizer();
	~ConfigTokenizer();

	bool Tokenize(const Platform::Path& path);

protected:
	void Error(const Token& token, const char* format, ...);
	void UnexpectedEndOfFile(const Token& token);
	void UnexpectedChar(const Token& token);
	void ExpectedToken(const Token& token, TokenType type);
	void UnexpectedToken(const Token& token, TokenType type);

	bool IsLiteralChar(char chr);

	bool EndOfCharacters();
	bool ReadToken();
	char PeekChar(int offset = 1);
	char ReadChar();
	void SkipToLineEnd();
	void SkipWhitespace();

	bool ExpectTokenAt(unsigned int index, TokenType type);

private:
	std::vector<Token> m_tokens;
	std::string m_data;
	Platform::Path m_path;
	unsigned int m_offset;
	int m_column;
	int m_line;

};

}; // namespace MicroBuild