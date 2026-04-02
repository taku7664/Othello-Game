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

protected:
	unsigned short	m_port;
};