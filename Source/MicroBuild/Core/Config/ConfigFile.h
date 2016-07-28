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
#include "Core/Config/ConfigTokenizer.h"

namespace MicroBuild {

// Result of an config file expression evaluation.
struct ConfigFileExpressionResult
{
	std::string Result;

	ConfigFileExpressionResult()
		: Result("")
	{
	}

	std::string ToString()
	{
		return Result;
	}

	float ToFloat()
	{
		return (float)atof(Result.c_str());
	}

	bool ToBool()
	{
		return (ToFloat() != 0);
	}

	void operator=(const float& lvalue)
	{
		char buffer[128];
		sprintf(buffer, "%f", lvalue);
		Result = buffer;
	}

	void operator=(const std::string& lvalue)
	{
		Result = lvalue;
	}

	void operator=(const int& lvalue)
	{
		char buffer[128];
		sprintf(buffer, "%i", lvalue);
		Result = buffer;
	}

	void operator=(const bool& lvalue)
	{
		Result = (lvalue ? "1" : "0");
	}
};

// A condition that determines if a key exists with the given defines/config.
struct ConfigFileExpression
{
	bool Invert;
	TokenType Operator;
	std::string Value;
	std::vector<ConfigFileExpression> Children;

	ConfigFileExpression()
		: Invert(false)
		, Operator(TokenType::Unknown)
	{
	}
};

// An individual value associated with a key, and its conditionals.
struct ConfigFileValue
{
	std::string Value;
	std::vector<ConfigFileExpression> Conditions;

	// Post resolve values.
	bool HasResolvedValue;
	std::string ResolvedValue;

	bool HasResolvedCondition;
	bool ConditionResult;

	ConfigFileValue()
		: HasResolvedValue(false)
		, HasResolvedCondition(false)
		, ConditionResult(true)
	{
	}
};

// An individual key.
struct ConfigFileKey
{
	std::string Name;
	std::vector<ConfigFileValue*> Values;
};

// An individual group of config keys.
struct ConfigFileGroup
{
	std::string Name;
	bool bUnmergable;
	std::map<std::string, ConfigFileKey*> Keys;
};

// Our base config file class. Implements a simple INI style file format.
class ConfigFile
{
public:
	typedef std::pair<std::string, std::string> KeyValuePair;

	ConfigFile();
	ConfigFile(const ConfigFile& other);
	~ConfigFile();

	void CopyFrom(const ConfigFile& other);

	void operator=(const ConfigFile& other);

	// Parses the config file located at the given path and converts in into
	// group and value keys.
	bool Parse(const Platform::Path& path);

	// Writes all the stored groups and values to the given file path. Be aware
	// this only serialize trivial values, it does not serialize conditional
	// values or anything requiring more than simple ini key-value pairs.
	bool Serialize(const Platform::Path& path);

	// Copies all the values that exist in another config file into this one.
	void Merge(const ConfigFile& file);

	// Sets or adds the given value to the group with the given name.
	void SetOrAddValue(
		const std::string& group,
		const std::string& key,
		const std::string& value,
		bool bOverwrite = false); 

	// Sets or adds all the values to the group with the given name.
	void SetOrAddValue(
		const std::string& group,
		const std::string& key,
		const std::vector<std::string>& value,
		bool bOverwrite = false);

	// Resolves all expression references and evaluates them. It also
	// performs token replacement within values.
	virtual void Resolve();

	// Gets all the values of a given key in the given group.
	std::vector<std::string> GetValues(
		const std::string& group, 
		const std::string& key);

	// Gets all the keys and values in the group.
	std::vector<KeyValuePair> GetPairs(
		const std::string& group);

	// Same as GetValues except only gets the first value.
	std::string GetValue(
		const std::string& group,
		const std::string& key);

	// Returns true if this config file has the given group/key combo defined.
	bool HasValue(
		const std::string& group,
		const std::string& key);

	// Same as GetValue except returns default if not defined.
	std::string GetValue(
		const std::string& group,
		const std::string& key,
		const std::string& defValue);

	// Flags a group as mergable or unmergable.
	void SetGroupUnmergable(
		const std::string& group,
		bool bUnmergable);

protected:

	std::vector<ConfigFileValue*> SetOrAddValue_Internal(
		const std::string& group,
		const std::string& key,
		const std::vector<std::string>& values,
		bool bOverwrite = false);

	void Clear();

	void Error(const Token& token, const char* format, ...);

	void UnexpectedToken(const Token& tok);
	void UnexpectedEndOfTokens(const Token& tok);
	void UnexpectedNestedGroup(const Token& tok);

	void PrettyPrintExpression(
		const ConfigFileExpression& expressions, int depth = 0);
	void PrettyPrintExpressions(
		std::vector<ConfigFileExpression>& expressions);

	bool EndOfTokens();
	Token& NextToken();
	Token& PeekToken();
	Token& CurrentToken();
	bool ExpectToken(TokenType type);

	bool ParseExpression(ConfigFileExpression& expression);
	bool ParseExpressionComparison(ConfigFileExpression& expression);
	bool ParseExpressionUnary(ConfigFileExpression& expression);
	bool ParseExpressionFactor(ConfigFileExpression& expression);
	bool ParseStatement();
	bool ParseAssignment();
	bool ParseIf();
	bool ParseGroup();
	bool ParseBlock();

	bool EvaluateConditions(
		std::vector<ConfigFileExpression>& conditions,
		std::string groupName);

	ConfigFileExpressionResult EvaluateExpression(
		ConfigFileExpression& expression, 
		const std::string& baseGroup);

	std::string ReplaceTokens(
		const std::string& value,
		const std::string& baseGroup);

	bool ResolveTokenReplacement(
		const std::string& key,
		const std::string& group,
		std::string& result);

	Platform::Path GetPath() const;

private:
	Platform::Path m_path;

	ConfigTokenizer m_tokenizer;
	int m_tokenIndex;

	std::string m_currentGroup;
	std::map<std::string, ConfigFileGroup*> m_groups;

	std::vector<ConfigFileExpression> m_expressionStack;
	
};

}; // namespace MicroBuild