#pragma once

namespace Debug
{
	class Console
	{
	public:
		static void Open();
		static void Close();
		static bool IsActive();
		static void Write(const char* str, bool endl = true);
		static void Write(const std::string& str, bool endl = true);
		static void WriteW(const wchar_t* wstr, bool endl = true);
		static void WriteW(const std::wstring& wstr, bool endl = true);
	private:
		static HANDLE hConsole;
	};
}