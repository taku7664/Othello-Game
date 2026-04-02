#pragma once

class CImDockWindow : public CImWindow
{
public:
	CImDockWindow(ImGuiID id, ImGuiID parentId = 0);
	virtual ~CImDockWindow();

public:
	void SetDockLayout(ImGuiDir dir, float splitRatio);

	BitFlag& GetImGuiDockFlags();
	BitFlag& GetCustomDockFlags();

	bool		AddChildImWindow(CImWindow* child);
	void		RemoveChildImWindow(ImGuiID id);
	IImWindow*  FindChildImWindow(ImGuiID id);

private:
	void OnPreBegin() override;
	void OnPostBegin() override;
	void OnPostEnd() override;

	void SubmitDockSpace();
	bool BeginBuildDockLayout();
	void EndBuildDockLayout();
	void PushDockStyle();
	void PopDockStyle();

protected:
	ImGuiID m_mainDockID;
	ImGuiID m_mainSplitedID;
	ImGuiID m_splitedID[ImGuiDir_COUNT];
	float	m_splitRatio[ImGuiDir_COUNT];

	BitFlag	m_imguiDockFlags;
	BitFlag	m_customDockFlags;
	bool	m_bNeedRebuildDockLayout;

	std::vector<CImWindow*> m_childImWindowVector;
	ImGui::Utillity::StyleBuilder m_dockStyleBuilder;
};
