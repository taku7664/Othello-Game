#pragma once

class IGameObject
{
protected:
	IGameObject() = default;
public:
	virtual ~IGameObject() = default;

public:
	virtual void Update() = 0;
};