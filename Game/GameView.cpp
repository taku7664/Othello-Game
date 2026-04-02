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
		ImGui::BeginChild( "SettingBar" , ImVec2( 0 , height + padding.y * 2.0f ) , ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );
		DrawSettingBar();
		ImGui::EndChild();
	}

	if ( GameCore::ActiveRoom )
	{
		GameCore::ActiveRoom->GetGameBoard().Show();
	}
}

void GameView::DrawSettingBar()
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
			ImGui::Utillity::HoveredToolTip( "게임을 시작하기 위해서는 플레이어가 2명 이상 필요합니다." , hoveredFlags );
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
				const ImVec4	textColor	 = ImGui::Utillity::ColorFromGuid( player->GetGUID() );
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
				ImGui::PushID( i );
				bool clicked = ImGui::Button( "##Players", ImVec2(availSize.x, 0) );
				styleBuilder.PushStyleColor( ImGuiCol_Text , textColor );
				ImGui::SameLine(cursorPos.x + style.FramePadding.x);
				ImGui::TextUnformatted( label.c_str() );
				if (GameCore::HostServer && clicked)
				{
					ImGui::OpenPopup(popupID);
				}
				if (GameCore::HostServer && ImGui::BeginPopup(popupID))
				{
					if(false == isLocal)
					{
						if ( ImGui::MenuItem( "강퇴" ) )
						{
							{
								Packet::S2C_PlayerKicked packet;
								packet.Guid = player->GetGUID();
								GameCore::HostServer->BroadCast(packet);
							}
							ImGui::CloseCurrentPopup();
						}
					}
					if ( ImGui::BeginMenu( "색 변경" ) )
					{
						if ( ImGui::MenuItem( "흰색" ) )
						{
							player->SetColorType( ColorType::White );
							ImGui::CloseCurrentPopup();
						}
						if ( ImGui::MenuItem( "검정색" ) )
						{
							player->SetColorType( ColorType::Black );
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndMenu();
					}
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
