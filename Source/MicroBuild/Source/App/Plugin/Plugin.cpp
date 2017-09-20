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

#include "Schemas/Plugin/PluginInterface.h"

#include "App/Plugin/PluginManager.h"
#include "App/Plugin/Plugin.h"

namespace MicroBuild {

IPluginInterface* CreatePluginInterface(PluginManager* manager, Plugin* plugin);

Plugin::Plugin(PluginManager* manager)
	: m_name("")
	, m_manager(manager)
{
}

Plugin::~Plugin()
{
	TerminatePlugin(m_pluginInterface);
	
	m_callbacks.clear();

	if (m_pluginInterface)
	{
		delete m_pluginInterface;
		m_pluginInterface = nullptr;
	}

	if (m_module.IsOpen())
	{
		m_module.Close();
	}
}

std::string Plugin::GetName()
{
	return m_name;
}

std::string Plugin::GetFileName()
{
	return m_fileName;
}

void Plugin::RegisterCallback(EPluginEvent Event, PluginCallbackSignature FuncPtr)
{
	PluginCallback callback;
	callback.Event = Event;
	callback.Callback = FuncPtr;
	m_callbacks.push_back(callback);
}

bool Plugin::OnEvent(EPluginEvent Event, PluginEventData* Data)
{
	for (PluginCallback& callback : m_callbacks)
	{
		if (callback.Event == Event)
		{
			if (!callback.Callback(Data))
			{
				return false;
			}
		}
	}

	return true;
}

bool Plugin::Load(Platform::Path& path)
{
	if (m_module.Open(path))
	{
		InitializePlugin = m_module.GetFunction<InitializePlugin_t>("InitializePlugin");
		TerminatePlugin = m_module.GetFunction<TerminatePlugin_t>("TerminatePlugin");
		 
		if (InitializePlugin == nullptr ||
			TerminatePlugin == nullptr)
		{
			Log(LogSeverity::Warning,
				"Failed to load plugin, missing symbol exports.\n");

			return false;
		}

		m_pluginInterface = CreatePluginInterface(m_manager, this);

		if (!InitializePlugin(m_pluginInterface))
		{
			Log(LogSeverity::Warning,
				"Failed to initialize plugin.\n");
			return false;
		}

		m_fileName = path.GetBaseName();

		//Log(LogSeverity::Info, "\tName: %s\n", m_pluginInterface->GetName().c_str());
		//Log(LogSeverity::Info, "\tDescription: %s\n", m_pluginInterface->GetDescription().c_str());

		return true;
	}

	return false;
}

} // namespace MicroBuild