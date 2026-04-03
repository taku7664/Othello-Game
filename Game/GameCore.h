#pragma once


struct GameCore
{
	inline static TimeManager		Time;
	inline static CGameManager		GameManager;
	inline static ServerManager		GameServer;
	inline static RenderManager     Renderer;
	inline static CImGuiManager     ImGuiManager;
	inline static CommandManager	CommandAction;
	inline static CChatManager		ChatManager;

	inline static IGameRoom*		ActiveRoom = nullptr;
	inline static CClientNetwork*	ClientServer = nullptr;
	inline static CHostNetwork*		HostServer = nullptr;

	inline static CMasterWindow* 	MasterWindow = nullptr;

	inline static std::string		ErrorTitle;
	inline static std::string		ErrorMessage;

	inline static IPlayer* GetLocalPlayer()
	{
		return ActiveRoom ? ActiveRoom->GetLocalPlayer() : nullptr;
	}
	inline static IPlayer* GetHostPlayer()
	{
		return ActiveRoom ? ActiveRoom->GetHostPlayer() : nullptr;
	}
	inline static IPlayer* GetPlayerFromGuid(GUID guid)
	{
		return ActiveRoom ? ActiveRoom->GetPlayerFromGuid(guid) : nullptr;
	}
	inline static IPlayer* GetPlayerFromConnectionID( int id )
	{
		return ActiveRoom ? ActiveRoom->GetPlayerFromId( id ) : nullptr;
	}

	inline static void SetErrorMessage(const char* title, const char* message)
	{
		ErrorTitle = title ? title : "";
		ErrorMessage = "원인: ";
		ErrorMessage += message ? message : "";
	}
};