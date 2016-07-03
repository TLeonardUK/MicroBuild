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
#include "Core/Templates/TemplateEvaluator.h"
#include "Core/Helpers/Time.h"
#include "Core/Helpers/Strings.h"

#include <sstream>

namespace MicroBuild {

TemplateEvaluator::TemplateEvaluator()
	: m_result("")
{
}

TemplateEvaluator::~TemplateEvaluator()
{
}

void TemplateEvaluator::Error(const std::string& id, const char* format, ...)
{
	va_list list;
	va_start(list, format);

	std::string message = Strings::FormatVa(format, list);

	va_end(list);

	std::string value = Strings::Format("%s: %s\n",
		id.c_str(),
		message.c_str());

	Log(LogSeverity::Fatal, "%s", value.c_str());
}

int TemplateEvaluator::AddArgument(
	const std::string& name,
	const std::string& value,
	int parentIndex)
{
	TemplateArgument arg;
	arg.Name = name;
	arg.Value = value;
	arg.ParentIndex = parentIndex;

	int index = (int)m_arguments.size();
	arg.Index = index;

	m_arguments.push_back(arg);

	if (parentIndex >= 0)
	{
		m_arguments[parentIndex].ChildrenIndices.push_back(index);
	}

	return index;
}

TemplateArgument* TemplateEvaluator::GetArgument(
	const std::string& name,
	std::vector<TemplateScope>* scopeStack)
{
	size_t pointOffset = name.find_last_of('.');

	// Scoped value.
	if (pointOffset != std::string::npos)
	{
		std::string parentName = name.substr(0, pointOffset);
		std::string valueName = name.substr(pointOffset + 1);

		TemplateArgument* parent = GetArgument(
			parentName, scopeStack);

		if (parent != nullptr)
		{
			for (int childIndex : parent->ChildrenIndices)
			{
				TemplateArgument* arg = &m_arguments[childIndex];
				if (Strings::CaseInsensitiveEquals(arg->Name, valueName))
				{
					return arg;
				}
			}
		}
	}

	// Base level.
	else
	{
		// Look through scope variables first.
		for (TemplateScope& scope : *scopeStack)
		{
			if (Strings::CaseInsensitiveEquals(scope.ArgumentName, name))
			{
				return &m_arguments[scope.RootArgument->ChildrenIndices[scope.Offset]];
			}
		}
		
		// Else look through defined arguments.
		for (TemplateArgument& argument : m_arguments)
		{
			if (argument.ParentIndex == -1 &&
				Strings::CaseInsensitiveEquals(argument.Name, name))
			{
				return &argument;
			}
		}
	}

	return nullptr;
}


void TemplateEvaluator::SkipToEndOfScope(
	int& cursor, std::vector<TemplateFragment>& fragments)
{
	int depth = 1;

	while (cursor < fragments.size() && depth > 0)
	{
		TemplateFragment& frag = fragments[cursor];
		if (frag.bIsTag)
		{
			if (frag.Tokens[0] == "end")
			{
				depth--;
			}
			else if (frag.Tokens[0] == "for")
			{
				depth++;
			}
		}

		cursor++;
	}
}

bool TemplateEvaluator::Evaluate(
	const std::string& id, 
	const std::string& output)
{
	Time::TimedScope timedScope(
		Strings::Format("[%s] Template Evaluation", id.c_str())
	);

	std::stringstream stream;
	std::vector<TemplateScope> scopeStack;
	std::vector<TemplateFragment> fragments;

	m_result = "";

	// Split up into fragments.
	size_t offset = 0;
	size_t lastOpenOffset = 0;
	while (offset < output.size())
	{
		size_t openOffset = output.find("{%", offset);

		// Next tag to expand!
		if (openOffset != std::string::npos)
		{
			// Initial fragment.		
			if (openOffset > offset)
			{
				TemplateFragment frag;
				frag.bIsTag = false;
				frag.Value = output.substr(offset, openOffset - offset);
				fragments.push_back(frag);
			}

			// Grab tag fragment.
			size_t closeOffset = output.find("%}", openOffset);
			if (closeOffset == std::string::npos)
			{
				Error(id, "Encountered unclosed tag.");
				return false;
			}
			else
			{
				TemplateFragment frag;
				frag.bIsTag = true;
				frag.Value = Strings::ToLowercase(Strings::Trim(
					output.substr(openOffset + 2, (closeOffset - openOffset) - 2)
				));
				frag.Tokens = Strings::Split(' ', frag.Value);
				fragments.push_back(frag);

				offset = closeOffset + 2;

				// If next value is a newline, and we are a scope tag, then
				// skip it to prevent empty-lines.
				if (offset < output.size())
				{
					if (output[offset] == '\n')
					{
						if (frag.Tokens.size() > 0)
						{
							if (frag.Tokens[0] == "for" ||
								frag.Tokens[0] == "end")
							{
								offset++;
							}
						}
					}
				}
			}

			lastOpenOffset = lastOpenOffset;
		}

		// No more tags, add everything left.
		else
		{
			TemplateFragment frag;
			frag.bIsTag = false;
			frag.Value = output.substr(offset);
			fragments.push_back(frag);

			break;
		}
	}

	// Parse through the fragments while taking control structures into account.
	int cursor = 0;
	while (cursor < fragments.size())
	{
		TemplateFragment& frag = fragments[cursor];
		if (frag.bIsTag)
		{
			// ----------------------------------------------------------------
			if (frag.Tokens[0] == "end")
			{
				if (frag.Tokens.size() != 1)
				{
					Error(id, "Encountered malformed end tag.");
					return false;
				}

				TemplateScope& scope = scopeStack[scopeStack.size() - 1];

				// If there are more variables left, jump back to start.
				if (scope.Offset + 1 < scope.RootArgument->ChildrenIndices.size())
				{
					scope.Offset++;
					cursor = scope.StartIndex;

					continue;
				}
				else
				{
					scopeStack.pop_back();
				}
			}

			// ----------------------------------------------------------------
			else if (frag.Tokens[0] == "for")
			{
				if (frag.Tokens.size() != 4 &&
					frag.Tokens[2] != "in")
				{
					Error(id, "Encountered malformed for tag.");
					return false;
				}

				TemplateArgument* arg = GetArgument(frag.Tokens[3], &scopeStack);
				if (arg == nullptr)
				{
					Error(id, "Encountered unknown template argument '%s'.",
						frag.Tokens[3].c_str());
					return false;
				}
				else
				{
					TemplateScope scope;
					scope.RootArgument = arg;
					scope.Offset = 0;
					scope.ArgumentName = frag.Tokens[1];
					scope.StartIndex = cursor + 1;

					if (arg->ChildrenIndices.size() <= 0)
					{
						cursor++;
						SkipToEndOfScope(cursor, fragments);
						continue;
					}
					else
					{
						scopeStack.push_back(scope);
					}
				}
			}

			// ----------------------------------------------------------------
			else
			{
				if (frag.Tokens.size() != 1)
				{
					Error(id, "Encountered malformed for value tag.");
					return false;
				}
		
				TemplateArgument* arg = GetArgument(frag.Tokens[0], &scopeStack);
				if (arg == nullptr)
				{
					Error(id, "Encountered unknown template argument '%s'.",
						frag.Tokens[0].c_str());
					return false;
				}
				else
				{
					stream << arg->Value;
				}
			}
		}
		else
		{
			stream << frag.Value;
		}

		cursor++;
	}

	m_result = stream.str();

	return true;
}

std::string TemplateEvaluator::GetResult()
{
	return m_result;
}

}; // namespace MicroBuild