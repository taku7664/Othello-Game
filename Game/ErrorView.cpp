#include "pch.h"
#include "ErrorView.h"

ErrorView::ErrorView()
{
}

ErrorView::~ErrorView()
{
}

void ErrorView::OnVisible()
{
	GameCore::Renderer.SetWindowTitle(L"오셀로 게임");
}

void ErrorView::OnHide()
{
	GameCore::SetErrorMessage( nullptr , nullptr );
}

void ErrorView::OnShow( Context& context )
{
	ImGuiIO& io = ImGui::GetIO();
	const ImVec2 windowSize = ImGui::GetWindowSize();
	const ImVec2 buttonSize( windowSize.x * 0.6f , 40.0f );
	const ImVec2 dummySize( 0.0f , 20.0f );
	const ImVec2 titleSize = ImGui::CalcTextSize( GameCore::ErrorTitle.c_str() );

	ImGui::SetCursorPosY( windowSize.y * 0.3f );
	ImText()( GameCore::ErrorTitle.c_str() , 1.5f , ImText::Align::Center );

	ImGui::Dummy( dummySize );
	ImGui::BeginChild( "ErrorMessage" , ImVec2( 0 , 0 ) , ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders );
	ImText()( GameCore::ErrorMessage.c_str() , 1.0f , ImText::Align::Center );
	ImGui::EndChild();

	ImGui::Dummy( dummySize );
	ImGui::SetCursorPosX( ( windowSize.x - buttonSize.x ) * 0.5f );
	if ( ImGui::Button( "타이틀 화면으로 가기" , buttonSize ) )
	{
		GameCore::MasterWindow->ChangeFrame( CMasterWindow::FRAME_TITLE );
	}
}