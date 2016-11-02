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

#include "PCH.h"

#include "Core/Parallel/Jobs/JobScheduler.h"

#include "Core/Helpers/Strings.h"

namespace MicroBuild {

thread_local int g_jobSchedulerthreadId = -1;

JobScheduler::JobScheduler(int ThreadCount)
	: m_Aborting(false)
{
	assert(ThreadCount > 0);

	// Push all empty job indices.
	for (int i = 0; i < MaxJobCount; i++)
	{
		m_FreeJobIndices.Push(i);
	}

	// Boot up all threads.
	for (int i = 0; i < ThreadCount; i++)
	{
		std::string ThreadName = Strings::Format("Job Pool Thread %i", i);
		
		m_Threads.push_back(new std::thread([this, i]() {
			g_jobSchedulerthreadId = i + 1;
			ThreadEntryPoint();
		}));
	}
}

JobScheduler::~JobScheduler()
{
	m_Aborting = true;

	{
		std::unique_lock<std::mutex> lock(m_WorkMutex);
		m_WorkCondVar.notify_all();
	}

	for (auto CurrentThread : m_Threads)
	{
		CurrentThread->join();
		delete CurrentThread;
	}
	m_Threads.clear();
}

bool JobScheduler::AllocateJob(JobHandle& Handle)
{
	if (m_FreeJobIndices.Pop(&Handle.Index))
	{
		m_Jobs[Handle.Index].Version = (m_JobVersionCounter++);
		m_Jobs[Handle.Index].Completed = false;
		m_Jobs[Handle.Index].Enqueued = false;
		m_Jobs[Handle.Index].DependenciesPending = 0;
		m_Jobs[Handle.Index].Dependencies.clear();
		m_Jobs[Handle.Index].Dependents.clear();
		Handle.Version = m_Jobs[Handle.Index].Version;
		return true;
	}
	return false;
}

JobHandle JobScheduler::CreateJob(JobCallback Callback)
{
	JobHandle Handle;
	bool bResult = AllocateJob(Handle);
	assert(bResult);// "Unable to allocate job index, possibly to many jobs queued.");

	if (bResult)
	{
		m_Jobs[Handle.Index].Callback = Callback;
	}

	return Handle;
}

JobScheduler::Job* JobScheduler::GetJob(JobHandle Handle)
{
	if (m_Jobs[Handle.Index].Version != Handle.Version)
	{
		return nullptr;
	}
	return &m_Jobs[Handle.Index];
}

void JobScheduler::AddDependency(JobHandle Primary, JobHandle DependentOn)
{
	Job* DependentOnJob = GetJob(DependentOn);
	Job* PrimaryJob = GetJob(Primary);

	assert(DependentOnJob != nullptr);// "Dependent job handle is no longer valid - its probably already been executed. Add job dependencies before enqueing them.");
	assert(PrimaryJob != nullptr);// "Parent job handle is no longer valid - its probably already been executed. Add job dependencies before enqueing them.");

	assert(!DependentOnJob->Enqueued);// "Dependent job has already been enqueued. Add job dependencies before enqueing jobs.");
	assert(!PrimaryJob->Enqueued);// "Dependent job has already been enqueued. Add job dependencies before enqueing jobs.");

	PrimaryJob->DependenciesPending++;
	PrimaryJob->Dependencies.push_back(DependentOn);
	DependentOnJob->Dependents.push_back(Primary);
}

void JobScheduler::Enqueue(JobHandle Handle)
{
	// Ensure we aren't already enqueued.
	Job* ParentJob = GetJob(Handle);

	assert(ParentJob != nullptr);// , "Parent job handle is no longer valid - its probably already been executed. Add job dependencies before enqueing them.");
	assert(!ParentJob->Enqueued);//, "Dependent job has already been enqueued. Add job dependencies before enqueing jobs.");

	if (ParentJob->DependenciesPending <= 0)
	{
		ParentJob->Enqueued = true;

		if (!m_QueuedJobs.Push(Handle.Index))
		{
			assert(false);// "Failed to enqueue job, job queue is probably full, increasing MaxJobCount may be required.");
		}
	}

	// Enqueue children first.
	for (JobHandle ChildHandle : ParentJob->Dependencies)
	{
		Job* ChildJob = GetJob(ChildHandle);
		if (ChildJob != nullptr && !ChildJob->Enqueued)
		{
			Enqueue(ChildHandle);
		}
	}

	{
		std::unique_lock<std::mutex> lock(m_WorkMutex);
		m_WorkCondVar.notify_one();
	}
}

bool JobScheduler::IsComplete(JobHandle Handle)
{
	if (m_Jobs[Handle.Index].Completed ||
		m_Jobs[Handle.Index].Version != Handle.Version)
	{
		return true;
	}
	return false;
}

void JobScheduler::Wait(JobHandle Handle)
{
	std::unique_lock<std::mutex> lock(m_WaitingMutex);

	while (!IsComplete(Handle))
	{
		m_WaitingCondVar.wait(lock);
	}
}

void JobScheduler::RunJob(int JobIndex)
{
	if (m_Jobs[JobIndex].Callback != nullptr)
	{
		m_Jobs[JobIndex].Callback();
	}

	for (JobHandle& Handle : m_Jobs[JobIndex].Dependents)
	{
		int Index = (--m_Jobs[Handle.Index].DependenciesPending);
		if (Index == 0)
		{
			Enqueue(Handle);
		}
	}

	{
		std::unique_lock<std::mutex> lock(m_WaitingMutex);
		m_Jobs[JobIndex].Completed = true;
		m_WaitingCondVar.notify_all();
	}

	m_FreeJobIndices.Push(JobIndex);
}

int JobScheduler::WaitForJob()
{
	std::unique_lock<std::mutex> lock(m_WorkMutex);

	while (!m_Aborting)
	{
		int JobIndex = 0;
		if (m_QueuedJobs.Pop(&JobIndex))
		{
			return JobIndex;
		}
		m_WorkCondVar.wait(lock);
	}

	return -1;
}

void JobScheduler::ThreadEntryPoint()
{
	while (!m_Aborting)
	{
		int JobIndex = WaitForJob();
		if (JobIndex >= 0)
		{
			RunJob(JobIndex);
		}
	}
}

int JobScheduler::GetThreadId()
{
	return g_jobSchedulerthreadId;
}

}; // namespace Ludo