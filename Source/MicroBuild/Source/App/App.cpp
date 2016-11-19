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
#include "App/Commands/Package.h"
#include "App/Commands/Clean.h"
#include "App/Commands/Help.h"
#include "App/Commands/Version.h"

#include "App/Ides/IdeType.h"

#include "App/Ides/MSBuild/Versions/VisualStudio_2015.h"
#include "App/Ides/Make/Make.h"
#include "App/Ides/XCode/XCode.h"

#include "Core/Config/ConfigFile.h"
#include "Core/Helpers/Time.h"
#include "Core/Helpers/Strings.h"
#include "Core/Helpers/Image.h"

#include "FreeImage.h"

// x86
// x64
// anycpu
// winrt
// ios
// android
// emscripten
// ps4
// xboxone

// c++
// c#

// visual studio
// monodevelop
// netbeans
// qtcreator
// codeblocks
// sharpdevelop
// codelite
// make
// xcode

namespace MicroBuild {

App::App(int argc, char* argv[])    
	: m_argv(argv)
	, m_argc(argc)
	, m_commandLineParser(MB_NAME, MB_DESCRIPTION, MB_COPYRIGHT)
	, m_pluginManager(this)
{
	Platform::Path::SetExecutablePath(argv[0]);

	FreeImage_Initialise(false);

	m_ides.push_back(new Ide_VisualStudio_2015());
	m_ides.push_back(new Ide_Make());
	m_ides.push_back(new Ide_XCode());

	m_commandLineParser.RegisterCommand(new GenerateCommand(this));
	m_commandLineParser.RegisterCommand(new BuildCommand(this));
	m_commandLineParser.RegisterCommand(new PackageCommand(this));
	m_commandLineParser.RegisterCommand(new CleanCommand(this));
	m_commandLineParser.RegisterCommand(new HelpCommand(this));
	m_commandLineParser.RegisterCommand(new VersionCommand(this));
}

App::~App()
{
	m_commandLineParser.DisposeCommands();

	for (IdeType* type : m_ides)
	{
		delete type;
	}
	m_ides.clear();
	
	FreeImage_DeInitialise();
}


void App::RegisterCommand(Command* command)
{
	m_commandLineParser.RegisterCommand(command);
}

std::vector<IdeType*> App::GetIdes() const
{
	return m_ides;
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

PluginManager* App::GetPluginManager()
{
	return &m_pluginManager;
}

int App::Run()
{
	if (!m_commandLineParser.PreParse(m_argc, m_argv))
	{
		return 1;
	}

	m_commandLineParser.PrintLicense();

#if MB_OPT_ENABLE_PLUGIN_SUPPORT
	if (!m_pluginManager.FindAndLoadAll())
	{
		return 1;
	}
#endif

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