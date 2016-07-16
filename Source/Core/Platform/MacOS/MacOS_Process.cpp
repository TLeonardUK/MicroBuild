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
#include "Core/Platform/Path.h"
#include "Core/Platform/Process.h"

#ifdef MB_PLATFORM_MACOS

namespace MicroBuild {
namespace Platform {

struct MacOS_Process
{
};

Process::Process()
{
	m_impl = new MacOS_Process();

	MacOS_Process* data = reinterpret_cast<MacOS_Process*>(m_impl);
}

Process::~Process()
{
	if (IsAttached())
	{
		Detach();
	}

	delete reinterpret_cast<MacOS_Process*>(m_impl);
}

bool Process::Open(
	const Path& command,
	const Path& workingDirectory,
	const std::vector<std::string>& arguments,
	bool bRedirectStdInOut)
{
	MacOS_Process* data = reinterpret_cast<MacOS_Process*>(m_impl);

	// todo
	return false;
}

void Process::Detach()
{
	MacOS_Process* data = reinterpret_cast<MacOS_Process*>(m_impl);
	// todo
}

void Process::Terminate()
{
	MacOS_Process* data = reinterpret_cast<MacOS_Process*>(m_impl);
	// todo
}

bool Process::IsRunning()
{
	MacOS_Process* data = reinterpret_cast<MacOS_Process*>(m_impl);

	// todo
	return false;
}

bool Process::IsAttached()
{
	// todo
	return false;
}

bool Process::Wait()
{
	// todo
	return false;
}

int Process::GetExitCode()
{
	// todo
	return 0;
}

bool Process::Write(void* buffer, uint64_t bufferLength)
{
	// todo
	return false;
}

bool Process::Read(void* Bbffer, uint64_t bufferLength)
{
	// todo
	return false;
}

bool Process::AtEnd()
{
	// todo
	return false;
}

void Process::Flush()
{
	// todo
}

uint64_t Process::BytesLeft()
{
	// todo
	return 0;
}

}; // namespace Platform
}; // namespace MicroBuild

#endif // MB_PLATFORM_MACOS