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

#include "App/Builder/Tasks/AccelerateTask.h"
#include "Core/Platform/Process.h"

namespace MicroBuild {

AccelerateTask::AccelerateTask(ProjectFile& projectFile, std::vector<std::shared_ptr<BuildTask>>& tasks, Toolchain* toolchain, Accelerator* accelerator)
	: BuildTask(BuildStage::Compile, true, true, false)
	, m_toolchain(toolchain)
	, m_accelerator(accelerator)
	, m_tasks(tasks)
	, m_projectFile(projectFile)
{
	m_subTaskCount = (int)m_tasks.size() + 1;
}

bool AccelerateTask::Execute()
{	
	int jobIndex = 0, totalJobs = 0;
	GetTaskProgress(jobIndex, totalJobs);

	TaskLog(LogSeverity::SilentInfo, 0, "Distributing %i tasks ...\n", m_tasks.size());
	
	Platform::Path batchPath = m_projectFile.Get_Project_IntermediateDirectory().AppendFragment(Strings::Format("%s.sndbs.bat", m_projectFile.Get_Project_Name().c_str()), true);

	std::vector<BuildAction> actions;

	for (auto& task : m_tasks)
	{
		actions.push_back(task->GetAction());		
	}

	if (!m_accelerator->RunActions(m_toolchain, this, m_projectFile, actions))
	{
		return false;
	}

	return true;
}

BuildAction AccelerateTask::GetAction()
{
	assert(false);

	BuildAction action;
	return action;
}

}; // namespace MicroBuild
