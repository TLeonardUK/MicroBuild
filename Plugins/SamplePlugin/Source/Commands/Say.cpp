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

#include "Core/Commands/CommandLineParser.h"
#include "Core/Commands/CommandStringArgument.h"

namespace MicroBuild {

SayCommand::SayCommand()
{
	SetName("say");
	SetShortName("s");
	SetDescription("Echos out the value passed on the command line.");

	CommandStringArgument* valueArg = new CommandStringArgument();
	valueArg->SetName("Value");
	valueArg->SetShortName("v");
	valueArg->SetDescription("Value to echo");
	valueArg->SetRequired(true );
	valueArg->SetPositional(true);
	valueArg->SetDefault("");
	valueArg->SetOutput(&m_value);
	RegisterArgument(valueArg);
}

bool SayCommand::Invoke(CommandLineParser* parser)
{
	UNUSED_PARAMETER(parser);

	Log(LogSeverity::Info,
		"Say: %s\n",
		m_value.c_str());

	return true;
}

}; // namespace MicroBuild