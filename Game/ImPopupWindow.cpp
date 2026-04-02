#include "pch.h"
#include "ImPopupWindow.h"

CImPopupWindow::CImPopupWindow(const ImPopupContext& context, std::function<void(ImPopupContext&)> func)
	: m_context(context)
	, m_renderFunc(func)
	, m_bIsRendered(false)
{
}

CImPopupWindow::CImPopupWindow(std::function<void(ImPopupContext&)> func)
	: m_context()
	, m_renderFunc(func)
	, m_bIsRendered(false)
{
}

CImPopupWindow::~CImPopupWindow()
{
}

bool CImPopupWindow::Render()
{
	if (false == m_bIsRendered)
	{
		ImGui::OpenPopup(m_context.Title.c_str());
	}
	if (false == (m_context.Flags & ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::SetNextWindowSize(m_context.Size);
    }
	if (ImGui::BeginPopupModal(m_context.Title.c_str(), &m_context.IsOpen, m_context.Flags))
    {
        if (m_renderFunc)
		{
			m_renderFunc(m_context);
		}
        ImGui::EndPopup();
    }
	if ( false == m_context.IsOpen )
	{
		return false;
	}
	m_bIsRendered = true;
	return true;
}