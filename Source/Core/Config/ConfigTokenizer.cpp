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
#include "Core/Config/ConfigTokenizer.h"
#include "Core/Platform/Path.h"
#include "Core/Helpers/Strings.h"
#include "Core/Helpers/Time.h"

namespace MicroBuild {

const char* TokenTypeLiteral[(int)TokenType::COUNT] = {
	"unknown",
	"expression",

	// Sections
	"group",
	"literal",
	"value",
	"string",

	// Operators
	"=",
	"==",
	"!=",
	"<",
	">",
	"<=",
	">=",
	"!",
	"&&",
	"||",

	// Symbols
	"{",
	"}",
	"(",
	")",

	// Keywords
	"include",
	"if",
	"else",
};

ConfigTokenizer::ConfigTokenizer()
{
}

ConfigTokenizer::~ConfigTokenizer()
{
}

bool ConfigTokenizer::EndOfCharacters()
{
	return m_offset >= m_data.size();
}

char ConfigTokenizer::ReadChar()
{
	char chr = m_data.at(m_offset++);
	if (chr == '\n')
	{
		m_column = 0;
		m_line++;
	}
	else
	{
		m_column++;
	}
	return chr;
}

char ConfigTokenizer::PeekChar(int offset)
{
	unsigned readOffset = m_offset + (offset - 1);
	if (readOffset >= m_data.size())
	{
		return '\0';
	}
	return m_data.at(readOffset);
}

void ConfigTokenizer::SkipToLineEnd()
{
	while (!EndOfCharacters() && ReadChar() != '\n')
		;
}

void ConfigTokenizer::SkipWhitespace()
{
	while (!EndOfCharacters())
	{
		char chr = PeekChar();
		if (chr == ' ' || chr == '\t')
		{
			ReadChar();
		}
		else
		{
			break;
		}
	}
}

void ConfigTokenizer::Error(const Token& token, const char* format, ...)
{
	va_list list;
	va_start(list, format);
	
	std::string message = Strings::FormatVa(format, list);

	va_end(list);

	std::string value = Strings::Format("%s(%i): %s\n",
		token.File.ToString().c_str(),
		token.Line + 1,
		message.c_str());

	Log(LogSeverity::Fatal, "%s", value.c_str());
}

void ConfigTokenizer::UnexpectedChar(const Token& token)
{
	Error(token, "Unexpected character '%s'.", token.Literal.c_str());
}

void ConfigTokenizer::UnexpectedEndOfFile(const Token& token)
{
	Error(token, "Encountered end of file while reading parsing group name.");
}

void ConfigTokenizer::ExpectedToken(const Token& token, TokenType type)
{
	Error(token, "Expected token '%s' after '%s'.", 
		TokenTypeLiteral[(int)type],
		token.Literal.c_str());
}

void ConfigTokenizer::UnexpectedToken(const Token& token, TokenType type)
{
	Error(token, "Unexpected token '%s', expected '%s'.", 
		TokenTypeLiteral[(int)type],
		token.Literal.c_str());
}

bool ConfigTokenizer::ExpectTokenAt(unsigned int index, TokenType type)
{
	if (index >= m_tokens.size())
	{
		ExpectedToken(m_tokens[m_tokens.size() - 1], type);
		return false;
	}
	if (m_tokens.at(index).Type != type)
	{
		UnexpectedToken(m_tokens[index], type);
		return false;
	}
	return true;
}

bool ConfigTokenizer::IsLiteralChar(char chr)
{
	if ((chr >= '0' && chr <= '9') ||
		(chr >= 'a' && chr <= 'z') ||
		(chr >= 'A' && chr <= 'Z') ||
		 chr == '_' ||
		 chr == '.')
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ConfigTokenizer::ReadToken()
{
	char chr = ReadChar();

	// Single-Line comment.
	if (chr == ';' || chr == '#')
	{
		SkipToLineEnd();
	}

	// Whitespace.
	else if (chr == ' ' || chr == '\t' || chr == '\n')
	{
		// Skip character.
	}

	// Group definition.
	else if (chr == '[')
	{
		Token tok(m_column, m_path, m_line, TokenType::Group, "");

		while (true)
		{
			if (EndOfCharacters())
			{
				UnexpectedEndOfFile(tok);
				return false;
			}
			
			chr = ReadChar();
			if (chr == ']')
			{
				break;
			}
			else
			{
				tok.Literal += chr;
			}
		}

		m_tokens.push_back(tok);
	}

	// String value.
	else if (chr == '"')
	{
		Token tok(m_column, m_path, m_line, TokenType::String, "");

		while (true)
		{
			if (EndOfCharacters())
			{
				UnexpectedEndOfFile(tok);
				return false;
			}

			chr = ReadChar();
			if (chr == '\\')
			{
				char la = PeekChar();
				if (la == '"')
				{
					tok.Literal += la;
					ReadChar();
				}
				break;
			}
			else if (chr == '"')
			{
				break;
			}
			else
			{
				tok.Literal += chr;
			}
		}

		m_tokens.push_back(tok);
	}

	// Operators.
	else if (chr == '{')
	{
		m_tokens.push_back(Token(m_column, m_path, m_line, 
			TokenType::Open_Brace, "{"));
	}
	else if (chr == '}')
	{
		m_tokens.push_back(Token(m_column, m_path, m_line,
			TokenType::Close_Brace, "}"));
	}
	else if (chr == '(')
	{
		m_tokens.push_back(Token(m_column, m_path, m_line,
			TokenType::Open_Parent, "("));
	}
	else if (chr == ')')
	{
		m_tokens.push_back(Token(m_column, m_path, m_line,
			TokenType::Close_Parent, ")"));
	}
	else if (chr == '=')
	{
		char la = PeekChar();
		if (la == '=')
		{
			ReadChar();
			m_tokens.push_back(Token(m_column, m_path, m_line,
				TokenType::Equal, "=="));
		}		
		else
		{
			m_tokens.push_back(Token(m_column, m_path, m_line,
				TokenType::Assignment, "="));
		}
	}
	else if (chr == '!')
	{
		char la = PeekChar();
		if (la == '=')
		{
			ReadChar();
			m_tokens.push_back(Token(m_column, m_path, m_line,
				TokenType::NotEqual, "!="));
		}
		else
		{
			m_tokens.push_back(Token(m_column, m_path, m_line,
				TokenType::Not, "!"));
		}
	}
	else if (chr == '<')
	{
		char la = PeekChar();
		if (la == '=')
		{
			ReadChar();
			m_tokens.push_back(Token(m_column, m_path, m_line,
				TokenType::LessEqual, "<="));
		}
		else
		{
			m_tokens.push_back(Token(m_column, m_path, m_line,
				TokenType::Less, "<"));
		}
	}
	else if (chr == '>')
	{
		char la = PeekChar();
		if (la == '=')
		{
			ReadChar();
			m_tokens.push_back(Token(m_column, m_path, m_line,
				TokenType::GreaterEqual, ">="));
		}
		else
		{
			m_tokens.push_back(Token(m_column, m_path, m_line,
				TokenType::Greater, ">"));
		}
	}
	else if (chr == '&')
	{
		char la = PeekChar();
		if (la == '&')
		{
			ReadChar();
			m_tokens.push_back(Token(m_column, m_path, m_line,
				TokenType::And, "&&"));
		}
		else
		{
			UnexpectedChar(Token(m_column, m_path, m_line,
				TokenType::Unknown, "&"));
			return false;
		}
	}
	else if (chr == '|')
	{
		char la = PeekChar();
		if (la == '|')
		{
			ReadChar();
			m_tokens.push_back(Token(m_column, m_path, m_line,
				TokenType::Or, "||"));
		}
		else
		{
			UnexpectedChar(Token(m_column, m_path, m_line,
				TokenType::Unknown, "|"));
			return false;
		}
	}

	// Literal value or variable assignment.
	else if (IsLiteralChar(chr))
	{
		// Read in the literal value.
		std::string lit(1, chr);
		while (!EndOfCharacters())
		{
			char la = PeekChar();
			if (IsLiteralChar(la))
			{
				ReadChar();
				lit += la;
			}
			else
			{
				break;
			}
		}

		TokenType type = TokenType::Literal;
		if (lit == "if")		
		{
			type = TokenType::If;
		}
		else if (lit == "include")
		{
			type = TokenType::Include;
		}
		else if (lit == "else")
		{
			type = TokenType::Else;
		}

		m_tokens.push_back(Token(m_column, m_path, m_line,
			type, lit));

		SkipWhitespace();

		// If an equals value follows this literal then everything following
		// this literal before the end of line/file is a value token.
		char la1 = PeekChar(1);
		char la2 = PeekChar(2);
		if (la1 == '=' && la2 != '=')
		{
			m_tokens.push_back(Token(m_column, m_path, m_line,
				TokenType::Assignment, "="));

			ReadChar();
			SkipWhitespace();

			std::string valueLit;
			while (!EndOfCharacters())
			{
				char la = PeekChar();
				if (la != '\n')
				{
					ReadChar();
					valueLit += la;
				}
				else
				{
					break;
				}
			}

			// Trim off any whitespace.
			while (valueLit.size() > 0)
			{
				char lastChr = *valueLit.rbegin();
				if (lastChr == ' ' || lastChr == '\t')
				{
					valueLit.pop_back();
				}
				else
				{
					break;
				}
			}

			m_tokens.push_back(Token(m_column, m_path, m_line,
				TokenType::Value, valueLit));
		}
	}

	// Unknown character.
	else
	{
		UnexpectedChar(Token(m_column, m_path, m_line,
			TokenType::Unknown, std::string(1, chr)));

		return false;
	}

	return true;
}

Token& ConfigTokenizer::GetToken(int index)
{
	return m_tokens[index];
}

int ConfigTokenizer::GetTokenCount()
{
	return (int)m_tokens.size();
}

bool ConfigTokenizer::Tokenize(const Platform::Path& path)
{
	m_path = path;
	m_offset = 0;
	m_line = 0;
	m_column = 0;
	m_tokens.clear();

	// Read the file data.
	{
		Time::TimedScope scope(
			Strings::Format("[%s] File read", path.ToString().c_str())
		);

		if (!path.Exists())
		{
			Log(LogSeverity::Fatal,
				"Failed to open included file '%s'.",
				path.ToString().c_str());

			return false;
		}

		if (!Strings::ReadFile(path, m_data))
		{
			Log(LogSeverity::Fatal,
				"Failed to read file '%s'.",
				path.ToString().c_str());

			return false;
		}
	}

	// Tokenization.
	{
		Time::TimedScope scope(
			Strings::Format("[%s] Tokenization", path.ToString().c_str())
		);

		// Reserve a bunch of memory up front to save some resizing.
		// Dividor is arbitrary, should be the average length of a token.
		m_tokens.reserve(m_data.size() / 5);

		// Keep reading tokens until we run out of characters or error.
		while (!EndOfCharacters())
		{
			if (!ReadToken())
			{
				return false;
			}
		}

		// Parse all include definitions.
		for (unsigned int i = 0; i < m_tokens.size(); i++)
		{
			Token& tok = m_tokens[i];
			if (tok.Type == TokenType::Include)
			{
				if (!ExpectTokenAt(i + 1, TokenType::Open_Parent) ||
					!ExpectTokenAt(i + 2, TokenType::String) ||
					!ExpectTokenAt(i + 3, TokenType::Close_Parent))
				{
					return false;
				}

				Platform::Path subPath = m_tokens[i + 2].Literal;
				if (!subPath.Exists())
				{
					if (subPath.IsRelative())
					{
						subPath = (path.GetDirectory() + subPath);
					}
				}

				ConfigTokenizer subTokenizer;
				if (!subTokenizer.Tokenize(subPath))
				{
					return false;
				}

				// Insert resulting tokens into correct place.
				m_tokens.erase(m_tokens.begin() + i, m_tokens.begin() + i + 4);
				m_tokens.insert(m_tokens.begin() + i, 
					subTokenizer.m_tokens.begin(), 
					subTokenizer.m_tokens.end());
			}
		}
	}

	return true;
}

}; // namespace MicroBuild