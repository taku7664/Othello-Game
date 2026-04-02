#pragma once
#include "IGameBoard.h"

class CGameBoard : public IGameBoard
{
public:
	CGameBoard() = default;
	CGameBoard(size_t row, size_t cols);
	~CGameBoard() = default;

public:
	void Show(float cellSize = 48.0f) override;
	void Resize(size_t row, size_t cols) override;
	void Clear(ColorType color = ColorType::None) override;
	bool IsExistStone(size_t row, size_t col) const override;
	bool IsValidCoord(size_t row, size_t col) const override;
	bool PlaceStone(ColorType color, size_t raw, size_t col) override;
	bool RemoveStone(size_t row, size_t col) override;

	size_t GetBoardRows() const override;
	size_t GetBoardCols() const override;
	size_t GetStoneCount() const override;
	const CBoardCell& GetCell(size_t row, size_t col) const override;
private:
	size_t m_rows = 0;
	size_t m_cols = 0;
	size_t m_stoneCount = 0;
	std::vector<std::vector<CBoardCell>> m_cells;
};

