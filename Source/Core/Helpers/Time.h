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

#include "Core/Platform/Path.h"

namespace MicroBuild {
namespace Time {

// Times the scope its contained within and prints out a log at the end.
struct TimedScope
{
	TimedScope()
	{
		m_scope = "";
		m_start = std::chrono::high_resolution_clock::now();
	}

	TimedScope(const std::string& name)
	{
		m_scope = name;
		m_start = std::chrono::high_resolution_clock::now();
	}

	~TimedScope()
	{
		if (!m_scope.empty())
		{
			float elapsedMs = GetElapsed();

			Log(LogSeverity::Verbose, "%s took %.2f ms.\n",
				m_scope.c_str(), elapsedMs);
		}
	}

	float GetElapsed()
	{
		std::chrono::duration<long long, std::nano> elapsed =
			std::chrono::high_resolution_clock::now() - m_start;

		return elapsed.count() / 1000000.0f;
	}

private:
	std::string m_scope;
	std::chrono::time_point<std::chrono::steady_clock> m_start;

};

}; // namespace Time
}; // namespace MicroBuild