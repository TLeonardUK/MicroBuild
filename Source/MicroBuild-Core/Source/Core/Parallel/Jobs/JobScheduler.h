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

#include "Core/Parallel/Types/ConcurrentQueue.h"

#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace MicroBuild {

// Represents an individual job that is being actively managed
// in the job scheduler.
struct JobHandle
{
public:
	JobHandle()
		: Index(-1)
		, Version(-1)
	{
	}

	bool IsValid()
	{
		return Index >= 0 && Version >= 0;
	}

private:

	friend class JobScheduler;

	int Index;
	int Version;
};

// The job scheduler allows the user to create multiple small jobs which
// will be executed in parallel by multiple threads. Individual jobs can also
// have dependency chains to ensure defined ordering.
class JobScheduler
{
public:

	// Callback signature.
	typedef std::function<void()> JobCallback;

	// Maximum number of jobs this scheduler can manage.
	const static int MaxJobCount = 1 * 1024;

private:

	struct Job
	{
		int						Version;
		JobCallback				Callback;
		std::atomic<int>		DependenciesPending;
		std::vector<JobHandle>	Dependencies;
		std::vector<JobHandle>	Dependents;
		bool					Completed;
		std::atomic<bool>		Enqueued;
	};

	std::vector<std::thread*> m_Threads;

	std::atomic<int> m_JobVersionCounter;

	ConcurrentQueue<int, MaxJobCount> m_QueuedJobs;

	Job m_Jobs[MaxJobCount];
	ConcurrentQueue<int, MaxJobCount> m_FreeJobIndices;

	std::mutex m_WorkMutex;
	std::condition_variable m_WorkCondVar;

	std::mutex m_WaitingMutex;
	std::condition_variable m_WaitingCondVar;

	bool m_Aborting;

private:
	void ThreadEntryPoint();
	bool AllocateJob(JobHandle& Handle);
	Job* GetJob(JobHandle Handle);
	void RunJob(int Index);
	int WaitForJob();

public:
	JobScheduler(int ThreadCount);
	~JobScheduler();

	// Creates a new job that will execute the given callback when run. No callback can be 
	// provided if the job is just intended to be a parent job for a group of other jobs.
	// The job will not be run until Enqueue is called.
	JobHandle CreateJob(JobCallback Callback = nullptr);

	// Adds a given job as a dependency on another job. The parent job will not be run
	// until all its dependents are executed.
	void AddDependency(JobHandle Job, JobHandle DependentOn);

	// Adds the job to the queue of jobs that are waiting for execution. This will also enqueue
	// all dependent tasks.
	void Enqueue(JobHandle Handle);

	// Blocks indefinitely until the given job is complete.
	void Wait(JobHandle Handle);

	// Returns the current completion state of the given job.
	bool IsComplete(JobHandle Handle);

	// Gets the thread-id the calling task is executing on, will return negative 
	// values if called from non-job thread.
	int GetThreadId();

	// Prints a debug tree showing the handle and all its decendents.
	void PrintJobTree();

};

};