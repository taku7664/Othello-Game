#include "pch.h"
#include "TimeManager.h"

TimeManager::TimeManager()
	: m_frameCount(0)
	, m_frameTime(0.f)
	, m_elapsedTime(0.f)
	, m_fps(0)
	, m_timeScale(1.f)
	, m_maximumDeltatime(0.5f)
	, m_deltaTime(0.f)
{
	QueryPerformanceCounter(&m_preTime);
	QueryPerformanceFrequency(&m_frequency);
}

TimeManager::~TimeManager()
{
}

void TimeManager::Clear()
{
	m_frameCount = 0;
	m_frameTime = 0.f;
	m_elapsedTime = 0.f;
	m_fps = 0;
	m_timeScale = 1.f;
	m_maximumDeltatime = 0.5f;
	m_deltaTime = 0.f;
	QueryPerformanceCounter(&m_preTime);
}

void TimeManager::Update()
{
	QueryPerformanceCounter(&m_curTime);
	m_deltaTime =
		static_cast<float>(m_curTime.QuadPart - m_preTime.QuadPart)
		/ static_cast<float>(m_frequency.QuadPart);

	m_preTime		 = m_curTime;
	m_frameTime		+= m_deltaTime;
	m_elapsedTime	+= m_deltaTime;

	++m_frameCount;
	m_deltaTime = min(m_deltaTime, m_maximumDeltatime);

	if (m_frameTime >= 1.f)
	{
		m_fps = (int)(m_frameCount / m_frameTime);
		m_frameTime = 0.f;
		m_frameCount = 0;
	}
}