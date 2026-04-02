#pragma once

class CMasterWindow : public CImWindow
{
	struct ViewData
	{
		std::unique_ptr<CView>	View;
		CView::Context			Context;
	};
public:
	enum FrameType
	{
		FRAME_TITLE,
		FRAME_ERROR,
		FRAME_GAME,
	};
public:
	using CImWindow::CImWindow;
	virtual ~CMasterWindow() = default;

public:
	void ChangeFrame( FrameType frame );

private:
	void OnUpdate() override;
	void OnCreate() override;
	void OnDestroy() override;
	void OnPreBegin() override;
	void OnPostBegin() override;
	void OnRenderStay() override;

private:
	void ShowMenuBar();
	void ShowFrame(FrameType frame, const ImVec2& cursorPos, const ImVec2& frameSize);
	float GetFrameTransitionEaseFactor();

private:
	FrameType m_curFrame = FRAME_TITLE;
	FrameType m_prevFrame = FRAME_TITLE;
	float m_frameTransitionTime = 0.9f;
	float m_frameTransitionTick = 0.0f;
	ImGui::Utillity::StyleBuilder m_styleBuidler;

	std::unordered_map<FrameType, ViewData> m_viewTable;
};
