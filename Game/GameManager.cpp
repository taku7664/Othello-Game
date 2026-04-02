#include "pch.h"
#include "GameManager.h"
#include "IGameBoard.h"
#include "IPlayer.h"
#include "GameRoom.h"

CGameManager::CGameManager()
	: m_playerProfile()
{
}

void CGameManager::Initialize()
{
    GameCore::CommandAction.RegisterCommand("/title", [this](const std::string& value) {
        m_gameRoom.SetRoomTitle(value.c_str());
        std::string msg = std::format(
            "방 제목을 \"{}\"(으)로 변경했습니다.",
            value
        );
        GameCore::ChatManager.PushChatMessage( GUID_NULL , msg.c_str());
        });

    GameCore::CommandAction.RegisterCommand("/nickname", [this](const std::string& value) {
		if (GameCore::ActiveRoom)
		{
			GameCore::GetLocalPlayer()->SetNickName(value.c_str());
			std::string msg = std::format(
				"닉네임을 \"{}\"(으)로 변경했습니다.",
				value
			);
			GameCore::ChatManager.PushChatMessage( GUID_NULL , msg.c_str());
		}
        });
	GameCore::CommandAction.RegisterCommand( "/color" , [ this ] ( const std::string& value ) {
		if ( GameCore::ActiveRoom )
		{
			if(value == "black") 
			{
				GameCore::GetLocalPlayer()->SetColorType(ColorType::Black);
			}
			else if(value == "white")
			{
				GameCore::GetLocalPlayer()->SetColorType(ColorType::White);
			}
		}
		} );
}

void CGameManager::Finalize()
{
}

void CGameManager::Update()
{
    m_gameRoom.Update();
}

bool CGameManager::MakeRoom(const PlayerProfile& profile, const char* title, unsigned short port)
{
    Debug::Log log("CGameManager::MakeRoom()");

    if (GameCore::ActiveRoom)
    {
        log.WriteLine(Debug::LOG_WARNING, "[WARN] ActiveRoom is Null Reference.");
        return false;
    }
	if (profile.Nickname.empty())
	{
        log.WriteLineW(Debug::LOG_WARNING, L"유효하지 않은 닉네임입니다.");
		return false;
	}
    if (false == GameCore::GameServer.StartHostServer(port))
    {
        return false;
    }
    if (false == GameCore::GameServer.StartClientServer("127.0.0.1", port))
    {
        return false;
    }

	m_playerProfile = profile;
    GameCore::ActiveRoom = &m_gameRoom;
	m_gameRoom.SetRoomTitle( title, false );

    Packet::C2S_JoinRequest joinPacket;
	joinPacket.Guid		= m_playerProfile.Guid;
    joinPacket.IsHost	= true;
    strcpy_s(joinPacket.Nickname,
		m_playerProfile.Nickname.length() + 1,
		m_playerProfile.Nickname.c_str()
    );
    GameCore::ClientServer->SendPacketToServer(joinPacket);

    return true;
}

bool CGameManager::JoinRoom(const PlayerProfile& profile, const char* hostIP, unsigned short port)
{
    Debug::Log log("CGameManager::JoinRoom()");

    if (GameCore::ActiveRoom)
    {
        log.WriteLine(Debug::LOG_WARNING, "[WARN] ActiveRoom is Null Reference.");
        return false;
    }
	if (profile.Nickname.empty())
	{
        log.WriteLineW(Debug::LOG_WARNING, L"유효하지 않은 닉네임입니다.");
		return false;
	}
    if (false == GameCore::GameServer.StartClientServer(hostIP, port))
    {
        GameCore::GameServer.EndServer();
        return false;
    }

    GameCore::ActiveRoom = &m_gameRoom;
	m_playerProfile = profile;

    Packet::C2S_JoinRequest joinPacket;
	joinPacket.Guid		= m_playerProfile.Guid;
	joinPacket.IsHost	= false;
    strcpy_s(joinPacket.Nickname,
		m_playerProfile.Nickname.length() + 1 ,
		m_playerProfile.Nickname.c_str()
    );
    GameCore::ClientServer->SendPacketToServer(joinPacket);

    return true;
}

void CGameManager::LeaveRoom(const char* errTitle, const char* errMessage)
{
	Debug::Log log("CGameManager::LeaveRoom()");

	if (errTitle)
	{
		GameCore::SetErrorMessage( errTitle , errMessage );
		GameCore::MasterWindow->ChangeFrame(CMasterWindow::FRAME_ERROR);
	}
	else
	{
		GameCore::MasterWindow->ChangeFrame(CMasterWindow::FRAME_TITLE);
	}

	GameCore::ChatManager.Clear();

	m_gameRoom.Clear();
	GameCore::ActiveRoom = nullptr;

	if ( GameCore::GameServer.IsRunningServer() )
	{
		if ( IPlayer* local = GameCore::GetLocalPlayer() )
		{
			Packet::C2S_LeaveRequest packet{ .Guid = local->GetGUID() };
			GameCore::ClientServer->SendPacketToServer( packet );
		}
		GameCore::GameServer.EndServer();
	}
}
