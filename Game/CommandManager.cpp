#include "pch.h"
#include "CommandManager.h"

CommandManager::CommandManager()
{
}

CommandManager::~CommandManager()
{
}

bool CommandManager::TryCommand(const std::string& command)
{
	if (command.empty() || command[0] != '/')
	{
		return false;
	}

	std::string header;
	std::string value;
	size_t spacePos = command.find(' ');

	if (spacePos == std::string::npos)
	{
		header = command;
		value = "";
	}
	else
	{
		header = command.substr(0, spacePos);
		value = command.substr(spacePos + 1);
	}

	if (m_commandTable.contains(header))
	{
		if (m_commandTable[header])
		{
			m_commandTable[header](value);
		}
		return true;
	}
	else
	{
		return false;
	}
}

void CommandManager::RegisterCommand(const char* header, std::function<void(const std::string&)> func)
{
	if (func && false == m_commandTable.contains(header))
	{
		m_commandTable[header] = func;
	}
}
