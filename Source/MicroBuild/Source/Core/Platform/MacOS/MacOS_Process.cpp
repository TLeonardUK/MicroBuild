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

#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>
#include <signal.h>

extern char **environ;

namespace MicroBuild {
namespace Platform {

struct MacOS_Process
{
	bool m_attached;
	pid_t m_processId;
};

Process::Process()
{
	m_impl = new MacOS_Process();

	MacOS_Process* data = reinterpret_cast<MacOS_Process*>(m_impl);
	data->m_processId = 0;
	data->m_attached = false;
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
	assert(!IsAttached());

	// Escape and pack arguments into a single command line incase it has spaces!
	std::vector<std::string> argvBase;
	argvBase.push_back(command.ToString());
	argvBase.insert(argvBase.begin() + 1, arguments.begin(), arguments.end());

    char** argv = new char*[argvBase.size() + 1];
	for (int i = 0; i < (int)argvBase.size(); i++)
	{
        argv[i] = new char[argvBase[i].size() + 1];
        memcpy(argv[i], argvBase[i].c_str(), argvBase[i].size() + 1);
	}
	argv[argvBase.size()] = nullptr;

	// Store state.
	data->m_attached = true;

	// Create process.
	
	// We have to change the working directory as posix_spawn inherits
	// the current processes one -_-
	Platform::Path originalWorkingDirectory = Platform::Path::GetWorkingDirectory();
	Platform::Path::SetWorkingDirectory(workingDirectory);

	int result = posix_spawn(
		&data->m_processId, 
		command.ToString().c_str(),
		nullptr,
		nullptr,
		argv,
		environ
	);

	Platform::Path::SetWorkingDirectory(originalWorkingDirectory);

	for (int i = 0; i < (int)argvBase.size(); i++)
	{
        delete[] argv[i];
    }
    delete[] argv;

	if (result != 0)
	{
		Log(LogSeverity::Warning, 
			"posix_spawn failed with error code %i.\n", result);

		Detach();
		return false;
	}

	return true;
}

void Process::Detach()
{
	MacOS_Process* data = reinterpret_cast<MacOS_Process*>(m_impl);
	assert(IsAttached());

	data->m_processId = 0;
	data->m_attached = false;
}

void Process::Terminate()
{
	MacOS_Process* data = reinterpret_cast<MacOS_Process*>(m_impl);
	assert(IsAttached());

	kill(data->m_processId, SIGKILL);
}

bool Process::IsRunning()
{
	MacOS_Process* data = reinterpret_cast<MacOS_Process*>(m_impl);
	assert(IsAttached());

	int result = kill(data->m_processId, 0);
	return (result == 0);
}

bool Process::IsAttached()
{
	MacOS_Process* data = reinterpret_cast<MacOS_Process*>(m_impl);

	return data->m_attached;
}

bool Process::Wait()
{
	MacOS_Process* data = reinterpret_cast<MacOS_Process*>(m_impl);
	assert(IsAttached());

	int resultCode = 0;
	waitpid(data->m_processId, &resultCode, 0);

	return true;
}

int Process::GetExitCode()
{
	MacOS_Process* data = reinterpret_cast<MacOS_Process*>(m_impl);
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

#endif // MB_PLATFORM_MACOS
