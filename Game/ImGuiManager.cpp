#include "pch.h"
#include "ImGuiManager.h"

CImGuiManager::CImGuiManager()
{
}

CImGuiManager::~CImGuiManager()
{
}

bool CImGuiManager::Initialize()
{
	return true;
}

void CImGuiManager::Finalize()
{
}

void CImGuiManager::Update()
{
	for(CImWindow* wnd : m_imWindowVector)
	{
		if(wnd)
		{
			wnd->Update();
		}
	}
	if (false == m_imPopupWindowQueue.empty())
	{
		CImPopupWindow& popup = m_imPopupWindowQueue.front();
		if(false == popup.Render())
		{
			m_imPopupWindowQueue.pop();
		}
	}
}

bool CImGuiManager::DestroyImWindow(ImGuiID id)
{
	auto it = m_imWindowTable.find(id);
	if (it != m_imWindowTable.end())
	{
		CImWindow* pWnd = it->second.get();
		if (pWnd)
		{
			pWnd->Finalize();
			ImGuiID destID	 = pWnd->GetID();
			ImGuiID parentID = pWnd->GetOwnerID();
			if(CImDockWindow* parent = dynamic_cast<CImDockWindow*>(FindImWindow(parentID)))
			{
				parent->RemoveChildImWindow(destID);
			}
			m_imWindowVector.erase( std::remove_if(
				m_imWindowVector.begin() ,
				m_imWindowVector.end() ,
				[ id ] ( CImWindow* wnd ) {
					return wnd->GetID() == id;
				} ) ,
				m_imWindowVector.end()
			);
		}
		m_imWindowTable.erase(it);
		return true;
	}
	return false;
}

IImWindow* CImGuiManager::FindImWindow(ImGuiID id)
{
	auto it = m_imWindowTable.find(id);
	if (it != m_imWindowTable.end())
	{
		return it->second.get();
	}
	return nullptr;
}

void CImGuiManager::OpenPopup(const ImPopupContext* context, std::function<void(ImPopupContext&)> func)
{
	if (nullptr == func)
	{
		return;
	}
	if (context)
	{
		CImPopupWindow popup = CImPopupWindow(*context, func);
		m_imPopupWindowQueue.push(std::move(popup));
	}
	else
	{
		CImPopupWindow popup = CImPopupWindow(func);
		m_imPopupWindowQueue.push(std::move(popup));
	}
}
