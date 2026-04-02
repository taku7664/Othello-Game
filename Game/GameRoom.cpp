#include "pch.h"
#include "GameRoom.h"

GameRoom::GameRoom()
	: m_roomTitle()
	, m_roomState(ROOM_STATE_WAITING)
	, m_maxPlayerCount(2)
	, m_hostPlayer(nullptr)
	, m_gameBoard(8,8)
{
}

GameRoom::~GameRoom()
{
}

void GameRoom::Update()
{
	for(auto& player : m_players)
	{
		player->Update();
	}
}

void GameRoom::Clear()
{
	m_roomTitle = "";
	m_roomState = ROOM_STATE_NONE;
	m_maxPlayerCount = 2;
	m_hostPlayer = nullptr;
	m_localPlayer = nullptr;
	m_players.clear();
}

void GameRoom::SetRoomState(RoomState state)
{
	m_roomState = state;
	UpdateRoomTitle();
}

RoomState GameRoom::GetRoomState() const
{
	return m_roomState;
}

bool GameRoom::CanStartGame() const
{
	size_t playerCount = m_players.size();
	if (playerCount < 2 && playerCount > m_maxPlayerCount)
	{
		return false;
	}
	
	bool existColor[(size_t)ColorType::Count] = { false, };
	for(auto& player : m_players)
	{
		ColorType color = player->GetColorType();
		existColor[(size_t)color] = true;
	}
	size_t colorCount = 0;
	for(bool exist : existColor)
	{
		colorCount += exist ? 1 : 0;
	}

	if(colorCount < 2)
	{
		return false;
	}

	return true;
}

void GameRoom::SetMaxPlayerCount(size_t count)
{
	m_maxPlayerCount = count;
}

size_t GameRoom::GetMaxPlayerCount() const
{
	return m_maxPlayerCount;
}

size_t GameRoom::GetCurrentPlayerCount() const
{
	return m_players.size();
}

IPlayer* GameRoom::GetPlayerFromIndex(size_t index) const
{
	if (m_players.size() > index)
	{
		return m_players[index].get();
	}
	return nullptr;
}

IPlayer* GameRoom::GetPlayerFromId(int id) const
{
	for(auto& player : m_players)
	{
		if(player->GetConnectionID() == id)
		{
			return player.get();
		}
	}
	return nullptr;
}

IPlayer* GameRoom::GetPlayerFromGuid(GUID guid) const
{
	for (auto& player : m_players)
	{
		if (player->GetGUID() == guid)
		{
			return player.get();
		}
	}
	return nullptr;
}

IPlayer* GameRoom::GetLocalPlayer() const
{
	return m_localPlayer;
}

IPlayer* GameRoom::GetHostPlayer() const
{
	return nullptr;
}

IGameBoard& GameRoom::GetGameBoard()
{
	return m_gameBoard;
}

void GameRoom::SetRoomTitle(const char* title)
{
	m_roomTitle = title;
	UpdateRoomTitle();
}

const std::string& GameRoom::GetRoomTitle() const
{
	return m_roomTitle;
}

void GameRoom::UpdateRoomTitle()
{
	std::string str = std::format(
		"오셀로 게임 [방 이름: {}] [방 상태: {}]" ,
		m_roomTitle ,
		StringToCurrentRoomState()
	);
	GameCore::Renderer.SetWindowTitle(Utillity::CharToWString(str.c_str()));
	if (GameCore::ClientServer)
	{
		// TODO: 서버에 알리기
	}
}

IPlayer* GameRoom::AddPlayer(const PlayerDesc& data)
{
	Debug::Log log("GameRoom::AddPlayer()");
	if (GetPlayerFromGuid(data.Guid))
	{
		log.WriteLine( Debug::LOG_WARNING , "이미 방에 존재하는 플레이어의 GUID입니다." );
		return nullptr;
	}
	m_players.push_back(std::make_unique<Player>(data));
	if (data.IsHost)
	{
		m_hostPlayer = m_players.back().get();

	}
	if(data.IsLocal)
	{
		m_localPlayer = m_players.back().get();
	}
	return m_players.back().get();
}

void GameRoom::RemovePlayer(GUID guid)
{
	auto iter = std::find_if(
		m_players.begin(), 
		m_players.end(), 
		[guid](const std::unique_ptr<Player>& player) {
		return player->GetGUID() == guid;
	});
	if (iter != m_players.end())
	{
		m_players.erase(iter);
	}
}

const char* GameRoom::StringToCurrentRoomState()
{
	switch (m_roomState)
	{
	case ROOM_STATE_NONE:
		return " ";
		break;
	case ROOM_STATE_WAITING:
		return "대기 중";
		break;
	case ROOM_STATE_GAME_PLAYING:
		return "게임 중";
		break;
	case ROOM_STATE_GAME_FINISH:
		return "게임 끝";
		break;
	default:
		break;
	}
	return nullptr;
}