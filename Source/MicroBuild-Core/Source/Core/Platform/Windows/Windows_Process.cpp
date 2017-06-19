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

#ifdef MB_PLATFORM_WINDOWS

#include <Windows.h>

namespace MicroBuild {
namespace Platform {

struct Windows_Process
{
	PROCESS_INFORMATION m_processInfo;
	STARTUPINFOA		m_startupInfo;
	SECURITY_ATTRIBUTES	m_securityAttributes;

	HANDLE				m_stdInWrite;
	HANDLE				m_stdInRead;
	HANDLE				m_stdOutWrite;
	HANDLE				m_stdOutRead;

	bool				m_bRedirectStdInOut;
	bool				m_attached;
};

Process::Process()
{
	m_impl = new Windows_Process();

	Windows_Process* data = reinterpret_cast<Windows_Process*>(m_impl);
	data->m_attached = false;
	data->m_bRedirectStdInOut = false;
	data->m_stdInRead = INVALID_HANDLE_VALUE;
	data->m_stdInWrite = INVALID_HANDLE_VALUE;
	data->m_stdOutRead = INVALID_HANDLE_VALUE;
	data->m_stdOutWrite = INVALID_HANDLE_VALUE;

	ZeroMemory(&data->m_processInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&data->m_startupInfo, sizeof(STARTUPINFOA));
	ZeroMemory(&data->m_securityAttributes, sizeof(SECURITY_ATTRIBUTES));
}

Process::~Process()
{
	if (IsAttached())
	{
		Detach();
	}

	delete reinterpret_cast<Windows_Process*>(m_impl);
}

bool Process::Open(
	const Path& command,
	const Path& workingDirectory,
	const std::vector<std::string>& arguments,
	bool bRedirectStdInOut)
{
	Windows_Process* data = reinterpret_cast<Windows_Process*>(m_impl);
	assert(!IsAttached());

	//Log(LogSeverity::Info, "Process::Open(%s %s)\n", command.ToString().c_str(), Strings::Join(arguments, " ").c_str());

	// Escape and pack arguments into a single command line incase it has spaces!
	std::string commandLine = Strings::Quoted(command.ToString());

	for (const std::string& arg : arguments)
	{
		commandLine += " ";
		commandLine += arg;
	}

	if (bRedirectStdInOut)
	{
		// Setup security attributes.
		ZeroMemory(&data->m_securityAttributes, sizeof(SECURITY_ATTRIBUTES));
		data->m_securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
		data->m_securityAttributes.bInheritHandle = TRUE;
		data->m_securityAttributes.lpSecurityDescriptor = NULL;

		// Create pipes to redirect the stdin/stdout and ensure our side of the pipes 
		// (read for stdout, write for stdin) are not inheritable.
		BOOL Result = CreatePipe(
			&data->m_stdOutRead, 
			&data->m_stdOutWrite, 
			&data->m_securityAttributes, 
			0
		);
		if (!Result)
		{
			Log(LogSeverity::Warning, 
				"CreatePipe failed with 0x%08x.\n", GetLastError());
			return false;
		}

		Result = SetHandleInformation(
			data->m_stdOutRead, 
			HANDLE_FLAG_INHERIT, 
			0
		);
		if (!Result)
		{
			Log(LogSeverity::Warning, 
				"SetHandleInformation failed with 0x%08x.\n", GetLastError());
			return false;
		}

		Result = CreatePipe(
			&data->m_stdInRead, 
			&data->m_stdInWrite, 
			&data->m_securityAttributes, 
			0
		);
		if (!Result)
		{
			Log(LogSeverity::Warning, 
				"CreatePipe failed with 0x%08x.\n", GetLastError());
			return false;
		}

		Result = SetHandleInformation(
			data->m_stdInWrite, 
			HANDLE_FLAG_INHERIT, 
			0
		);
		if (!Result)
		{
			Log(LogSeverity::Warning, 
				"SetHandleInformation failed with 0x%08x.\n", GetLastError());
			return false;
		}
	}
	else
	{
		data->m_stdInRead = INVALID_HANDLE_VALUE;
		data->m_stdInWrite = INVALID_HANDLE_VALUE;
		data->m_stdOutRead = INVALID_HANDLE_VALUE;
		data->m_stdOutWrite = INVALID_HANDLE_VALUE;
	}


	// Setup process information.
	ZeroMemory(&data->m_processInfo, sizeof(PROCESS_INFORMATION));

	// Setup startup info.
	ZeroMemory(&data->m_startupInfo, sizeof(STARTUPINFOA));
	data->m_startupInfo.cb = sizeof(STARTUPINFOA);

	if (bRedirectStdInOut)
	{
		data->m_startupInfo.hStdError = data->m_stdOutWrite;
		data->m_startupInfo.hStdOutput = data->m_stdOutWrite;
		data->m_startupInfo.hStdInput = data->m_stdInRead;
		data->m_startupInfo.dwFlags |= STARTF_USESTDHANDLES;
	}
	
	// Store state.
	data->m_attached = true;
	data->m_bRedirectStdInOut = bRedirectStdInOut;

	// Time to create the process at last!
	//Log(LogSeverity::Info, "Command: %s\n", commandLine.c_str());
	BOOL result = CreateProcessA(
		nullptr,										// lpApplicationName
		(LPSTR)commandLine.c_str(),						// lpCommandLine
		nullptr,										// lpProcessAttributes
		nullptr,										// lpThreadAttributes
		true,											// bInheritHandles
		0,												// dwCreationFlags
		nullptr,										// lpEnvironment
		(LPCSTR)workingDirectory.ToString().c_str(),	// lpCurrentDirectory
		&data->m_startupInfo,							// lpStartupInfo
		&data->m_processInfo							// lpProcessInformation
	);
	if (!result)
	{
		Log(LogSeverity::Warning, 
			"CreateProcess failed with 0x%08x.\n", GetLastError());

		Detach();
		return false;
	}

	// We do not need the childs write handles anymore, so close them up, if we don't do
	// this we can end up in some wierd hanging situations with the syncronous IO API as the child
	// will not be able to close the pipes when they exit.
	if (data->m_stdOutWrite != INVALID_HANDLE_VALUE)
	{
		CloseHandle(data->m_stdOutWrite);
		data->m_stdOutWrite = INVALID_HANDLE_VALUE;
	}
	if (data->m_stdInRead != INVALID_HANDLE_VALUE)
	{
		CloseHandle(data->m_stdInRead);
		data->m_stdInRead = INVALID_HANDLE_VALUE;
	}

	return true;
}

void Process::Detach()
{
	Windows_Process* data = reinterpret_cast<Windows_Process*>(m_impl);

	assert(IsAttached());

	// dispose of everything.
	if (data->m_stdInWrite != INVALID_HANDLE_VALUE)
	{
		CloseHandle(data->m_stdInWrite);
		data->m_stdInWrite = INVALID_HANDLE_VALUE;
	}
	if (data->m_stdInRead != INVALID_HANDLE_VALUE)
	{
		CloseHandle(data->m_stdInRead);
		data->m_stdInRead = INVALID_HANDLE_VALUE;
	}
	if (data->m_stdOutWrite != INVALID_HANDLE_VALUE)
	{
		CloseHandle(data->m_stdOutWrite);
		data->m_stdOutWrite = INVALID_HANDLE_VALUE;
	}
	if (data->m_stdOutRead != INVALID_HANDLE_VALUE)
	{
		CloseHandle(data->m_stdOutRead);
		data->m_stdOutRead = INVALID_HANDLE_VALUE;
	}
	if (data->m_processInfo.hProcess != INVALID_HANDLE_VALUE)
	{
		CloseHandle(data->m_processInfo.hProcess);
		data->m_processInfo.hProcess = INVALID_HANDLE_VALUE;
	}
	if (data->m_processInfo.hThread != INVALID_HANDLE_VALUE)
	{
		CloseHandle(data->m_processInfo.hThread);
		data->m_processInfo.hThread = INVALID_HANDLE_VALUE;
	}

	data->m_attached = false;
}

void Process::Terminate()
{
	Windows_Process* data = reinterpret_cast<Windows_Process*>(m_impl);

	assert(IsAttached());

	TerminateProcess(data->m_processInfo.hProcess, 1);
}

bool Process::IsRunning()
{
	Windows_Process* data = reinterpret_cast<Windows_Process*>(m_impl);

	assert(IsAttached());

	DWORD exitCode = 0;

	BOOL result = GetExitCodeProcess(data->m_processInfo.hProcess, &exitCode);
	if (!result)
	{
		Log(LogSeverity::Warning, 
			"GetExitCodeProcess failed with 0x%08x.\n", GetLastError());
	}

	return (exitCode == STILL_ACTIVE);
}

bool Process::IsAttached()
{
	Windows_Process* data = reinterpret_cast<Windows_Process*>(m_impl);
	return data->m_attached;
}

bool Process::Wait()
{
	Windows_Process* data = reinterpret_cast<Windows_Process*>(m_impl);

	assert(IsAttached());

	DWORD Status = WaitForSingleObject(data->m_processInfo.hProcess, INFINITE);
	if (Status == WAIT_TIMEOUT)
	{
		return false;
	}
	else
	{
		return true;
	}
}

int Process::GetExitCode()
{
	Windows_Process* data = reinterpret_cast<Windows_Process*>(m_impl);

	assert(IsAttached());

	DWORD exitCode = 0;

	BOOL result = GetExitCodeProcess(data->m_processInfo.hProcess, &exitCode);
	if (!result)
	{
		Log(LogSeverity::Warning, 
			"GetExitCodeProcess failed with 0x%08x.\n", GetLastError());
	}

	assert(exitCode != STILL_ACTIVE);

	return exitCode;
}

size_t Process::Internal_Write(void* buffer, uint64_t bufferLength)
{
	Windows_Process* data = reinterpret_cast<Windows_Process*>(m_impl);

	assert(IsAttached());
	assert(data->m_bRedirectStdInOut);

	DWORD bytesOutput = 0;
	BOOL result = WriteFile(
		data->m_stdInWrite, 
		buffer, 
		(DWORD)bufferLength, 
		&bytesOutput, 
		nullptr
	);
	if (!result)
	{
		//Log(LogSeverity::Warning, 
		//	"WriteFile failed with 0x%08x.\n", GetLastError());
		return 0;
	}

	return bytesOutput;
}

size_t Process::Internal_Read(void* buffer, uint64_t bufferLength)
{
	Windows_Process* data = reinterpret_cast<Windows_Process*>(m_impl);

	assert(IsAttached());
	assert(data->m_bRedirectStdInOut);

	DWORD bytesOutput = 0;
	BOOL result = ReadFile(
		data->m_stdOutRead, 
		buffer, 
		(DWORD)bufferLength, 
		&bytesOutput, 
		nullptr
	);
	if (!result)
	{
		//Log(LogSeverity::Warning, 
		//	"ReadFile failed with 0x%08x.\n", GetLastError());
		return 0;
	}

	return bytesOutput;
}

void Process::Flush()
{
	Windows_Process* data = reinterpret_cast<Windows_Process*>(m_impl);

	assert(IsAttached());
	assert(data->m_bRedirectStdInOut);

	FlushFileBuffers(data->m_stdInWrite);
}

uint64_t Process::Internal_BytesLeft()
{
	Windows_Process* data = reinterpret_cast<Windows_Process*>(m_impl);

	assert(IsAttached());
	assert(data->m_bRedirectStdInOut);

	DWORD bytesAvailable = 0;
	BOOL result = PeekNamedPipe(
		data->m_stdOutRead, 
		nullptr, 
		0, 
		nullptr, 
		&bytesAvailable, 
		nullptr
	);
	if (!result)
	{
		//Log(LogSeverity::Warning, 
		//	"PeekNamedPipe failed with 0x%08x.\n", GetLastError());
		return 0;
	}

	return bytesAvailable;
}

}; // namespace Platform
}; // namespace MicroBuild

#endif // MB_PLATFORM_WINDOWS
