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
	bool DrawGameBoard(IGameBoard& board, size_t& clickedRow, size_t& clickedCol);
	void DrawRoomSetting(bool isHost);

	void ShowChat();
	void DrawPlayerList();
	void DrawPlayerPopup(IPlayer* player);
	void DrawChatBoard();

private:
	std::vector<std::string> m_chatMessages;
};
