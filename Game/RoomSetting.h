#pragma once
struct RoomSetting
{
	size_t		MaxPlayerCount = 2;
	size_t		Row = 8;
	size_t		Col = 8;

	bool operator == ( const RoomSetting& other )
	{
		return MaxPlayerCount == other.MaxPlayerCount &&
			Row == other.Row && Col == other.Col;
	}

	bool operator !=  ( const RoomSetting& other )
	{
		return !operator==( other );
	}
};
