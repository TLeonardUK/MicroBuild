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
		m_bIsVerbose = true;
	}

	TimedScope(const std::string& name, bool bIsVerbose = true)
	{
		m_scope = name;
		m_start = std::chrono::high_resolution_clock::now();
		m_bIsVerbose = bIsVerbose;
	}

	~TimedScope()
	{
		if (!m_scope.empty())
		{
#if 0
			float elapsedMs = GetElapsed();
			Log(m_bIsVerbose ? LogSeverity::Verbose : LogSeverity::Warning, "%s took %.2f ms.\n",
				m_scope.c_str(), elapsedMs);
#endif
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
	std::chrono::high_resolution_clock::time_point m_start;
	bool m_bIsVerbose;

};

}; // namespace Time
}; // namespace MicroBuild
