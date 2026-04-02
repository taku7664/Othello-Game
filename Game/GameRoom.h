#pragma once
#include "IGameRoom.h"

class Player;

class GameRoom : public IGameRoom
{
public:
	GameRoom();
	virtual ~GameRoom();

public:
	void Update() override;

	void Clear();

public:
	void SetRoomTitle(const char* title) override;
	const std::string& GetRoomTitle() const override;

	void SetMaxPlayerCount(size_t count) override;
	size_t GetMaxPlayerCount() const override;

	void SetRoomState(RoomState state);
	RoomState GetRoomState() const override;

	bool CanStartGame() const override;

	size_t GetCurrentPlayerCount() const override;
	IPlayer* GetPlayerFromIndex(size_t index) const override;
	IPlayer* GetPlayerFromId(int id) const override;
	IPlayer* GetPlayerFromGuid(GUID guid) const override;
	IPlayer* GetLocalPlayer() const override;
	IPlayer* GetHostPlayer() const override;

	IGameBoard& GetGameBoard() override;

public:
	IPlayer* AddPlayer(const PlayerDesc& data) override;
	void RemovePlayer(GUID guid) override;

private:
	void UpdateRoomTitle();
	const char* StringToCurrentRoomState();
	
protected:
	std::string		m_roomTitle;
	RoomState		m_roomState;
	size_t          m_maxPlayerCount;

	Player* m_hostPlayer;
	Player* m_localPlayer;
	std::vector<std::unique_ptr<Player>> m_players;

	CGameBoard m_gameBoard;
};