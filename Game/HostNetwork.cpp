#include "pch.h"
#include "HostNetwork.h"

CHostNetwork::CHostNetwork(unsigned short port)
	: CNetwork(port)
	, m_socket(INVALID_SOCKET)
	, m_address({})
{
}

CHostNetwork::~CHostNetwork()
{
}

bool CHostNetwork::Initialize()
{
	Debug::Log log("CHostNetwork::CHostNetwork()");

    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == m_socket)
    {
        log.WriteLine(Debug::LOG_WARNING, "[WARN] socket() failed with error: ", WSAGetLastError());
        return false;
    }

    u_long ulMode = 1; // 0 = Blocking, 1 = Non-Blocking
    if (ioctlsocket(m_socket, FIONBIO, &ulMode) != NO_ERROR)
    {
        log.WriteLine(Debug::LOG_WARNING, "[WARN] ioctlsocket() failed with error: ", WSAGetLastError());
        return false;
    }

    m_address.sin_family = AF_INET;
    m_address.sin_port = htons(m_port);
    m_address.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(m_socket, (SOCKADDR*)&m_address, sizeof(m_address)) == SOCKET_ERROR)
    {
        log.WriteLine(Debug::LOG_WARNING, "[WARN] bind() failed with error: ", WSAGetLastError());
        return false;
    }

    if (listen(m_socket, SOMAXCONN) == SOCKET_ERROR)
    {
        log.WriteLine(Debug::LOG_WARNING, "[WARN] listen() failed with error: ", WSAGetLastError());
        return false;
    }

    log.WriteLine(Debug::LOG_INFO, "listen succeed! [port: ", m_port, "]");
    return true;
}

void CHostNetwork::Finalize()
{
    Debug::Log log("CHostNetwork::Finalize()");
    Close();
}

void CHostNetwork::Update()
{
    if (m_socket == INVALID_SOCKET)
    {
        return;
    }

    fd_set readfds, writefds;
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_SET(m_socket, &readfds);

    for (auto& c : m_connections)
    {
        if (c.Socket != INVALID_SOCKET)
        {
            FD_SET(c.Socket, &readfds);
            if (false == c.SendQueue.empty())
            {
                FD_SET(c.Socket, &writefds);
            }
        }
    }

    timeval tv{};
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    int activity = select(0, &readfds, &writefds, nullptr, &tv);
    if (SOCKET_ERROR == activity)
    {
        Debug::Log::WriteLine(Debug::LOG_WARNING, "[WARN] select() failed: ", WSAGetLastError());
        return;
    }
    else if (activity > 0)
    {
        if (FD_ISSET(m_socket, &readfds))
        {
            AcceptClients();
        }

        ReceiveFromClients(readfds);
        SendToClients(writefds);
    }
}

bool CHostNetwork::HeartBeat()
{
	const float deltaTime = GameCore::Time.GetUnScaledDeltaTime();
	m_heartBeatTimer += deltaTime;
	if ( m_heartBeatTimer > m_heartBeatTick )
	{
		Debug::Log::WriteLine( Debug::LOG_INFO , "Host HeartBeat." );
		m_heartBeatTimer = 0.0f;
		BroadCast<Packet::Com_HeartBeat>( {} );
	}

	bool isDisconnected = false;
	for ( Connection& connection : m_connections )
	{
		if ( connection.IdleTime > m_timeoutTime )
		{
			HandleConnectionDisconnect( connection );
			isDisconnected = true;
		}
		else
		{
			connection.IdleTime += deltaTime;
		}
	}
	if ( isDisconnected )
	{
		RefreshInvalidConnections();
	}
	return true;
}

void CHostNetwork::Close()
{
    closesocket(m_socket);
    m_socket = INVALID_SOCKET;
    m_address = {};
    for (Connection& c : m_connections)
    {
        c.ResetConnection();
    }
    m_connections.clear();
}

void CHostNetwork::RefreshInvalidConnections()
{
	auto remiveIt = std::remove_if( m_connections.begin() , m_connections.end() , [ ] ( const Connection& c ) {
		return c.Socket == INVALID_SOCKET;
		} );
	m_connections.erase( remiveIt , m_connections.end() );
}

void CHostNetwork::AcceptClients()
{
    Debug::Log log("CHostNetwork::AcceptClients()");

    while (true)
    {
        SOCKADDR_IN clientAddr{};
        int len = sizeof(clientAddr);
        SOCKET clientSock = accept(m_socket, reinterpret_cast<sockaddr*>(&clientAddr), &len);
        if (clientSock == INVALID_SOCKET)
        {
            break;
        }

        u_long ulMode = 1; // 0 = Blocking, 1 = Non-Blocking
        if (ioctlsocket(clientSock, FIONBIO, &ulMode) != NO_ERROR)
        {
            log.WriteLine(Debug::LOG_WARNING, "[WARN] ioctlsocket() failed with error: ", WSAGetLastError());
            closesocket(clientSock);
            continue;
        }
        
        Connection& connection = m_connections.emplace_back();
        connection.Socket = clientSock;
        connection.Address = clientAddr;
        connection.ID = GameCore::GameServer.GetNewServerInstanceID();

        char ip[INET_ADDRSTRLEN] = {};
        inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip));

        log.WriteLine(Debug::LOG_INFO, "accept succeed! [ip: ", ip, "]");

        //room_->PrintSystem(std::string("Incoming connection from ") + ip + ":" + std::to_string(ntohs(clientAddr.sin_port)));
    }
}

void CHostNetwork::ReceiveFromClients(fd_set& readfds)
{
    for (Connection& connection : m_connections)
    {
        if (connection.Socket == INVALID_SOCKET ||
            !FD_ISSET(connection.Socket, &readfds))
        {
            continue;
        }
        connection.RecievePackets();
		if ( false == connection.RecvQueue.empty() )
		{
			connection.IdleTime = 0.0f;
			for ( Buffer& buf : connection.RecvQueue )
			{
				PacketHeader header = GetHeaderFromPacketBuffer( buf.data() );
				const char* body = GetBodyFromPacketBuffer( buf.data() );
				HandlePacket( connection , header , body );
			}
			connection.RecvQueue.clear();
		}
    }
}

void CHostNetwork::SendToClients(fd_set& writefds)
{
    for (Connection& connection : m_connections)
    {
        if (connection.Socket == INVALID_SOCKET ||
            !FD_ISSET(connection.Socket, &writefds))
        {
            continue;
        }
        connection.SendPackets();
    }
}

void CHostNetwork::HandleConnectionDisconnect( Connection& connection )
{
	if ( IPlayer* player = GameCore::GetPlayerFromConnectionID( connection.ID ) )
	{
		const std::string mainCause = "Lost Connection Timeout";
		Packet::S2C_PlayerDisConnected packet;
		packet.Guid = player->GetGUID();
		strcpy_s( packet.MainCause , mainCause.length() + 1 , mainCause.c_str() );
		BroadCast( packet );
	}
	connection.Socket = INVALID_SOCKET;
}

#define PACKET_IF(type) \
if(typeName == typeid(Packet::type).name() + 15) {	\
	Handle_##type(connection, header, PacketCast<Packet::type>(body));	\
	return;	\
}

void CHostNetwork::HandlePacket(Connection& connection, PacketHeader header, const char* body)
{
	std::string typeName(header.TypeName);

	PACKET_IF(C2S_JoinRequest)
	PACKET_IF(C2S_LeaveRequest)
	PACKET_IF(C2S_PlaceStone)
	PACKET_IF(Com_ChatMessage)
	PACKET_IF(Com_PlayerRefreshed)
}

void CHostNetwork::Handle_C2S_JoinRequest(Connection& connection, PacketHeader header, const Packet::C2S_JoinRequest* body)
{
    Debug::Log log("CHostNetwork::HandleJoinPlayer()");
    if (IGameRoom* gameRoom = GameCore::ActiveRoom)
    {
		size_t currPlayerCount = gameRoom->GetCurrentPlayerCount();
		size_t maxPlayerCount = gameRoom->GetRoomSetting().MaxPlayerCount;
		if ( currPlayerCount < maxPlayerCount )
		{
			/// RoomPacket
			{
            Packet::Com_RoomRefreshed packet;
			packet.IsNew = true;
			packet.RefreshFlags = ~0;
			packet.Setting = gameRoom->GetRoomSetting();
			packet.State = gameRoom->GetRoomState();
            strcpy_s(packet.Title,
                gameRoom->GetRoomTitle().length() + 1,
                gameRoom->GetRoomTitle().c_str()
            );
            connection.PushPacket(packet);
			}
			/// NewPlayer Packet
			{
			    size_t currPlayerCount = gameRoom->GetCurrentPlayerCount();
			    for (size_t i = 0; i < currPlayerCount; ++i)
			    {
			        if (IPlayer* player = gameRoom->GetPlayerFromIndex(i))
			        {
			            Packet::S2C_PlayerJoined packet;
						packet.ConnectionID = player->GetConnectionID();
			            packet.Guid = player->GetGUID();
			            packet.IsHost = player->IsHost();
			            packet.IsNew = false;
			            strcpy_s(packet.Nickname,
			                player->GetNickName().length() + 1,
			                player->GetNickName().c_str()
			            );
			            connection.PushPacket(packet);
			        }
			    }
			}
			/// OldPlayer Packet
			{
				Packet::S2C_PlayerJoined packet;
				packet.ConnectionID = connection.ID;
				packet.Guid = body->Guid;
				packet.IsHost = body->IsHost;
				packet.IsNew = true;
				strcpy_s(packet.Nickname,
					strlen(body->Nickname) + 1,
					body->Nickname
				);
				BroadCast(packet);
			}
		}
		else
		{
			/// Already Full
			std::string errTitle = "접속에 실패하셨습니다.";
			std::string errDesc  = "방의 인원이 이미 가득 찼습니다.";
			size_t		descLen  = errDesc.size();
			size_t		descSize = descLen + 1;
			size_t		bodySize = sizeof(Packet::Com_Error) + descSize;

			std::vector<char> buffer(bodySize);
			Packet::Com_Error* packet = reinterpret_cast<Packet::Com_Error*>(buffer.data());
			strcpy_s(packet->ErrTitle, errTitle.length() + 1, errTitle.c_str());
			memcpy(buffer.data() + sizeof(Packet::Com_Error), errDesc.data(), descSize);

			connection.PushPacket(*packet, bodySize);
		}
    }
}

void CHostNetwork::Handle_C2S_LeaveRequest( Connection& connection , PacketHeader header , const Packet::C2S_LeaveRequest* body )
{
	Debug::Log log( "CHostNetwork::Handle_C2S_LeaveRequest()" );
	{
		Packet::S2C_PlayerDisConnected packet;
		packet.Guid = body->Guid;
		for (Connection& c : m_connections)
		{
			c.PushPacket<Packet::S2C_PlayerDisConnected>(packet);
		}
	}
}
void CHostNetwork::Handle_C2S_PlaceStone( Connection& connection , PacketHeader header , const Packet::C2S_PlaceStone* body )
{
	Debug::Log log( "CHostNetwork::Handle_C2S_PlaceStone()" );
	Packet::S2C_PlaceStone packet;
	packet.Guid = body->Guid;
	packet.Row = body->Row;
	packet.Col = body->Col;
	for (Connection& c : m_connections)
	{
		c.PushPacket<Packet::S2C_PlaceStone>(packet);
	}
}

void CHostNetwork::Handle_Com_ChatMessage(Connection& connection, PacketHeader header, const Packet::Com_ChatMessage* body)
{
    Debug::Log log("CHostNetwork::Handle_ChatMessage()");
    for (Connection& c : m_connections)
    {
        c.PushPacket(*body, header.BodySize);
    }
}

void CHostNetwork::Handle_Com_PlayerRefreshed( Connection& connection , PacketHeader header , const Packet::Com_PlayerRefreshed* body )
{
	Debug::Log log( "CHostNetwork::Handle_Com_PlayerRefreshed()" );
	for (Connection& c : m_connections)
    {
        c.PushPacket(*body);
    }
}