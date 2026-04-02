#pragma once

class CommandManager final
{
	using CommandTable = std::unordered_map<std::string, std::function<void(const std::string&)>>;
public:
	CommandManager();
	~CommandManager();
	CommandManager(const CommandManager&) = delete;
	CommandManager& operator=(const CommandManager&) = delete;

public:
	bool TryCommand(const std::string& cmd);
	void RegisterCommand(const char* header, std::function<void(const std::string&)> func);

private:
	CommandTable m_commandTable;

};