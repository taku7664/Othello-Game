namespace Packet
{
	////////////////////////
	/// Client to Server
	////////////////////////
	struct C2S_JoinRequest
	{
		GUID Guid			= {};
		char Nickname[64]	= { '0', };
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
		char Nickname[64] = { '0', };
		bool IsHost = false;
		bool IsNew = true;
	};
	struct S2C_PlayerLeaved
	{
		GUID Guid = {};
	};
	struct S2C_PlayerKicked
	{
		GUID Guid = {};
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
	struct Com_Error
	{
		char errTitle[64] = { '0', };
	};
	struct Com_ChatMessage
	{
		GUID FromGuid = GUID_NULL;
	};
	struct Com_PlayerRefreshed
	{
		GUID DestGuid = GUID_NULL;
		BitFlag RefreshFlags = 0;
		char Nickname[64] = { '0', };
		ColorType Color = ColorType::None;
	};
	struct Com_RoomRefreshed
	{
		bool        IsNew = false;
		BitFlag		RefreshFlags = 0;
		char		Title[64] = { '0', };
		RoomSetting	Setting = {};
		RoomState	State = RoomState::ROOM_STATE_NONE;
	};
}

