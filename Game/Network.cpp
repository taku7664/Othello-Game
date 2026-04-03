#include "pch.h"
#include "Network.h"

CNetwork::CNetwork(unsigned short port)
	: m_port(port)
	, m_heartBeatTick(3.0f)
	, m_heartBeatTimer(0.0f)
	, m_timeoutTime(10.0f)
{
}

unsigned short CNetwork::GetPort() const
{
	return m_port;
}