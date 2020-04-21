#include <windows.h>
#include <detours.h>

#include "preprocessor_optimizer_injectee.h"
#include "log_file.h"

std::unique_ptr<LogFile> g_logFile;

CreateFileWType Real_CreateFileW = CreateFileW;

__declspec(dllexport) void Stub()
{
    ::DebugBreak();
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
    if (DetourIsHelperProcess())
    {
        return TRUE;
    }

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_logFile.reset(new LogFile);
        g_logFile->Write(L"a");

        DetourRestoreAfterWith();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)Real_CreateFileW, Mine_CreateFileW);
        DetourTransactionCommit();
    }

    else if (dwReason == DLL_PROCESS_DETACH)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)Real_CreateFileW, Mine_CreateFileW);
        DetourTransactionCommit();

        g_logFile.reset();
    }

    return TRUE;
}
