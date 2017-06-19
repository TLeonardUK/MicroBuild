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
#include "Core/Helpers/Strings.h"

#ifdef MB_PLATFORM_LINUX

#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

extern char **environ;

namespace MicroBuild {
namespace Platform {

struct Linux_Process
{
	bool m_attached;
	pid_t m_processId;
	posix_spawn_file_actions_t m_spawn_action;
	int m_cout_pipe[2];
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

	for (const std::string& arg : arguments)
	{
		argvBase.push_back(Strings::StripQuotes(arg, true));
	}		
	
	char** argv = new char*[argvBase.size() + 1];
	for (int i = 0; i < (int)argvBase.size(); i++)
	{
		argv[i] = new char[argvBase[i].size() + 1];
		memcpy(argv[i], argvBase[i].c_str(), argvBase[i].size() + 1);
		//Log(LogSeverity::Warning, "Argv[%i] = %s\n", i, argv[i]);	
	}
	argv[argvBase.size()] = nullptr;

	// Store state.
	data->m_attached = true;

	// Create spawn state.
	if (pipe(data->m_cout_pipe))
	{
		return false;
	}

	posix_spawn_file_actions_init(&data->m_spawn_action);
	posix_spawn_file_actions_addclose(&data->m_spawn_action, data->m_cout_pipe[0]);
	posix_spawn_file_actions_adddup2(&data->m_spawn_action, data->m_cout_pipe[1], STDOUT_FILENO);
	posix_spawn_file_actions_adddup2(&data->m_spawn_action, data->m_cout_pipe[1], STDERR_FILENO);
 
	// Create process.
	
	// We have to change the working directory as posix_spawn inherits
	// the current processes one -_-
	Platform::Path originalWorkingDirectory = Platform::Path::GetWorkingDirectory();
	Platform::Path::SetWorkingDirectory(workingDirectory);

	int result = posix_spawn(
		&data->m_processId, 
		command.ToString().c_str(),
		&data->m_spawn_action,
		nullptr,
		argv,
		environ
	);

	Platform::Path::SetWorkingDirectory(originalWorkingDirectory);

	close(data->m_cout_pipe[1]);

	for (int i = 0; i < (int)argvBase.size(); i++)
	{
        	delete[] argv[i];
	}
	delete[] argv;
    
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

	close(data->m_cout_pipe[0]);
	//close(data->m_cout_pipe[1]);

	posix_spawn_file_actions_destroy(&data->m_spawn_action);
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
	return resultCode;
}

size_t Process::Internal_Write(void* buffer, uint64_t bufferLength)
{	
	Linux_Process* data = reinterpret_cast<Linux_Process*>(m_impl);
	assert(IsAttached());

	ssize_t count = write(data->m_cout_pipe[1], buffer, bufferLength);
	return count < 0 ? 0 : (size_t)count;
}

size_t Process::Internal_Read(void* buffer, uint64_t bufferLength)
{
	Linux_Process* data = reinterpret_cast<Linux_Process*>(m_impl);
	assert(IsAttached());

	ssize_t count = read(data->m_cout_pipe[0], buffer, bufferLength);
	return count < 0 ? 0 : (size_t)count;
}

void Process::Flush()
{
	// I don't think there is a way to explicitly flush pipes? 
}

uint64_t Process::Internal_BytesLeft()
{	
	Linux_Process* data = reinterpret_cast<Linux_Process*>(m_impl);
	assert(IsAttached());

	size_t bytesAvailable = 0;
	int result = ioctl(data->m_cout_pipe[0], FIONREAD, &bytesAvailable);
	
	//Log(LogSeverity::Warning, "(BytesLeft) Result=%i Bytes=%i\n", result, (int)bytesAvailable);
	return result >= 0 ? (uint64_t)bytesAvailable : 0;
}

}; // namespace Platform
}; // namespace MicroBuild

#endif // MB_PLATFORM_LINUX
