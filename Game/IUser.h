#pragma once
#include "IGameObject.h"

class IUser : public IGameObject
{
protected:
	IUser() = default;
	~IUser() = default;

private:
	virtual void Update() override = 0;

public:
	/// <summary> 커넥션ID를 반환합니다. </summary>
	virtual int GetConnectionID() const = 0;
	virtual GUID GetGUID() const = 0;

	/// <summary> 유저의 닉네임을 설정합니다.(와이드 문자열) </summary>
	virtual void SetNickName(const char* nickname) = 0;

	/// <summary> 유저의 닉네임을 반환합니다.(와이드 문자열) </summary>
	virtual const std::string& GetNickName() const = 0;

	/// <summary> 채팅 메세지를 서버에 전달합니다. </summary>
	virtual void SendChatMessage(const char* message) = 0;
};

