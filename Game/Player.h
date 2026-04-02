#pragma once
#include "IPlayer.h"

class Player : public IPlayer
{
	enum RefreshFlag 	
	{
		REFRESH_FLAG_NONE = 0,
		REFRESH_FLAG_NICKNAME = 1 << 0,
		REFRESH_FLAG_COLOR = 1 << 1 ,
	};
public:
	Player();
	Player(const PlayerDesc& data);
	virtual ~Player();

public:
	// IPlayer을(를) 통해 상속됨
	void Update() override;

	void SendChatMessage( const char* message ) override;
	void RefreshFromPacket(const Packet::Com_PlayerRefreshed& packet);

public:
	int  GetConnectionID() const override;
	GUID GetGUID() const override;

	bool IsHost() const override;
	bool IsLocal() const override;

	void SetNickName(const char* nickname) override;
	const std::string& GetNickName() const override;

	void SetColorType(ColorType color) override;
	ColorType GetColorType() const override;

private:
	int					m_connectionId;
	GUID				m_guid;
	const bool			m_bIsHost;
	const bool			m_bIsLocal;

	std::string			m_nickname;
	ColorType			m_colorType;

	BitFlag				m_refreshFlags;
};