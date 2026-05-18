#pragma once

class IGameBoard
{
protected:
	IGameBoard() = default;
	virtual ~IGameBoard() = default;

public:
	/// <summary>
	/// 보드를 렌더링합니다.
	/// </summary>
	virtual void Show(float cellSize = 48.0f) = 0;

	/// <summary>
	/// 주어진 크기만큼 보드를 생성합니다.
	/// </summary>
	/// <param name="raws">생성할 보드의 x축 크기</param>
	/// <param name="cols">생성할 보드의 y축 크기</param>
	virtual void Resize(size_t raws, size_t cols) = 0;

	/// <summary>
	/// 보드에 놓인 돌을 모두 제거합니다.
	/// </summary>
	virtual void Clear(ColorType color = ColorType::None) = 0;

	/// <summary>
	/// 오셀로 시작 배치를 적용합니다.
	/// </summary>
	virtual void InitializeOthelloBoard() = 0;

	/// <summary>
	/// 유효한 좌표인지 반환합니다.
	/// </summary>
	/// <param name="row">보드의 x좌표</param>
	/// <param name="col">보드의 y좌표</param>
	virtual bool IsValidCoord( size_t row , size_t col ) const = 0;

	/// <summary>
	/// 해당 좌표에 돌이 존재하는지 여부를 반환합니다.
	/// </summary>
	/// <param name="x">보드의 x좌표</param>
	/// <param name="y">보드의 y좌표</param>
	virtual bool IsExistStone(size_t row, size_t col) const = 0;

	/// <summary>
	/// 해당 좌표에 돌을 놓습니다. 돌이 정상적으로 놓인 경우 true를 반환합니다.
	/// </summary>
	/// <param name="x">놓은 보드의 x좌표</param>
	/// <param name="y">놓을 보드의 y좌표</param>
	/// <param name="color">돌의 색깔</param>
	virtual bool PlaceStone(ColorType color, size_t raw, size_t col) = 0;

	/// <summary>
	/// 해당 좌표의 색상을 직접 설정합니다.
	/// </summary>
	virtual bool SetCellColor(size_t row, size_t col, ColorType color) = 0;

	/// <summary>
	/// 해당 좌표에 돌을 제거합니다. 돌이 정상적으로 제거된 경우 true를 반환합니다.
	/// </summary>
	virtual bool RemoveStone(size_t row, size_t col) = 0;

	/// <summary> 보드의 x크기를 반환합니다. </summary>
	virtual size_t GetBoardRows() const = 0;
	/// <summary> 보드의 y크기를 반환합니다. </summary>
	virtual size_t GetBoardCols() const = 0;
	/// <summary> 보드에 놓인 돌의 갯수를 반환합니다. </summary>
	virtual size_t GetStoneCount() const = 0;

	virtual const CBoardCell& GetCell(size_t row, size_t col) const = 0;
	virtual ColorType GetCellColor(size_t row, size_t col) const = 0;
};
