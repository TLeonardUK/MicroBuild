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

#include "App/Builder/SourceControl/Providers/GitSourceControlProvider.h"
#include "Core/Platform/Process.h"
#include "Core/Helpers/Strings.h"
#include "Core/Helpers/StringConverter.h"

namespace MicroBuild {

GitSourceControlProvider::GitSourceControlProvider()
{
	std::vector<std::string> formatTags;
	formatTags.push_back("%h");		// abbreviated commit hash.
	formatTags.push_back("%an");	// author name.
	formatTags.push_back("%B");		// body.
	formatTags.push_back("%at");	// author time.

	std::string seperator = "%x01"; // Record-seperator, can be anything, just needs to be something that won't be in a changelist log.

	m_logFormat = "";
	for (auto& tag : formatTags)
	{
		m_logFormat += tag;
		m_logFormat += seperator;
	}
}

bool GitSourceControlProvider::ParseChangelists(const std::string& output, std::vector<SourceControlChangelist>& changelists)
{
	std::vector<std::string> splitValues = Strings::Split('\x01', output, false, false);
	for (int i = 0; i < (int)splitValues.size() - 3; i += 4)
	{
		SourceControlChangelist list;
		list.Id = splitValues[i];
		list.Author = splitValues[i + 1];
		list.Description = splitValues[i + 2];

		// TODO: Reliant on undefined behaviour (time_t is not assured to be a long), please fix asap.
		list.Date = (time_t)strtol(splitValues[i + 3].c_str(), nullptr, 10);

		changelists.push_back(list);
	}
	return true;
}

bool GitSourceControlProvider::RunGitCommand(const std::vector<std::string>& arguments, const Platform::Path& folder, std::string* output)
{
	Platform::Process process;
	if (!process.Open("git", folder, arguments, true))
	{
		return false;
	}

	std::string stdeo = process.ReadToEnd();
	if (process.GetExitCode() != 0)
	{
		printf("%s", stdeo.c_str());
		return false;
	}

	if (output != nullptr)
	{
		*output = stdeo;
	}

	return true;
}

bool GitSourceControlProvider::Connect(const Platform::Path& rootPath)
{
	m_rootPath = rootPath;

	// Check that path is a git folder.
	if (!m_rootPath.AppendFragment(".git", true).Exists())
	{
		Log(LogSeverity::Warning, "Folder does not appear to be a git respository: %s.\n", m_rootPath.ToString().c_str());
		return false;
	}

	// Check git is installed.
	if (!RunGitCommand({ "--version" }, rootPath))
	{
		Log(LogSeverity::Warning, "Git does not appear to be installed.\n");
		return false;
	}

	return true;
}

bool GitSourceControlProvider::GetChangelist(const Platform::Path& path, SourceControlChangelist& changelist)
{
	std::vector<std::string> arguments;

	arguments.push_back("log");
	arguments.push_back("-n");
	arguments.push_back("1");
	arguments.push_back(Strings::Format("--pretty=\"%s\"", m_logFormat.c_str()));
	arguments.push_back("--");
	arguments.push_back(path.ToString());

	std::string output = "";

	if (!RunGitCommand(arguments, m_rootPath, &output))
	{
		Log(LogSeverity::Warning, "Failed to execute git command.\n");
		return false;
	}

	std::vector<SourceControlChangelist> allChangelists;

	if (!ParseChangelists(output, allChangelists))
	{
		return false;
	}	

	if (allChangelists.size() == 0)
	{
		return false;
	}

	changelist = allChangelists[0];

	return true;
}

bool GitSourceControlProvider::GetTotalChangelists(int& totalChangelists)
{
	std::vector<std::string> arguments;

	arguments.push_back("rev-list");
	arguments.push_back("--all");
	arguments.push_back("--count");

	std::string output = "";

	if (!RunGitCommand(arguments, m_rootPath, &output))
	{
		Log(LogSeverity::Warning, "Failed to execute git command.\n");
		return false;
	}

	totalChangelists = CastFromString<int>(output);

	return true;
}

bool GitSourceControlProvider::GetHistory(const Platform::Path& path, std::vector<SourceControlChangelist>& history, int count, int offset)
{
	std::vector<std::string> arguments;

	arguments.push_back("log");
	arguments.push_back(Strings::Format("--max-count=%i", count));
	arguments.push_back(Strings::Format("--skip=%i", offset));
	arguments.push_back(Strings::Format("--pretty=\"%s\"", m_logFormat.c_str()));
	arguments.push_back("--");
	arguments.push_back(path.ToString());

	std::string output = "";

	if (!RunGitCommand(arguments, m_rootPath, &output))
	{
		Log(LogSeverity::Warning, "Failed to execute git command.\n");
		return false;
	}

	if (!ParseChangelists(output, history))
	{
		return false;
	}

	return true;
}

bool GitSourceControlProvider::Checkout(const Platform::Path& path)
{
	MB_UNUSED_PARAMETER(path);

	// git doesn't have to do anything here.

	return true;
}

bool GitSourceControlProvider::Commit(const std::vector<Platform::Path>& files, const std::string& commitMessage)
{
	std::vector<std::string> arguments;

	arguments.push_back("commit");
	arguments.push_back("-m");
	arguments.push_back(Strings::Quoted(commitMessage, true));
	for (auto& file : files)
	{
		arguments.push_back(file.ToString());
	}

	if (!RunGitCommand(arguments, m_rootPath))
	{
		Log(LogSeverity::Warning, "Failed to execute git command.\n");
		return false;
	}

	return true;
}

bool GitSourceControlProvider::Exists(const Platform::Path& path, bool& bExists)
{
	std::vector<std::string> arguments;

	arguments.push_back("status");
	arguments.push_back(path.ToString());
	arguments.push_back("--porcelain");

	std::string output = "";

	if (!RunGitCommand(arguments, m_rootPath, &output))
	{
		Log(LogSeverity::Warning, "Failed to execute git command.\n");
		return false;
	}

	bExists = (output.size() != 0);
	return true;
}

bool GitSourceControlProvider::Add(const Platform::Path& path)
{
	std::vector<std::string> arguments;

	arguments.push_back("add");
	arguments.push_back(path.ToString());

	if (!RunGitCommand(arguments, m_rootPath))
	{
		Log(LogSeverity::Warning, "Failed to execute git command.\n");
		return false;
	}

	return true;
}

bool GitSourceControlProvider::Sync()
{
	if (!RunGitCommand({ "pull" }, m_rootPath))
	{
		Log(LogSeverity::Warning, "Failed to execute git command.\n");
		return false;
	}

	if (!RunGitCommand({ "push" }, m_rootPath))
	{
		Log(LogSeverity::Warning, "Failed to execute git command.\n");
		return false;
	}

	return true;
}

}; // namespace MicroBuild
