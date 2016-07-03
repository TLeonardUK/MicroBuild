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

#include "Core/Config/ConfigFile.h"
#include "Core/Helpers/Strings.h"
#include "Core/Helpers/StringConverter.h"

namespace MicroBuild {

#define SCHEMA_FILE "App/Config/BaseSchema.inc"
#define SCHEMA_CLASS BaseConfigFile
#include "App/Config/SchemaGlobalDecl.h"
#undef SCHEMA_FILE
#undef SCHEMA_CLASS

// Base config file that our project/workspace files are based off, used to
// define various file-agnostic variables (host vars etc).
class BaseConfigFile : public ConfigFile
{
public:
	BaseConfigFile();
	~BaseConfigFile();

	virtual void Resolve() override;

	// Shows a validation error.
	void ValidateError(const char* format, ...) const;

protected:

	// If path is relative it is made absolute based on the workspace file 
	// path, otherwise it is returned as-is.
	Platform::Path ResolvePath(Platform::Path& value) const;

	// Validates this configuration file works with this version of microbuild.
	bool ValidateVersion(std::vector<std::string>& values) const;

	// Expands any wildcard filters in the given path list.
	bool ExpandPaths(std::vector<std::string>& options) const;

	// Validates that all the values in the given array are also in 
	// the optins arrays.
	bool ValidateOptions(
		std::vector<std::string>& values,
		std::vector<std::string>& options,
		std::string& group,
		std::string& key) const;

private:

#define SCHEMA_FILE "App/Config/BaseSchema.inc"
#define SCHEMA_CLASS BaseConfigFile
#define SCHEMA_IS_BASE1
#include "App/Config/SchemaDecl.h"
#undef SCHEMA_FILE
#undef SCHEMA_CLASS
#undef SCHEMA_IS_BASE

};

}; // namespace MicroBuild