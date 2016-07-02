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

namespace MicroBuild {

// Base config file that our project/workspace files are based off, used to
// define various file-agnostic variables (host vars etc).
class BaseConfigFile : public ConfigFile
{
public:
	BaseConfigFile();
	~BaseConfigFile();

	// Set the target configuration we are extracting data for.
	void SetTargetConfiguration(const std::string& config);

	// Set the target platform we are extracting data for.
	void SetTargetPlatform(const std::string& platform);

	// Set the target ide we are extracting data for.
	void SetTargetIde(const std::string& ide);

	virtual void Resolve() override;

protected:


private:
	std::string m_targetConfiguration;
	std::string m_targetPlatform;
	std::string m_targetIde;

};

}; // namespace MicroBuild