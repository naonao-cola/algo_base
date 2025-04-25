#pragma once

#include <iostream>
#include <locale.h>


#if defined(_WIN32)

class StringConvert
{
public:
    static wchar_t*     AnsiToUnicode(const char* lpszStr);
    static char*        UnicodeToAnsi(const wchar_t* lpszStr);
    static char*        AnsiToUtf8(const char* lpszStr);
    static char*        Utf8ToAnsi(const char* lpszStr);
    static char*        UnicodeToUtf8(const wchar_t* lpszStr);
    static wchar_t*     Utf8ToUnicode(const char* lpszStr);
    static bool         AnsiToUnicode(const char* lpszAnsi, wchar_t* lpszUnicode, int nLen);
    static bool         UnicodeToAnsi(const wchar_t* lpszUnicode, char* lpszAnsi, int nLen);
    static bool         AnsiToUtf8(const char* lpszAnsi, char* lpszUtf8, int nLen);
    static bool         Utf8ToAnsi(const char* lpszUtf8, char* lpszAnsi, int nLen);
    static bool         UnicodeToUtf8(const wchar_t* lpszUnicode, char* lpszUtf8, int nLen);
    static bool         Utf8ToUnicode(const char* lpszUtf8, wchar_t* lpszUnicode, int nLen);
    static std::wstring AnsiToUnicode(const std::string& strAnsi);
    static std::string  UnicodeToAnsi(const std::wstring& strUnicode);
    static std::string  AnsiToUtf8(const std::string& strAnsi);
    static std::string  Utf8ToAnsi(const std::string& strUtf8);
    static std::string  UnicodeToUtf8(const std::wstring& strUnicode);
    static std::wstring Utf8ToUnicode(const std::string& strUtf8);
};

#elif defined(__linux__)
class StringConvert
{
public:
    static char*       AnsiToUtf8(const char* lpszStr);
    static char*       Utf8ToAnsi(const char* lpszStr);
    static std::string AnsiToUtf8(const std::string& strAnsi);
    static std::string Utf8ToAnsi(const std::string& strUtf8);
};

#endif