#pragma once
namespace Utillity
{
	// wchar_t* → std::string (UTF-8)
	std::string WCharToString(const wchar_t* wstr);
	bool WCharToString(const wchar_t* wstr, std::string& out);

	// char* → std::wstring (UTF-8)
	std::wstring CharToWString(const char* str);
	bool CharToWString(const char* str, std::wstring& out);

	std::wstring U8ToWString(std::string_view utf8_str);

	std::string WStringToU8(std::wstring_view wstring);

    //컴파일 타임에 문자열 길이를 측정합니다.
    template <std::size_t N>
    constexpr std::size_t Strlen(const char(&str)[N])
    {
        return N - 1;
    }

    //컴파일 타임에 평가가 가능한 문자열을 잘라 반환해줍니다.
    template <std::size_t N, std::size_t START, std::size_t COUNT>
    constexpr std::array<char, COUNT + 1> SliceLiteral(const char(&str)[N])
    {
        static_assert(START + COUNT <= N - 1, "string size overrflow");
        std::array<char, COUNT + 1> result{};
        for (std::size_t i = 0; i < COUNT; ++i)
        {
            result[i] = str[i + START];
        }
        result[COUNT] = '\0';
        return result;
    };
}
