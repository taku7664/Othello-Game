namespace Packet
{
	////////////////////////
	/// Client to Server
	////////////////////////
	struct C2S_JoinRequest
	{
		GUID Guid			= {};
		char Nickname[64]	= { '\0', };
		bool IsHost			= false;
	};
	struct C2S_LeaveRequest
	{
		GUID Guid = {};
	};
	struct C2S_PlaceStone
	{
		GUID Guid = {};
		size_t Row = 0;
		size_t Col = 0;
	};
	struct C2S_Surrender
	{
		GUID Guid = {};
	};
	struct C2S_UndoRequest
	{
		GUID Guid = {};
	};

	////////////////////////
	/// Server to Client
	////////////////////////
	struct S2C_PlayerJoined
	{
		int  ConnectionID = 0;
		GUID Guid = {};
		char Nickname[64] = { '\0', };
		ColorType Color = ColorType::None;
		bool IsHost = false;
		bool IsNew = true;
	};
	struct S2C_PlayerDisConnected
	{
		GUID Guid = {};
		char MainCause[64] = { '\0', };
		char SubCause[64] = { '\0', };
	};
	struct S2C_PlayerOrderChanged
	{
		GUID Guid = {};
		size_t NewIndex = 0;
	};
	struct S2C_GameStateRequest
	{
		GUID From = GUID_NULL;
		RoomState State = RoomState::ROOM_STATE_NONE;
	};
	struct S2C_UndoRequest
	{
		GUID From = GUID_NULL;
	};
	struct CellChange
	{
		size_t Row = 0;
		size_t Col = 0;
		ColorType Color = ColorType::None;
	};
	struct S2C_GameStarted
	{
		size_t Rows = 0;
		size_t Cols = 0;
		GUID CurrentTurnGuid = GUID_NULL;
		ColorType CurrentTurn = ColorType::Black;
		RoomState State = RoomState::ROOM_STATE_GAME_PLAYING;
		ColorType Winner = ColorType::None;
		GameFinishReason FinishReason = GameFinishReason::None;
		size_t MoveCount = 0;
		size_t CycleCount = 0;
		size_t BlackCount = 0;
		size_t WhiteCount = 0;
		float TurnRemainTime = 0.0f;
		size_t CellCount = 0;
	};
	struct S2C_PlaceStone
	{
		GUID Guid = {};
		size_t Row = 0;
		size_t Col = 0;
		GUID CurrentTurnGuid = GUID_NULL;
		ColorType CurrentTurn = ColorType::Black;
		RoomState State = RoomState::ROOM_STATE_GAME_PLAYING;
		ColorType Winner = ColorType::None;
		GameFinishReason FinishReason = GameFinishReason::None;
		size_t MoveCount = 0;
		size_t CycleCount = 0;
		size_t BlackCount = 0;
		size_t WhiteCount = 0;
		float TurnRemainTime = 0.0f;
		size_t ChangedCount = 0;
	};
	struct S2C_GameStatus
	{
		GUID CurrentTurnGuid = GUID_NULL;
		ColorType CurrentTurn = ColorType::Black;
		RoomState State = RoomState::ROOM_STATE_GAME_PLAYING;
		ColorType Winner = ColorType::None;
		GameFinishReason FinishReason = GameFinishReason::None;
		size_t MoveCount = 0;
		size_t CycleCount = 0;
		size_t BlackCount = 0;
		size_t WhiteCount = 0;
		float TurnRemainTime = 0.0f;
	};

	////////////////////////
	/// Common
	////////////////////////
	struct Com_HeartBeat
	{
	};
	struct Com_Error
	{
		char ErrTitle[64] = { '\0', };
	};
	struct Com_ChatMessage
	{
		GUID FromGuid = GUID_NULL;
	};
	struct Com_PlayerRefreshed
	{
		GUID DestGuid = GUID_NULL;
		BitFlag RefreshFlags = 0;
		char Nickname[64] = { '\0', };
		ColorType Color = ColorType::None;
		VoteState Vote = VoteState::None;
	};
	struct Com_RoomRefreshed
	{
		bool        IsNew = false;
		BitFlag		RefreshFlags = 0;
		char		Title[64] = { '\0', };
		RoomSetting	Setting = {};
		RoomState	State = RoomState::ROOM_STATE_NONE;
	};
}

