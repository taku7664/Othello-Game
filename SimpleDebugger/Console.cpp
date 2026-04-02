#include "pch.h"
#include "Console.h"

HANDLE Debug::Console::hConsole = NULL;

void Debug::Console::Open()
{
#ifdef _DEBUG
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONIN$", "r", stdin);
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif 
}

void Debug::Console::Close()
{
#ifdef _DEBUG
    FreeConsole();
    hConsole = NULL;
#endif 

}

bool Debug::Console::IsActive()
{
    return hConsole != NULL;
}

void Debug::Console::Write(const char* str, bool endl)
{
    if (IsActive())
    {
        DWORD written;
        WriteConsoleA(hConsole, str, (DWORD)strlen(str), &written, NULL);
        if (endl)
        {
            WriteConsoleA(hConsole, "\n", (DWORD)strlen("\n"), &written, NULL);
        }
    }
}

void Debug::Console::Write(const std::string& str, bool endl)
{
    if (IsActive())
    {
        DWORD written;
        WriteConsoleA(hConsole, str.c_str(), (DWORD)str.length(), &written, NULL);
        if (endl)
        {
            WriteConsoleA(hConsole, "\n", (DWORD)strlen("\n"), &written, NULL);
        }
    }
}

void Debug::Console::WriteW(const wchar_t* wstr, bool endl)
{
    if (IsActive())
    {
        DWORD written;
        WriteConsoleW(hConsole, wstr, lstrlenW(wstr), &written, NULL);
        if (endl)
        {
            WriteConsoleW(hConsole, L"\n", lstrlenW(L"\n"), &written, NULL);
        }
    }
}

void Debug::Console::WriteW(const std::wstring& wstr, bool endl)
{
    if (IsActive())
    {
        DWORD written;
        WriteConsoleW(hConsole, wstr.c_str(), (DWORD)wstr.length(), &written, NULL);
        if (endl)
        {
            WriteConsoleW(hConsole, L"\n", lstrlenW(L"\n"), &written, NULL);
        }
    }
}
