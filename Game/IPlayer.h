#pragma once
#include "IUser.h"

class IPlayer : public IUser
{
protected:
	IPlayer() = default;
public:
	virtual ~IPlayer() = default;

public:
	/// <summary> 호스트 여부를 반환합니다. </summary>
	virtual bool IsHost() const = 0;
	/// <summary> 로컬 플레이어 여부를 반환합니다. </summary>
	virtual bool IsLocal() const = 0;

	/// <summary> 유저의 닉네임을 설정합니다. </summary>
	virtual void SetNickName(const char* nickname) = 0;
	/// <summary> 유저의 닉네임을 반환합니다. </summary>
	virtual const std::string& GetNickName() const = 0;

	/// <summary> 플레이어의 색깔을 반환합니다. </summary>
	virtual void SetColorType(ColorType color) = 0;
	/// <summary> 플레이어의 색깔을 반환합니다. </summary>
	virtual ColorType GetColorType() const = 0;

	/// <summary> 플레이어의 준비 상태를 설정합니다. </summary>
	virtual void SetReady(bool isReady) = 0;
	/// <summary> 플레이어의 준비 상태를 반환합니다. </summary>
	virtual bool IsReady() = 0;
	/// <summary> 플레이어의 게임 시작 투표 상태를 설정합니다. </summary>
	virtual void SetVoteState(VoteState voteState) = 0;
	/// <summary> 플레이어의 게임 시작 투표 상태를 반환합니다. </summary>
	virtual VoteState GetVoteState() = 0;
};
