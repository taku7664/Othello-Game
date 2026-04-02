#pragma once

namespace Debug
{
	enum LogLevel
	{
		LOG_NONE = -1,
		LOG_TRACE,
		LOG_DEBUG,
		LOG_INFO,
		LOG_WARNING,
		LOG_ERROR,
		LOG_FATAL
	};
	class Log
	{
	public:
		Log(const char* scope);
		~Log();

		static void BeginSpace();
		static void EndSpace();

		template<typename... Args>
		static void Write(LogLevel level, Args&&... log);
		template<typename... Args>
		static void WriteLine(LogLevel level, Args&&... log);

		template<typename... Args>
		static void WriteW(LogLevel level, Args&&... log);
		template<typename... Args>
		static void WriteLineW(LogLevel level, Args&&... log);

		static const wchar_t* GetLastMessage();

	private:
		static void WriteInternal(const char* log); // constructor, destructor only use

	private:
		static int tabSpacing;
		static std::wstring lastMessage;
		static LogLevel messageFilterLevel;
	};

	template<typename... Args>
	void Log::Write(LogLevel level, Args&&... log)
	{
		#ifdef _DEBUG
		std::ostringstream oss;
		for (int i = 0; i < tabSpacing; ++i)
		{
			oss << '\t';
		}
		if (level >= messageFilterLevel)
		{
			std::ostringstream messageOss;
			(messageOss << ... << std::forward<Args>(log));
			std::string messageStr = messageOss.str();
			Utillity::CharToWString(messageStr.c_str(), lastMessage);
			oss << messageStr.c_str();
		}
		else
		{
			(oss << ... << std::forward<Args>(log));
		}
		std::string out = oss.str();
		OutputDebugStringA(out.c_str());
		Console::Write(out);
		#endif
	}

	template<typename... Args>
	void Log::WriteLine(LogLevel level, Args&&... log)
	{
		#ifdef _DEBUG
		std::ostringstream oss;
		for (int i = 0; i < tabSpacing; ++i)
		{
			oss << '\t';
		}
		if (level >= messageFilterLevel)
		{
			std::ostringstream messageOss;
			(messageOss << ... << std::forward<Args>(log));
			std::string messageStr = messageOss.str();
			Utillity::CharToWString(messageStr.c_str(), lastMessage);
			oss << messageStr.c_str();
		}
		else
		{
			(oss << ... << std::forward<Args>(log));
		}
		oss << '\n';
		std::string out = oss.str();
		OutputDebugStringA(out.c_str());
		Console::Write(out, false);
		#endif
	}

	template<typename... Args>
	void Log::WriteW(LogLevel level, Args&&... log)
	{
		#ifdef _DEBUG
		std::wostringstream oss;
		for (int i = 0; i < tabSpacing; ++i)
		{
			oss << L'\t';
		}
		if (level >= messageFilterLevel)
		{
			std::wostringstream message;
			(message << ... << std::forward<Args>(log));
			lastMessage = message.str();
			oss << lastMessage.c_str();
		}
		else
		{
			(oss << ... << std::forward<Args>(log));
		}
		std::wstring out = oss.str();
		OutputDebugStringW(out.c_str());
		Console::WriteW(out);
#endif
	}

	template<typename... Args>
	void Log::WriteLineW(LogLevel level, Args&&... log)
	{
		#ifdef _DEBUG
		std::wostringstream oss;
		for (int i = 0; i < tabSpacing; ++i)
		{
			oss << L'\t';
		}
		if (level >= messageFilterLevel)
		{
			std::wostringstream message;
			(message << ... << std::forward<Args>(log));
			lastMessage = message.str();
			oss << lastMessage.c_str();
		}
		else
		{
			(oss << ... << std::forward<Args>(log));
		}
		oss << L'\n';
		std::wstring out = oss.str();
		OutputDebugStringW(out.c_str());
		Console::WriteW(out, false);
		#endif
	}
}