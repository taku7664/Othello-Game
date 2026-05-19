
enum class VoteState
{
	None,
	Accepted,
	Rejected,
};

enum class GameFinishReason
{
	None,
	BoardFull,
	NoLegalMove,
	MaxCycle,
	Surrender,
	PlayerLeft,
};

struct PlayerProfile
{
	const GUID	Guid = GenerateGUID();
	std::string	Nickname;

	PlayerProfile() = default;
	PlayerProfile(const PlayerProfile& other) = default;
	PlayerProfile& operator=(const PlayerProfile& other)
	{
		if (this != &other)
		{
			Nickname = other.Nickname;
		}
		return *this;
	}
};

struct PlayerDesc
{
	int			ConnectionID = 0;
	GUID		Guid = {};
	std::string	Nickname;
	ColorType	Color = ColorType::None;
	bool		IsHost = false;
	bool		IsLocal = false;
};
