#pragma once

class TimeManager final
{
public:
	TimeManager();
	~TimeManager();
	TimeManager(const TimeManager&) = delete;
	TimeManager& operator=(const TimeManager&) = delete;

public:
	void	Clear();
	void	Update();

	float	GetScaledDeltaTime() { return m_deltaTime * m_timeScale; }
	float	GetUnScaledDeltaTime() { return m_deltaTime; }
	float	GetTimeScale() { return m_timeScale; }
	float	GetMaximumDeltaTime() { return m_maximumDeltatime; }
	float	GetElapsedTime() { return m_elapsedTime; }
	int		GetFps() { return m_fps; }

	void	SetTimeScale(float _val) { m_timeScale = _val; }
	void	SetMaximumDeltaTime(float _maximumDt) { m_maximumDeltatime = _maximumDt; }

private:
	LARGE_INTEGER		m_preTime;
	LARGE_INTEGER		m_curTime;
	LARGE_INTEGER		m_frequency;
	int					m_frameCount;
	float				m_frameTime;
	float				m_elapsedTime;
	int					m_fps;

	float				m_timeScale;
	float				m_maximumDeltatime;
	float				m_deltaTime;
};

