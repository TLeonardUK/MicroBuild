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
#include "App/App.h"

#include "App/Commands/Generate.h"
#include "App/Commands/Build.h"
#include "App/Commands/Deploy.h"
#include "App/Commands/Clean.h"
#include "App/Commands/Help.h"
#include "App/Commands/Version.h"

#include "App/Ides/IdeType.h"
#include "App/Ides/MSBuild/VisualStudio_2015.h"

#include "App/Platforms/PlatformType.h"
#include "App/Platforms/Windows_x86/Windows_x86.h"
#include "App/Platforms/Windows_x64/Windows_x64.h"

#include "Core/Config/ConfigFile.h"
#include "Core/Helpers/Time.h"
#include "Core/Helpers/Strings.h"

namespace MicroBuild {

App::App(int argc, char* argv[])    
	: m_argc(argc)
	, m_argv(argv)
	, m_commandLineParser(MB_NAME, MB_DESCRIPTION, MB_COPYRIGHT)
{
	m_ides.push_back(new Ide_VisualStudio_2015());

	m_platforms.push_back(new Platform_Windows_x86());
	m_platforms.push_back(new Platform_Windows_x64());

	m_commandLineParser.RegisterCommand(new GenerateCommand(this));
	m_commandLineParser.RegisterCommand(new BuildCommand(this));
	m_commandLineParser.RegisterCommand(new DeployCommand(this));
	m_commandLineParser.RegisterCommand(new CleanCommand(this));
	m_commandLineParser.RegisterCommand(new HelpCommand(this));
	m_commandLineParser.RegisterCommand(new VersionCommand(this));
}

App::~App()
{
	for (IdeType* type : m_ides)
	{
		delete type;
	}
	for (PlatformType* type : m_platforms)
	{
		delete type;
	}

	m_platforms.clear();
	m_ides.clear();
}

std::vector<IdeType*> App::GetIdes() const
{
	return m_ides;
}

std::vector<PlatformType*> App::GetPlatforms() const
{
	return m_platforms;
}

IdeType* App::GetIdeByShortName(const std::string& shortName) const
{
	for (IdeType* type : m_ides)
	{
		if (Strings::CaseInsensitiveEquals(shortName, type->GetShortName()))
		{
			return type;
		}
	}
	return nullptr;
}

int App::Run()
{
	if (m_commandLineParser.Parse(m_argc, m_argv))
	{
		if (m_commandLineParser.HasCommands())
		{
			if (m_commandLineParser.DispatchCommands())
			{
				return 0;
			}
		}
		else
		{
			m_commandLineParser.PrintHelp();
		}
	}

	return 1;
}

} // namespace MicroBuild