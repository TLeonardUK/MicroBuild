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

struct TemplateArgument;

// Fragment of a template, text or tag.
struct TemplateFragment
{
	bool bIsTag;
	std::string Value;
	std::vector<std::string> Tokens;
};

// Type of template scope.
enum class TemplateScopeType
{
	For,
};

// Template scope types.
struct TemplateScope
{
	std::string ArgumentName;
	TemplateArgument* RootArgument;
	int Offset;
	int StartIndex;
};

// Argument that can be passed to a template.
struct TemplateArgument
{
	std::string Name;
	std::string Value;
	int Index;
	int ParentIndex;
	std::vector<int> ChildrenIndices;
};

// Takes a template as a string and expands template tags to produce a 
// final string output.
class TemplateEvaluator
{
public:
	TemplateEvaluator();
	~TemplateEvaluator();

	// Adds an argument to the template evaluator.
	int AddArgument(
		const std::string& name, 
		const std::string& value, 
		int parentIndex = -1);

	// Evaluates the given template.
	bool Evaluate(const std::string& id, const std::string& input);

	// Gets the expanded version of the last template evaluated.
	std::string GetResult();

protected:
	void Error(const std::string& id, const char* format, ...);

	TemplateArgument* GetArgument(
		const std::string& name, 
		std::vector<TemplateScope>* scopeStack);

	void SkipToEndOfScope(
		int& offset, std::vector<TemplateFragment>& fragments);

private:
	std::vector<TemplateArgument> m_arguments;
	std::string m_result;
	
};

}; // namespace MicroBuild

