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
	m_cells.resize( m_rows );
	for ( size_t r = 0; r < m_rows; ++r )
	{
		m_cells[ r ].reserve( m_cols );
		for ( size_t c = 0; c < m_cols; ++c )
		{
			m_cells[ r ].emplace_back( r , c );
		}
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
	m_stoneCount = ( color == ColorType::None ) ? 0 : m_rows * m_cols;
}

void CGameBoard::InitializeOthelloBoard()
{
	Clear();
	if ( m_rows < 2 || m_cols < 2 )
	{
		return;
	}

	const size_t upperRow = ( m_rows / 2 ) - 1;
	const size_t lowerRow = m_rows / 2;
	const size_t leftCol = ( m_cols / 2 ) - 1;
	const size_t rightCol = m_cols / 2;

	SetCellColor( upperRow , leftCol , ColorType::White );
	SetCellColor( lowerRow , rightCol , ColorType::White );
	SetCellColor( upperRow , rightCol , ColorType::Black );
	SetCellColor( lowerRow , leftCol , ColorType::Black );
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
	return row < m_rows && col < m_cols;
}

bool CGameBoard::PlaceStone( ColorType color , size_t row , size_t col )
{
	if ( color == ColorType::None || false == IsValidCoord( row , col ) )
	{
		return false;
	}

	CBoardCell& cell = m_cells[ row ][ col ];
	if ( cell.GetColor() != ColorType::None )
	{
		return false;
	}

	cell.SetColor( color );
	++m_stoneCount;
	// TODO: 돌이 놓인 위치를 저장해서 나중에 제거할 때 활용하기
	return true;
}

bool CGameBoard::SetCellColor( size_t row , size_t col , ColorType color )
{
	if ( false == IsValidCoord( row , col ) )
	{
		return false;
	}

	CBoardCell& cell = m_cells[ row ][ col ];
	const ColorType prevColor = cell.GetColor();
	if ( prevColor == color )
	{
		return true;
	}

	if ( prevColor == ColorType::None && color != ColorType::None )
	{
		++m_stoneCount;
	}
	else if ( prevColor != ColorType::None && color == ColorType::None )
	{
		--m_stoneCount;
	}

	cell.SetColor( color );
	return true;
}

bool CGameBoard::RemoveStone( size_t row , size_t col )
{
	if ( false == IsValidCoord( row , col ) )
	{
		return false;
	}

	CBoardCell& cell = m_cells[ row ][ col ];
	if ( cell.GetColor() == ColorType::None )
	{
		return false;
	}

	cell.SetColor( ColorType::None );
	--m_stoneCount;
	return true;
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
	return m_stoneCount;
}

const CBoardCell& CGameBoard::GetCell( size_t row , size_t col ) const
{
	return m_cells[ row ][ col ];
}

ColorType CGameBoard::GetCellColor( size_t row , size_t col ) const
{
	if ( false == IsValidCoord( row , col ) )
	{
		return ColorType::None;
	}
	return m_cells[ row ][ col ].GetColor();
}

void CGameBoard::Show(float cellSize)
{
	ImGui::Utillity::StyleBuilder styleBuilder;
	styleBuilder.PushStyleVar( ImGuiStyleVar_ItemSpacing , ImVec2( 4 , 4 ) );
	styleBuilder.PushStyleVar( ImGuiStyleVar_FramePadding , ImVec2( 0 , 0 ) );
	for ( size_t r = 0; r < m_rows; ++r )
	{
		for ( size_t c = 0; c < m_cols; ++c )
		{
			m_cells[ r ][ c ].Show( cellSize );
			if ( c != m_cols - 1 )
				ImGui::SameLine();
		}
	}
}
