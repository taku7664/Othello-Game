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

	/// <summary> 플레이어의 색깔을 반환합니다. </summary>
	virtual void SetColorType(ColorType color) = 0;
	/// <summary> 플레이어의 색깔을 반환합니다. </summary>
	virtual ColorType GetColorType() const = 0;

};

