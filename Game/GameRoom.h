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
	bool GetPlayerIndexFromGuid(GUID guid, size_t& outIndex) const override;
	IPlayer* GetLocalPlayer() const override;
	IPlayer* GetHostPlayer() const override;

	IGameBoard& GetGameBoard() override;
	GUID GetCurrentTurnGuid() const override;
	ColorType GetCurrentTurnColor() const override;
	ColorType GetWinnerColor() const override;
	GameFinishReason GetFinishReason() const override;
	size_t GetBlackStoneCount() const override;
	size_t GetWhiteStoneCount() const override;
	size_t GetMoveCount() const override;
	size_t GetCycleCount() const override;
	float GetTurnRemainTime() const override;
	const std::string& GetCellMoveInfo(size_t row, size_t col) const override;

public:
	void StartGame() override;
	void CancelGame() override;
	void ApplyGameStartedPacket(const Packet::S2C_GameStarted& packet);
	void ApplyGameStatusPacket(const Packet::S2C_GameStatus& packet);
	void ApplyPlaceStonePacket(const Packet::S2C_PlaceStone& packet);
	void ApplyMoveInfoPacket(const Packet::S2C_PlaceStone& packet);
	bool TryPlaceStone(GUID guid, size_t row, size_t col, std::vector<Packet::CellChange>& outChanges);
	bool TrySurrender(GUID guid);
	bool TryFinishGameByPlayerLeft(GUID guid);
	void OpenUndoVotePopup(GUID proposer);
	bool TryUndoLastMove(std::vector<Packet::CellChange>& outChanges);
	void FillPlaceStoneStatus(Packet::S2C_PlaceStone& packet) const;
	void FillGameStatus(Packet::S2C_GameStatus& packet) const;

	IPlayer* AddPlayer(const PlayerDesc& data) override;
	void RemovePlayer(GUID guid) override;
	bool MovePlayerToIndex(GUID guid, size_t newIndex) override;

private:
	void InitializeGame();
	void BroadcastGameStarted();
	void BroadcastGameStatus(bool force = false);
	void ApplyStatus(RoomState state, ColorType currentTurn, ColorType winner, GameFinishReason reason,
		size_t moveCount, size_t cycleCount, size_t blackCount, size_t whiteCount, float turnRemainTime);
	void TryOpenGameResultPopup();
	void OpenGameResultPopup();
	bool CollectFlippedCells(ColorType color, size_t row, size_t col, std::vector<Packet::CellChange>& outFlips) const;
	bool HasLegalMove(ColorType color) const;
	bool HasLegalMoveForPlayer(const IPlayer& player) const;
	bool IsBoardFull() const;
	void CountStones();
	void ResetTurnTimer();
	void AdvanceTurnAfterAction();
	Player* GetCurrentTurnPlayer() const;
	Player* GetNextTurnPlayerFrom(GUID guid, bool* wrapped = nullptr) const;
	void FinishGame(GameFinishReason reason);
	void CancelGameConfirmed();
	ColorType GetOpponentColor(ColorType color) const;
	const char* GameFinishReasonToString(GameFinishReason reason) const;
	void UpdateRoomTitle();

	void OpenVotePopup(RoomState requestState);
	void ShowVotePopup( IImPopupWindow& wnd, RoomState requestState );
	void ShowUndoVotePopup(IImPopupWindow& wnd, GUID proposer);

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

	struct MoveHistory
	{
		GUID PrevTurnGuid = GUID_NULL;
		ColorType PrevTurnColor = ColorType::None;
		size_t PrevMoveCount = 0;
		size_t PrevCycleCount = 0;
		std::vector<Packet::CellChange> PreviousCells;
		std::vector<std::string> PreviousMoveInfos;
	};

	float m_voteTimer;
	GUID m_currentTurnGuid;
	ColorType m_currentTurn;
	ColorType m_winnerColor;
	GameFinishReason m_finishReason;
	size_t m_moveCount;
	size_t m_cycleCount;
	size_t m_blackStoneCount;
	size_t m_whiteStoneCount;
	float m_turnRemainTime;
	float m_statusBroadcastTimer;
	bool m_gameResultPopupOpened;
	std::vector<MoveHistory> m_moveHistory;
	std::vector<std::string> m_cellMoveInfos;
	inline static const std::string EmptyMoveInfo = "";
	inline static float m_voteTime = 10.0f;
};
