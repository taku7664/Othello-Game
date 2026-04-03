#pragma once
#include "Network.h"

class CClientNetwork : public CNetwork
{
public:
	CClientNetwork(const char* hostIP, unsigned short port);
	virtual ~CClientNetwork();

public:
	bool Initialize() override;
	void Finalize() override;
	void Update() override;
	bool HeartBeat() override;

	template<typename PacketT>
	void SendPacketToServer(const PacketT& packet, size_t bodySize = 0);

	int GetConnectionID() const;

private:
	void HandlePacket(PacketHeader header, const char* body);
	void SendToHost();
	void ReceiveFromHost();

private:
	void Handle_S2C_PlayerJoined(PacketHeader header, const Packet::S2C_PlayerJoined* body);
	void Handle_S2C_PlayerDisConnected(PacketHeader header, const Packet::S2C_PlayerDisConnected* body);
	void Handle_S2C_PlaceStone(PacketHeader header, const Packet::S2C_PlaceStone* body);
	void Handle_Com_Error(PacketHeader header, const Packet::Com_Error* body);
	void Handle_Com_ChatMessage(PacketHeader header, const Packet::Com_ChatMessage* body);
	void Handle_Com_PlayerRefreshed(PacketHeader header, const Packet::Com_PlayerRefreshed* body);
	void Handle_Com_RoomRefreshed(PacketHeader header, const Packet::Com_RoomRefreshed* body);


private:
	Connection	m_connection;
	std::string	m_hostIP;
};

template<typename PacketT>
inline void CClientNetwork::SendPacketToServer(const PacketT& packet, size_t bodySize)
{
	m_connection.PushPacket(packet, bodySize);
}
