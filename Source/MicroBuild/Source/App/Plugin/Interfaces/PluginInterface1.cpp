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
#include "App/Plugin/Plugin.h"
#include "App/Plugin/PluginManager.h"
#include "Core/Plugin/Interfaces/PluginInterface1.h"

namespace MicroBuild {

class PluginInterface1_Impl : public PluginInterface1
{
public:
	PluginInterface1_Impl(PluginManager* manager, Plugin* plugin)
		: m_manager(manager)
		, m_plugin(plugin)
	{
	}

	virtual void SetName(const std::string& value) override
	{
		m_name = value;
	}

	virtual std::string GetName() const override
	{
		return m_name;
	}

	virtual void SetDescription(const std::string& value) override
	{
		m_description = value;
	}

	virtual std::string GetDescription() const override
	{
		return m_description;
	}

	virtual void RegisterCommand(Command* cmd) override
	{
		m_manager->GetApp()->RegisterCommand(cmd);
	}

	virtual void RegisterCallback(EPluginEvent Event, PluginCallbackSignature FuncPtr) override
	{
		m_plugin->RegisterCallback(Event, FuncPtr);
	}

private:
	std::string m_name;
	std::string m_description;
	PluginManager* m_manager;
	Plugin* m_plugin;

};

IPluginInterface* CreatePluginInterface1(PluginManager* manager, Plugin* plugin)
{
	return new PluginInterface1_Impl(manager, plugin);
}

} // namespace MicroBuild