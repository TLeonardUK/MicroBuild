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
	for (auto groupIter : m_groups)
	{
		for (auto keyIter : groupIter.second->Keys)
		{
			for (auto valueIter : keyIter.second->Values)
			{
				delete valueIter;
			}

			delete keyIter.second;
		}

		delete groupIter.second;
	}

	m_groups.clear();
}

bool ConfigFile::EndOfTokens()
{
	return m_tokenIndex >= m_tokenizer.GetTokenCount();
}

Token& ConfigFile::NextToken()
{
	return m_tokenizer.GetToken(m_tokenIndex++);
}

Token& ConfigFile::PeekToken()
{
	static Token dummyToken(TokenType::Unknown);
	if (m_tokenIndex >= m_tokenizer.GetTokenCount())
	{
		return dummyToken;
	}
	return m_tokenizer.GetToken(m_tokenIndex);
}

Token& ConfigFile::CurrentToken()
{
	assert(m_tokenIndex > 0);
	return m_tokenizer.GetToken(m_tokenIndex - 1);
}

bool ConfigFile::ExpectToken(TokenType type)
{
	if (EndOfTokens())
	{
		UnexpectedEndOfTokens(CurrentToken());
		return false;
	}
	Token& tok = NextToken();
	if (tok.Type != type)
	{
		UnexpectedToken(tok);
		return false;
	}
	return true;
}

void ConfigFile::Error(const Token& token, const char* format, ...)
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

void ConfigFile::UnexpectedEndOfTokens(const Token& tok)
{
	Error(tok, "Unexpected end of tokens.");
}

void ConfigFile::UnexpectedToken(const Token& tok)
{
	Error(tok, "Unexpected token '%s'.", tok.Literal.c_str());
}

void ConfigFile::UnexpectedNestedGroup(const Token& tok)
{
	Error(tok, "Groups cannot be nested inside conditionals.");
}

void ConfigFile::ConfigFile::PrettyPrintExpression(
	const ConfigFileExpression& expression, int depth)
{
	std::string tabDepthPadding(depth, '\t');

	Log(LogSeverity::Verbose, "%sOp:%s Value:%s\n", 
		tabDepthPadding.c_str(),
		TokenTypeLiteral[(int)expression.Operator],
		expression.Value.c_str());

	for (const ConfigFileExpression& expr : expression.Children)
	{
		PrettyPrintExpression(expr, depth + 1);
	}
}

void ConfigFile::PrettyPrintExpressions(
	std::vector<ConfigFileExpression>& expressions)
{
	for (const ConfigFileExpression& expr : expressions)
	{
		Log(LogSeverity::Verbose, "[Expression]\n");
		PrettyPrintExpression(expr, 0);
	}
}

bool ConfigFile::ParseGroup()
{
	if (m_expressionStack.size() != 0)
	{
		UnexpectedNestedGroup(CurrentToken());
		return false;
	}

	std::string groupName = CurrentToken().Literal;

	auto iter = m_groups.find(groupName);
	if (iter == m_groups.end())
	{
		ConfigFileGroup* group = new ConfigFileGroup();
		group->Name = groupName;
		m_groups.insert(
			std::pair<std::string, ConfigFileGroup*>(groupName, group)
		);
	}

	m_currentGroup = groupName;

	return true;
}

bool ConfigFile::ParseAssignment()
{
	std::string keyName = CurrentToken().Literal;

	if (!ExpectToken(TokenType::Assignment))
	{
		return false;
	}
	if (!ExpectToken(TokenType::Value))
	{
		return false;
	}

	ConfigFileValue* keyValue = new ConfigFileValue();
	keyValue->Value = CurrentToken().Literal;
	keyValue->Conditions = m_expressionStack;
	keyValue->ConditionResult = true;

	ConfigFileGroup* group = m_groups[m_currentGroup];

	auto iter = group->Keys.find(keyName);
	if (iter == group->Keys.end())
	{
		ConfigFileKey* key = new ConfigFileKey();
		key->Name = keyName;
		key->Values.push_back(keyValue);
		group->Keys.insert(
			std::pair<std::string, ConfigFileKey*>(keyName, key)
		);
	}
	else
	{
		iter->second->Values.push_back(keyValue);
	}

	return true;
}

bool ConfigFile::ParseExpression(ConfigFileExpression& expression)
{
	ConfigFileExpression lValue;
	if (!ParseExpressionComparison(lValue))
	{
		return false;
	}

	while (!EndOfTokens())
	{
		Token& nextTok = PeekToken();
		if (nextTok.Type != TokenType::And &&
			nextTok.Type != TokenType::Or)
		{
			break;
		}

		NextToken();

		ConfigFileExpression rValue;
		if (!ParseExpressionComparison(rValue))
		{
			return false;
		}

		ConfigFileExpression result;
		result.Children.push_back(lValue);
		result.Children.push_back(rValue);
		result.Operator = nextTok.Type;
		lValue = result;
	}

	expression = lValue;
	return true;
}

bool ConfigFile::ParseExpressionComparison(ConfigFileExpression& expression)
{

	ConfigFileExpression lValue;
	if (!ParseExpressionUnary(lValue))
	{
		return false;
	}

	while (!EndOfTokens())
	{
		Token& nextTok = PeekToken();
		if (nextTok.Type != TokenType::Less &&
			nextTok.Type != TokenType::LessEqual && 
			nextTok.Type != TokenType::Greater &&
			nextTok.Type != TokenType::GreaterEqual &&
			nextTok.Type != TokenType::Equal &&
			nextTok.Type != TokenType::NotEqual)
		{
			break;
		}

		NextToken();

		ConfigFileExpression rValue;
		if (!ParseExpressionUnary(rValue))
		{
			return false;
		}

		ConfigFileExpression result;
		result.Children.push_back(lValue);
		result.Children.push_back(rValue);
		result.Operator = nextTok.Type;
		lValue = result;
	}

	expression = lValue;
	return true;
}

bool ConfigFile::ParseExpressionUnary(ConfigFileExpression& expression)
{
	bool bHasUnary = false;

	if (PeekToken().Type == TokenType::Not)
	{
		NextToken();

		ConfigFileExpression lValue;
		if (!ParseExpressionFactor(lValue))
		{
			return false;
		}

		ConfigFileExpression rValue;
		rValue.Operator = TokenType::Not;
		rValue.Children.push_back(lValue);
		expression = rValue;

	}
	else
	{
		if (!ParseExpressionFactor(expression))
		{
			return false;
		}
	}

	return true;
}

bool ConfigFile::ParseExpressionFactor(ConfigFileExpression& expression)
{
	Token& factorToken = NextToken();
	switch (factorToken.Type)
	{
	case TokenType::Open_Parent:
		{
			ConfigFileExpression subExpression;
			subExpression.Operator = TokenType::Expression;

			if (!ParseExpression(subExpression))
			{
				return false;
			}

			expression = subExpression;

			if (!ExpectToken(TokenType::Close_Parent))
			{
				return false;
			}

			break;
		}
	case TokenType::Literal:
	case TokenType::String:
		{
			ConfigFileExpression subExpression;
			subExpression.Operator = TokenType::Literal;
			subExpression.Value = factorToken.Literal;
			expression = subExpression;
			break;
		}
	default:
		{
			UnexpectedToken(factorToken);
			return false;
		}
	}

	return true;
}

bool ConfigFile::ParseIf()
{
	if (!ExpectToken(TokenType::Open_Parent))
	{
		return false;
	}

	ConfigFileExpression condition;
	condition.Operator = TokenType::Expression;

	if (!ParseExpression(condition))
	{
		return false;
	}
	
	if (!ExpectToken(TokenType::Close_Parent))
	{
		return false;
	}
	if (!ExpectToken(TokenType::Open_Brace))
	{
		return false;
	}
	
	m_expressionStack.push_back(condition);

	if (!ParseBlock())
	{
		return false;
	}

	m_expressionStack.pop_back();
	
	if (!ExpectToken(TokenType::Close_Brace))
	{
		return false;
	}

	if (PeekToken().Type == TokenType::Else)
	{
		NextToken();

		condition.Invert = !condition.Invert;
		m_expressionStack.push_back(condition);

		if (PeekToken().Type == TokenType::If)
		{
			NextToken();
			ParseIf();
		}
		else 
		{
			if (!ExpectToken(TokenType::Open_Brace))
			{
				return false;
			}

			if (!ParseBlock())
			{
				return false;
			}

			if (!ExpectToken(TokenType::Close_Brace))
			{
				return false;
			}
		}

		m_expressionStack.pop_back();

	}

	return true;
}

bool ConfigFile::ParseBlock()
{
	while (true)
	{
		if (EndOfTokens())
		{
			return false;
		}
		if (PeekToken().Type == TokenType::Close_Brace)
		{
			break;
		}
		if (!ParseStatement())
		{
			return false;
		}
	}

	return true;
}

bool ConfigFile::ParseStatement()
{
	Token& tok = NextToken();
	switch (tok.Type)
	{
	case TokenType::Group:
		{
			if (ParseGroup())
			{
				return true;
			}
			break;
		}
	case TokenType::Literal:
		{
			if (ParseAssignment())
			{
				return true;
			}
			break;
		}
	case TokenType::If:
		{
			if (ParseIf())
			{
				return true;
			}
			break;
		}
	default:
		{
			UnexpectedToken(tok);
			break;
		}
	}

	return false;
}

bool ConfigFile::Parse(const Platform::Path& path)
{
	m_path = path;
	m_tokenIndex = 0;
	m_currentGroup = "";
	m_groups.clear();
	m_expressionStack.clear();

	// Insert the global group.
	ConfigFileGroup* globalGroup = new ConfigFileGroup();
	globalGroup->Name = "";
	m_groups.insert(
		std::pair<std::string, ConfigFileGroup*>(globalGroup->Name, globalGroup)
	);

	// Break the file down into tokens.
	if (!m_tokenizer.Tokenize(path))
	{
		return false;
	}

	// Parse tokens into a key/value representation.
	{
		Time::TimedScope scope(
			Strings::Format("[%s] Parsing", path.ToString().c_str())
		);

		while (!EndOfTokens())
		{
			if (!ParseStatement())
			{
				return false;
			}
		}
	}

	return true;
}

void ConfigFile::SetOrAddValue(
	const std::string& group,
	const std::string& key,
	const std::string& value,
	bool bOverwrite)
{
	std::vector<std::string> values;
	values.push_back(value);

	SetOrAddValue_Internal(group, key, values, bOverwrite);
}

void ConfigFile::SetOrAddValue(
	const std::string& group,
	const std::string& key,
	const std::vector<std::string>& value,
	bool bOverwrite)
{
	SetOrAddValue_Internal(group, key, value, bOverwrite);
}

std::vector<ConfigFileValue*> ConfigFile::SetOrAddValue_Internal(
	const std::string& group,
	const std::string& key,
	const std::vector<std::string>& values,
	bool bOverwrite)
{
	// Create group if it dosen't exist.
	auto iter = m_groups.find(group);
	if (iter == m_groups.end())
	{
		ConfigFileGroup* newGroup = new ConfigFileGroup();
		newGroup->Name = group;
		m_groups.insert(
			std::pair<std::string, ConfigFileGroup*>(newGroup->Name, newGroup)
		);
	}

	std::vector<ConfigFileValue*> keyValues;
	for (const std::string& value : values)
	{
		ConfigFileValue* keyValue = new ConfigFileValue();
		keyValue->Value = value;
		keyValue->ConditionResult = true;
		keyValues.push_back(keyValue);
	}
	
	ConfigFileGroup* realGroup = m_groups[group];

	// Create key if it dosesn't exist, otherwise add it.
	auto keyIter = realGroup->Keys.find(key);
	if (keyIter == realGroup->Keys.end())
	{
		ConfigFileKey* newKey = new ConfigFileKey();
		newKey->Name = key;
		newKey->Values.insert(
			newKey->Values.end(), keyValues.begin(), keyValues.end()
		);
		realGroup->Keys.insert(
			std::pair<std::string, ConfigFileKey*>(newKey->Name, newKey)
			);
	}
	else
	{
		if (bOverwrite)
		{
			for (auto iter : keyIter->second->Values)
			{
				delete iter;
			}
			keyIter->second->Values.clear();
		}

		keyIter->second->Values.insert(
			keyIter->second->Values.end(), keyValues.begin(), keyValues.end()
		);
	}

	return keyValues;
}

bool ConfigFile::ResolveTokenReplacement(
	const std::string& key,
	const std::string& group,
	std::string& result)
{
	std::string finalKey = key;
	std::string finalGroup = group;

	bool bIsExplicitGroup = false;

	size_t offset = finalKey.find('.');
	if (offset != std::string::npos)
	{
		finalGroup = finalKey.substr(0, offset);
		finalKey = finalKey.substr(offset + 1);

		bIsExplicitGroup = true;
	}

	result = "";

	auto iter = m_groups.find(finalGroup);
	if (iter != m_groups.end())
	{
		ConfigFileGroup* newGroup = iter->second;
		auto keyIter = newGroup->Keys.find(finalKey);
		
		// If key is not in the group we are in, and group was not
		// explicitly set, then try and find it in the global scope.
		if (keyIter == newGroup->Keys.end() && !bIsExplicitGroup)
		{
			newGroup = m_groups.at("");
			keyIter = newGroup->Keys.find(finalKey);
		}
		
		if (keyIter != newGroup->Keys.end())
		{
			for (ConfigFileValue* value : keyIter->second->Values)
			{
				if (!value->HasResolvedCondition)
				{
					value->HasResolvedCondition = true;
					value->ConditionResult = EvaluateConditions(
						value->Conditions, newGroup->Name);
				}

				if (value->ConditionResult)
				{
					if (!value->HasResolvedValue)
					{
						value->HasResolvedValue = true;
						value->ResolvedValue = ReplaceTokens(
							value->Value,
							newGroup->Name
						);
					}

					result = value->ResolvedValue;

					return true;
				}
			}
		}
	}

	return false;
}

std::string ConfigFile::ReplaceTokens(
	const std::string& value,
	const std::string& baseGroup
	)
{
	std::string result = value;

	while (true)
	{
		size_t start_offset = result.find("$(");
		if (start_offset != std::string::npos)
		{
			size_t end_offset = result.find(")", start_offset);
			
			std::string key = result.substr(
				start_offset + 2,
				(end_offset - start_offset) - 2);

			std::string resolvedToken;
			bool res = ResolveTokenReplacement(
				key, 
				baseGroup,
				resolvedToken);

			if (!res)
			{
				// todo: emit a warning here?
				resolvedToken = "[Invalid Token Expansion]";
			}

			result.erase(
				result.begin() + start_offset,
				result.begin() + end_offset + 1);

			result.insert(
				start_offset, 
				resolvedToken);
		}
		else
		{
			break;
		}
	}

	return result;
}

ConfigFileExpressionResult ConfigFile::EvaluateExpression(
	ConfigFileExpression& expression,
	const std::string& baseGroup)
{
	ConfigFileExpressionResult result;

	switch (expression.Operator)
	{
	case TokenType::Expression:
		{
			assert(expression.Children.size() == 1);
			result = EvaluateExpression(expression.Children[0], baseGroup);
			break;
		}
	case TokenType::Not:
		{
			assert(expression.Children.size() == 1);
			result = EvaluateExpression(expression.Children[0], baseGroup);
			result = !result.ToBool();
			break;
		}
	case TokenType::Greater:
		{
			assert(expression.Children.size() == 2);

			ConfigFileExpressionResult lValueResult =
				EvaluateExpression(expression.Children[0], baseGroup);
			ConfigFileExpressionResult rValueResult =
				EvaluateExpression(expression.Children[1], baseGroup);

			result = (lValueResult.ToFloat() > rValueResult.ToFloat());

			break;
		}
	case TokenType::GreaterEqual:
		{
			assert(expression.Children.size() == 2);

			ConfigFileExpressionResult lValueResult =
				EvaluateExpression(expression.Children[0], baseGroup);
			ConfigFileExpressionResult rValueResult =
				EvaluateExpression(expression.Children[1], baseGroup);

			result = (lValueResult.ToFloat() >= rValueResult.ToFloat());

			break;
		}
	case TokenType::Less:
		{
			assert(expression.Children.size() == 2);

			ConfigFileExpressionResult lValueResult =
				EvaluateExpression(expression.Children[0], baseGroup);
			ConfigFileExpressionResult rValueResult =
				EvaluateExpression(expression.Children[1], baseGroup);

			result = (lValueResult.ToFloat() < rValueResult.ToFloat());

			break;
		}
	case TokenType::LessEqual:
		{
			assert(expression.Children.size() == 2);

			ConfigFileExpressionResult lValueResult =
				EvaluateExpression(expression.Children[0], baseGroup);
			ConfigFileExpressionResult rValueResult =
				EvaluateExpression(expression.Children[1], baseGroup);

			result = (lValueResult.ToFloat() <= rValueResult.ToFloat());

			break;
		}
	case TokenType::Equal:
		{
			assert(expression.Children.size() == 2);

			ConfigFileExpressionResult lValueResult =
				EvaluateExpression(expression.Children[0], baseGroup);
			ConfigFileExpressionResult rValueResult =
				EvaluateExpression(expression.Children[1], baseGroup);

			result = (lValueResult.Result == rValueResult.Result);

			break;
		}
	case TokenType::NotEqual:
		{
			assert(expression.Children.size() == 2);

			ConfigFileExpressionResult lValueResult =
				EvaluateExpression(expression.Children[0], baseGroup);
			ConfigFileExpressionResult rValueResult =
				EvaluateExpression(expression.Children[1], baseGroup);

			result = (lValueResult.Result != rValueResult.Result);

			break;
		}
	case TokenType::And:
		{
			assert(expression.Children.size() == 2);

			ConfigFileExpressionResult lValueResult =
				EvaluateExpression(expression.Children[0], baseGroup);
			ConfigFileExpressionResult rValueResult =
				EvaluateExpression(expression.Children[1], baseGroup);

			result = lValueResult.ToBool() && rValueResult.ToBool();

			break;
		}
	case TokenType::Or:
		{
			assert(expression.Children.size() == 2);

			ConfigFileExpressionResult lValueResult = 
				EvaluateExpression(expression.Children[0], baseGroup);
			ConfigFileExpressionResult rValueResult =
				EvaluateExpression(expression.Children[1], baseGroup);

			result = lValueResult.ToBool() || rValueResult.ToBool();

			break;
		}
	case TokenType::Literal:
		{
			assert(expression.Children.size() == 0);

			bool ret = ResolveTokenReplacement(
				expression.Value, baseGroup, result.Result
			);


			if (!ret)
			{
				result.Result = expression.Value;
			}

			break;
		}
	}

	if (expression.Invert)
	{
		result = !result.ToBool();
	}


	assert((int)expression.Operator != 0xdddddddd);

	return result;
}

bool ConfigFile::EvaluateConditions(
	std::vector<ConfigFileExpression>& conditions, 
	std::string groupName)
{
	bool bResult = true;
	
	// todo: fix this, this should be totally unneccessary, without it
	//		 at the moment we end up with some invalidated-iter problems.
	std::vector<ConfigFileExpression> copy = conditions;

	for (ConfigFileExpression& cond : copy)
	{
		if (!EvaluateExpression(cond, groupName).ToBool())
		{
			bResult = false;
			break;
		}
	}

	return bResult;
}

void ConfigFile::Resolve()
{
	// Go through each key value and do token replacement.
	{
		Time::TimedScope scope(
			Strings::Format("[%s] Pre Resolve Setup",
				m_path.ToString().c_str())
			);

		for (auto groupIter : m_groups)
		{
			for (auto keyIter : groupIter.second->Keys)
			{
				for (auto valueIter : keyIter.second->Values)
				{
					valueIter->HasResolvedValue = false;
					valueIter->HasResolvedCondition = false;
				}
			}
		}
	}

	// Evaluate each expression.
	{
		Time::TimedScope scope(
			Strings::Format("[%s] Expression Evaluation",
				m_path.ToString().c_str())
			);

		for (auto groupIter : m_groups)
		{
			for (auto keyIter : groupIter.second->Keys)
			{
				for (auto valueIter : keyIter.second->Values)
				{
					if (!valueIter->HasResolvedCondition)
					{
						valueIter->HasResolvedCondition = true;
						valueIter->ConditionResult = EvaluateConditions(
							valueIter->Conditions,
							groupIter.second->Name
						);
					}
				}
			}
		}
	}

	// Go through each key value and do token replacement.
	{
		Time::TimedScope scope(
			Strings::Format("[%s] Token Replacement",
				m_path.ToString().c_str())
			);

		for (auto groupIter : m_groups)
		{
			for (auto keyIter : groupIter.second->Keys)
			{
				for (auto valueIter : keyIter.second->Values)
				{
					if (!valueIter->HasResolvedValue)
					{
						valueIter->HasResolvedValue = true;
						valueIter->ResolvedValue = ReplaceTokens(
							valueIter->Value,
							groupIter.second->Name
						);
					}
				}
			}
		}
	}
}

std::vector<std::string> ConfigFile::GetValues(
	const std::string& group,
	const std::string& key)
{
	std::vector<std::string> result;

	auto iter = m_groups.find(group);
	if (iter != m_groups.end())
	{
		ConfigFileGroup* newGroup = iter->second;
		auto keyIter = newGroup->Keys.find(key);
		if (keyIter != newGroup->Keys.end())
		{
			for (const ConfigFileValue* value : keyIter->second->Values)
			{
				if (value->ConditionResult)
				{
					result.push_back(value->ResolvedValue);
				}
			}
		}
	}

	return result;
}

std::string ConfigFile::GetValue(
	const std::string& group,
	const std::string& key)
{
	std::vector<std::string> values = GetValues(group, key);
	if (values.size() == 0)
	{
		return "";
	}
	return values[0];
}

std::string ConfigFile::GetValue(
	const std::string& group,
	const std::string& key,
	const std::string& defValue)
{
	std::vector<std::string> values = GetValues(group, key);
	if (values.size() == 0)
	{
		return defValue;
	}
	return values[0];
}

bool ConfigFile::HasValue(
	const std::string& group,
	const std::string& key)
{
	std::vector<std::string> values = GetValues(group, key);
	return (values.size() != 0);
}

void ConfigFile::Merge(const ConfigFile& file)
{
	for (auto& groupIter : file.m_groups)
	{
		for (auto& keyIter : groupIter.second->Keys)
		{
			// Do not insert values if they already exist.
			if (HasValue(groupIter.second->Name, keyIter.second->Name))
			{
				continue;
			}

			for (auto& valueIter : keyIter.second->Values)
			{
				std::vector<std::string> values;
				values.push_back(valueIter->Value);

				std::vector<ConfigFileValue*> result = SetOrAddValue_Internal(groupIter.second->Name,
					keyIter.second->Name,
					values);
				
				for (ConfigFileValue* value : result)
				{
					*value = ConfigFileValue(*valueIter);
				}
			}
		}
	}
}

Platform::Path ConfigFile::GetPath() const
{
	return m_path;
}

}; // namespace MicroBuild