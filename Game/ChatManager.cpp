#include "pch.h"
#include "ChatManager.h"

CChatManager::CChatManager()
{
}

bool CChatManager::Initialize()
{
	return true;
}

void CChatManager::Finalize()
{
	Clear();
}

void CChatManager::Clear()
{
	m_messages.clear();
}


void CChatManager::PushChatMessage(GUID from, const char* body)
{
	Message msg;
	msg.From = from;
	msg.Chat = body;
	// System Message
	if (from == GUID_NULL)
	{
		msg.Header = "[System]";
	}
	// User Message
	else if (GameCore::ActiveRoom)
	{
		if(IPlayer* dest = GameCore::ActiveRoom->GetPlayerFromGuid(from))
		{
			msg.Header = dest->GetNickName();
		}
		else
		{
			msg.Header = "Unknown Player";
		}
	}
	m_messages.push_back(std::move(msg));
}

const std::vector<CChatManager::Message>& CChatManager::GetChatMessages() const
{
	return m_messages;
}
