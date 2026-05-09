#include "pch.h"

void GameView::DrawPlayerList()
{
	ImGuiStyle& style = ImGui::GetStyle();
	const float		rounding = 3.0f;
	const ImVec2	availSize = ImGui::GetContentRegionAvail();
	const char* popupID = "##player_popup";
	if ( GameCore::ActiveRoom )
	{
		size_t playerCount = GameCore::ActiveRoom->GetCurrentPlayerCount();
		for ( size_t i = 0; i < playerCount; ++i )
		{
			IPlayer* player = GameCore::ActiveRoom->GetPlayerFromIndex( i );
			if ( player )
			{
				const bool		isHost = player->IsHost();
				const bool		isLocal = player->IsLocal();
				const ImU32		defaultColor = IM_COL32( 255 , 255 , 255 , 40 );
				const ImU32		hoveredColor = IM_COL32( 255 , 255 , 255 , 20 );
				const ImU32		activeColor = IM_COL32( 255 , 255 , 255 , 60 );
				const ImVec4	nameColor = ImGui::Utillity::ColorFromGuid( player->GetGUID() );
				const ImVec4	stoneColor = ColorTypeToImVec4( player->GetColorType() );
				const ImVec2    cursorPos = ImGui::GetCursorPos();

				std::string label = std::format( "{}{}{}" ,
					player->GetNickName() ,
					isHost ? " (Host)" : "" ,
					isLocal ? " (Local)" : "" );

				ImGui::Utillity::StyleBuilder styleBuilder;
				styleBuilder.PushStyleVar( ImGuiStyleVar_FrameRounding , rounding );
				styleBuilder.PushStyleColor( ImGuiCol_Button , defaultColor );
				styleBuilder.PushStyleColor( ImGuiCol_ButtonHovered , hoveredColor );
				styleBuilder.PushStyleColor( ImGuiCol_ButtonActive , activeColor );
				ImGui::PushID( ( int ) i );
				bool clickedLeft = ImGui::Button( "##Players" , ImVec2( availSize.x , 0 ) );
				bool clickedRight = ImGui::IsItemHovered() && ImGui::IsMouseClicked( ImGuiMouseButton_Right );
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
				if ( GameCore::HostServer && ( clickedLeft || clickedRight ) )
				{
					ImGui::OpenPopup( popupID );
				}
				if ( ImGui::BeginPopup( popupID ) )
				{
					DrawPlayerPopup( player );
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