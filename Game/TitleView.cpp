#include "pch.h"
#include "TitleView.h"

TitleView::TitleView()
{
}

TitleView::~TitleView()
{
}

void TitleView::OnVisible()
{
	GameCore::Renderer.SetWindowTitle(L"오셀로 게임");
}

void TitleView::OnShow(Context& context)
{
	ImGuiIO& io = ImGui::GetIO();
	const ImVec2 windowSize = ImGui::GetWindowSize();
	const ImVec2 buttonSize( 150.0f , 40.0f );
	const ImVec2 dummySize( 0.0f , 10.0f );
	const char* titleText = "즐거운 오셀로게임";
	const char* creatorText = "제작자: 박주형";
	const ImVec2 titleSize = ImGui::CalcTextSize( titleText );
	const ImVec2 creatorSize = ImGui::CalcTextSize( creatorText );

	ImGui::SetCursorPosY( windowSize.y * 0.3f );
	ImText()( titleText , 2.0f , ImText::Align::Center );

	ImGui::Utillity::StyleBuilder styleBuidler;
	styleBuidler.PushStyleVar( ImGuiStyleVar_FrameRounding , 5.0f );

	ImGui::Dummy( dummySize );
	ImGui::SetCursorPosX( ( windowSize.x - buttonSize.x ) * 0.5f );
	if ( ImGui::Button( "방 만들기" , buttonSize ) )
	{
		ImPopupContext popupContext;
		popupContext.Title = "오셀로 게임 [방 만들기]";
		popupContext.Flags = ImGuiWindowFlags_AlwaysAutoResize;
		GameCore::ImGuiManager.OpenPopup( &popupContext , [ this ] ( ImPopupContext& c ) {
			ShowMakeRoom( c );
			} );
	}

	ImGui::SetCursorPosX( ( windowSize.x - buttonSize.x ) * 0.5f );
	if ( ImGui::Button( "방 참가하기" , buttonSize ) )
	{
		ImPopupContext popupContext;
		popupContext.Title = "오셀로 게임 [방 참가하기]";
		popupContext.Flags = ImGuiWindowFlags_AlwaysAutoResize;
		GameCore::ImGuiManager.OpenPopup( &popupContext , [ this ] ( ImPopupContext& c ) {
			ShowJoinRoom( c );
			} );
	}

	ImGui::SetCursorPosY( windowSize.y - creatorSize.y );
	ImText()( creatorText , 0.8f , ImText::Align::Right );
}

void TitleView::ShowMakeRoom( ImPopupContext& c )
{
	const float LABEL_WIDTH = 80.0f;
	const float CONTENT_WIDTH = 130.0f;

	static char roomname[ 64 ] = "";
	static char nickname[ 64 ] = "";
	static bool usePort = false;
	static int port = 7070;

	ImGuiIO& io = ImGui::GetIO();

	ImText()( "기본 정보" , 0.7f );

	ImGui::Utillity::TextWithVerticalSeparator( "방 이름" , LABEL_WIDTH );
	ImGui::SetNextItemWidth( CONTENT_WIDTH );
	ImGui::InputText( "##Roomname" , roomname , IM_ARRAYSIZE( roomname ) );

	ImGui::Utillity::TextWithVerticalSeparator( "닉네임" , LABEL_WIDTH );
	ImGui::SetNextItemWidth( CONTENT_WIDTH );
	ImGui::InputText( "##Nickname" , nickname , IM_ARRAYSIZE( nickname ) );

	ImGui::Separator();

	ImText()( "고급 설정" , 0.7f );

	float startX = ImGui::GetCursorPosX();
	ImGui::Checkbox( "##UsePort" , &usePort );
	ImGui::SameLine();
	ImGui::TextUnformatted( "포트" );

	ImGui::SameLine( startX );
	ImGui::Utillity::TextWithVerticalSeparator( "" , LABEL_WIDTH );
	ImGui::SameLine();
	ImGui::BeginDisabled( !usePort );
	ImGui::SetNextItemWidth( CONTENT_WIDTH );
	ImGui::InputInt( "##Port" , &port , 0 );
	ImGui::EndDisabled();

	ImVec2 buttonSize( 70.0f , 30.0f );
	if ( ImGui::Button( "만들기" , buttonSize ) )
	{
		PlayerProfile profile;
		profile.Nickname = nickname;
		if ( GameCore::GameManager.MakeRoom( profile , roomname , port ) )
		{
		}
		else
		{
			GameCore::SetErrorMessage( "방 생성 실패" , Utillity::WCharToString( Debug::Log::GetLastMessage() ).c_str() );
			GameCore::MasterWindow->ChangeFrame( CMasterWindow::FRAME_ERROR );
		}
		c.IsOpen = false;
	}
	ImGui::SameLine();
	if ( ImGui::Button( "취소" , buttonSize ) )
	{
		c.IsOpen = false;
	}
}

void TitleView::ShowJoinRoom( ImPopupContext& c )
{
	const float LABEL_WIDTH = 80.0f;
	const float CONTENT_WIDTH = 130.0f;

	static char nickname[ 64 ] = "";
	static char hostIP[ 64 ] = "127.0.0.1";
	static bool usePort = false;
	static int port = 7070;

	ImGuiIO& io = ImGui::GetIO();

	ImText()( "기본 정보" , 0.7f );

	ImGui::Utillity::TextWithVerticalSeparator( "닉네임" , LABEL_WIDTH );
	ImGui::SetNextItemWidth( CONTENT_WIDTH );
	ImGui::InputText( "##Nickname" , nickname , IM_ARRAYSIZE( nickname ) );

	ImGui::Utillity::TextWithVerticalSeparator( "호스트 IP" , LABEL_WIDTH );
	ImGui::SetNextItemWidth( CONTENT_WIDTH );
	ImGui::InputText( "##HostIP" , hostIP , IM_ARRAYSIZE( hostIP ) );

	ImGui::Separator();

	ImText()( "고급 설정" , 0.7f );

	float startX = ImGui::GetCursorPosX();
	ImGui::Checkbox( "##UsePort" , &usePort );
	ImGui::SameLine();
	ImGui::TextUnformatted( "포트" );

	ImGui::SameLine( startX );
	ImGui::Utillity::TextWithVerticalSeparator( "" , LABEL_WIDTH );
	ImGui::SameLine();
	ImGui::BeginDisabled( !usePort );
	ImGui::SetNextItemWidth( CONTENT_WIDTH );
	ImGui::InputInt( "##Port" , &port , 0 );
	ImGui::EndDisabled();

	ImVec2 buttonSize( 70.0f , 30.0f );
	if ( ImGui::Button( "참가하기" , buttonSize ) )
	{
		PlayerProfile profile;
		profile.Nickname = nickname;
		if ( GameCore::GameManager.JoinRoom( profile , hostIP , port ) )
		{
		}
		else
		{
			GameCore::SetErrorMessage( "방 참가 실패" , Utillity::WCharToString( Debug::Log::GetLastMessage()).c_str() );
			GameCore::MasterWindow->ChangeFrame( CMasterWindow::FRAME_ERROR );
		}
		c.IsOpen = false;
		
	}
	ImGui::SameLine();
	if ( ImGui::Button( "취소" , buttonSize ) )
	{
		c.IsOpen = false;
	}
}

void TitleView::ShowMakeFail( ImPopupContext& c , const std::string& errorMsg )
{
	const ImVec2 displaySize = ImGui::GetWindowSize();
	const ImVec2 buttonSize( 70.0f , 30.0f );
	const ImVec2 errorSize = ImGui::CalcTextSize( errorMsg.c_str() );
	const ImVec2 padding = ImGui::GetStyle().FramePadding;

	ImText()( "방 생성에 실패하였습니다." , 1.0f, ImText::Align::Center );
	ImGui::BeginChild( "ErrorMessage" , ImVec2( errorSize.x + padding.x * 2.0f + 5.0f , 0 ) , ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders );
	ImText()( errorMsg.c_str() , 1.0f, ImText::Align::Center );
	ImGui::EndChild();

	ImGui::SetCursorPosX( ( displaySize.x - buttonSize.x ) * 0.5f );
	if ( ImGui::Button( "확인" , buttonSize ) )
	{
		c.IsOpen = false;
	}
}

void TitleView::ShowJoinFail( ImPopupContext& c , const std::string& errorMsg )
{
	const ImVec2 displaySize = ImGui::GetWindowSize();
	const ImVec2 buttonSize( 70.0f , 30.0f );
	const ImVec2 errorSize = ImGui::CalcTextSize( errorMsg.c_str() );
	const ImVec2 padding = ImGui::GetStyle().FramePadding;

	ImText()( "방 참가에 실패하였습니다." , 1.0f , ImText::Align::Center );
	ImGui::BeginChild( "ErrorMessage" , ImVec2( errorSize.x + padding.x * 2.0f + 5.0f, 0 ) , ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders );
	ImText()( errorMsg.c_str() , 1.0f , ImText::Align::Center );
	ImGui::EndChild();

	ImGui::SetCursorPosX( ( displaySize.x - buttonSize.x ) * 0.5f );
	if ( ImGui::Button( "확인" , buttonSize ) )
	{
		c.IsOpen = false;
	}
}
