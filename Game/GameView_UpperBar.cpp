#include "pch.h"

void GameView::DrawUpperGameBar()
{
	if ( IPlayer* local = GameCore::GetLocalPlayer() )
	{
		ImGuiStyle& style = ImGui::GetStyle();
		const ImVec2 availSize	= ImGui::GetContentRegionAvail();
		const ImVec2 padding	= style.FramePadding;
		const ImVec2 spacing	= style.ItemSpacing;
		const ImVec2 buttonSize = ImVec2( 50.0f , ImGui::GetFrameHeight() );
		const bool   isPlaying  = ROOM_STATE_GAME_PLAYING == GameCore::ActiveRoom->GetRoomState();

		ImGui::Utillity::StyleBuilder styleBuilder;
		styleBuilder.PushStyleVar( ImGuiStyleVar_FrameRounding , 3.0f );

		const int tooltipFlags = ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayNormal;
		if ( GameCore::HostServer )
		{
			if (isPlaying)
			{
				if ( ImGui::Button( "게임 취소" ) )
				{
					Packet::S2C_GameStateRequest packet{
						.State = ROOM_STATE_WAITING
					};
					GameCore::HostServer->BroadCast( packet );
				}
			}
			else
			{
				//ImGui::Utillity::DisableScope disableScope( false == GameCore::ActiveRoom->CanStartGame() );
				if ( ImGui::Button( "게임 시작" ) )
				{
					const size_t playerCount = GameCore::ActiveRoom->GetCurrentPlayerCount();
					for ( size_t i = 0; i < playerCount; ++i )
					{
						if ( IPlayer* player = GameCore::ActiveRoom->GetPlayerFromIndex( i ) )
						{
							player->SetVoteState( VoteState::None );
						}
					}
					Packet::S2C_GameStateRequest packet{
						.State = ROOM_STATE_GAME_PLAYING
					};
					GameCore::HostServer->BroadCast( packet );
				}
				//if ( disableScope.IsDisabled() )
					ImGui::Utillity::HoveredToolTip( "게임을 시작하기 위해서는 다음과 같은 조건이 필요합니다.\n- 플레이어가 2명 이상 있어야합니다.\n` 플레이어가 전부 준비 완료된 상태여야 합니다.\n- 서로 다른 색이 있는 돌이 2개 이상 있어야합니다." , tooltipFlags );
			}
			//ImGui::SameLine();
			//{
			//	ImGui::Utillity::DisableScope disableScope( isPlaying );
			//	if ( ImGui::Button( "종료" , buttonSize ) )
			//	{
			//
			//	}
			//	if ( disableScope.IsDisabled() )
			//		ImGui::Utillity::HoveredToolTip( "게임을 종료하기 위해서는 게임이 시작한 상태여야합니다." , tooltipFlags );
			//}
		}
		else
		{
		}
		ImGui::SameLine();
		ImGui::SetCursorPosX( availSize.x - buttonSize.x + spacing.x );
		{
			ImGui::Utillity::DisableScope disableScope( !isPlaying );
			if ( ImGui::Button( "무르기" , buttonSize ) )
			{
			}
		}
	}
}
