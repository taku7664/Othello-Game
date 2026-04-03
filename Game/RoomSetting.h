#pragma once
struct RoomSetting
{
	int		MaxPlayerCount = 2;
	int     MaxCycle = 0;
	int		Row = 8;
	int		Col = 8;

	bool operator == ( const RoomSetting& other )
	{
		return MaxPlayerCount == other.MaxPlayerCount &&
			MaxCycle == other.MaxCycle &&
			Row == other.Row && 
			Col == other.Col;
	}

	bool operator != ( const RoomSetting& other )
	{
		return !operator==( other );
	}
};
