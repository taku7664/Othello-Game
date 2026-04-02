#pragma once

class CChatManager
{
public:
	CChatManager();
	~CChatManager() = default;
	CChatManager( const CChatManager& ) = delete;
	CChatManager& operator=( const CChatManager& ) = delete;

public:
	struct Message
	{
		GUID		From;
		std::string Header;
		std::string Chat;
	};

public:
	bool Initialize();
	void Finalize();
	void Clear();

public:
	void PushChatMessage(GUID from, const char* msg);
	const std::vector<Message>& GetChatMessages() const;

private:
	std::vector<Message> m_messages;
};

