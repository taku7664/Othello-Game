#include "pch.h"
#include "MasterWindow.h"

void CMasterWindow::OnUpdate()
{
}

void CMasterWindow::OnCreate()
{
	SetTitle( "게임" );

	UINT flags = 
		ImGuiWindowFlags_NoTitleBar | 
		ImGuiWindowFlags_NoCollapse | 
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | 
		ImGuiWindowFlags_NoBringToFrontOnFocus | 
		ImGuiWindowFlags_NoNavFocus |
		ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_NoDocking;
	GetImGuiWindowFlags().Set( flags );

	m_viewTable[ FRAME_TITLE ] = { std::make_unique< TitleView >() , {
		"Title" ,
		ImVec2( 0 , 0 ) ,
		ImGuiChildFlags_None ,
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse ,} 
	};

	m_viewTable[ FRAME_ERROR ] = { std::make_unique< ErrorView >() , {
		"Error" ,
		ImVec2( 0 , 0 ) ,
		ImGuiChildFlags_None ,
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse ,}
	};

	m_viewTable[ FRAME_GAME ] = { std::make_unique< GameView >() , {
		"Game" ,
		ImVec2( 0 , 0 ) ,
		ImGuiChildFlags_None ,
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse ,} 
	};
}

void CMasterWindow::OnDestroy()
{
}

void CMasterWindow::OnPreBegin()
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos( viewport->Pos );
	ImGui::SetNextWindowSize( viewport->Size );
	ImGui::SetNextWindowViewport( viewport->ID );
	m_styleBuidler.PushStyleVar( ImGuiStyleVar_WindowRounding , 0.0f );
	m_styleBuidler.PushStyleVar( ImGuiStyleVar_WindowBorderSize , 0.0f );
}

void CMasterWindow::OnPostBegin()
{
	m_styleBuidler.PopStyle();
}

namespace
{
	float easeInOutBack( float t ) {
		const float c1 = 1.70158f;
		const float c2 = c1 * 1.525f;

		return ( t < 0.5f )
			? ( (float)std::pow( 2.0f * t , 2 ) * ( ( c2 + 1 ) * 2.0f * t - c2 ) ) / 2.0f
			: ( (float)std::pow( 2.0f * t - 2.0f , 2 ) * ( ( c2 + 1 ) * ( t * 2.0f - 2.0f ) + c2 ) + 2.0f ) / 2.0f;
	}
}

void CMasterWindow::OnRenderStay()
{
	if ( false == ImGui::Utillity::IsWindowDrawable() )
	{
		return;
	}

	ShowMenuBar();

	const ImVec2 baseCursorPos = ImGui::GetCursorPos();
	const ImVec2 frameSize = ImGui::GetContentRegionAvail();

	if ( m_frameTransitionTick > 0.0f )
	{
		m_frameTransitionTick -= ImMin(ImGui::GetIO().DeltaTime, 0.1f);
		if ( m_frameTransitionTick < 0.0f )
		{
			m_frameTransitionTick = 0.0f;
			m_viewTable[ m_prevFrame ].View->OnHide();
		}
		float t = 1.0f - ( m_frameTransitionTick / m_frameTransitionTime );
		const float ease = easeInOutBack( t );
		const float width = frameSize.x;

		const int prevIndex = static_cast< int >( m_prevFrame );
		const int curIndex = static_cast< int >( m_curFrame );
		const float direction = ( curIndex >= prevIndex ) ? 1.0f : -1.0f;

		const ImVec2 prevPos( baseCursorPos.x - direction * width * ease , baseCursorPos.y );
		const ImVec2 curPos( baseCursorPos.x + direction * width * ( 1.0f - ease ) , baseCursorPos.y );

		ShowFrame( m_prevFrame , prevPos , frameSize );
		ShowFrame( m_curFrame , curPos , frameSize );
	}
	else
	{
		ShowFrame( m_curFrame , baseCursorPos , frameSize );
	}
}

void CMasterWindow::ShowMenuBar()
{
	if ( ImGui::BeginMenuBar() )
	{
		if(ImGui::BeginMenu( "Test" ) )
		{
			if ( ImGui::MenuItem( "Exit" ) )
			{
				GameCore::GameManager.LeaveRoom();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}

void CMasterWindow::ShowFrame( FrameType frame , const ImVec2& cursorPos , const ImVec2& frameSize )
{
	ViewData& data = m_viewTable[ frame ];
	data.Context.Size = frameSize;
	ImGui::SetCursorPos(cursorPos);
	data.View->Show( data.Context );
}

void CMasterWindow::ChangeFrame( FrameType frame)
{
	if (frame == m_curFrame)
	{
		return;
	}
	m_prevFrame = m_curFrame;
	m_curFrame = frame;
	m_frameTransitionTick = m_frameTransitionTime;
	m_viewTable[m_curFrame].View->OnVisible();
}