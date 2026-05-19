#include "pch.h"
#include "GameView.h"

GameView::GameView()
{
}

GameView::~GameView()
{
}

void ShowColumnPlitter( OUT float& width )
{
	const float  plitterWidth = 4.0f;

	ImGuiWindow* window = GImGui->CurrentWindow;
	if ( window )
	{
		ImRect	rect = window->Rect();
		float	columWidth = rect.Max.x - rect.Min.x;
		float	columHeight = rect.Max.y - rect.Min.y;

		ImGui::SameLine();
		ImGui::PushID( &width );
		ImGui::Button( "##Plitter" , ImVec2( plitterWidth , columHeight ) );
		if ( true == ImGui::IsItemActive() )
		{
			float padding = ImGui::GetStyle().FramePadding.x;
			float mouseX = ImGui::GetIO().MousePos.x;
			width = mouseX - rect.Min.x - ( padding * 2.0f ) - plitterWidth * 0.5f;
		}
		ImGui::PopID();
	}
}

void GameView::OnShow( Context& context )
{
	const ImVec2 displaySize = context.Size;
	static float width = displaySize.x * 0.6f;
	ImGui::BeginChild( "GameBoard" , ImVec2( width , 0 ) , ImGuiChildFlags_Borders , ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );
	{
		ShowGame();
	}
	ImGui::EndChild();
	
	ShowColumnPlitter(width);
	width = ImClamp( width , 200.0f , displaySize.x - 200.0f );
	ImGui::SameLine();
	
	ImGui::BeginChild( "Chat" , ImVec2( 0 , 0 ) , ImGuiChildFlags_Borders , ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );
	{
		ShowChat();
	}
	ImGui::EndChild();
}

void GameView::ShowGame()
{
	ImGuiStyle&  style		= ImGui::GetStyle();
	const ImVec2 availSize	= ImGui::GetContentRegionAvail();
	const ImVec2 padding	= style.WindowPadding;
	const float  height		= ImGui::GetFrameHeight();

	ImGui::BeginChild( "##UpperGameBar" , ImVec2( 0 , height + padding.y * 2.0f ) , ImGuiChildFlags_Borders , ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );
	DrawUpperGameBar();
	ImGui::EndChild();

	ImGui::BeginChild( "##MainGameBar" , ImVec2( 0 , 0 ) , ImGuiChildFlags_Borders , ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );
	DrawMainGameBar();
	ImGui::EndChild();
}



void GameView::DrawMainGameBar()
{
	if ( GameCore::ActiveRoom )
	{
		if ( RoomState::ROOM_STATE_WAITING != GameCore::ActiveRoom->GetRoomState() )
		{
			DrawGameStatus();
			size_t clickedRow = 0;
			size_t clickedCol = 0;
			IGameBoard& board = GameCore::ActiveRoom->GetGameBoard();
			IPlayer* localPlayer = GameCore::GetLocalPlayer();
			const bool canClick = localPlayer &&
				GameCore::ActiveRoom->GetRoomState() == ROOM_STATE_GAME_PLAYING &&
				localPlayer->GetGUID() == GameCore::ActiveRoom->GetCurrentTurnGuid();
			const ColorType previewColor = localPlayer ? localPlayer->GetColorType() : ColorType::None;
			if ( DrawGameBoard( board , canClick , previewColor , clickedRow , clickedCol ) )
			{
				if ( GameCore::ClientServer )
				{
					if ( localPlayer )
					{
						Packet::C2S_PlaceStone packet;
						packet.Guid = localPlayer->GetGUID();
						packet.Row = clickedRow;
						packet.Col = clickedCol;
						GameCore::ClientServer->SendPacketToServer( packet );
					}
				}
			}
		}
		else if ( IPlayer* self = GameCore::GetLocalPlayer() )
		{
			const bool isHost = self ? self->IsHost() : false;
			DrawRoomSetting(isHost);
		}
	}
}

void GameView::DrawGameStatus()
{
	IGameRoom* room = GameCore::ActiveRoom;
	if ( nullptr == room )
	{
		return;
	}

	const auto colorName = [ ] ( ColorType color ) {
		return ColorTypeToString( color );
	};
	const auto finishReason = [ ] ( GameFinishReason reason ) {
		switch ( reason )
		{
		case GameFinishReason::BoardFull:
			return "보드가 가득 찼습니다.";
		case GameFinishReason::NoLegalMove:
			return "양쪽 모두 둘 수 있는 위치가 없습니다.";
		case GameFinishReason::MaxCycle:
			return "최대 사이클에 도달했습니다.";
		case GameFinishReason::Surrender:
			return "항복했습니다.";
		default:
			return "";
		}
	};
	if ( room->GetRoomState() == ROOM_STATE_GAME_FINISH )
	{
		std::string result = std::format(
			"게임 종료 | 승자: {} | 검정 {} : 하양 {} | {}",
			colorName( room->GetWinnerColor() ),
			room->GetBlackStoneCount(),
			room->GetWhiteStoneCount(),
			finishReason( room->GetFinishReason() )
		);
		ImGui::TextUnformatted( result.c_str() );
	}
	else
	{
		std::string status = std::format(
			"현재 턴: {} | 검정 {} : 하양 {} | 내 제한 시간: {} | 수: {} | 사이클: {}",
			colorName( room->GetCurrentTurnColor() ),
			room->GetBlackStoneCount(),
			room->GetWhiteStoneCount(),
			( GameCore::GetLocalPlayer() && GameCore::GetLocalPlayer()->GetGUID() == room->GetCurrentTurnGuid() )
				? std::to_string( ImMax( 0, static_cast<int>( std::ceil( room->GetTurnRemainTime() ) ) ) )
				: std::string( "-" ),
			room->GetMoveCount(),
			room->GetCycleCount()
		);
		ImGui::TextUnformatted( status.c_str() );
	}
	ImGui::Separator();
}

bool GameView::DrawGameBoard( IGameBoard& board , bool canClick , ColorType previewColor , size_t& clickedRow , size_t& clickedCol )
{
	const size_t rows = board.GetBoardRows();
	const size_t cols = board.GetBoardCols();
	if ( rows == 0 || cols == 0 )
	{
		return false;
	}

	ImGuiStyle& style = ImGui::GetStyle();
	const ImVec2 availSize = ImGui::GetContentRegionAvail();
	const float gap = 3.0f;
	const float gapWidth = gap * static_cast<float>( cols - 1 );
	const float gapHeight = gap * static_cast<float>( rows - 1 );
	const float cellByWidth = ImMax( 1.0f, availSize.x - gapWidth ) / static_cast<float>( cols );
	const float cellByHeight = ImMax( 1.0f, availSize.y - gapHeight ) / static_cast<float>( rows );
	const float cellSize = ImMax( 4.0f , ImMin( cellByWidth , cellByHeight ) );
	const ImVec2 boardSize(
		cellSize * static_cast<float>( cols ) + gapWidth,
		cellSize * static_cast<float>( rows ) + gapHeight
	);
	const ImVec2 startCursor = ImGui::GetCursorPos() + ImVec2(
		ImMax( 0.0f , ( availSize.x - boardSize.x ) * 0.5f ),
		ImMax( 0.0f , ( availSize.y - boardSize.y ) * 0.5f )
	);

	bool clicked = false;
	bool hasPreview = false;
	size_t previewRow = 0;
	size_t previewCol = 0;
	std::vector<Packet::CellChange> previewFlips;
	std::vector<size_t> previewLineEndIndices;
	std::vector<ImVec2> cellPositions( rows * cols );
	std::vector<size_t> legalCellIndices;
	std::vector<size_t> illegalCellIndices;
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing , ImVec2( gap , gap ) );
	ImGui::PushStyleVar( ImGuiStyleVar_FramePadding , ImVec2( 0 , 0 ) );
	ImGui::PushStyleColor( ImGuiCol_Button , IM_COL32( 40 , 120 , 40 , 255 ) );
	ImGui::PushStyleColor( ImGuiCol_ButtonHovered , IM_COL32( 70 , 160 , 70 , 255 ) );
	ImGui::PushStyleColor( ImGuiCol_ButtonActive , IM_COL32( 30 , 100 , 30 , 255 ) );
	ImGui::SetCursorPos( startCursor );
	for ( size_t r = 0; r < rows; ++r )
	{
		for ( size_t c = 0; c < cols; ++c )
		{
			const ImVec2 cellCursor = startCursor + ImVec2(
				static_cast<float>( c ) * ( cellSize + gap ),
				static_cast<float>( r ) * ( cellSize + gap )
			);
			ImGui::SetCursorPos( cellCursor );
			ImGui::PushID( static_cast<int>( r * cols + c ) );
			const ImVec2 screenPos = ImGui::GetCursorScreenPos();
			cellPositions[ r * cols + c ] = screenPos;
			const ColorType color = board.GetCellColor( r , c );
			const bool canInteractCell = canClick && color == ColorType::None;
			bool pressed = false;
			if ( false == canClick )
			{
				ImGui::Utillity::DisableScope disableScope( true );
				pressed = ImGui::Button( "##cell" , ImVec2( cellSize , cellSize ) );
			}
			else
			{
				if ( color != ColorType::None )
				{
					ImGui::PushStyleColor( ImGuiCol_ButtonHovered , IM_COL32( 40 , 120 , 40 , 255 ) );
					ImGui::PushStyleColor( ImGuiCol_ButtonActive , IM_COL32( 40 , 120 , 40 , 255 ) );
				}
				pressed = ImGui::Button( "##cell" , ImVec2( cellSize , cellSize ) );
				if ( color != ColorType::None )
				{
					ImGui::PopStyleColor( 2 );
				}
			}

			if ( canInteractCell )
			{
				std::vector<Packet::CellChange> flips;
				const bool isLegalCell = CollectPreviewFlips( board , previewColor , r , c , flips );
				if ( isLegalCell )
				{
					legalCellIndices.push_back( r * cols + c );
				}
				else
				{
					illegalCellIndices.push_back( r * cols + c );
				}

				if ( ImGui::IsItemHovered() )
				{
					hasPreview = true;
					previewRow = r;
					previewCol = c;
					previewFlips = flips;
					previewLineEndIndices.clear();
					CollectPreviewLineEnds( board, previewColor, r, c, previewLineEndIndices );
				}

				if ( pressed && isLegalCell )
				{
					clicked = true;
					clickedRow = r;
					clickedCol = c;
				}
			}
			else if ( color != ColorType::None && GameCore::ActiveRoom && ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled ) )
			{
				const std::string& moveInfo = GameCore::ActiveRoom->GetCellMoveInfo( r, c );
				if ( false == moveInfo.empty() )
				{
					ImGui::SetTooltip( "%s", moveInfo.c_str() );
				}
			}

			if ( color != ColorType::None )
			{
				const ImVec2 center = screenPos + ImVec2( cellSize , cellSize ) * 0.5f;
				const float radius = ( cellSize * 0.5f ) - ImMax( 3.0f , cellSize * 0.12f );
				if ( color == ColorType::Black )
				{
					drawList->AddCircleFilled( center , radius , IM_COL32( 20 , 20 , 20 , 255 ) );
				}
				else if ( color == ColorType::White )
				{
					drawList->AddCircleFilled( center , radius , IM_COL32( 240 , 240 , 240 , 255 ) );
					drawList->AddCircle( center , radius , IM_COL32( 60 , 60 , 60 , 255 ) , 0 , 2.0f );
				}
			}
			ImGui::PopID();
		}
	}

	const auto drawCellMarker = [ & ] ( size_t cellIndex , ImU32 color ) {
		const ImVec2 screenPos = cellPositions[ cellIndex ];
		const float markerSize = cellSize * 0.5f;
		const ImVec2 markerMin = screenPos + ImVec2( ( cellSize - markerSize ) * 0.5f , ( cellSize - markerSize ) * 0.5f );
		const ImVec2 markerMax = markerMin + ImVec2( markerSize , markerSize );
		drawList->AddRectFilled( markerMin , markerMax , color );
	};

	for ( const size_t cellIndex : illegalCellIndices )
	{
		drawCellMarker( cellIndex , IM_COL32( 255 , 150 , 150 , 75 ) );
	}
	for ( const size_t cellIndex : legalCellIndices )
	{
		drawCellMarker( cellIndex , IM_COL32( 135 , 206 , 235 , 75 ) );
	}

	if ( hasPreview )
	{
		const auto drawStone = [ & ] ( size_t row , size_t col , ColorType color , ImU32 fillColor ) {
			const ImVec2 screenPos = cellPositions[ row * cols + col ];
			const ImVec2 center = screenPos + ImVec2( cellSize , cellSize ) * 0.5f;
			const float radius = ( cellSize * 0.5f ) - ImMax( 3.0f , cellSize * 0.12f );
			drawList->AddCircleFilled( center , radius , fillColor );
			if ( color == ColorType::White )
			{
				drawList->AddCircle( center , radius , IM_COL32( 60 , 60 , 60 , 128 ) , 0 , 2.0f );
			}
		};

		const ImU32 previewStoneColor = ( previewColor == ColorType::Black )
			? IM_COL32( 20 , 20 , 20 , 128 )
			: IM_COL32( 240 , 240 , 240 , 128 );
		const ImVec2 previewCenter = cellPositions[ previewRow * cols + previewCol ] + ImVec2( cellSize, cellSize ) * 0.5f;
		for ( const Packet::CellChange& flip : previewFlips )
		{
			drawStone( flip.Row , flip.Col , ColorType::None , IM_COL32( 60 , 140 , 255 , 128 ) );
		}
		for ( const size_t endIndex : previewLineEndIndices )
		{
			const ImVec2 endCenter = cellPositions[ endIndex ] + ImVec2( cellSize, cellSize ) * 0.5f;
			drawList->AddLine( previewCenter, endCenter, IM_COL32( 60, 140, 255, 128 ), 3.0f );
		}
		drawStone( previewRow , previewCol , previewColor , previewStoneColor );
	}

	ImGui::PopStyleColor( 3 );
	ImGui::PopStyleVar( 2 );

	return clicked;
}

ColorType GameView::GetOpponentColor( ColorType color ) const
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

bool GameView::CollectPreviewFlips( IGameBoard& board , ColorType color , size_t row , size_t col , std::vector<Packet::CellChange>& outFlips ) const
{
	if ( color == ColorType::None || false == board.IsValidCoord( row , col ) || board.GetCellColor( row , col ) != ColorType::None )
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
	const int rows = static_cast<int>( board.GetBoardRows() );
	const int cols = static_cast<int>( board.GetBoardCols() );
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
			const ColorType cellColor = board.GetCellColor( static_cast<size_t>( r ) , static_cast<size_t>( c ) );
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

void GameView::CollectPreviewLineEnds( IGameBoard& board , ColorType color , size_t row , size_t col , std::vector<size_t>& outEndCellIndices ) const
{
	if ( color == ColorType::None || false == board.IsValidCoord( row , col ) || board.GetCellColor( row , col ) != ColorType::None )
	{
		return;
	}

	const ColorType opponent = GetOpponentColor( color );
	if ( opponent == ColorType::None )
	{
		return;
	}

	const int directions[ 8 ][ 2 ] = {
		{ -1, -1 }, { -1, 0 }, { -1, 1 },
		{ 0, -1 },             { 0, 1 },
		{ 1, -1 },  { 1, 0 },  { 1, 1 },
	};
	const int rows = static_cast<int>( board.GetBoardRows() );
	const int cols = static_cast<int>( board.GetBoardCols() );
	const int startRow = static_cast<int>( row );
	const int startCol = static_cast<int>( col );

	for ( const auto& dir : directions )
	{
		bool hasOpponentBetween = false;
		int r = startRow + dir[ 0 ];
		int c = startCol + dir[ 1 ];
		while ( r >= 0 && c >= 0 && r < rows && c < cols )
		{
			const ColorType cellColor = board.GetCellColor( static_cast<size_t>( r ), static_cast<size_t>( c ) );
			if ( cellColor == opponent )
			{
				hasOpponentBetween = true;
			}
			else if ( cellColor == color )
			{
				if ( hasOpponentBetween )
				{
					outEndCellIndices.push_back( static_cast<size_t>( r ) * board.GetBoardCols() + static_cast<size_t>( c ) );
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
}

void GameView::DrawRoomSetting(bool isHost)
{
	const ImVec2 availSize = ImGui::GetContentRegionAvail();
	const float  labelX = availSize.x * 0.3f;
	static std::string	roomTitle	= GameCore::ActiveRoom->GetRoomTitle();
	const RoomSetting&	currSetting = GameCore::ActiveRoom->GetRoomSetting();
	static RoomSetting	newSetting	= currSetting;

	ImText()( "방 설정" , 2.0f );
	ImGui::Separator();
	ImGui::Utillity::DisableScope disableScope( !isHost );
	ImGui::PushID( "title" );
	{
		static bool isDirty = false;
		ImGui::Utillity::TextWithVerticalSeparator( "방 제목" , labelX );
		if ( ImGui::InputText( "##room_title" , &roomTitle ) )
		{
			isDirty = true;
		}
		ImGui::Utillity::TextWithVerticalSeparator( "포트 번호" , labelX );
		{
			ImGui::Utillity::DisableScope disableScope;
			int port = GameCore::ClientServer ? (int)GameCore::ClientServer->GetPort() : 0;
			ImGui::InputInt( "##room_port" , &port , 0 );
		}
		{
			ImGui::Utillity::DisableScope disableScope( !isDirty );
			if ( ImGui::Button( "변경" ) )
			{
				if ( isDirty )
				{
					GameCore::ActiveRoom->SetRoomTitle( roomTitle.c_str() );
					isDirty = false;
				}
			}
		}
		ImGui::SameLine();
		{
			ImGui::Utillity::DisableScope disableScope( !isDirty );
			if ( ImGui::Button( "취소" ) )
			{
				roomTitle = GameCore::ActiveRoom->GetRoomTitle();
				isDirty = false;
			}
		}
	}
	ImGui::PopID();
	ImGui::Separator();
	ImGui::PushID( "currSetting" );
	{
		ImText()( "기본 설정" , 0.7f );

		ImGui::Utillity::TextWithVerticalSeparator( "최대 인원 수" , labelX );
		if ( ImGui::InputInt( "##max_player" , &newSetting.MaxPlayerCount ) )
		{
			newSetting.MaxPlayerCount = ImClamp( newSetting.MaxPlayerCount , 2 , 4 );
		}
		ImGui::Utillity::HoveredToolTip( "2 ~ 4 사이 값을 입력해주세요." );

		ImGui::Utillity::TextWithVerticalSeparator( "보드 열 크기 (X)" , labelX );
		if ( ImGui::InputInt( "##board_col" , &newSetting.Col , 2 ) )
		{
			newSetting.Col = newSetting.Col & ~1; // 하위비트를 버림으로써 짝수로 변경
			newSetting.Col = ImClamp( newSetting.Col , 4 , 16 );
		}
		ImGui::Utillity::HoveredToolTip( "4 ~ 16 사이 값을 입력해주세요.\n2의 배수를 입력해주세요." );

		ImGui::Utillity::TextWithVerticalSeparator( "보드 행 크기 (Y)" , labelX );
		if ( ImGui::InputInt( "##board_row" , &newSetting.Row , 2 ) )
		{
			newSetting.Row = newSetting.Row & ~1;
			newSetting.Row = ImClamp( newSetting.Row , 4 , 16 );
		}
		ImGui::Utillity::HoveredToolTip( "4 ~ 16 사이 값을 입력해주세요.\n2의 배수를 입력해주세요." );

		ImText()( "고급 설정" , 0.7f );

		ImGui::Utillity::TextWithVerticalSeparator( "최대 사이클 수" , labelX );
		if ( ImGui::InputInt( "##max_cycle" , &newSetting.MaxCycle ) )
		{
			newSetting.MaxCycle = ImClamp( newSetting.MaxCycle , 0 , 999 );
		}
		ImGui::Utillity::HoveredToolTip( "0 ~ 999 사이 값을 입력해주세요.\n0이면 보드가 전부 찰 때까지 둡니다." );

		ImGui::Utillity::TextWithVerticalSeparator( "제한 시간" , labelX );
		if ( ImGui::InputInt( "##timer" , &newSetting.Timer ) )
		{
			newSetting.Timer = ImClamp( newSetting.Timer , 0 , 999 );
		}
		ImGui::Utillity::HoveredToolTip( "0 ~ 999 사이 값을 입력해주세요.\n0이면 제한 시간이 없습니다." );

		bool isDirty = newSetting != currSetting;
		{
			ImGui::Utillity::DisableScope disableScope( !isDirty );
			if (ImGui::Button( "변경" ) )
			{
				if ( isDirty )
				{
					GameCore::ActiveRoom->SetRoomSetting( newSetting );
				}
			}
		}
		ImGui::SameLine();
		{
			ImGui::Utillity::DisableScope disableScope( !isDirty );
			if ( ImGui::Button( "취소" ) )
			{
				newSetting = currSetting;
			}
		}
		ImGui::SameLine();
		if ( ImGui::Button( "기본 값" ) )
		{
			newSetting = { };
		}
	}
	ImGui::PopID();
	ImGui::Separator();
	ImText()( "플레이어 설정" , 2.0f );
	ImGui::Separator();
	ImGui::PushID( "players" );
	{
		if ( GameCore::ActiveRoom )
		{
			DrawPlayerList();
		}
		//DrawPlayerPopup
	}
	ImGui::PopID();

	if ( false == isHost )
	{
		roomTitle	= GameCore::ActiveRoom->GetRoomTitle();
		newSetting	= currSetting;
	}
}

void GameView::ShowChat()
{
	ImGuiStyle& style = ImGui::GetStyle();
	const ImVec2 availSize = ImGui::GetContentRegionAvail();
	const ImVec2 padding = style.FramePadding;
	const ImVec2 spacing = style.ItemSpacing;
	const char* buttonText = "Enter";
	const ImVec2 buttonSize = ImVec2( ImGui::CalcTextSize( buttonText ).x + spacing.x , ImGui::GetFrameHeight() );

	ImGui::BeginChild( "##Players" , ImVec2( 0 , (availSize.y * 0.3f) - padding.y ) , ImGuiChildFlags_Borders );
	DrawPlayerList();
	ImGui::EndChild();

	ImGui::BeginChild( "##ChatHistory" , ImVec2( 0 , (availSize.y * 0.7f) - buttonSize.y - padding.y ) , ImGuiChildFlags_Borders );
	DrawChatBoard();
	ImGui::EndChild();

	static char inputBuffer[256] = "";
	ImGui::SetNextItemWidth( availSize.x - buttonSize.x - spacing.x );
	if ( ImGui::InputText( "##ChatInput" , inputBuffer , ARRAYSIZE(inputBuffer) , ImGuiInputTextFlags_EnterReturnsTrue) )
	{
		if(inputBuffer[0] != '\0' && GameCore::ActiveRoom)
		{
			GameCore::GetLocalPlayer()->SendChatMessage(inputBuffer);
		}
		inputBuffer[ 0 ] = '\0';
		ImGui::SetKeyboardFocusHere( -1 );
	}

	ImGui::SameLine();
	if (ImGui::Button(buttonText , buttonSize))
	{
		if(inputBuffer[0] != '\0' && GameCore::ActiveRoom)
		{
			GameCore::GetLocalPlayer()->SendChatMessage(inputBuffer);
		}
		inputBuffer[ 0 ] = '\0';
	}
}

#define POPUP_MENUITEM_CHANGE_COLOR(colorType)\
{\
	ColorType color = colorType;\
	if ( ImGui::MenuItem( ColorTypeToString( color ) , nullptr , color == player->GetColorType() ) ) {\
		player->SetColorType( color );\
		ImGui::CloseCurrentPopup();\
	}\
}
void GameView::DrawPlayerPopup(IPlayer* player)
{
	const bool	 isHost		= player->IsHost();
	const bool	 isLocal	= player->IsLocal();
	const char*  nickname   = player->GetNickName().c_str();
	const ImVec4 textColor	= ImGui::Utillity::ColorFromGuid( player->GetGUID() );

	ImGui::Utillity::StyleBuilder styleBuilder;
	styleBuilder.PushStyleColor( ImGuiCol_Text , textColor );
	ImText()( nickname , 0.7f, ImText::Align::Center );
	styleBuilder.PopStyle();
	ImGui::Separator();

	const bool disable = GameCore::ActiveRoom && GameCore::ActiveRoom->GetRoomState() != ROOM_STATE_WAITING;
	ImGui::Utillity::DisableScope disableScope( disable );

	if ( isHost )
	{
		if ( ImGui::MenuItem( "강퇴" ) )
		{
			std::string cause = "강퇴";
			Packet::S2C_PlayerDisConnected packet;
			packet.Guid = player->GetGUID();
			strcpy_s(packet.MainCause, cause.length() + 1, cause.c_str());
			GameCore::HostServer->BroadCast( packet );
			ImGui::CloseCurrentPopup();
		}
	}
	if ( ImGui::BeginMenu( "색 변경" ) )
	{
		POPUP_MENUITEM_CHANGE_COLOR( ColorType::White )
		POPUP_MENUITEM_CHANGE_COLOR( ColorType::Black )
		ImGui::EndMenu();
	}
	if ( ImGui::BeginMenu( "순서 변경" ) )
	{
		size_t currentIndex = 0;
		const bool hasIndex = GameCore::ActiveRoom->GetPlayerIndexFromGuid( player->GetGUID(), currentIndex );
		const size_t playerCount = GameCore::ActiveRoom->GetCurrentPlayerCount();
		const auto movePlayer = [ player ] ( size_t newIndex ) {
			if ( nullptr == GameCore::HostServer || nullptr == GameCore::ActiveRoom )
			{
				return;
			}
			if ( false == GameCore::ActiveRoom->MovePlayerToIndex( player->GetGUID(), newIndex ) )
			{
				return;
			}

			Packet::S2C_PlayerOrderChanged packet {
				.Guid = player->GetGUID(),
				.NewIndex = newIndex
			};
			GameCore::HostServer->BroadCast( packet );
		};
		if ( ImGui::MenuItem( "맨 위로" ) )
		{
			movePlayer( 0 );
		}
		if ( ImGui::MenuItem( "한 순서 위로" ) )
		{
			if ( hasIndex && currentIndex > 0 )
			{
				movePlayer( currentIndex - 1 );
			}
		}
		if ( ImGui::MenuItem( "한 순서 아래로" ) )
		{
			if ( hasIndex && currentIndex + 1 < playerCount )
			{
				movePlayer( currentIndex + 1 );
			}
		}
		if ( ImGui::MenuItem( "맨 아래로" ) )
		{
			if ( playerCount > 0 )
			{
				movePlayer( playerCount - 1 );
			}
		}
		ImGui::EndMenu();
	}
}

void GameView::DrawChatBoard()
{
	ImGuiListClipper clipper;

	ImGuiStyle&		style			= ImGui::GetStyle();
	const ImVec2	padding			= style.FramePadding;
	const ImVec2	spacing			= style.ItemSpacing;
	const auto&		message			= GameCore::ChatManager.GetChatMessages();
	const float		rounding		= 3.0f;
	const char*		popupID			= "##player_popup";

	clipper.Begin( static_cast< int >( message.size() ) , ImGui::GetFrameHeight() );
	while ( clipper.Step() )
	{
		for ( int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i )
		{
			ImGui::PushID( i );
			if ( message[i].From == GUID_NULL )
			{
				if ( message[i].Chat == "---" )
				{
					ImGui::Separator();
				}
			}
			IPlayer*		destPlayer		= GameCore::GetPlayerFromGuid(message[i].From);
			const bool		isLocal			= destPlayer ? destPlayer->IsLocal() : false;
			const ImU32		defaultColor	= isLocal ? IM_COL32( 255 , 255 , 255 , 0 ) : IM_COL32( 255 , 255 , 255 , 0 );
			const ImU32		hoveredColor	= isLocal ? IM_COL32( 255 , 255 , 255 , 0 ) : IM_COL32( 255 , 255 , 255 , 0 );
			const ImU32		activeColor		= isLocal ? IM_COL32( 255 , 255 , 255 , 0 ) : IM_COL32( 255 , 255 , 255 , 0 );
			const ImVec4	textColor		= ImGui::Utillity::ColorFromGuid( message[i].From );
			const ImVec2	headerSize		= ImGui::CalcTextSize( message[i].Header.c_str() ) + ImVec2( spacing.x , spacing.y );
			{
				ImGui::Utillity::StyleBuilder styleBuilder;
				styleBuilder.PushStyleVar( ImGuiStyleVar_FrameRounding, rounding);
				styleBuilder.PushStyleColor( ImGuiCol_Button, defaultColor);
				styleBuilder.PushStyleColor( ImGuiCol_ButtonHovered, hoveredColor);
				styleBuilder.PushStyleColor( ImGuiCol_ButtonActive, hoveredColor);
				styleBuilder.PushStyleColor( ImGuiCol_Text, textColor);
				ImGui::Button(message[i].Header.c_str(), headerSize);
				styleBuilder.PopStyle();
				if(destPlayer && ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				{
					ImGui::OpenPopup(popupID);
				}
				if (ImGui::BeginPopup(popupID))
				{
					DrawPlayerPopup(destPlayer);
					ImGui::EndPopup();
				}
			}
			ImGui::SameLine();
			{
				ImGui::Utillity::StyleBuilder styleBuilder;
				styleBuilder.PushStyleColor( ImGuiCol_Text, IM_COL32( 255 , 255 , 255 , 255 ));
				ImGui::TextWrapped( message[i].Chat.c_str() );
			}
			ImGui::PopID();
		}
	}
	clipper.End();
}
