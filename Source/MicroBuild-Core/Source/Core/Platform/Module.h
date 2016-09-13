#pragma once
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

// Represents a dynamically loaded module on the local filesystem (so/dll/dylib)
class Module
{
protected:
	void* m_impl; // Semi-pimpl idiom, contains any platform specific data.

public:

	// No copy construction please.
	Module(const Module& other) = delete;

	// Construction.
	Module();
	~Module();

	// Opens an instance of the module.
	bool Open(const Path& command);

	// Closes the module.
	void Close();

	// Gets the exported symbol pointer witht he given name from the module.
	void* GetSymbol(const std::string& name);

	// Returns true if the module is open and accessible.
	bool IsOpen();

	// Gets the exported function pointer with the given name from the module.
	template <typename FunctionType>
	FunctionType GetFunction(const std::string& name)
	{
		return reinterpret_cast<FunctionType>(GetSymbol(name));
	}

};

};
};