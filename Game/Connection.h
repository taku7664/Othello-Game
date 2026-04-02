#pragma once

using Buffer = std::vector<char>;

struct Connection
{
public:
	Connection();
	~Connection();
	Connection(const Connection&) = delete;
	Connection& operator=(const Connection&) = delete;
	Connection(Connection&& other) noexcept;
	Connection& operator=(Connection&& other) noexcept;

	template<typename T>
	void PushPacket(const T& body, size_t bodySize = 0);
	void SendPackets();
	void RecievePackets();
	void ResetConnection();

	int						ID = 0;
	SOCKET					Socket = INVALID_SOCKET;
	SOCKADDR_IN				Address = {};
	std::vector<Buffer>		SendQueue;
	std::vector<Buffer>		RecvQueue;
	Buffer					RecvBuffer;
};

template<typename T>
inline void Connection::PushPacket(const T& body, size_t bodySize)
{
	const size_t headerSize = sizeof(PacketHeader);
	bodySize = (bodySize == 0) ? sizeof(T) : bodySize;

	PacketHeader header {};
	const char* name = typeid( T ).name() + 15;
	strcpy_s(header.TypeName, sizeof(header.TypeName), name);
	header.BodySize = bodySize;

	Buffer& buffer = SendQueue.emplace_back(headerSize + bodySize);
	memcpy(buffer.data(), &header, headerSize);
	memcpy(buffer.data() + headerSize, &body, bodySize);
}
