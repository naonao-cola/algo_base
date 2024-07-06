#pragma once
#include <windows.h>
#include <dbghelp.h>
#include <iostream>


#pragma comment(lib, "dbghelp.lib")


class CrashCatch
{
public:
    static BOOL InitializeSymbolHandler();
    static BOOL LoadDllSymbols();
    static void PrintStackTrace(CONTEXT *context, std::ostream &output);
    static void PrintExceptionStackTrace();
    static LONG WINAPI MyUnhandledExceptionFilter(EXCEPTION_POINTERS *exceptionInfo);
};
