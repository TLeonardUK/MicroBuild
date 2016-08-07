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
#include "App/Commands/Version.h"

#include "Core/Commands/CommandComboArgument.h"
#include "Core/Commands/CommandPathArgument.h"
#include "Core/Commands/CommandFlagArgument.h"
#include "Core/Commands/CommandStringArgument.h"

namespace MicroBuild {

VersionCommand::VersionCommand(App* app)
	: m_app(app)
{
	SetName("version");
	SetShortName("q");
	SetDescription("Shows version and compile date information.");
}

bool VersionCommand::Invoke(CommandLineParser* parser)
{
	UNUSED_PARAMETER(parser);

	Log(LogSeverity::Info, "Version %.2f\n", MB_VERSION);
	Log(LogSeverity::Info, "Compiled on %s\n", MB_COMPILE_DATE);

	return true;
}

}; // namespace MicroBuild