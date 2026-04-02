#include "pch.h"
#include "Connection.h"

Connection::Connection()
{
	ID = GameCore::GameServer.GetNewServerInstanceID();
}

Connection::~Connection()
{
}

Connection::Connection(Connection&& other) noexcept
    : ID(other.ID), Socket(other.Socket), Address(other.Address), SendQueue(std::move(other.SendQueue))
{
    other.ID = 0;
    other.Socket = INVALID_SOCKET;
    other.Address = {};
    other.SendQueue.clear();
}

Connection& Connection::operator=(Connection&& other) noexcept
{
    if (this != &other) 
    {
        if (INVALID_SOCKET != Socket)
        {
            closesocket(Socket);
        }

        ID = other.ID;
        Socket = other.Socket;
        Address = other.Address;
        SendQueue = std::move(other.SendQueue);

        other.ID = 0;
        other.Socket = INVALID_SOCKET;
        other.Address = {};
        other.SendQueue.clear();
    }
    return *this;
}

void Connection::SendPackets()
{
	if (SendQueue.empty())
	{
		return;
	}

	Debug::Log log("Connection::Send()");
	for (int i = 0; i < SendQueue.size();)
	{
		Buffer& buffer		= SendQueue[i];
		size_t bufferSize	= buffer.size();
		int sent = send(Socket, (char*)buffer.data(), (int)bufferSize, 0);
		if (sent > 0)
		{
			if (sent == bufferSize)
			{
				++i;
				log.WriteLine(Debug::LOG_INFO, bufferSize, "bytes send succeed.");
			}
			else
			{
				buffer.erase(buffer.begin(), buffer.begin() + sent);
				break;
			}
		}
		else
		{
            ++i;
            log.WriteLine(Debug::LOG_WARNING, "[WARN] send() failed with error: ", WSAGetLastError());
		}
	}
	SendQueue.clear();
}

// TODO: 후에 StreamQueue로 변경
void Connection::RecievePackets()
{
    char temp[4096];

    while (true)
    {
        int recvBytes = recv(Socket, temp, sizeof(temp), 0);

        if (recvBytes > 0)
        {
            // 1) 받은 데이터를 누적 버퍼 뒤에 붙임
            RecvBuffer.insert(RecvBuffer.end(), temp, temp + recvBytes);

            // 2) 완성된 패킷이 있으면 계속 꺼내서 RecvQueue로 이동
            while (true)
            {
                // 헤더조차 다 안 왔으면 더 받아야 함
                if (RecvBuffer.size() < sizeof(PacketHeader))
                    break;

                // 맨 앞에서 PacketHeader 읽기
                PacketHeader header;
                std::memcpy(&header, RecvBuffer.data(), sizeof(PacketHeader));

                // 패킷 전체가 아직 안 왔으면 다음 recv까지 대기
                if (RecvBuffer.size() < sizeof(PacketHeader) + header.BodySize)
                    break;

                // 패킷 하나를 RecvQueue에 옮김
                Buffer packet(RecvBuffer.begin(), RecvBuffer.begin() + sizeof(PacketHeader) + header.BodySize);
                RecvQueue.push_back(std::move(packet));

                // 앞에서 꺼낸 패킷만큼 제거
                RecvBuffer.erase(RecvBuffer.begin(), RecvBuffer.begin() + sizeof(PacketHeader) + header.BodySize);
            }
        }
        else if (recvBytes == 0)
        {
            // 상대가 정상 종료
            // 필요하면 Disconnect 처리
            break;
        }
        else
        {
            const int err = WSAGetLastError();

            if (err == WSAEWOULDBLOCK)
            {
                // 논블로킹 소켓에서 지금 더 읽을 게 없음
                break;
            }

            // 실제 에러
            // 필요하면 로그/Disconnect 처리
            break;
        }
    }
}

void Connection::ResetConnection()
{
	if (INVALID_SOCKET != Socket)
	{
		closesocket(Socket);
	}
	Socket = INVALID_SOCKET;
	Address = {};
    SendQueue.clear();
    RecvQueue.clear();
    RecvBuffer.clear();
}