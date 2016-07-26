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

#ifdef MB_PLATFORM_LINUX

#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>

extern char **environ;

namespace MicroBuild {
namespace Platform {

struct Linux_Process
{
	bool m_attached;
	pid_t m_processId;
};

Process::Process()
{
	m_impl = new Linux_Process();

	Linux_Process* data = reinterpret_cast<Linux_Process*>(m_impl);
	data->m_processId = 0;
	data->m_attached = false;
}

Process::~Process()
{
	if (IsAttached())
	{
		Detach();
	}

	delete reinterpret_cast<Linux_Process*>(m_impl);
}

bool Process::Open(
	const Path& command,
	const Path& workingDirectory,
	const std::vector<std::string>& arguments,
	bool bRedirectStdInOut)
{
	Linux_Process* data = reinterpret_cast<Linux_Process*>(m_impl);
	assert(!IsAttached());

	// Escape and pack arguments into a single command line incase it has spaces!
	std::vector<std::string> argvBase;
	argvBase.push_back(command.ToString());
	argvBase.insert(argvBase.begin(), arguments.begin(), arguments.end());

	std::vector<char*> argv;
	for (const std::string& arg : argvBase)
	{
		// Gross, plz change signature of posix_spawn so this isn't neccessary.
		argv.push_back(const_cast<char*>(arg.c_str())); 
	}
	argv.push_back('\0');

	// Store state.
	data->m_attached = true;

	// Create process.
	int result = posix_spawn(
		&data->m_processId, 
		command.ToString().c_str(),
		nullptr,
		nullptr,
		reinterpret_cast<char* const*>(argv.data()),
		environ
	);

	if (result != 0)
	{
		Log(LogSeverity::Warning, 
			"spawn failed with 0x%08x.\n", result);

		Detach();
		return false;
	}

	return true;
}

void Process::Detach()
{
	Linux_Process* data = reinterpret_cast<Linux_Process*>(m_impl);
	assert(IsAttached());

	data->m_processId = 0;
	data->m_attached = false;
}

void Process::Terminate()
{
	Linux_Process* data = reinterpret_cast<Linux_Process*>(m_impl);
	assert(IsAttached());

	kill(data->m_processId, SIGKILL);
}

bool Process::IsRunning()
{
	Linux_Process* data = reinterpret_cast<Linux_Process*>(m_impl);
	assert(IsAttached());

	int result = kill(data->m_processId, 0);
	return (result == 0);
}

bool Process::IsAttached()
{
	Linux_Process* data = reinterpret_cast<Linux_Process*>(m_impl);

	return data->m_attached;
}

bool Process::Wait()
{
	Linux_Process* data = reinterpret_cast<Linux_Process*>(m_impl);
	assert(IsAttached());

	int resultCode = 0;
	waitpid(data->m_processId, &resultCode, 0);
	return true;
}

int Process::GetExitCode()
{
	Linux_Process* data = reinterpret_cast<Linux_Process*>(m_impl);
	assert(IsAttached());

	int resultCode = 0;
	waitpid(data->m_processId, &resultCode, WNOHANG);
	return WEXITSTATUS(resultCode);
}

bool Process::Write(void* buffer, uint64_t bufferLength)
{	
	// Not currently implemented on this platform.
	assert(false);
	return false;
}

bool Process::Read(void* Bbffer, uint64_t bufferLength)
{
	// Not currently implemented on this platform.
	assert(false);
	return false;
}

bool Process::AtEnd()
{
	// Not currently implemented on this platform.
	assert(false);
	return false;
}

void Process::Flush()
{
	// Not currently implemented on this platform.
	assert(false);
}

uint64_t Process::BytesLeft()
{	
	// Not currently implemented on this platform.
	assert(false);
	return 0;
}

}; // namespace Platform
}; // namespace MicroBuild

#endif // MB_PLATFORM_LINUX
