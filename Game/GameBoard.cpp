#include "pch.h"
#include "GameBoard.h"

CGameBoard::CGameBoard( size_t raws , size_t cols )
{
	Resize( raws , cols );
}

void CGameBoard::Resize( size_t raws , size_t cols )
{
	m_rows = raws;
	m_cols = cols;
	m_cells.clear();
	m_cells.resize(m_rows);
	for (auto& row : m_cells)
	{
		row.resize(m_cols);
	}
	m_stoneCount = 0;
}

void CGameBoard::Clear(ColorType color)
{
	for ( size_t r = 0; r < m_rows; ++r )
	{
		for ( size_t c = 0; c < m_cols; ++c )
		{
			m_cells[ r ][ c ].SetColor( color );
		}
	}
	m_stoneCount = 0;
}

bool CGameBoard::IsExistStone( size_t row , size_t col ) const
{
	if ( IsValidCoord( row , col ) )
	{
		const CBoardCell& cell = GetCell( row , col );
		const ColorType color = cell.GetColor();
		return color != ColorType::None;
	}
	return false;
}

bool CGameBoard::IsValidCoord( size_t row , size_t col ) const
{
	return row >= 0 && row < m_rows && col >= 0 && col < m_cols;
}

bool CGameBoard::PlaceStone( ColorType color , size_t row , size_t col )
{
	if ( IsValidCoord( row , col ) )
	{
		CBoardCell& cell = m_cells[ row ][ col ];
		cell.SetColor( color );
		++m_stoneCount;
		// TODO: 돌이 놓인 위치를 저장해서 나중에 제거할 때 활용하기
		return true;
	}
	return false;
}

bool CGameBoard::RemoveStone( size_t row , size_t col )
{
	if ( IsValidCoord( row , col ) )
	{
		CBoardCell& cell = m_cells[ row ][ col ];
		cell.SetColor( ColorType::None );
		--m_stoneCount;
		return true;
	}
	return false;
}

size_t CGameBoard::GetBoardRows() const
{
	return m_rows;
}

size_t CGameBoard::GetBoardCols() const
{
	return m_cols;
}

size_t CGameBoard::GetStoneCount() const
{
	return size_t();
}

const CBoardCell& CGameBoard::GetCell( size_t row , size_t col ) const
{
	return m_cells[ row ][ col ];
}

void CGameBoard::Show(float cellSize)
{
	ImGui::Utillity::StyleBuilder styleBuilder;
	styleBuilder.PushStyleVar( ImGuiStyleVar_ItemSpacing , ImVec2( 4 , 4 ) );
	styleBuilder.PushStyleVar( ImGuiStyleVar_FramePadding , ImVec2( 0 , 0 ) );
	for ( int r = 0; r < m_rows; ++r )
	{
		for ( int c = 0; c < m_cols; ++c )
		{
			if ( m_cells[ r ][ c ].Show( cellSize ) )
			{
				if (GameCore::ClientServer)
				{
					Packet::C2S_PlaceStone packet;
					packet.Color = GameCore::GetLocalPlayer() ? GameCore::GetLocalPlayer()->GetColorType() : ColorType::None;
					packet.Guid = GameCore::GetLocalPlayer() ? GameCore::GetLocalPlayer()->GetGUID() : GUID_NULL;
					packet.Row = r;
					packet.Col = c;
					GameCore::ClientServer->SendPacketToServer<Packet::C2S_PlaceStone>(packet);
				}
				else
				{
					PlaceStone(GameCore::GetLocalPlayer()->GetColorType() , r , c);
				}
			}
			if ( c != m_cols - 1 )
				ImGui::SameLine();
		}
	}
}
