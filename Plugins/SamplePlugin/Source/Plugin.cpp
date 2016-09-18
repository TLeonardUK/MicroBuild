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

#include "Commands/Say.h"
#include "Core/Plugin/PluginInterface.h"

MicroBuildPlugin();

namespace MicroBuild {

bool PluginMain(CurrentPluginInterface* pluginInterface)
{
	pluginInterface->SetName("Sample Plugin");
	pluginInterface->SetDescription("Sample plugin implementation, registers "
		"the 'say' command line.");

	pluginInterface->RegisterCommand(new MicroBuild::SayCommand());

	pluginInterface->RegisterCallback(EPluginEvent::PostProcessWorkspaceFile, [](PluginEventData* Data) {
		MB_UNUSED_PARAMETER(Data);
		return true;
	});

	pluginInterface->RegisterCallback(EPluginEvent::PostProcessProjectFile, [](PluginEventData* Data) {
		MB_UNUSED_PARAMETER(Data);
		return true;
	});

	return true;
}

}; // namespace MicroBuild