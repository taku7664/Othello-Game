#pragma once
#include "INetworkEventHandler.h"

class CNetwork
{
public:
	CNetwork(unsigned short port);
	virtual ~CNetwork() = default;

public:
	virtual bool Initialize() = 0;
	virtual void Finalize() = 0;
	virtual void Update() = 0;
	virtual bool HeartBeat() = 0;

	unsigned short GetPort() const;

protected:
	unsigned short	m_port;
	float			m_heartBeatTick;
	float			m_heartBeatTimer;
	float			m_timeoutTime;
};