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
	if (GameCore::GetLocalPlayer() && GameCore::GetLocalPlayer()->IsHost()) 
	{
		ImGui::BeginChild( "##UpperGameBar" , ImVec2( 0 , height + padding.y * 2.0f ) , ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );
		DrawUpperGameBar();
		ImGui::EndChild();
	}

	ImGui::BeginChild( "##MainGameBar" , ImVec2( 0 , 0 ) , ImGuiChildFlags_Borders , ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );
	DrawMainGameBar();
	ImGui::EndChild();
}

void GameView::DrawUpperGameBar()
{
	ImGui::Utillity::StyleBuilder styleBuilder;
	styleBuilder.PushStyleVar( ImGuiStyleVar_FrameRounding , 3.0f );
	IGameRoom* room = GameCore::ActiveRoom;

	const int hoveredFlags = ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayNormal;
	{
		ImGui::Utillity::DisableScope disableScope(room && false == room->CanStartGame());
		if ( ImGui::Button( "시작" ) )
		{

		}
		if( disableScope.IsDisabled() ) 
			ImGui::Utillity::HoveredToolTip( "게임을 시작하기 위해서는 다음과 같은 조건이 필요합니다.\n- 플레이어가 2명 이상 있어야합니다.\n- 서로 다른 색이 있는 돌이 2개 이상 있어야합니다." , hoveredFlags );
	}
	ImGui::SameLine();
	{
		ImGui::Utillity::DisableScope disableScope( room && ROOM_STATE_GAME_PLAYING != room->GetRoomState() );
		if ( ImGui::Button( "종료" ) )
		{

		}
		if ( disableScope.IsDisabled() )
			ImGui::Utillity::HoveredToolTip( "게임을 종료하기 위해서는 게임이 시작한 상태여야합니다." , hoveredFlags );
	}
	ImGui::SameLine();
	{
		const char* popupID = "Setting";
		if ( ImGui::Button( "설정" ) )
		{
			ImGui::OpenPopup( popupID );
		}
		if ( ImGui::BeginPopup( popupID ) )
		{
			ImGui::Text( "Hello from popup!" );

			if ( ImGui::MenuItem( "Close" ) )
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}
	}
}

void GameView::DrawMainGameBar()
{
	if ( GameCore::ActiveRoom )
	{
		if ( RoomState::ROOM_STATE_WAITING != GameCore::ActiveRoom->GetRoomState() )
		{
			GameCore::ActiveRoom->GetGameBoard().Show();
		}
		else if ( IPlayer* self = GameCore::GetLocalPlayer() )
		{
			const bool isHost = self ? self->IsHost() : false;
			DrawRoomSetting(isHost);
		}
	}
}

void GameView::DrawRoomSetting(bool isHost)
{
	const ImVec2 availSize = ImGui::GetContentRegionAvail();
	const float  labelX = availSize.x * 0.3f;
	const RoomSetting& setting = GameCore::ActiveRoom->GetRoomSetting();

	static std::string	roomTitle		= GameCore::ActiveRoom->GetRoomTitle();
	static int			maxPlayerCount	= ( int ) setting.MaxPlayerCount;
	static int			row				= ( int ) setting.Row;
	static int			col				= ( int ) setting.Col;

	ImGui::Utillity::DisableScope disableScope( !isHost );
	ImText()( "방 설정" , 2.0f );
	ImGui::Separator();
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
			int port = ( int ) GameCore::HostServer->GetPort();
			ImGui::InputInt( "##room_port" , &port , 0 );
		}
		if ( ImGui::Button( "변경" ) )
		{
			if ( isDirty )
			{
				GameCore::ActiveRoom->SetRoomTitle( roomTitle.c_str() );
				isDirty = false;
			}
		}
		ImGui::SameLine();
		if ( ImGui::Button( "취소" ) )
		{
			roomTitle = GameCore::ActiveRoom->GetRoomTitle();
			isDirty = false;
		}
	}
	ImGui::PopID();
	ImGui::Separator();
	ImGui::PushID( "setting" );
	{
		static bool isDirty = false;
		ImGui::Utillity::TextWithVerticalSeparator( "최대 인원 수" , labelX );
		if ( ImGui::InputInt( "##max_player" , &maxPlayerCount ) )
		{
			maxPlayerCount = ImClamp( maxPlayerCount , 2 , 4 );
			isDirty = true;
		}
		ImGui::Utillity::HoveredToolTip( "2 ~ 4 사이 값을 입력해주세요." );

		ImGui::Utillity::TextWithVerticalSeparator( "보드 열 크기 (X)" , labelX );
		if ( ImGui::InputInt( "##board_row" , &row , 2 ) )
		{
			row = row & ~1; // 하위비트를 버림으로써 짝수로 변경
			row = ImClamp( row , 4 , 16 );
			isDirty = true;
		}
		ImGui::Utillity::HoveredToolTip( "4 ~ 16 사이 값을 입력해주세요.\n2의 배수를 입력해주세요." );

		ImGui::Utillity::TextWithVerticalSeparator( "보드 행 크기 (Y)" , labelX );
		if ( ImGui::InputInt( "##board_col" , &col , 2 ) )
		{
			row = row & ~1;
			row = ImClamp( row , 4 , 16 );
			isDirty = true;
		}
		ImGui::Utillity::HoveredToolTip( "4 ~ 16 사이 값을 입력해주세요.\n2의 배수를 입력해주세요." );

		if ( ImGui::Button( "변경" ) )
		{
			if ( isDirty )
			{
				RoomSetting setting{
					.MaxPlayerCount = ( size_t ) maxPlayerCount,
					.Row = ( size_t ) row, .Col = ( size_t ) col
				};
				GameCore::ActiveRoom->SetRoomSetting( setting );
				isDirty = false;
			}
		}
		ImGui::SameLine();
		if ( ImGui::Button( "취소" ) )
		{
			maxPlayerCount = ( int ) setting.MaxPlayerCount;
			row = ( int ) setting.Row;
			col = ( int ) setting.Col;
			isDirty = false;
		}
	}
	ImGui::PopID();

	if ( false == isHost )
	{
		roomTitle		= GameCore::ActiveRoom->GetRoomTitle();
		maxPlayerCount	= ( int ) setting.MaxPlayerCount;
		row				= ( int ) setting.Row;
		col				= ( int ) setting.Col;
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

void GameView::DrawPlayerList()
{
	ImGuiStyle&		style		= ImGui::GetStyle();
	const float		rounding	= 3.0f;
	const ImVec2	availSize	= ImGui::GetContentRegionAvail();
	const char*		popupID		= "##PlayerPopup";
	if (GameCore::ActiveRoom)
	{
		size_t playerCount = GameCore::ActiveRoom->GetCurrentPlayerCount();
		for(size_t i = 0; i < playerCount; ++i)
		{
			IPlayer* player = GameCore::ActiveRoom->GetPlayerFromIndex(i);
			if (player)
			{
				const bool		isHost = player->IsHost();
				const bool		isLocal = player->IsLocal();
				const ImU32		defaultColor = IM_COL32( 255 , 255 , 255 , 40 );
				const ImU32		hoveredColor = IM_COL32( 255 , 255 , 255 , 20 );
				const ImU32		activeColor  = IM_COL32( 255 , 255 , 255 , 60 );
				const ImVec4	nameColor	 = ImGui::Utillity::ColorFromGuid( player->GetGUID() );
				const ImVec4	stoneColor	 = ColorTypeToImVec4( player->GetColorType() );
				const ImVec2    cursorPos	 = ImGui::GetCursorPos();

				std::string label = std::format( "{}{}{}" ,
					player->GetNickName() ,
					isHost ? " (Host)" : "" ,
					isLocal ? " (Local)" : "" );
				
				ImGui::Utillity::StyleBuilder styleBuilder;
				styleBuilder.PushStyleVar( ImGuiStyleVar_FrameRounding , rounding );
				styleBuilder.PushStyleColor( ImGuiCol_Button , defaultColor );
				styleBuilder.PushStyleColor( ImGuiCol_ButtonHovered , hoveredColor );
				styleBuilder.PushStyleColor( ImGuiCol_ButtonActive , activeColor );
				ImGui::PushID( (int)i );
				bool clicked = ImGui::Button( "##Players", ImVec2(availSize.x, 0) );
				ImGui::SameLine( cursorPos.x + style.FramePadding.x );
				{
					ImGui::Utillity::StyleBuilder styleBuilder;
					styleBuilder.PushStyleColor( ImGuiCol_Text , stoneColor );
					ImGui::TextUnformatted( "●" );
				}
				ImGui::SameLine();
				{
					ImGui::Utillity::StyleBuilder styleBuilder;
					styleBuilder.PushStyleColor( ImGuiCol_Text , nameColor );
					ImGui::TextUnformatted( label.c_str() );
				}
				styleBuilder.PopStyle();
				if (GameCore::HostServer && clicked)
				{
					ImGui::OpenPopup(popupID);
				}
				if (GameCore::HostServer && ImGui::BeginPopup(popupID))
				{
					DrawPlayerPopup(player);
					ImGui::EndPopup();
				}
				ImGui::PopID();
			}
		}
	}
	else
	{
		ImGui::Text( "No active room." );
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

	if ( false == isLocal )
	{
		if ( ImGui::MenuItem( "강퇴" ) )
		{
			{
				Packet::S2C_PlayerKicked packet;
				packet.Guid = player->GetGUID();
				GameCore::HostServer->BroadCast( packet );
			}
			ImGui::CloseCurrentPopup();
		}
	}
	if ( ImGui::BeginMenu( "색 변경" ) )
	{
		POPUP_MENUITEM_CHANGE_COLOR(ColorType::White)
		POPUP_MENUITEM_CHANGE_COLOR(ColorType::Black)
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
	

	clipper.Begin( static_cast< int >( message.size() ) , ImGui::GetFrameHeight() );
	while ( clipper.Step() )
	{
		for ( int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i )
		{
			ImGui::PushID( i );
			if ( message[ i ].From == GUID_NULL )
			{
				if ( message[ i ].Chat == "---" )
				{
					ImGui::Separator();
				}
			}
			const bool		isLocal			= GameCore::ActiveRoom && message[ i ].From == GameCore::GetLocalPlayer()->GetGUID();
			const ImU32		defaultColor	= isLocal ? IM_COL32( 255 , 255 , 255 , 0 ) : IM_COL32( 255 , 255 , 255 , 0 );
			const ImU32		hoveredColor	= isLocal ? IM_COL32( 255 , 255 , 255 , 0 ) : IM_COL32( 255 , 255 , 255 , 0 );
			const ImU32		activeColor		= isLocal ? IM_COL32( 255 , 255 , 255 , 0 ) : IM_COL32( 255 , 255 , 255 , 0 );
			const ImVec4	textColor		= ImGui::Utillity::ColorFromGuid( message[ i ].From );
			const ImVec2	headerSize		= ImGui::CalcTextSize( message[ i ].Header.c_str() ) + ImVec2( spacing.x , spacing.y );

			{
				ImGui::Utillity::StyleBuilder styleBuilder;
				styleBuilder.PushStyleVar( ImGuiStyleVar_FrameRounding, rounding);
				styleBuilder.PushStyleColor( ImGuiCol_Button, defaultColor);
				styleBuilder.PushStyleColor( ImGuiCol_ButtonHovered, hoveredColor);
				styleBuilder.PushStyleColor( ImGuiCol_ButtonActive, hoveredColor);
				styleBuilder.PushStyleColor( ImGuiCol_Text, textColor);
				ImGui::Button(message[i].Header.c_str(), headerSize);
			}
			ImGui::SameLine();
			{
				ImGui::Utillity::StyleBuilder styleBuilder;
				styleBuilder.PushStyleColor( ImGuiCol_Text, IM_COL32( 255 , 255 , 255 , 255 ));
				ImGui::TextWrapped( message[ i ].Chat.c_str() );
			}
			ImGui::PopID();
		}
	}
	clipper.End();
}
