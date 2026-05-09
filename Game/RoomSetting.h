#pragma once
struct RoomSetting
{
	int		MaxPlayerCount = 2;
	int     MaxCycle = 0;
	int		Row = 8;
	int		Col = 8;
	int		Timer = 30;

	bool operator == ( const RoomSetting& other ) const
	{
		return MaxPlayerCount == other.MaxPlayerCount &&
			MaxCycle == other.MaxCycle &&
			Row == other.Row && 
			Col == other.Col &&
			Timer == other.Timer;
	}

	bool operator != ( const RoomSetting& other ) const
	{
		return !operator==( other );
	}
};
