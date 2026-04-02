#pragma once
#include "Network.h"

class CHostNetwork : public CNetwork
{
public:
    CHostNetwork(unsigned short port);
    virtual ~CHostNetwork();

public:
    bool Initialize() override;
    void Finalize() override;
    void Update() override;

	unsigned short GetPort() const;

	template<typename PacketT>
	void BroadCast(const PacketT& packet , size_t bodySize = 0);
	template<typename PacketT>
	void SendToConnection( int id , const PacketT& packet , size_t bodySize = 0 );

private:
    void Close();
    void AcceptClients();
    void ReceiveFromClients(fd_set& readfds);
    void SendToClients(fd_set& writefds);

private:
    void HandlePacket(Connection& connection, PacketHeader header, const char* body);
    void Handle_C2S_JoinRequest(Connection& connection, PacketHeader header, const Packet::C2S_JoinRequest* body);
    void Handle_C2S_LeaveRequest(Connection& connection, PacketHeader header, const Packet::C2S_LeaveRequest* body);
    void Handle_C2S_PlaceStone(Connection& connection, PacketHeader header, const Packet::C2S_PlaceStone* body);
    void Handle_Com_ChatMessage(Connection& connection, PacketHeader header, const Packet::Com_ChatMessage* body);
    void Handle_Com_PlayerRefreshed(Connection& connection, PacketHeader header, const Packet::Com_PlayerRefreshed* body);

private:
    SOCKET			        m_socket;
    SOCKADDR_IN		        m_address;
    std::vector<Connection> m_connections;
};

template<typename PacketT>
inline void CHostNetwork::BroadCast( const PacketT& packet , size_t bodySize )
{
	for(Connection& connection : m_connections)
	{
		connection.PushPacket(packet, bodySize);
	}
}

template<typename PacketT>
inline void CHostNetwork::SendToConnection( int id , const PacketT& packet , size_t bodySize )
{
	for ( Connection& connection : m_connections )
	{
		if ( connection.ID == id )
		{
			connection.PushPacket( packet , bodySize );
		}
	}
}
