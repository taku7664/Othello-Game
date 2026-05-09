#pragma once
#include "IGameRoom.h"

class Player;


class GameRoom : public IGameRoom
{
	enum RefreshFlag
	{
		REFRESH_FLAG_NONE = 0 ,
		REFRESH_FLAG_TITLE = 1 << 0 ,
		REFRESH_FLAG_ROOM_SETTING = 1 << 1,
		REFRESH_FLAG_ROOM_STATE = 1 << 2,
	};
public:
	GameRoom();
	virtual ~GameRoom();

public:
	void Update() override;
	void RefreshFromPacket(const Packet::Com_RoomRefreshed& packet, bool notify = true);

	void Clear();

public:
	void SetRoomTitle(const char* title, bool dirty = true) override;
	const std::string& GetRoomTitle() const override;

	void SetRoomSetting( const RoomSetting& setting , bool dirty = true ) override;
	const RoomSetting& GetRoomSetting() const override;

	void SetRoomState(RoomState state , bool dirty = true );
	RoomState GetRoomState() const override;

	bool CanStartGame() const override;

	size_t   GetCurrentPlayerCount() const override;
	IPlayer* GetPlayerFromIndex(size_t index) const override;
	IPlayer* GetPlayerFromId(int id) const override;
	IPlayer* GetPlayerFromGuid(GUID guid) const override;
	IPlayer* GetLocalPlayer() const override;
	IPlayer* GetHostPlayer() const override;

	IGameBoard& GetGameBoard() override;

public:
	void StartGame() override;

	IPlayer* AddPlayer(const PlayerDesc& data) override;
	void RemovePlayer(GUID guid) override;

private:
	void InitializeGame();
	void UpdateRoomTitle();

	void ShowVotePopup( IImPopupWindow& wnd );

	bool IsReadyAllPlayers();
	const char* StringToCurrentRoomState();
	
protected:
	std::string		m_roomTitle;
	RoomSetting		m_roomSetting;
	RoomState		m_roomState;
	BitFlag			m_refreshFlags = 0;


	Player* m_hostPlayer;
	Player* m_localPlayer;
	std::vector<std::unique_ptr<Player>> m_players;

	CGameBoard m_gameBoard;

	float m_voteTimer;
	inline static float m_voteTime = 10.0f;
};