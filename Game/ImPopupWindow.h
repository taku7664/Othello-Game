#pragma once

class CImPopupWindow
{
public:
	CImPopupWindow(const ImPopupContext& context, std::function<void(ImPopupContext&)> func);
	CImPopupWindow(std::function<void(ImPopupContext&)> func);
	~CImPopupWindow();

public:
	bool Render();

private:
	ImPopupContext m_context;
	std::function<void(ImPopupContext&)> m_renderFunc;
	bool m_bIsRendered;
};

