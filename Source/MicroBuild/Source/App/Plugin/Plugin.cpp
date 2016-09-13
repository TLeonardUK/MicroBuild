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

#include "Core/Plugin/PluginInterface.h"
#include "Core/Plugin/Interfaces/PluginInterface1.h"

#include "App/Plugin/PluginManager.h"
#include "App/Plugin/Plugin.h"

namespace MicroBuild {

IPluginInterface* CreatePluginInterface1(PluginManager* manager, Plugin* plugin);

Plugin::Plugin(PluginManager* manager)
	: m_name("")
	, m_manager(manager)
{
}

Plugin::~Plugin()
{
	if (m_module.IsOpen())
	{
		m_module.Close();
	}
}

std::string Plugin::GetName()
{
	return m_name;
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
		GetPluginInterfaceVersion = m_module.GetFunction<GetPluginInterfaceVersion_t>("GetPluginInterfaceVersion");
		InitializePlugin = m_module.GetFunction<InitializePlugin_t>("InitializePlugin");

		if (GetPluginInterfaceVersion == nullptr ||
			InitializePlugin == nullptr)
		{
			Log(LogSeverity::Warning,
				"Failed to load plugin, missing symbol exports.\n");

			return false;
		}

		int interfaceVersion = GetPluginInterfaceVersion();
		switch (interfaceVersion)
		{
			case 1: 
			{
				m_pluginInterface = CreatePluginInterface1(m_manager, this); 
				break;
			}
			default:
			{
				Log(LogSeverity::Warning,
					"Failed to load plugin, unsupported plugin version %i.\n",
					interfaceVersion);
				return false;
			}
		}

		if (!InitializePlugin(m_pluginInterface))
		{
			Log(LogSeverity::Warning,
				"Failed to initialize plugin.\n",
				interfaceVersion);
			return false;
		}

		PluginInterface1* baseInterface = static_cast<PluginInterface1*>(m_pluginInterface);

		Log(LogSeverity::Info, "\tName: %s\n", baseInterface->GetName().c_str());
		Log(LogSeverity::Info, "\tDescription: %s\n", baseInterface->GetDescription().c_str());


		return true;
	}

	return false;
}

} // namespace MicroBuild