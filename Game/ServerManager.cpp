#include "pch.h"
#include "ServerManager.h"

ServerManager::ServerManager()
{
}

ServerManager::~ServerManager()
{
	if (m_hostNetwork)
	{
		m_hostNetwork->Finalize();
	}
	if (m_clientNetwork)
	{
		m_clientNetwork->Finalize();
	}
}

unsigned short ServerManager::GetDefaultPort()
{
	return 7070;
}

int ServerManager::GetNewServerInstanceID()
{
	return ++SIID;
}

void ServerManager::Update()
{
	if (m_hostNetwork)
	{
		m_hostNetwork->Update();
	}
	if (m_clientNetwork)
	{
		m_clientNetwork->Update();
	}
}

bool ServerManager::IsRunningServer() const
{
	if (m_hostNetwork || m_clientNetwork)
	{
		return true;
	}
	return false;
}

bool ServerManager::StartHostServer(unsigned short port)
{
	Debug::Log log("ServerManager::StartHostServer()");

	if (m_hostNetwork)
	{
		log.WriteLine(Debug::LOG_INFO, "already running host server.");
		return false;
	}
	m_hostNetwork = std::make_unique<CHostNetwork>(port);
	if (m_hostNetwork->Initialize())
	{
		GameCore::HostServer = m_hostNetwork.get();
		return true;
	}
	else
	{
		m_hostNetwork->Finalize();
		m_hostNetwork.reset();
		GameCore::HostServer = nullptr;
		return false;
	}
}

bool ServerManager::StartClientServer(const char* hostIP, unsigned short port)
{
	Debug::Log log("ServerManager::StartClientServer()");

	if (m_clientNetwork)
	{
		log.WriteLine(Debug::LOG_INFO, "already running client server.");
		return false;
	}
	m_clientNetwork = std::make_unique<CClientNetwork>(hostIP, port);
	if (m_clientNetwork->Initialize())
	{
		GameCore::ClientServer = m_clientNetwork.get();
		return true;
	}
	else
	{
		m_clientNetwork->Finalize();
		m_clientNetwork.reset();
		GameCore::ClientServer = nullptr;
		return false;
	}
}

void ServerManager::EndServer()
{
	Debug::Log log("ServerManager::EndServer()");

	if (m_hostNetwork)
	{
		m_hostNetwork->Update();
		m_hostNetwork->Finalize();
		m_hostNetwork.reset();
		GameCore::HostServer = nullptr;
	}
	if (m_clientNetwork)
	{
		m_clientNetwork->Update();
		m_clientNetwork->Finalize();
		m_clientNetwork.reset();
		GameCore::ClientServer = nullptr;
	}
}

bool ServerManager::Initialize()
{
	bool bIsSuccess = true;
	Debug::Log log("ServerManager::Initialize()");

	WSADATA wsaData{};
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
	{
		log.WriteLine(Debug::LOG_WARNING, "[WARN] WSAStartup failed with error: ", WSAGetLastError());
		bIsSuccess = false;
	}

	return bIsSuccess;
}

void ServerManager::Finalize()
{
	bool bIsSuccess = true;
	Debug::Log log("ServerManager::Finalize()");

	EndServer();

	if (WSACleanup() != NO_ERROR)
	{
		log.WriteLine(Debug::LOG_WARNING, "[WARN] WSACleanup failed with error: ", WSAGetLastError());
		bIsSuccess = false;
	}
}

void ServerManager::AddEventHandler(INetworkEventHandler* handler)
{
	for (auto& h : m_subscriberHandlers)
	{
		if (nullptr == h)
		{
			h = handler;
			return;
		}
	}
	m_subscriberHandlers.push_back(handler);
}

void ServerManager::RemoveEventHandler(INetworkEventHandler* handler)
{
	for (auto& h : m_subscriberHandlers)
	{
		h = nullptr;
	}
}