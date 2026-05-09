#include "pch.h"
#include "GameRoom.h"

GameRoom::GameRoom()
	: m_roomTitle()
	, m_roomState(ROOM_STATE_NONE)
	, m_refreshFlags(0)
	, m_hostPlayer(nullptr)
	, m_localPlayer(nullptr)
	, m_gameBoard(8,8)
	, m_voteTimer(0.0f)
{
}

GameRoom::~GameRoom()
{
}

void GameRoom::Update()
{
	if ( m_refreshFlags )
	{
		if ( GameCore::HostServer )
		{
			Packet::Com_RoomRefreshed packet;
			packet.IsNew = false;
			packet.Setting = m_roomSetting;
			packet.State = m_roomState;
			packet.RefreshFlags = m_refreshFlags;
			strcpy_s( packet.Title ,
				m_roomTitle.length() + 1 ,
				m_roomTitle.c_str()
			);
			GameCore::HostServer->BroadCast( packet );
		}
		m_refreshFlags.Clear();
	}
	for(auto& player : m_players)
	{
		player->Update();
	}

	if ( IsReadyAllPlayers() && GameCore::HostServer )
	{

	}
}

void GameRoom::RefreshFromPacket(const Packet::Com_RoomRefreshed& packet, bool notify )
{
	if ( packet.RefreshFlags[ REFRESH_FLAG_TITLE ] )
	{
		SetRoomTitle( packet.Title, false );
		if ( notify )
		{
			std::string msg = std::format(
				"방 제목이 \"{}\"(으)로 변경되었습니다." ,
				m_roomTitle.c_str());
			GameCore::ChatManager.PushChatMessage( GUID_NULL , msg.c_str() );
		}
	}
	if ( packet.RefreshFlags[ REFRESH_FLAG_ROOM_SETTING ] )
	{
		SetRoomSetting( packet.Setting , false );
		if ( notify )
		{
			std::string msg = "방 설정이 변경되었습니다.";
			GameCore::ChatManager.PushChatMessage( GUID_NULL , msg.c_str() );
		}
	}
	if ( packet.RefreshFlags[ REFRESH_FLAG_ROOM_STATE ] )
	{
		SetRoomState( packet.State , false );
		if ( notify )
		{
			std::string msg = "방 상태가 변경되었습니다.";
			GameCore::ChatManager.PushChatMessage( GUID_NULL , msg.c_str() );
		}
	}
}

void GameRoom::Clear()
{
	m_roomTitle = "";
	m_roomState = ROOM_STATE_NONE;
	m_hostPlayer = nullptr;
	m_localPlayer = nullptr;
	m_players.clear();
	m_gameBoard.Clear();
}

void GameRoom::SetRoomTitle(const char* title, bool dirty)
{
	if ( m_roomTitle != title )
	{
		m_roomTitle = title;
		UpdateRoomTitle();
		m_refreshFlags += dirty ? REFRESH_FLAG_TITLE : REFRESH_FLAG_NONE;
	}
}

const std::string& GameRoom::GetRoomTitle() const
{
	return m_roomTitle;
}

void GameRoom::SetRoomSetting(const RoomSetting& setting, bool dirty)
{
	if ( m_roomSetting != setting )
	{
		m_roomSetting = setting;
		m_refreshFlags += dirty ? REFRESH_FLAG_ROOM_SETTING : REFRESH_FLAG_NONE;
	}
}

const RoomSetting& GameRoom::GetRoomSetting() const
{
	return m_roomSetting;
}

void GameRoom::SetRoomState(RoomState state, bool dirty)
{
	if ( m_roomState != state )
	{
		m_roomState = state;
		UpdateRoomTitle();
		m_refreshFlags += dirty ? REFRESH_FLAG_ROOM_STATE : REFRESH_FLAG_NONE;
	}
}

RoomState GameRoom::GetRoomState() const
{
	return m_roomState;
}

bool GameRoom::CanStartGame() const
{
	const size_t playerCount = m_players.size();
	const size_t maxPlayerCount = m_roomSetting.MaxPlayerCount;
	if (playerCount < 2 && playerCount > maxPlayerCount)
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
	for(size_t i = 1; i < (size_t)ColorType::Count; ++i)
	{	// None 제외
		const bool exist = existColor[i];
		colorCount += exist ? 1 : 0;
	}
	if(colorCount < 2)
	{
		return false;
	}

	return true;
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
	return m_hostPlayer;
}

IGameBoard& GameRoom::GetGameBoard()
{
	return m_gameBoard;
}

void GameRoom::UpdateRoomTitle()
{
	std::string str = std::format(
		"오셀로 게임 [방 이름: {}] [방 상태: {}]" ,
		m_roomTitle ,
		StringToCurrentRoomState()
	);
	GameCore::Renderer.SetWindowTitle(Utillity::CharToWString(str.c_str()));
}

void GameRoom::StartGame()
{
	ImPopupDesc desc {
		.Title = "게임 시작 요청",
		.OnRenderEnterFunc = [ this ] ( IImPopupWindow& wnd ) { 
			m_voteTimer = m_voteTime; 
			m_localPlayer->SetReady( false );
		},
		.OnRenderStayFunc = [ this ] ( IImPopupWindow& wnd ) { 
			m_voteTimer -= ImGui::GetIO().DeltaTime;
			ShowVotePopup( wnd ); 
		},
		.OnRenderExitFunc = [ this ] ( IImPopupWindow& wnd ) {
			m_voteTimer = 0.0f;
		},
	};
	GameCore::ImGuiManager.OpenPopup( desc );
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

void GameRoom::InitializeGame()
{
	if ( m_roomState == ROOM_STATE_WAITING )
	{
		// TODO: 세팅 기반 보드 만들기
		SetRoomState( ROOM_STATE_GAME_PLAYING );
	}
}

void GameRoom::ShowVotePopup( IImPopupWindow& wnd )
{
	ImGui::PushID( this );
	// Start Game
	{
		size_t			readyCount = 0;
		const size_t	playerCount = m_players.size();
		const int		timer = static_cast<int>( m_voteTimer );
		const auto		drawLine = [ ] ( bool isReady , const char* nickname ) {
			ImGui::Utillity::DisableScope disiable; {
				ImGui::Utillity::Checkbox( "##is_ready" , &isReady );
			}
			ImGui::SameLine();
			ImGui::TextUnformatted( nickname );
			};

		std::string inner = std::format( "게임을 시작하시겠습니까? ({})" ,
			timer );
		ImGui::TextUnformatted( inner.c_str() );

		const float height		= ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.x;
		const int	childFlags	= ImGuiChildFlags_Borders;
		const int	windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
		ImGui::BeginChild( "##players" , ImVec2( 0 , height * static_cast<float>(playerCount) ) , childFlags, windowFlags );
		for ( auto& player : m_players )
		{
			const ImVec4 textColor = ImGui::Utillity::ColorFromGuid( player->GetGUID() );
			ImGui::Utillity::StyleBuilder styleBuilder;
			styleBuilder.PushStyleColor( ImGuiCol_Text , textColor );
			drawLine( player->IsReady() , player->GetNickName().c_str() );
			readyCount += player->IsReady() ? 1 : 0;
		}
		ImGui::EndChild();
		
		{
			ImGui::Utillity::DisableScope disiable( m_localPlayer->IsReady() );
			if ( ImGui::Button( "수락" ) )
			{
				m_localPlayer->SetReady( true );
			}
			ImGui::SameLine();
			if ( ImGui::Button( "거절" ) )
			{
				wnd.Close();
			}
		}

		if ( readyCount == playerCount )
		{
			InitializeGame();
			wnd.Close();
		}
		if ( m_voteTimer < 0.0f )
		{
			wnd.Close();
		}
	}
	ImGui::PopID();
}

bool GameRoom::IsReadyAllPlayers()
{
	for ( auto& player : m_players )
	{
		if ( false == player->IsReady() )
		{
			return false;
		}
	}
	return true;
}

const char* GameRoom::StringToCurrentRoomState()
{
	switch ( m_roomState )
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