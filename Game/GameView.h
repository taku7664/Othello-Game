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
	void DrawSettingBar();

	void ShowChat();
	void DrawPlayerList();
	void DrawChatBoard();

private:
	std::vector<std::string> m_chatMessages;
};

