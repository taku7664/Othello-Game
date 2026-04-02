#include "pch.h"
#include "ClientNetwork.h"

CClientNetwork::CClientNetwork(const char* hostIP, unsigned short port)
	: CNetwork(port)
	, m_hostIP(hostIP)
{
}

CClientNetwork::~CClientNetwork()
{
}

bool CClientNetwork::Initialize()
{
	bool result = true;
	Debug::Log log("ClientRoom::Initialize()");

	if (m_hostIP.empty())
	{
		log.WriteLine(Debug::LOG_WARNING, "[WARN] HostIP is empty.");
		return false;
	}

	if (INVALID_SOCKET != m_connection.Socket)
	{
		log.WriteLine(Debug::LOG_WARNING, "[WARN] connection is already connected.");
		return false;
	}

	SOCKET& sock = m_connection.Socket;
	SOCKADDR_IN& addr = m_connection.Address;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sock)
	{
		log.WriteLine(Debug::LOG_WARNING, "[WARN] socket() failed with error: ", WSAGetLastError());
		result = false;
	}

	u_long ulMode = 1; // 0 = Blocking, 1 = Non-Blocking
	if (ioctlsocket(sock, FIONBIO, &ulMode) != NO_ERROR)
	{
		log.WriteLine(Debug::LOG_WARNING, "[WARN] ioctlsocket() failed with error: ", WSAGetLastError());
		result = false;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(m_port);
	if (inet_pton(AF_INET, m_hostIP.c_str(), &addr.sin_addr) <= 0)
	{
		log.WriteLine(Debug::LOG_WARNING, "[WARN] inet_pton() failed.");
		result = false;
	}

	int ret = connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != WSAEWOULDBLOCK && err != WSAEINPROGRESS && err != WSAEINVAL)
		{
			log.WriteLine(Debug::LOG_WARNING, "[WARN] connect() failed immediately with error: ", err);
			closesocket(sock);
			sock = INVALID_SOCKET;
			return false;
		}
	}
	// wait connect
	fd_set writefds;
	FD_ZERO(&writefds);
	FD_SET(sock, &writefds);

	timeval tv{};
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	ret = select(0, nullptr, &writefds, nullptr, &tv);
	if (ret == 0)
	{
		log.WriteLine(Debug::LOG_WARNING, "[WARN] connect timeout.");
		return false;
	}
	else if (ret == SOCKET_ERROR)
	{
		log.WriteLine(Debug::LOG_WARNING, "[WARN] select() failed with error: ", WSAGetLastError());
		return false;
	}

	int so_error = 0;
	int len = sizeof(so_error);
	getsockopt(sock, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&so_error), &len);

	if (so_error != 0)
	{
		log.WriteLine(Debug::LOG_WARNING, "[WARN] connect failed with SO_ERROR: ", so_error);
		return false;
	}

	log.WriteLine(Debug::LOG_INFO, "connect succeed! [", m_hostIP.c_str(), " : ", m_port, "]");
	return true;
}

void CClientNetwork::Finalize()
{
	Debug::Log log("CClientNetwork::Finalize()");

	m_port = 0;
	m_hostIP.clear();
	m_connection.ResetConnection();
}

void CClientNetwork::Update()
{
	if (INVALID_SOCKET == m_connection.Socket)
	{
		return;
	}

	fd_set readfds;
	fd_set writefds;
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);

	FD_SET(m_connection.Socket, &readfds);
	if (!m_connection.SendQueue.empty())
	{
		FD_SET(m_connection.Socket, &writefds);
	}

	timeval tv{};
	tv.tv_sec = 0;
	tv.tv_usec = 10000;

	int ret = select(0, &readfds, &writefds, nullptr, &tv);
	if (ret == SOCKET_ERROR)
	{
		std::cerr << "select() failed: " << WSAGetLastError() << "\n";
		return;
	}

	if (FD_ISSET(m_connection.Socket, &writefds))
	{
		SendToHost();
	}

	if (FD_ISSET(m_connection.Socket, &readfds))
	{
		ReceiveFromHost();
	}
}

int CClientNetwork::GetConnectionID() const
{
	return m_connection.ID;
}

#define PACKET_IF(type) \
if(typeName == typeid(Packet::type).name() + 15) {	\
	Handle_##type(header, PacketCast<Packet::type>(body));	\
	return;	\
}

void CClientNetwork::HandlePacket(PacketHeader header, const char* body)
{
	Debug::Log log("CClientNetwork::Handle_Packet()");
	std::string typeName(header.TypeName);
	PACKET_IF(S2C_PlayerJoined)
	PACKET_IF(S2C_PlayerLeaved)
	PACKET_IF(S2C_PlayerKicked)
	PACKET_IF(S2C_PlaceStone)
	PACKET_IF(Com_Error)
	PACKET_IF(Com_ChatMessage)
	PACKET_IF(Com_PlayerRefreshed)
	PACKET_IF(Com_RoomRefreshed)
}

void CClientNetwork::SendToHost()
{
	m_connection.SendPackets();
}

void CClientNetwork::ReceiveFromHost()
{
	m_connection.RecievePackets();
	for (Buffer& buf : m_connection.RecvQueue)
	{
		PacketHeader header	= GetHeaderFromPacketBuffer(buf.data());
		const char*  body   = GetBodyFromPacketBuffer(buf.data());
		HandlePacket(header, body);
	}
	m_connection.RecvQueue.clear();
}

void CClientNetwork::Handle_S2C_PlayerJoined(PacketHeader header, const Packet::S2C_PlayerJoined* body)
{
	Debug::Log log("CClientNetwork::Handle_S2C_PlayerJoined()");

	if (IGameRoom* gameRoom = GameCore::ActiveRoom)
	{
		PlayerDesc desc;
		desc.ConnectionID = body->ConnectionID;
		desc.Guid = body->Guid;
		desc.Nickname = body->Nickname;
		desc.IsHost = body->IsHost;
		desc.IsLocal = (desc.Guid == GameCore::GameManager.GetGUID());
		if(IPlayer* player = gameRoom->AddPlayer(desc))
		{
			std::string msg = std::format(
				"\"{}\"님이 접속하였습니다." ,
				player->GetNickName()
			);
			if ( body->IsNew )
			{
				GameCore::ChatManager.PushChatMessage( GUID_NULL , msg.c_str() );
				log.WriteLineW( Debug::LOG_INFO , msg.c_str() );
			}
			GameCore::MasterWindow->ChangeFrame( CMasterWindow::FRAME_GAME );
		}
		else
		{
			std::string lastError = Utillity::WCharToString( Debug::Log::GetLastMessage() );
			GameCore::GameManager.LeaveRoom( "방 접속에 실패하셨습니다." , lastError.c_str() );
		}
	}
}

void CClientNetwork::Handle_S2C_PlayerLeaved(PacketHeader header, const Packet::S2C_PlayerLeaved* body)
{
	Debug::Log log("CClientNetwork::Handle_S2C_PlayerLeaved()");
	if (IPlayer* player = GameCore::GetPlayerFromGuid(body->Guid))
	{
		std::string msg = std::format( "{}님이 퇴장하셨습니다." ,
			player->GetNickName() );
		GameCore::ChatManager.PushChatMessage( GUID_NULL , msg.c_str() );
		GameCore::ActiveRoom->RemovePlayer(body->Guid);
	}
}

void CClientNetwork::Handle_S2C_PlayerKicked( PacketHeader header , const Packet::S2C_PlayerKicked* body )
{
	Debug::Log log( "CClientNetwork::Handle_S2C_PlayerKicked()" );
	if ( IPlayer* local = GameCore::GetLocalPlayer() )
	{
		if ( local->GetGUID() == body->Guid )
		{
			GameCore::GameManager.LeaveRoom( "방에서 퇴장하셨습니다." , "강퇴" );
		}
		else if( IPlayer* dest = GameCore::GetPlayerFromGuid( body->Guid ) )
		{
			std::string msg = std::format( "{}님이 강퇴당하셨습니다." ,
				dest->GetNickName() );
			GameCore::ChatManager.PushChatMessage( GUID_NULL , msg.c_str() );
			GameCore::ActiveRoom->RemovePlayer( body->Guid );
		}
	}
}

void CClientNetwork::Handle_S2C_PlaceStone( PacketHeader header , const Packet::S2C_PlaceStone* body )
{
	Debug::Log log("CClientNetwork::HandlS2C_PlaceStone()");

	if (IGameRoom* gameRoom = GameCore::ActiveRoom)
	{
		if (IPlayer* player = gameRoom->GetPlayerFromGuid(body->Guid))
		{
			ColorType color = player->GetColorType();
			IGameBoard& board = gameRoom->GetGameBoard();
			board.PlaceStone(color, body->Row, body->Col);
		}
	}
}

void CClientNetwork::Handle_Com_Error( PacketHeader header , const Packet::Com_Error* body )
{
	Debug::Log log("CClientNetwork::Handle_Com_Error()");

	const char* raw = reinterpret_cast<const char*>(body);
	const size_t offset = sizeof(Packet::Com_Error);
	const char* desc = reinterpret_cast<const char*>(raw + offset);
	GameCore::GameManager.LeaveRoom(body->ErrTitle, desc);
}

void CClientNetwork::Handle_Com_ChatMessage(PacketHeader header, const Packet::Com_ChatMessage* body)
{
	Debug::Log log("CClientNetwork::Handle_Com_ChatMessage()");

	const char* raw = reinterpret_cast<const char*>(body);
	const size_t offset = sizeof(Packet::Com_ChatMessage);
	const char* msg = reinterpret_cast<const char*>(raw + offset);
	GameCore::ChatManager.PushChatMessage(body->FromGuid, msg);
}

void CClientNetwork::Handle_Com_PlayerRefreshed(PacketHeader header, const Packet::Com_PlayerRefreshed* body)
{
	Debug::Log log( "CClientNetwork::Handle_Com_PlayerRefreshed()" );

	if (Player* dest = dynamic_cast<Player*>(GameCore::GetPlayerFromGuid(body->DestGuid)))
	{
		dest->RefreshFromPacket(*body);
	}
}

void CClientNetwork::Handle_Com_RoomRefreshed( PacketHeader header , const Packet::Com_RoomRefreshed* body )
{
	Debug::Log log( "CClientNetwork::Handle_Com_RoomRefreshed()" );

	if (GameRoom* gameRoom = dynamic_cast<GameRoom*>(GameCore::ActiveRoom))
	{
		gameRoom->RefreshFromPacket(*body, !body->IsNew);
	}
}