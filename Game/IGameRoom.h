#pragma once
#include "IPlayer.h"
#include "IGameBoard.h"
#include "IGameObject.h"

class IGameRoom : public IGameObject
{
protected:
	IGameRoom() = default;
public:
	virtual ~IGameRoom() = default;

private:
	virtual void Update() = 0;

public:

	/// <summary> 게임 방의 타이틀을 변경합니다. </summary>
	virtual void SetRoomTitle(const char* title, bool dirty = true) = 0;
	/// <summary> 게임 방의 타이틀을 반환합니다. </summary>
	virtual const std::string& GetRoomTitle() const = 0;

	/// <summary> 게임 방의 세팅을 설정합니다. </summary>
	virtual void SetRoomSetting(const RoomSetting& setting, bool dirty = true) = 0;
	/// <summary> 게임 방의 세팅을 반환합니다. </summary>
	virtual const RoomSetting& GetRoomSetting() const = 0;

	/// <summary> 게임 방의 상태를 반환합니다. </summary>
	virtual RoomState GetRoomState() const = 0;

	/// <summary> 게임이 시작 가능한 상태인지 여부를 반환합니다. </summary>
	virtual bool CanStartGame() const = 0;

	/// <summary> 현재 게임 방의 플레이어 수를 반환합니다. </summary>
	virtual size_t GetCurrentPlayerCount() const = 0;
	/// <summary> 게임 방의 플레이어를 인덱스를 통해 반환합니다. </summary>
	virtual IPlayer* GetPlayerFromIndex(size_t index) const = 0;
	/// <summary> 게임 방의 플레이어를 ID를 통해 반환합니다. </summary>
	virtual IPlayer* GetPlayerFromId(int id) const = 0;
	/// <summary> 게임 방의 플레이어를 GUID를 통해 반환합니다. </summary>
	virtual IPlayer* GetPlayerFromGuid(GUID guid) const = 0;
	/// <summary> 게임 방의 로컬 플레이어를 반환합니다. </summary>
	virtual IPlayer* GetLocalPlayer() const = 0;
	/// <summary> 게임 방의 호스트 플레이어를 반환합니다. </summary>
	virtual IPlayer* GetHostPlayer() const = 0;

	/// <summary> 게임 방의 보드를 반환합니다. </summary>
	virtual IGameBoard& GetGameBoard() = 0;

	virtual IPlayer* AddPlayer(const PlayerDesc& data) = 0;
	virtual void	 RemovePlayer(GUID guid) = 0;
};

