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

void BuildTask::TaskLog(LogSeverity Severity, const char* format, ...)
{
	va_list list;
	va_start(list, format);
	std::string message = Strings::FormatVa(format, list);
	va_end(list);

	Log(Severity, "[%i] (%4i/%4i) %s", 
		GetTaskThreadId(), 
		m_jobIndex,
		m_totalJobs,
		message.c_str()
	);	
}

}; // namespace MicroBuild