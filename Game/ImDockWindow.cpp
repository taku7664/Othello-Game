#include "pch.h"
#include "ImDockWindow.h"

CImDockWindow::CImDockWindow(ImGuiID id, ImGuiID parentId)
	: CImWindow(id, parentId)
	, m_bNeedRebuildDockLayout(true)
{
	m_mainDockID = ImHashData(this, sizeof(void*), parentId);
	m_imWndClass.ClassId = m_mainDockID;
	m_imWndClass.DockingAllowUnclassed = false;
	m_imWndClass.DockingAlwaysTabBar = false;
}

CImDockWindow::~CImDockWindow()
{
	std::vector<ImGuiID> childIds(m_childImWindowVector.size());
	for(auto& childWnd : m_childImWindowVector)
	{
		if(childWnd)
		{
			childIds.push_back(childWnd->GetID());
		}
	}
	for (auto& childId : childIds)
	{
		GameCore::ImGuiManager.DestroyImWindow(childId);
	}
}

void CImDockWindow::SetDockLayout(ImGuiDir dir, float splitRatio)
{
	m_splitRatio[dir] = splitRatio;
}

BitFlag& CImDockWindow::GetImGuiDockFlags()
{
	return m_imguiDockFlags;
}

BitFlag& CImDockWindow::GetCustomDockFlags()
{
	return m_customDockFlags;
}

bool CImDockWindow::AddChildImWindow(CImWindow* child)
{
	if (nullptr == child)
	{
		return false;
	}
	if(nullptr == FindChildImWindow(child->GetID()))
	{
		m_childImWindowVector.push_back(child);
		ImGuiDir dir = child->GetInitDockLayout();
		child->m_ownerWindow = this;
		return true;
	}
	return false;
}

void CImDockWindow::RemoveChildImWindow(ImGuiID id)
{
	m_childImWindowVector.erase( std::remove_if(
		m_childImWindowVector.begin(),
		m_childImWindowVector.end(),
		[id] (CImWindow* a) {
			return a->GetID() == id;
		}),
		m_childImWindowVector.end()
	);
}

IImWindow* CImDockWindow::FindChildImWindow(ImGuiID id)
{
	for (auto pWnd : m_childImWindowVector)
	{
		if(pWnd->GetID() == id)
		{
			return pWnd;
		}
	}
	return nullptr;
}

void CImDockWindow::OnPreBegin()
{
	PushDockStyle();
}

void CImDockWindow::OnPostBegin()
{
	bool isBeginDockBuild = false;
	isBeginDockBuild = BeginBuildDockLayout();

	SubmitDockSpace();
	PopDockStyle();

	for(int i = 0; i < m_childImWindowVector.size(); ++i)
	{
		if(CImWindow* childWnd = m_childImWindowVector[i])
		{
			if(isBeginDockBuild)
			{
				const char* label = childWnd->GetTitle();
				ImGuiID splitID = m_splitedID[childWnd->m_initDockLayoutDirection];
				ImGui::DockBuilderDockWindow(label , splitID);
			}
			childWnd->Update();
		}
	}

	if (isBeginDockBuild)
	{
		EndBuildDockLayout();
	}
}

void CImDockWindow::OnPostEnd()
{
}

void CImDockWindow::SubmitDockSpace()
{
	//////////////////////////////////////////
	// Sumit the DockSpace
	//////////////////////////////////////////
	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();
	float       minWinSizeX = style.WindowMinSize.x;
	style.WindowMinSize.x = 370.0f;
	if ( io.ConfigFlags & ImGuiConfigFlags_DockingEnable )
	{
		ImGui::DockSpace(m_mainDockID , ImVec2(0.0f , 0.0f) , m_imguiDockFlags.operator int(), &m_imWndClass);
	}
	style.WindowMinSize.x = minWinSizeX;
}

bool CImDockWindow::BeginBuildDockLayout()
{
	if (true == m_bNeedRebuildDockLayout)
    {
		m_bNeedRebuildDockLayout = false;
        ImGui::DockBuilderRemoveNode(m_mainDockID);
        ImGui::DockBuilderAddNode(m_mainDockID, m_imguiDockFlags.operator int());
        ImGuiID mainId = m_mainDockID;

		for(int i = 0; i < ImGuiDir_COUNT; ++i)
		{
			if(m_splitRatio[i] > 0.0f)
			{
				ImGuiID id = ImGui::DockBuilderSplitNode(
					mainId,
					(ImGuiDir)i,
					m_splitRatio[i],
					NULL,
					&mainId
				);
				m_splitedID[i] = id;
			}
		}
		m_mainSplitedID = mainId;
        return true;
    }
    return false;
}

void CImDockWindow::EndBuildDockLayout()
{
	ImGui::DockBuilderFinish(m_mainDockID);
}

void CImDockWindow::PushDockStyle()
{
	if (m_customDockFlags.Has(IMDOCKWINDOW_FLAG_FULLSCREEN))
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos( viewport->Pos );
		ImGui::SetNextWindowSize( viewport->Size );
		ImGui::SetNextWindowViewport( viewport->ID );
		m_dockStyleBuilder.PushStyleVar(ImGuiStyleVar_WindowRounding , 0.0f);
		m_dockStyleBuilder.PushStyleVar(ImGuiStyleVar_WindowBorderSize , 0.0f);
	}
	if (m_customDockFlags.Has(IMDOCKWINDOW_FLAG_PADDING))
	{
		m_dockStyleBuilder.PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f , 0.0f));
	}
}

void CImDockWindow::PopDockStyle()
{
	m_dockStyleBuilder.PopStyle();
}
