#pragma once

class ServerManager
{
public:
	ServerManager();
	~ServerManager();
	ServerManager(const ServerManager&) = delete;
	ServerManager& operator=(const ServerManager&) = delete;

public:
	/// <summary>
	/// 기본 포트 번호를 반환합니다.
	/// </summary>
	unsigned short GetDefaultPort();
	int  GetNewServerInstanceID();

	bool Initialize();
	void Finalize();

	/// <summary>
	/// 서버를 업데이트합니다. 클라이언트로부터 패킷이 도착한 경우 패킷을 처리합니다.
	/// </summary>
	void Update();

	bool IsRunningServer() const;
	bool StartHostServer(unsigned short port);
	bool StartClientServer(const char* hostIP, unsigned short port);
	void EndServer();

	void AddEventHandler(INetworkEventHandler* handler);
	void RemoveEventHandler(INetworkEventHandler* handler);

private:
	int SIID = 0;
	std::unique_ptr<CHostNetwork> m_hostNetwork;
	std::unique_ptr<CClientNetwork> m_clientNetwork;
	std::vector<INetworkEventHandler*> m_subscriberHandlers;
};