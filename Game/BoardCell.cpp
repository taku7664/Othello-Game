#include "pch.h"
#include "BoardCell.h"

CBoardCell::CBoardCell( size_t row , size_t col , ColorType color )
	: m_row( row ) , m_col( col ) , m_color( color )
{
}

void CBoardCell::SetColor( ColorType color )
{
	m_color = color;
}

ColorType CBoardCell::GetColor() const
{
	return m_color;
}

size_t CBoardCell::GetRow() const
{
	return m_row;
}

size_t CBoardCell::GetCol() const
{
	return m_col;
}

bool CBoardCell::BeginButton( ImVec2 size )
{
	ImGui::Utillity::StyleBuilder styleBuilder;

	const float rounding = 3.0f;
	const ImU32 defaultColor = IM_COL32( 40 , 120 , 40 , 255 );
	const ImU32 hoveredColor = (m_color == ColorType::None) ? IM_COL32( 100 , 200 , 100 , 255 ) : IM_COL32( 160 , 60 , 60 , 255 );
	const ImU32 activeColor  = (m_color == ColorType::None) ? IM_COL32( 20 , 100 , 20 , 255 ) : IM_COL32( 120 , 40 , 40 , 255 );
	styleBuilder.PushStyleVar( ImGuiStyleVar_FrameRounding , rounding );
	styleBuilder.PushStyleColor( ImGuiCol_Button , defaultColor );
	styleBuilder.PushStyleColor( ImGuiCol_ButtonHovered , hoveredColor );
	styleBuilder.PushStyleColor( ImGuiCol_ButtonActive , hoveredColor );

	const bool clicked = ImGui::Button( "##cell" , size );

	return clicked;
}

bool CBoardCell::BeginStone( ImVec2 center , float radius )
{
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	switch ( m_color )
	{
	case ColorType::None:
		break;
	case ColorType::Black:
		drawList->AddCircleFilled( center , radius , IM_COL32( 20 , 20 , 20 , 255 ) );
		break;
	case ColorType::White:
		drawList->AddCircleFilled( center , radius , IM_COL32( 240 , 240 , 240 , 255 ) );
		drawList->AddCircle( center , radius , IM_COL32( 60 , 60 , 60 , 255 ) , 0 , 2.0f );
		break;
	default:
		break;
	}
	return false;
}

bool CBoardCell::Show( float cellSize )
{
	ImGui::PushID(this);

	ImVec2 screenPos = ImGui::GetCursorScreenPos();
	ImVec2 buttonSize(cellSize, cellSize);
	bool clicked = BeginButton(buttonSize);

	ImVec2 center(screenPos + (buttonSize * 0.5f));
	float radius = (cellSize * 0.5f) - 6.0f;
	BeginStone(center, radius);

	ImGui::PopID();

	return clicked;
}
