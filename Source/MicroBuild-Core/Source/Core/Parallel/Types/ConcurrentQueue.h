/*
Ludo Game Engine
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

#include "Core/Platform/Platform.h"

#include <functional>
#include <utility>
#include <initializer_list>
#include <atomic>

namespace MicroBuild {

// Provides a fixed size queue that can be accessed from multiple threads in a lock-free manner.
template <
	typename ElementType,
	int MaxCapacity 
>
class ConcurrentQueue 
{
protected:
	ElementType			m_Data[MaxCapacity];
	int					m_Tail;
	int					m_Head;
	std::atomic<int>	m_UncomittedTail;
	std::atomic<int>	m_UncomittedHead;

public:

	ConcurrentQueue()
		: m_Tail(0)
		, m_Head(0)
		, m_UncomittedTail(0)
		, m_UncomittedHead(0)
	{
	}

	~ConcurrentQueue()
	{
	}

	// Push an element into the queue. Optionally blocks until there is space in 
	// the queue. Returns true if element was successfully inserted.
	bool Push(const ElementType& Element, bool bBlockUntilAvailable = false)
	{
		while (true)
		{
			int OldHead = m_Head;
			int NewHead = OldHead + 1;

			int Diff = (NewHead - m_Tail);
			if (Diff < 0 || Diff > MaxCapacity)
			{
				if (bBlockUntilAvailable)
				{
					continue;
				}
				else
				{
					break;
				}
			}
			 
			if (m_UncomittedHead.compare_exchange_strong(OldHead, NewHead))
			{
				m_Data[OldHead % MaxCapacity] = Element;
				m_Head = m_UncomittedHead;
				return true;
			}
			else
			{
				Platform::RelaxCpu();
			}
		}
		return false;
	}

	// Pops an element from the queue. Optionally blocks until there is something
	// in the queue to pop. Returns true if the element was popped successfully.
	bool Pop(ElementType* Output, bool bBlockUntilAvailable = false)
	{
		while (true)
		{
			int OldTail = m_Tail;
			int NewTail = OldTail + 1;

			int Diff = (m_Head - NewTail);
			if (Diff < 0 || Diff > MaxCapacity)
			{
				if (bBlockUntilAvailable)
				{
					continue;
				}
				else
				{
					break;
				}
			}

			if (m_UncomittedTail.compare_exchange_strong(OldTail, NewTail))
			{
				*Output = m_Data[OldTail % MaxCapacity];
				m_Tail = m_UncomittedTail;
				return true;
			}
			else
			{
				Platform::RelaxCpu();
			}
		}
		return false;
	}

	// Clears out the queue of all elements.
	void Empty()
	{
		m_Top = m_Data;
		m_ComittedTop = m_Top;
	}

	// Returns true if the queue has any elements in it.
	bool IsEmpty() const
	{
		return (m_ComittedTop == m_Data);
	}

	// Returns the number of elements currently in the queue.
	int Length() const
	{
		return (m_ComittedTop - m_Data);
	}

};

}; // namespace Ludo