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
#include "App/Config/BaseConfigFile.h"

namespace MicroBuild {

BaseConfigFile::BaseConfigFile()
{
}

BaseConfigFile::~BaseConfigFile()
{
}

void BaseConfigFile::SetTargetConfiguration(const std::string& config)
{
	m_targetConfiguration = config;
}

void BaseConfigFile::SetTargetPlatform(const std::string& platform)
{
	m_targetPlatform = platform;
}

void BaseConfigFile::SetTargetIde(const std::string& ide)
{
	m_targetIde = ide;
}

void BaseConfigFile::Resolve()
{
	// Host settings.
#if defined(MB_PLATFORM_WINDOWS)
	SetOrAddValue("Host", "Platform", "Windows");
#elif defined(MB_PLATFORM_LINUX)
	SetOrAddValue("Host", "Platform", "Linux");
#elif defined(MB_PLATFORM_MACOS)
	SetOrAddValue("Host", "Platform", "MacOS");
#else
	#error Unimplemented platform.
#endif

	// Target settings
	SetOrAddValue("Target", "Ide", m_targetIde  );
	SetOrAddValue("Target", "Platform", m_targetPlatform);
	SetOrAddValue("Target", "Configuration", m_targetConfiguration);

	// Environment settings.
	// todo
	//SetOrAddValue("Environment", "WorkingDir", Path::GetWorkingDir());

	ConfigFile::Resolve();
}

}; // namespace MicroBuild