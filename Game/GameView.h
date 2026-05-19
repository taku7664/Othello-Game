#pragma once
#include "View.h"

class GameView : public CView
{
public:
	GameView();
	virtual ~GameView();

public:
	void OnShow(Context& context) override;

private:
	void ShowGame();
	void DrawUpperGameBar();
	void DrawMainGameBar();
	void DrawGameStatus();
	bool DrawGameBoard(IGameBoard& board, bool canClick, ColorType previewColor, size_t& clickedRow, size_t& clickedCol);
	ColorType GetOpponentColor(ColorType color) const;
	bool CollectPreviewFlips(IGameBoard& board, ColorType color, size_t row, size_t col, std::vector<Packet::CellChange>& outFlips) const;
	void CollectPreviewLineEnds(IGameBoard& board, ColorType color, size_t row, size_t col, std::vector<size_t>& outEndCellIndices) const;
	void DrawRoomSetting(bool isHost);

	void ShowChat();
	void DrawPlayerList();
	void DrawPlayerPopup(IPlayer* player);
	void DrawChatBoard();

private:
	std::vector<std::string> m_chatMessages;
};
