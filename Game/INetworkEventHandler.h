#pragma once

class INetworkEventHandler
{
public:
    virtual void HandleJoinPlayer(PacketHeader header, const char* body) = 0;
    virtual void HandleLeavePlayer(PacketHeader header, const char* body) = 0;
    virtual void HandleChatMessage(PacketHeader header, const char* body) = 0;
};