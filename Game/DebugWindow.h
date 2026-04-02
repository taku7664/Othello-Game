#pragma once

class CDebugWindow : public CImWindow
{
public:
	using CImWindow::CImWindow;
	virtual ~CDebugWindow() = default;

private:
	void OnCreate() override;
	void OnRenderStay() override;
};

