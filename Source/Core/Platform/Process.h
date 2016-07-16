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

#pragma once

#include "PCH.h"
#include "Core/Platform/Path.h"

namespace MicroBuild {
namespace Platform {

// Represents a process running on the local system, allows the creation
// and execution of process, as well as reading and writing from their 
// stdio/stdout pipes.
class Process 
{
protected:
	void* m_impl; // Semi-pimpl idiom, contains any platform specific data.

public:

	// No copy construction please.
	Process(const Process& other) = delete;

	// Construction.
	Process();
	~Process();

	// Opens a process with the given arguments.
	bool Open(
		const Path& command, 
		const Path& workingDirectory, 
		const std::vector<std::string>& arguments, 
		bool bRedirectStdInOut = false);

	// Dettaches from the processes stdout/stdin pipes.
	void Detach();

	// Attempts to terminate the process. This is not guaranteed to work immediately,
	// it depends how the OS deals with process termination.
	void Terminate();

	// Returns true if the process is currently running.
	bool IsRunning();

	// Returns true if we are currently attached to the processes stdin
	// and stdout pipes.
	bool IsAttached();

	// Waits until the process finishes.
	bool Wait();

	// Gets the exit code of the process.
	int GetExitCode();

	// Does a blocking write from the process's stdint, blocks until entire
	// buffer has been confumed or the process ends
	bool Write(void* buffer, uint64_t bufferLength);

	// Does a blocking read from the process's stdout, blocks until entire
	// buffer has been read or process ends.
	bool Read(void* buffer, uint64_t bufferLength);

	// Returns true if we are at the end of the process's stdout.
	bool AtEnd();

	// Flushes any output you've written to the process.
	void Flush();

	// Returns the number of bytes left that can be read.
	uint64_t BytesLeft();

};

};
};