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
#include "App/Plugin/Plugin.h"
#include "App/Plugin/PluginManager.h"
#include "Core/Platform/Path.h"
#include "Core/Helpers/Strings.h"

namespace MicroBuild {

PluginManager::PluginManager(App* app)
	: m_app(app)
{
	std::string extension = "";
	std::string architecture = "";
	std::string configuration = "";

#if defined(MB_PLATFORM_WINDOWS)
	extension = "dll";
#elif defined(MB_PLATFORM_MACOS)
	extension = "dylib";
#elif defined(MB_PLATFORM_LINUX)
	extension = "so";
#endif

#if defined(MB_ARCHITECTURE_X64)
	architecture = "x64";
#elif defined(MB_ARCHITECTURE_X86)
	architecture = "x86";
#endif

#if defined(MB_DEBUG_BUILD)
	configuration = "Debug";
#elif defined(MB_RELEASE_BUILD)
	configuration = "Release";
#elif defined(MB_SHIPPING_BUILD)
	configuration = "Shipping";
#endif

	m_fileFormat =
		Strings::Format(
			".plugin.%s.%s.%s",
			architecture.c_str(),
			configuration.c_str(),
			extension.c_str()
		);
	m_recursiveFilter =
		Strings::Format(
			"**%s",
			m_fileFormat.c_str()
		);
	m_filter =
		Strings::Format(
			"*%s",
			m_fileFormat.c_str()
		);

	Platform::Path exeDirectory = Platform::Path::GetExecutablePath().GetDirectory();
	m_recursivePathFilter = exeDirectory.AppendFragment(m_recursiveFilter, true);
	m_pluginDirectory = exeDirectory;// .AppendFragment("Plugins", true);
}

PluginManager::~PluginManager()
{
	UnloadAll();
}

void PluginManager::UnloadAll()
{
	for (Plugin* plugin : m_plugins)
	{
		delete plugin;
	}
	m_plugins.clear();
}

bool PluginManager::FindAndLoadAll()
{
	Platform::Path pluginPathFilter = 
		m_pluginDirectory.AppendFragment(m_recursiveFilter, true);

	if (!m_pluginDirectory.Exists())
	{
		m_pluginDirectory.CreateAsDirectory();
	}

	std::vector<Platform::Path> matchingPaths =
		Platform::Path::MatchFilter(m_recursivePathFilter);

	matchingPaths =
		Platform::Path::MatchFilter(pluginPathFilter);
	
	for (auto path : matchingPaths)
	{
		Platform::Path loadPath = path;	
		LoadPluginByName(loadPath.GetBaseName());
	}

	if (matchingPaths.size() > 0)
	{
		Log(LogSeverity::Info, "\n");
	}

	return true;
}

std::vector<Plugin*> PluginManager::GetPlugins()
{
	return m_plugins;
}

App* PluginManager::GetApp()
{
	return m_app;
}

bool PluginManager::OnEvent(EPluginEvent Event, PluginEventData* Data)
{
	for (Plugin* plugin : m_plugins)
	{
		if (!plugin->OnEvent(Event, Data))
		{
			return false;
		}
	}
	return true;
}

Plugin* PluginManager::GetPluginByName(const std::string& fileName)
{
	for (Plugin* plugin : m_plugins)
	{
		if (plugin->GetFileName() == fileName)
		{
			return plugin;
		}
	}
	return nullptr;
}

bool PluginManager::IsPluginLoaded(const std::string& fileName)
{
	return GetPluginByName(fileName) != nullptr;
}

bool PluginManager::UnloadPluginByName(const std::string& fileName)
{
	Log(LogSeverity::Info, "Unloading plugin: %s\n", fileName.c_str());

	Plugin* plugin = GetPluginByName(fileName);
	if (plugin == nullptr)
	{
		return false;
	}

	m_plugins.erase(std::find(m_plugins.begin(), m_plugins.end(), plugin));

	delete plugin;

	return true;
}

bool PluginManager::LoadPluginByName(const std::string& fileName)
{
	Log(LogSeverity::Info, "Loading plugin: %s\n", fileName.c_str());

	Platform::Path path = m_pluginDirectory.AppendFragment(
		Strings::Format(
			"%s%s",
			fileName.c_str(),
			m_fileFormat.c_str()
		),
		true
	);

	Plugin* plugin = new Plugin(this);
	if (!plugin->Load(path))
	{
		Log(LogSeverity::Info, "\tPlugin load failed.\n");
		return false;
	}
	else
	{
		m_plugins.push_back(plugin);
	}

	return true;
}

} // namespace MicroBuild