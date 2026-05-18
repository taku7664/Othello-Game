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
		ColorType Color = ColorType::None;
	};

	////////////////////////
	/// Server to Client
	////////////////////////
	struct S2C_PlayerJoined
	{
		int  ConnectionID = 0;
		GUID Guid = {};
		char Nickname[64] = { '\0', };
		bool IsHost = false;
		bool IsNew = true;
	};
	struct S2C_PlayerDisConnected
	{
		GUID Guid = {};
		char MainCause[64] = { '\0', };
		char SubCause[64] = { '\0', };
	};
	struct S2C_GameStateRequest
	{
		GUID From = GUID_NULL;
		RoomState State = RoomState::ROOM_STATE_NONE;
	};
	struct S2C_PlaceStone
	{
		GUID Guid = {};
		size_t Row = 0;
		size_t Col = 0;
		ColorType Color = ColorType::None;
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

