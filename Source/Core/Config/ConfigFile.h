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

// A condition that determines if a key exists with the given defines/config.
class ConfigFileCondition
{
	std::vector<ConfigFileCondition> Conditions;
};

// An individual key and its associated conditions.
class ConfigFileKey
{
	std::string Name;
	std::string Value;
	ConfigFileCondition Condition;
};

// An individual group of config keys.
class ConfigFileGroup
{
	std::string Name;
	std::vector<ConfigFileKey> Keys;
};

// Our base config file class. Implements a simple INI style file format.
class ConfigFile
{
public:
	ConfigFile();
	~ConfigFile();

	// Parses the config file located at the given path and converts in into
	// group and value keys.
	bool Parse(const Platform::Path& path);

	// Copies all the values that exist in another config file into this one.
	void InheritFrom(const ConfigFile& file);

	// Sets or adds the given value to the group with the given name.
	void SetOrAddValue(
		const std::string& group,
		const std::string& key); 

	// Evaluates all the value conditions after any defines have been set.
	// This saves doing duplicate processing each time we call GetValues.
	void EvaluateConditions();

	// Gets all the values of a given key in the given group.
	std::vector<std::string> GetValues(
		const std::string& group, 
		const std::string& key);

private:
	Platform::Path m_path;
	FILE* m_file;

	std::vector<ConfigFileGroup> m_groups;

};

}; // namespace MicroBuild