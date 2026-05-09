#pragma once

class CImWindow;

class CImGuiManager final
{
public:
	CImGuiManager();
	~CImGuiManager();
	CImGuiManager(const CImGuiManager&) = delete;
	CImGuiManager& operator=(const CImGuiManager&) = delete;
	
public:
	bool Initialize();
	void Finalize();
	void Update();

public:
	template<typename T>
	ImGuiID		CreateImWindow(const char* key, ImGuiID parentId = 0);
	bool		DestroyImWindow(ImGuiID id);
	IImWindow*	FindImWindow(ImGuiID id);

	void OpenPopup(const ImPopupDesc& desc);

private:
	std::unordered_map<ImGuiID, std::unique_ptr<CImWindow>> m_imWindowTable;
	std::vector<CImWindow*>		m_imWindowVector;
	std::queue<CImPopupWindow>	m_imPopupWindowQueue;
};

template<typename T>
inline ImGuiID CImGuiManager::CreateImWindow(const char* key, ImGuiID parentId)
{
	if(nullptr == key)
	{
		return 0;
	}

	ImGuiID hashedID = ImHashStr(key);
	if(false == m_imWindowTable.contains(hashedID))
	{
		m_imWindowTable[hashedID] = std::make_unique<T>(hashedID , parentId);
		if(CImDockWindow* parent = dynamic_cast<CImDockWindow*>(FindImWindow(parentId)))
		{
			parent->AddChildImWindow(m_imWindowTable[hashedID].get());
		}
		m_imWindowTable[hashedID]->Initialize();
		m_imWindowVector.push_back(m_imWindowTable[hashedID].get());
		return m_imWindowTable[hashedID]->GetID();
	}
	return 0;
}
