#include "pch.h"
#include "Player.h"

Player::Player()
	: m_connectionId(0)
	, m_guid(GUID_NULL)
	, m_bIsHost(false)
	, m_bIsLocal(false)
	, m_refreshFlags(REFRESH_FLAG_NONE)
	, m_colorType(ColorType::Black)
	, m_bIsReady(false)
{
}

Player::Player(const PlayerDesc& data)
	: m_connectionId(data.ConnectionID)
	, m_guid(data.Guid)
	, m_bIsHost(data.IsHost)
	, m_bIsLocal(data.IsLocal)
	, m_refreshFlags(REFRESH_FLAG_NONE)
	, m_nickname(data.Nickname)
	, m_colorType(ColorType::Black)
	, m_bIsReady(false)
{
}

Player::~Player()
{
}

void Player::Update()
{
	if (m_refreshFlags != REFRESH_FLAG_NONE)
	{
		if (GameCore::ClientServer)
		{
			Packet::Com_PlayerRefreshed packet;
			packet.DestGuid		= GetGUID();
			packet.RefreshFlags = m_refreshFlags;
			packet.Color		= m_colorType;
			packet.IsReady		= m_bIsReady;
			strcpy_s(packet.Nickname,
				m_nickname.length() + 1,
				m_nickname.c_str()
			);
			GameCore::ClientServer->SendPacketToServer(packet);
		}
		m_refreshFlags = REFRESH_FLAG_NONE;
	}
}

void Player::SetNickName(const char* nickname)
{
	if (m_nickname != nickname)
	{
		m_nickname = nickname;
		m_refreshFlags += REFRESH_FLAG_NICKNAME;
	}
}

const std::string& Player::GetNickName() const
{
	return m_nickname;
}

void Player::SendChatMessage(const char* msgStr)
{
	const std::string message = msgStr;
	const size_t msgLen = message.length();

	if (msgLen >= 1 && message[0] == '/')
	{
		if (false == GameCore::CommandAction.TryCommand(msgStr))
		{
			std::string msg = std::format(
				"\"{}\" 알 수 없는 명령어입니다.",
				message
			);
			GameCore::ChatManager.PushChatMessage( GUID_NULL , msgStr);
		}
	}
	else
	{
		if (GameCore::ClientServer)
		{
			size_t strLen = message.size();
			size_t strSize = strLen + 1;
			size_t bodySize = sizeof(Packet::Com_ChatMessage) + strSize;
			std::vector<char> buffer(bodySize);

			Packet::Com_ChatMessage* packet = reinterpret_cast<Packet::Com_ChatMessage*>(buffer.data());
			packet->FromGuid = GetGUID();
			memcpy(buffer.data() + sizeof(Packet::Com_ChatMessage), message.data(), strSize);

			GameCore::ClientServer->SendPacketToServer(*packet, bodySize);
		}
	}
}

void Player::RefreshFromPacket( const Packet::Com_PlayerRefreshed& packet )
{
	if (packet.RefreshFlags[REFRESH_FLAG_NICKNAME])
	{
		m_nickname = packet.Nickname;
	}
	if (packet.RefreshFlags[REFRESH_FLAG_COLOR])
	{
		m_colorType = packet.Color;
		std::string msg = std::format(
			"{}님이 색을 \"{}\"(으)로 변경했습니다." ,
			m_nickname.c_str(),
			ColorTypeToString(m_colorType)
		);
		GameCore::ChatManager.PushChatMessage( GUID_NULL , msg.c_str() );
	}
	if ( packet.RefreshFlags[ REFRESH_FLAG_READY ] )
	{
		m_bIsReady = packet.IsReady;
	}
}

int Player::GetConnectionID() const
{
	return m_connectionId;
}

GUID Player::GetGUID() const
{
	return m_guid;
}

bool Player::IsHost() const
{
	return m_bIsHost;
}

bool Player::IsLocal() const
{
	return m_bIsLocal;
}

void Player::SetColorType( ColorType color )
{
	if ( m_colorType != color )
	{
		m_colorType = color;
		m_refreshFlags += REFRESH_FLAG_COLOR;
	}
}

ColorType Player::GetColorType() const
{
	return m_colorType;
}

void Player::SetReady( bool isReady )
{
	if ( m_bIsReady != isReady )
	{
		m_bIsReady = isReady;
		m_refreshFlags += REFRESH_FLAG_READY;
	}
}

bool Player::IsReady()
{
	return m_bIsReady;
}