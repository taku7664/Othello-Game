#pragma once
class CBoardCell
{
public:
	CBoardCell() = default;
	CBoardCell( size_t row , size_t col , ColorType color = ColorType::None );
	~CBoardCell() = default;

public:
	// true를 반환하면 해당 칸이 클릭된 것
	bool Show( float cellSize = 48.0f );

	void		SetColor( ColorType color );
	ColorType	GetColor() const;
	size_t		GetRow() const;
	size_t		GetCol() const;

private:
	bool BeginButton(ImVec2 size);
	bool BeginStone(ImVec2 center, float radius);

private:
	size_t m_row = 0;
	size_t m_col = 0;
	ColorType m_color = ColorType::None;
};

