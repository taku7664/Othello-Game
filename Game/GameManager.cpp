#include "pch.h"
#include "GameManager.h"
#include "IGameBoard.h"
#include "IPlayer.h"
#include "GameRoom.h"
#include "DebugWindow.h"

CGameManager::CGameManager()
	: m_playerProfile()
{
}

void CGameManager::Initialize()
{
    GameCore::CommandAction.RegisterCommand("/debug", [this](const std::string& value) {
		GameCore::ImGuiManager.CreateImWindow<CDebugWindow>( "debug" );
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
}

void CGameManager::Finalize()
{
	if (GameCore::ActiveRoom)
	{
		LeaveRoom();
	}
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

	if ( nullptr == GameCore::ActiveRoom )
	{
		log.WriteLine( Debug::LOG_WARNING , "[WARN] ActiveRoom is Null Reference." );
		return;
	}

	m_gameRoom.Clear();
	GameCore::ActiveRoom = nullptr;
	GameCore::ChatManager.Clear();

	if (errTitle)
	{
		GameCore::SetErrorMessage( errTitle , errMessage );
		GameCore::MasterWindow->ChangeFrame(CMasterWindow::FRAME_ERROR);
	}
	else
	{
		GameCore::MasterWindow->ChangeFrame(CMasterWindow::FRAME_TITLE);
	}

	if ( GameCore::GameServer.IsRunningServer() )
	{
		if ( GameCore::HostServer )
		{
			std::string errTitle = "방에서 퇴장했습니다.";
			std::string errDesc  = "호스트가 방에서 퇴장했습니다.";
			size_t		descLen = errDesc.size();
			size_t		descSize = descLen + 1;
			size_t		bodySize = sizeof( Packet::Com_Error ) + descSize;

			std::vector<char> buffer( bodySize );
			Packet::Com_Error* packet = reinterpret_cast< Packet::Com_Error* >( buffer.data() );
			strcpy_s( packet->ErrTitle , errTitle.length() + 1 , errTitle.c_str() );
			memcpy( buffer.data() + sizeof( Packet::Com_Error ) , errDesc.data() , descSize );

			GameCore::HostServer->BroadCast( *packet , bodySize );
		}
		else if ( IPlayer* local = GameCore::GetLocalPlayer() )
		{
			Packet::C2S_LeaveRequest packet{ .Guid = local->GetGUID() };
			GameCore::ClientServer->SendPacketToServer( packet );
		}
		GameCore::GameServer.EndServer();
	}
}
