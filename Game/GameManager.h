#pragma once
#include "INetworkEventHandler.h"

class GameRoom;
class IGameRoom;

class CGameManager
{
public:
	CGameManager();
	~CGameManager() = default;
	CGameManager(const CGameManager&) = delete;
	CGameManager& operator=(const CGameManager&) = delete;

public:
	void Initialize();
	void Finalize();
	void Update();

public:
	bool MakeRoom(const PlayerProfile& profile, const char* title, unsigned short port);
	bool JoinRoom(const PlayerProfile& profile, const char* hostIP, unsigned short port);
	void LeaveRoom(const char* errTitle = nullptr, const char* errMessage = nullptr);

	inline GUID GetGUID() const { return m_playerProfile.Guid; }

private:
	PlayerProfile				m_playerProfile;
	GameRoom					m_gameRoom;
};

