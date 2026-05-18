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
	, m_currentTurn(ColorType::Black)
	, m_winnerColor(ColorType::None)
	, m_finishReason(GameFinishReason::None)
	, m_moveCount(0)
	, m_cycleCount(0)
	, m_blackStoneCount(0)
	, m_whiteStoneCount(0)
	, m_turnRemainTime(0.0f)
	, m_statusBroadcastTimer(0.0f)
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

	if ( GameCore::HostServer && m_roomState == ROOM_STATE_GAME_PLAYING )
	{
		const int timer = m_roomSetting.Timer;
		if ( timer > 0 )
		{
			const float deltaTime = GameCore::Time.GetUnScaledDeltaTime();
			m_turnRemainTime -= deltaTime;
			m_statusBroadcastTimer += deltaTime;
			if ( m_turnRemainTime <= 0.0f )
			{
				m_turnRemainTime = 0.0f;
				AdvanceTurnAfterAction();
				BroadcastGameStatus( true );
			}
			else if ( m_statusBroadcastTimer >= 0.25f )
			{
				BroadcastGameStatus();
			}
		}
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
	m_refreshFlags.Clear();
	m_hostPlayer = nullptr;
	m_localPlayer = nullptr;
	m_players.clear();
	m_gameBoard.Clear();
	m_voteTimer = 0.0f;
	m_currentTurn = ColorType::Black;
	m_winnerColor = ColorType::None;
	m_finishReason = GameFinishReason::None;
	m_moveCount = 0;
	m_cycleCount = 0;
	m_blackStoneCount = 0;
	m_whiteStoneCount = 0;
	m_turnRemainTime = 0.0f;
	m_statusBroadcastTimer = 0.0f;
}

void GameRoom::SetRoomTitle(const char* title, bool dirty)
{
	if ( nullptr == title )
	{
		title = "";
	}

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
	const size_t maxPlayerCount = static_cast<size_t>( m_roomSetting.MaxPlayerCount );
	if ( playerCount < 2 || playerCount > maxPlayerCount )
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

ColorType GameRoom::GetCurrentTurnColor() const
{
	return m_currentTurn;
}

ColorType GameRoom::GetWinnerColor() const
{
	return m_winnerColor;
}

GameFinishReason GameRoom::GetFinishReason() const
{
	return m_finishReason;
}

size_t GameRoom::GetBlackStoneCount() const
{
	return m_blackStoneCount;
}

size_t GameRoom::GetWhiteStoneCount() const
{
	return m_whiteStoneCount;
}

size_t GameRoom::GetMoveCount() const
{
	return m_moveCount;
}

size_t GameRoom::GetCycleCount() const
{
	return m_cycleCount;
}

float GameRoom::GetTurnRemainTime() const
{
	return m_turnRemainTime;
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
	OpenVotePopup( ROOM_STATE_GAME_PLAYING );
}

void GameRoom::CancelGame()
{
	OpenVotePopup( ROOM_STATE_WAITING );
}

void GameRoom::OpenVotePopup( RoomState requestState )
{
	if ( nullptr == m_localPlayer )
	{
		return;
	}

	const char* title = requestState == ROOM_STATE_WAITING ? "게임 취소 요청" : "게임 시작 요청";
	ImPopupDesc desc {
		.Title = title,
		.OnRenderEnterFunc = [ this ] ( IImPopupWindow& wnd ) {
			m_voteTimer = m_voteTime;
			if ( m_localPlayer )
			{
				m_localPlayer->SetVoteState( VoteState::None );
			}
		},
		.OnRenderStayFunc = [ this, requestState ] ( IImPopupWindow& wnd ) {
			m_voteTimer -= ImGui::GetIO().DeltaTime;
			ShowVotePopup( wnd, requestState );
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
		Player* removedPlayer = iter->get();
		if ( m_hostPlayer == removedPlayer )
		{
			m_hostPlayer = nullptr;
		}
		if ( m_localPlayer == removedPlayer )
		{
			m_localPlayer = nullptr;
		}
		m_players.erase(iter);
	}
}

void GameRoom::InitializeGame()
{
	if ( m_roomState == ROOM_STATE_WAITING )
	{
		m_gameBoard.Resize( m_roomSetting.Row , m_roomSetting.Col );
		m_gameBoard.InitializeOthelloBoard();
		m_currentTurn = ColorType::Black;
		m_winnerColor = ColorType::None;
		m_finishReason = GameFinishReason::None;
		m_moveCount = 0;
		m_cycleCount = 0;
		CountStones();
		ResetTurnTimer();
		SetRoomState( ROOM_STATE_GAME_PLAYING );
	}
}

void GameRoom::CountStones()
{
	m_blackStoneCount = 0;
	m_whiteStoneCount = 0;
	const size_t rows = m_gameBoard.GetBoardRows();
	const size_t cols = m_gameBoard.GetBoardCols();
	for ( size_t r = 0; r < rows; ++r )
	{
		for ( size_t c = 0; c < cols; ++c )
		{
			switch ( m_gameBoard.GetCellColor( r , c ) )
			{
			case ColorType::Black:
				++m_blackStoneCount;
				break;
			case ColorType::White:
				++m_whiteStoneCount;
				break;
			default:
				break;
			}
		}
	}
}

void GameRoom::ResetTurnTimer()
{
	m_turnRemainTime = static_cast<float>( m_roomSetting.Timer );
	m_statusBroadcastTimer = 0.0f;
}

ColorType GameRoom::GetOpponentColor( ColorType color ) const
{
	if ( color == ColorType::Black )
	{
		return ColorType::White;
	}
	if ( color == ColorType::White )
	{
		return ColorType::Black;
	}
	return ColorType::None;
}

bool GameRoom::CollectFlippedCells( ColorType color , size_t row , size_t col , std::vector<Packet::CellChange>& outFlips ) const
{
	if ( color == ColorType::None || false == m_gameBoard.IsValidCoord( row , col ) || m_gameBoard.GetCellColor( row , col ) != ColorType::None )
	{
		return false;
	}

	const ColorType opponent = GetOpponentColor( color );
	if ( opponent == ColorType::None )
	{
		return false;
	}

	const int directions[ 8 ][ 2 ] = {
		{ -1, -1 }, { -1, 0 }, { -1, 1 },
		{ 0, -1 },             { 0, 1 },
		{ 1, -1 },  { 1, 0 },  { 1, 1 },
	};
	const int rows = static_cast<int>( m_gameBoard.GetBoardRows() );
	const int cols = static_cast<int>( m_gameBoard.GetBoardCols() );
	const int startRow = static_cast<int>( row );
	const int startCol = static_cast<int>( col );

	std::vector<Packet::CellChange> totalFlips;
	for ( const auto& dir : directions )
	{
		std::vector<Packet::CellChange> lineFlips;
		int r = startRow + dir[ 0 ];
		int c = startCol + dir[ 1 ];
		while ( r >= 0 && c >= 0 && r < rows && c < cols )
		{
			const ColorType cellColor = m_gameBoard.GetCellColor( static_cast<size_t>( r ) , static_cast<size_t>( c ) );
			if ( cellColor == opponent )
			{
				lineFlips.push_back( Packet::CellChange{
					.Row = static_cast<size_t>( r ),
					.Col = static_cast<size_t>( c ),
					.Color = color
				} );
			}
			else if ( cellColor == color )
			{
				if ( false == lineFlips.empty() )
				{
					totalFlips.insert( totalFlips.end() , lineFlips.begin() , lineFlips.end() );
				}
				break;
			}
			else
			{
				break;
			}

			r += dir[ 0 ];
			c += dir[ 1 ];
		}
	}

	outFlips.insert( outFlips.end() , totalFlips.begin() , totalFlips.end() );
	return false == totalFlips.empty();
}

bool GameRoom::HasLegalMove( ColorType color ) const
{
	if ( color == ColorType::None )
	{
		return false;
	}

	const size_t rows = m_gameBoard.GetBoardRows();
	const size_t cols = m_gameBoard.GetBoardCols();
	for ( size_t r = 0; r < rows; ++r )
	{
		for ( size_t c = 0; c < cols; ++c )
		{
			std::vector<Packet::CellChange> flips;
			if ( CollectFlippedCells( color , r , c , flips ) )
			{
				return true;
			}
		}
	}
	return false;
}

bool GameRoom::IsBoardFull() const
{
	const size_t cellCount = m_gameBoard.GetBoardRows() * m_gameBoard.GetBoardCols();
	return cellCount > 0 && m_gameBoard.GetStoneCount() >= cellCount;
}

void GameRoom::FinishGame( GameFinishReason reason )
{
	CountStones();
	m_finishReason = reason;
	if ( m_blackStoneCount > m_whiteStoneCount )
	{
		m_winnerColor = ColorType::Black;
	}
	else if ( m_whiteStoneCount > m_blackStoneCount )
	{
		m_winnerColor = ColorType::White;
	}
	else
	{
		m_winnerColor = ColorType::None;
	}
	SetRoomState( ROOM_STATE_GAME_FINISH );
	m_turnRemainTime = 0.0f;
}

void GameRoom::CancelGameConfirmed()
{
	m_gameBoard.Clear();
	m_currentTurn = ColorType::Black;
	m_winnerColor = ColorType::None;
	m_finishReason = GameFinishReason::None;
	m_moveCount = 0;
	m_cycleCount = 0;
	m_blackStoneCount = 0;
	m_whiteStoneCount = 0;
	m_turnRemainTime = 0.0f;
	m_statusBroadcastTimer = 0.0f;
	SetRoomState( ROOM_STATE_WAITING );
	BroadcastGameStatus( true );
}

void GameRoom::AdvanceTurnAfterAction()
{
	if ( m_roomState != ROOM_STATE_GAME_PLAYING )
	{
		return;
	}

	CountStones();
	if ( IsBoardFull() )
	{
		FinishGame( GameFinishReason::BoardFull );
		return;
	}

	const ColorType prevTurn = m_currentTurn;
	const ColorType opponent = GetOpponentColor( m_currentTurn );
	const bool opponentCanMove = HasLegalMove( opponent );
	const bool currentCanMove = HasLegalMove( m_currentTurn );
	if ( opponentCanMove )
	{
		m_currentTurn = opponent;
	}
	else if ( currentCanMove )
	{
		GameCore::ChatManager.PushChatMessage( GUID_NULL , "둘 수 있는 위치가 없어 턴을 넘깁니다." );
	}
	else
	{
		FinishGame( GameFinishReason::NoLegalMove );
		return;
	}

	if ( prevTurn == ColorType::White && m_currentTurn == ColorType::Black )
	{
		++m_cycleCount;
	}
	if ( m_roomSetting.MaxCycle > 0 && m_cycleCount >= static_cast<size_t>( m_roomSetting.MaxCycle ) )
	{
		FinishGame( GameFinishReason::MaxCycle );
		return;
	}

	ResetTurnTimer();
}

bool GameRoom::TryPlaceStone( GUID guid , size_t row , size_t col , std::vector<Packet::CellChange>& outChanges )
{
	if ( m_roomState != ROOM_STATE_GAME_PLAYING )
	{
		return false;
	}

	IPlayer* player = GetPlayerFromGuid( guid );
	if ( nullptr == player || player->GetColorType() != m_currentTurn )
	{
		return false;
	}

	std::vector<Packet::CellChange> flips;
	if ( false == CollectFlippedCells( m_currentTurn , row , col , flips ) )
	{
		return false;
	}

	outChanges.push_back( Packet::CellChange{
		.Row = row,
		.Col = col,
		.Color = m_currentTurn
	} );
	outChanges.insert( outChanges.end() , flips.begin() , flips.end() );

	for ( const Packet::CellChange& change : outChanges )
	{
		m_gameBoard.SetCellColor( change.Row , change.Col , change.Color );
	}
	++m_moveCount;
	AdvanceTurnAfterAction();
	return true;
}

bool GameRoom::TrySurrender( GUID guid )
{
	if ( m_roomState != ROOM_STATE_GAME_PLAYING )
	{
		return false;
	}

	IPlayer* player = GetPlayerFromGuid( guid );
	if ( nullptr == player )
	{
		return false;
	}

	m_finishReason = GameFinishReason::Surrender;
	m_winnerColor = GetOpponentColor( player->GetColorType() );
	CountStones();
	SetRoomState( ROOM_STATE_GAME_FINISH );
	m_turnRemainTime = 0.0f;
	return true;
}

void GameRoom::BroadcastGameStarted()
{
	if ( nullptr == GameCore::HostServer )
	{
		return;
	}

	InitializeGame();

	const size_t rows = m_gameBoard.GetBoardRows();
	const size_t cols = m_gameBoard.GetBoardCols();
	const size_t cellCount = rows * cols;
	const size_t bodySize = sizeof( Packet::S2C_GameStarted ) + sizeof( Packet::CellChange ) * cellCount;
	std::vector<char> buffer( bodySize );

	Packet::S2C_GameStarted* packet = reinterpret_cast<Packet::S2C_GameStarted*>( buffer.data() );
	packet->Rows = rows;
	packet->Cols = cols;
	packet->CurrentTurn = ColorType::Black;
	packet->State = m_roomState;
	packet->Winner = m_winnerColor;
	packet->FinishReason = m_finishReason;
	packet->MoveCount = m_moveCount;
	packet->CycleCount = m_cycleCount;
	packet->BlackCount = m_blackStoneCount;
	packet->WhiteCount = m_whiteStoneCount;
	packet->TurnRemainTime = m_turnRemainTime;
	packet->CellCount = cellCount;

	Packet::CellChange* cells = reinterpret_cast<Packet::CellChange*>( buffer.data() + sizeof( Packet::S2C_GameStarted ) );
	size_t index = 0;
	for ( size_t r = 0; r < rows; ++r )
	{
		for ( size_t c = 0; c < cols; ++c )
		{
			cells[ index++ ] = Packet::CellChange{
				.Row = r,
				.Col = c,
				.Color = m_gameBoard.GetCellColor( r , c )
			};
		}
	}

	GameCore::HostServer->BroadCast( *packet , bodySize );
}

void GameRoom::ApplyGameStartedPacket( const Packet::S2C_GameStarted& packet )
{
	m_gameBoard.Resize( packet.Rows , packet.Cols );
	m_gameBoard.Clear();

	const char* raw = reinterpret_cast<const char*>( &packet );
	const Packet::CellChange* cells = reinterpret_cast<const Packet::CellChange*>( raw + sizeof( Packet::S2C_GameStarted ) );
	for ( size_t i = 0; i < packet.CellCount; ++i )
	{
		m_gameBoard.SetCellColor( cells[ i ].Row , cells[ i ].Col , cells[ i ].Color );
	}

	ApplyStatus( packet.State , packet.CurrentTurn , packet.Winner , packet.FinishReason ,
		packet.MoveCount , packet.CycleCount , packet.BlackCount , packet.WhiteCount , packet.TurnRemainTime );
}

void GameRoom::ApplyStatus( RoomState state , ColorType currentTurn , ColorType winner , GameFinishReason reason ,
	size_t moveCount , size_t cycleCount , size_t blackCount , size_t whiteCount , float turnRemainTime )
{
	const RoomState prevState = m_roomState;
	const GameFinishReason prevReason = m_finishReason;
	m_currentTurn = currentTurn;
	m_winnerColor = winner;
	m_finishReason = reason;
	m_moveCount = moveCount;
	m_cycleCount = cycleCount;
	m_blackStoneCount = blackCount;
	m_whiteStoneCount = whiteCount;
	m_turnRemainTime = turnRemainTime;
	SetRoomState( state );
	TryOpenGameResultPopup( prevState, prevReason );
}

void GameRoom::TryOpenGameResultPopup( RoomState prevState , GameFinishReason prevReason )
{
	if ( m_roomState != ROOM_STATE_GAME_FINISH || m_finishReason != GameFinishReason::Surrender )
	{
		return;
	}
	if ( prevState == ROOM_STATE_GAME_FINISH && prevReason == GameFinishReason::Surrender )
	{
		return;
	}
	OpenGameResultPopup();
}

void GameRoom::OpenGameResultPopup()
{
	const size_t blackCount = m_blackStoneCount;
	const size_t whiteCount = m_whiteStoneCount;
	const ColorType winner = m_winnerColor;
	const auto colorName = [ ] ( ColorType color ) {
		switch ( color )
		{
		case ColorType::Black:
			return "검정";
		case ColorType::White:
			return "하양";
		default:
			return "무승부";
		}
	};
	ImPopupDesc desc {
		.Title = "게임 결과",
		.Flags = ImGuiWindowFlags_AlwaysAutoResize,
		.OnRenderStayFunc = [ blackCount, whiteCount, winner, colorName ] ( IImPopupWindow& wnd ) {
			std::string stoneText = std::format( "돌 수  검정: {} / 하양: {}", blackCount, whiteCount );
			std::string winnerText = std::format( "승자 색: {}", colorName( winner ) );
			ImGui::TextUnformatted( stoneText.c_str() );
			ImGui::TextUnformatted( winnerText.c_str() );
			ImGui::Separator();
			if ( ImGui::Button( "확인" , ImVec2( 80.0f , 0.0f ) ) )
			{
				wnd.Close();
			}
		},
	};
	GameCore::ImGuiManager.OpenPopup( desc );
}

void GameRoom::FillGameStatus( Packet::S2C_GameStatus& packet ) const
{
	packet.CurrentTurn = m_currentTurn;
	packet.State = m_roomState;
	packet.Winner = m_winnerColor;
	packet.FinishReason = m_finishReason;
	packet.MoveCount = m_moveCount;
	packet.CycleCount = m_cycleCount;
	packet.BlackCount = m_blackStoneCount;
	packet.WhiteCount = m_whiteStoneCount;
	packet.TurnRemainTime = m_turnRemainTime;
}

void GameRoom::FillPlaceStoneStatus( Packet::S2C_PlaceStone& packet ) const
{
	packet.CurrentTurn = m_currentTurn;
	packet.State = m_roomState;
	packet.Winner = m_winnerColor;
	packet.FinishReason = m_finishReason;
	packet.MoveCount = m_moveCount;
	packet.CycleCount = m_cycleCount;
	packet.BlackCount = m_blackStoneCount;
	packet.WhiteCount = m_whiteStoneCount;
	packet.TurnRemainTime = m_turnRemainTime;
}

void GameRoom::BroadcastGameStatus( bool force )
{
	if ( nullptr == GameCore::HostServer )
	{
		return;
	}
	if ( false == force && m_roomState != ROOM_STATE_GAME_PLAYING )
	{
		return;
	}

	Packet::S2C_GameStatus packet;
	FillGameStatus( packet );
	GameCore::HostServer->BroadCast( packet );
	m_statusBroadcastTimer = 0.0f;
}

void GameRoom::ApplyGameStatusPacket( const Packet::S2C_GameStatus& packet )
{
	ApplyStatus( packet.State , packet.CurrentTurn , packet.Winner , packet.FinishReason ,
		packet.MoveCount , packet.CycleCount , packet.BlackCount , packet.WhiteCount , packet.TurnRemainTime );
}

void GameRoom::ApplyPlaceStonePacket( const Packet::S2C_PlaceStone& packet )
{
	ApplyStatus( packet.State , packet.CurrentTurn , packet.Winner , packet.FinishReason ,
		packet.MoveCount , packet.CycleCount , packet.BlackCount , packet.WhiteCount , packet.TurnRemainTime );
}

void GameRoom::ShowVotePopup( IImPopupWindow& wnd, RoomState requestState )
{
	if ( nullptr == m_localPlayer )
	{
		wnd.Close();
		return;
	}

	ImGui::PushID( this );
	// Start Game
	{
		size_t			readyCount = 0;
		const size_t	playerCount = m_players.size();
		const int		timer = static_cast<int>( m_voteTimer );
		const auto		drawLine = [ ] ( Player* player ) {

			const VoteState voteState = player->GetVoteState();
			const char*		nickname = player->GetNickName().c_str();

			const ImVec4 textColor = ImGui::Utillity::ColorFromGuid( player->GetGUID() );
			ImGui::Utillity::StyleBuilder styleBuilder;
			styleBuilder.PushStyleColor( ImGuiCol_Text , textColor );

			ImGui::PushID( nickname );
			bool checked = voteState != VoteState::None;
			const auto markType = voteState == VoteState::Rejected
				? ImGui::Utillity::CheckMarkType::X
				: ImGui::Utillity::CheckMarkType::Check;
			ImGui::Utillity::DisableScope disiable; {
				ImGui::Utillity::Checkbox( "##is_ready" , &checked, markType );
			}
			ImGui::SameLine();
			ImGui::TextUnformatted( nickname );
			ImGui::PopID();
			};

		const char* actionText = requestState == ROOM_STATE_WAITING ? "취소" : "시작";
		std::string inner = std::format( "게임을 {}하시겠습니까? ({})" ,
			actionText,
			timer );
		ImGui::TextUnformatted( inner.c_str() );

		const float height		= ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.x;
		const int	childFlags	= ImGuiChildFlags_Borders;
		const int	windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
		ImGui::BeginChild( "##players" , ImVec2( 0 , height * static_cast<float>(playerCount) ) , childFlags, windowFlags );
		drawLine( m_localPlayer );
		readyCount += m_localPlayer->IsReady() ? 1 : 0;
		for ( auto& player : m_players )
		{
			if( m_localPlayer == player.get() )
				continue;

			drawLine( player.get() );
			readyCount += player->IsReady() ? 1 : 0;
		}
		ImGui::EndChild();
		
		{
			ImGui::Utillity::DisableScope disiable( m_localPlayer->IsReady() );
			if ( ImGui::Button( "수락" ) )
			{
				m_localPlayer->SetVoteState( VoteState::Accepted );
			}
			ImGui::SameLine();
			if ( ImGui::Button( "거절" ) )
			{
				m_localPlayer->SetVoteState( VoteState::Rejected );
				wnd.Close();
			}
		}

		if ( playerCount > 0 && readyCount == playerCount )
		{
			if ( GameCore::HostServer )
			{
				if ( requestState == ROOM_STATE_WAITING )
				{
					CancelGameConfirmed();
				}
				else
				{
					BroadcastGameStarted();
				}
			}
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
	if ( m_players.empty() )
	{
		return false;
	}

	for ( auto& player : m_players )
	{
		if ( false == player->IsReady() )
		{
			return false;
		}
	}
	return true;
}

const char* GameRoom::GameFinishReasonToString( GameFinishReason reason ) const
{
	switch ( reason )
	{
	case GameFinishReason::None:
		return "";
	case GameFinishReason::BoardFull:
		return "보드가 가득 찼습니다.";
	case GameFinishReason::NoLegalMove:
		return "양쪽 모두 둘 수 있는 위치가 없습니다.";
	case GameFinishReason::MaxCycle:
		return "최대 사이클에 도달했습니다.";
	case GameFinishReason::Surrender:
		return "항복했습니다.";
	default:
		break;
	}
	return "Unknown";
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
	return "Unknown";
}
