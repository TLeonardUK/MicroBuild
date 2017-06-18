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

#include "App/Builder/Tasks/BuildTask.h"

namespace MicroBuild {
	
BuildTask::BuildTask(BuildStage stage, bool bCanRunInParallel, bool bGiveJobIndex, bool bCanDistribute)
	: m_stage(stage)
	, m_bCanRunInParallel(bCanRunInParallel)
	, m_bCanDistribute(bCanDistribute)
	, m_jobIndex(-1)
	, m_totalJobs(-1)
	, m_bGiveJobIndex(bGiveJobIndex)
	, m_subTaskCount(1)
{
}

BuildStage BuildTask::GetBuildState()
{
	return m_stage;
}

bool BuildTask::CanRunInParallel()
{
	return m_bCanRunInParallel;
}

bool BuildTask::CanDistribute()
{
	return m_bCanDistribute;
}

bool BuildTask::ShouldGiveJobIndex()
{
	return m_bGiveJobIndex;
}
	
int BuildTask::GetTaskThreadId()
{
	return m_threadId;
}

void BuildTask::GetTaskThreadId(int id)
{
	m_threadId = id;
}

void BuildTask::SetTaskProgress(int jobIndex, int totalJobs)
{
	m_jobIndex = jobIndex;
	m_totalJobs = totalJobs;
}

void BuildTask::GetTaskProgress(int& jobIndex, int& totalJobs)
{
	jobIndex = m_jobIndex;
	totalJobs = m_totalJobs;
}

int BuildTask::GetSubTaskCount()
{
	return m_subTaskCount;
}

void BuildTask::TaskLog(LogSeverity Severity, int subTaskIndex, const char* format, ...)
{
	va_list list;
	va_start(list, format);
	std::string message = Strings::FormatVa(format, list);
	va_end(list);

	/*
	Log(Severity, "[%i] (%4i/%4i) %s", 
		GetTaskThreadId(), 
		m_jobIndex,
		m_totalJobs,
		message.c_str()
	);
	*/
	
	if (m_jobIndex == -1)
	{	
		Log(Severity, "[-   /-   ] %s", 
			message.c_str()
		);
	}
	else
	{
		Log(Severity, "[%4i/%4i] %s", 
			m_jobIndex + subTaskIndex,
			m_totalJobs,
			message.c_str()
		);
	}
}

bool BuildTask::Execute()
{
	BuildAction action = GetAction();

	int jobIndex = 0, totalJobs = 0;
	GetTaskProgress(jobIndex, totalJobs);

	if (!action.StatusMessage.empty())
	{
		TaskLog(LogSeverity::SilentInfo, 0, "%s", action.StatusMessage.c_str());
	}

	Platform::Process process;
	if (!process.Open(action.Tool, action.Tool.GetDirectory(), action.Arguments, true))
	{
		return false;
	}

	action.Output = process.ReadToEnd();
	action.ExitCode = process.GetExitCode();
	return action.PostProcessDelegate(action);
}

}; // namespace MicroBuild