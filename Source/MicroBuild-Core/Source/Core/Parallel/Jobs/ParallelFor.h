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

#include "Core/Parallel/Jobs/JobScheduler.h"

#include <vector>
#include <functional>

namespace MicroBuild {

// Result of an individual iteration in a ParallelFor, can be used
// do effectively continue or abort the iteration.
enum class ParallelForTaskResult
{
	Continue,
	Break,
};

// Result of the entire ParallelFor, determins if it completed normally
// or was broken out of.
enum class ParallelForResult
{
	Broken,
	Complete
};

// Creates jobs in the given scheduler for each element in the tasks array. The callback is 
// invoked for each individual task.
template <typename TaskType>
ParallelForResult ParallelFor(JobScheduler* Scheduler, std::vector<TaskType>& Tasks, std::function<ParallelForTaskResult(TaskType& TaskType)> Callback)
{
	ParallelForResult Result = ParallelForResult::Complete;

	JobHandle GroupTask = Scheduler->CreateJob();

	for (int i = 0; i < Tasks.Length(); i++)
	{
		JobHandle IterTask = Scheduler->CreateJob([&Result, &Tasks, Callback, i] () {
			
			if (Result == ParallelForResult::Broken)
			{
				return;
			}

			ParallelForTaskResult TaskResult = Callback(Tasks[i]);
			if (TaskResult == ParallelForTaskResult::Break)
			{
				Result = ParallelForResult::Broken;
			}
		});

		Scheduler->AddDependency(GroupTask, IterTask);
	}

	Scheduler->Enqueue(GroupTask);
	Scheduler->Wait(GroupTask);

	return Result;
}

};