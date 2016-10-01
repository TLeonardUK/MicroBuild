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

#include <mutex>
#include <condition_variable>

namespace MicroBuild {
namespace Platform {

// Simple class that provides a C++11 style semaphore class.
class Sempahore
{
public:
	Sempahore(int count = 0)
		: m_count(count)
	{
	}

	inline void Signal()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_count++;
		m_condVar.notify_one();
	}

	inline void Wait()
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		while (m_count == 0)
		{
			m_condVar.wait(lock);
		}

		m_count--;
	}

private:
	std::mutex m_mutex;
	std::condition_variable m_condVar;
	int m_count;

};

}; // namespace Platform
}; // namespace MicroBuild