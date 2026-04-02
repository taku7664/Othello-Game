#include "pch.h"
#include "Log.h"

int Debug::Log::tabSpacing = 0;
std::wstring Debug::Log::lastMessage = L"";
Debug::LogLevel Debug::Log::messageFilterLevel = Debug::LOG_WARNING;

Debug::Log::Log(const char* scope)
{
#ifdef _DEBUG
	WriteInternal(scope);
	WriteInternal("{");
	BeginSpace();
#endif
}

Debug::Log::~Log()
{
#ifdef _DEBUG
	EndSpace();
	WriteInternal("}");
#endif
}

void Debug::Log::BeginSpace()
{
#ifdef _DEBUG
	++tabSpacing;
#endif
}

void Debug::Log::EndSpace()
{
#ifdef _DEBUG
	if (tabSpacing > 0)
	{
		--tabSpacing;
	}
	else
	{
		assert(false && "Invalid Log Scope.");
	}
#endif
}

const wchar_t* Debug::Log::GetLastMessage()
{
	return lastMessage.c_str();
}

void Debug::Log::WriteInternal(const char* log)
{
	#ifdef _DEBUG
	std::ostringstream oss;
	for (int i = 0; i < tabSpacing; ++i)
	{
		oss << '\t';
	}
	oss << log << '\n';
	std::string out = oss.str();
	OutputDebugStringA(out.c_str());
	Console::Write(out, false);
	#endif
}