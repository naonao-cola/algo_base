#include "CrashCatch.h"
#include <fstream>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <sstream>
#include "logger.h"


#define STACK_TRACE_SIZE 64
#define MAX_SYMBOL_NAME_LEN 256

namespace fs = std::filesystem;


BOOL CrashCatch::InitializeSymbolHandler()
{
    if (!SymInitialize(GetCurrentProcess(), NULL, TRUE))
        return FALSE;

    TCHAR currentDirectory[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, currentDirectory);
    if (!SymSetSearchPath(GetCurrentProcess(), currentDirectory)) {
        std::cout<<"SymSetSearchPath fail!"<<std::endl;
        return FALSE;
    }

    return TRUE;
}

BOOL CrashCatch::LoadDllSymbols()
{
    HANDLE hProcess = GetCurrentProcess();
    DWORD64 baseAddress = (DWORD64)GetModuleHandle(NULL);
    return SymLoadModule64(hProcess, NULL, NULL, NULL, baseAddress, 0);
}

void CrashCatch::PrintStackTrace(CONTEXT *context, std::ostream &output)
{
    LOGE("********************* CRASH **********************");
    void *stack[STACK_TRACE_SIZE];
    USHORT frames = CaptureStackBackTrace(0, STACK_TRACE_SIZE, stack, NULL);

    SYMBOL_INFO *symbol = (SYMBOL_INFO *)malloc(sizeof(SYMBOL_INFO) + MAX_SYMBOL_NAME_LEN);
    symbol->MaxNameLen = MAX_SYMBOL_NAME_LEN;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);

    std::string crashLogStr = fmt::format("Crash Stack Trace:\n");

    for (USHORT i = 0; i < frames; i++)
    {
        DWORD64 address = (DWORD64)(stack[i]);
        SymFromAddr(GetCurrentProcess(), address, 0, symbol);

        IMAGEHLP_LINE64 line;
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

        DWORD displacement;
        if (SymGetLineFromAddr64(process, address, &displacement, &line))
        {
            crashLogStr = fmt::format("{}\t#{} {} - {}:{}\n", crashLogStr, i, symbol->Name, line.FileName, line.LineNumber);
        }
        else
        {
            crashLogStr = fmt::format("{}\t#{} {} - (no source file information)\n", crashLogStr, i, symbol->Name);
        }
    }

    free(symbol);
    output << crashLogStr << std::endl;
    LOGE(crashLogStr);
    LOGE("***************************************************");
}

LONG WINAPI CrashCatch::MyUnhandledExceptionFilter(EXCEPTION_POINTERS *exceptionInfo)
{
    LOGE("*********** Crash Occur ************");

    fs::path crashDir("./crash");
    if (!fs::exists(crashDir)) {
        fs::create_directories(crashDir);
    }

    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm localTime = *std::localtime(&time);

    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y%m%d_%H%M%S");
    DWORD threadId = GetCurrentThreadId();
    char dumpFileName[128];
    char traceFileName[128];
    sprintf(dumpFileName, "./crash/%s__%u_crash_dump.dmp", oss.str().c_str(), threadId);
    sprintf(traceFileName, "./crash/%s__%u_crash_trace.txt", oss.str().c_str(), threadId);
    std::ofstream logFile(traceFileName);
    if (logFile.is_open())
    {
        PrintStackTrace(exceptionInfo->ContextRecord, logFile);
        logFile.close();
        LOGI("Save crash trace file:{}", traceFileName);
    } else {
        LOGE("Create File fail! {}", traceFileName);
    }
    
    HANDLE hFile = CreateFileA(dumpFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != NULL) {
        MINIDUMP_EXCEPTION_INFORMATION exceptionParam;
        exceptionParam.ThreadId = GetCurrentThreadId();
        exceptionParam.ExceptionPointers = exceptionInfo;
        exceptionParam.ClientPointers = FALSE;
        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &exceptionParam, NULL, NULL);
        CloseHandle(hFile);
        LOGI("Save crash dump file:{}", dumpFileName);
    } else {
        LOGE("Create File fail! {}", dumpFileName);
    }

    Sleep(6000);
    return EXCEPTION_CONTINUE_SEARCH;
}

void CrashCatch::PrintExceptionStackTrace()
{
    const int maxStackTraceSize = 64;
    void* stackTrace[maxStackTraceSize];
    unsigned short frames = CaptureStackBackTrace(0, maxStackTraceSize, stackTrace, NULL);
    
    // 初始化符号处理库
    SymInitialize(GetCurrentProcess(), NULL, TRUE);
    
    // 获取符号信息
    const int symbolInfoSize = sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR);
    SYMBOL_INFO* symbolInfo = (SYMBOL_INFO*)malloc(symbolInfoSize);
    symbolInfo->MaxNameLen = MAX_SYM_NAME;
    symbolInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
    
    std::string crashLogStr = fmt::format("Exception Stack Trace:");
    for (unsigned short i = 0; i < frames; i++) {
        
        DWORD64 address = (DWORD64)(stackTrace[i]);
        SymFromAddr(GetCurrentProcess(), address, NULL, symbolInfo);

        IMAGEHLP_LINE64 line;
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
        DWORD displacement;
        
        if (SymGetLineFromAddr64(GetCurrentProcess(), address, &displacement, &line))
        {
            crashLogStr = fmt::format("{}\t#{} {} - {}:{}\n", crashLogStr, i, symbolInfo->Name, line.FileName, line.LineNumber);
        } else {
            crashLogStr = fmt::format("{}\t#{} {} - (no source file information)\n", crashLogStr, i, symbolInfo->Name);
        }
    }

    LOGE(crashLogStr);

    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm localTime = *std::localtime(&time);
    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y%m%d_%H%M%S");
    DWORD threadId = GetCurrentThreadId();
    char traceFileName[128];
    sprintf(traceFileName, "./crash/%s__%u_exception_trace.txt", oss.str().c_str(), threadId);
    std::ofstream logFile(traceFileName);
    if (logFile.is_open())
    {
        logFile << crashLogStr << std::endl;
        logFile.close();
        LOGI("Save crash trace file:{}", traceFileName);
    } else {
        LOGE("Create File fail! {}", traceFileName);
    }

    free(symbolInfo);
}