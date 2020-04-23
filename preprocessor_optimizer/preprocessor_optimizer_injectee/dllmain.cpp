#include <windows.h>
#include <detours.h>

#include "preprocessor_optimizer_injectee.h"
#include "log_file.h"
#include "../common/shared_cache.h"

std::unique_ptr<LogFile> g_logFile;
std::unique_ptr<SharedCache> g_notFoundFiles;
CreateFileWType Real_CreateFileW = CreateFileW;
GetFileAttributesWType Real_GetFileAttributesW = GetFileAttributesW;

__declspec(dllexport) void Stub()
{
    ::DebugBreak();
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
    try
    {
        if (DetourIsHelperProcess())
        {
            return TRUE;
        }

        if (dwReason == DLL_PROCESS_ATTACH)
        {
            g_logFile.reset(new LogFile);

            g_notFoundFiles.reset(new SharedCache);

            DetourRestoreAfterWith();

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)Real_CreateFileW, Mine_CreateFileW);
            DetourAttach(&(PVOID&)Real_GetFileAttributesW, Mine_GetFileAttributesW);
            DetourTransactionCommit();
        }

        else if (dwReason == DLL_PROCESS_DETACH)
        {
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourDetach(&(PVOID&)Real_CreateFileW, Mine_CreateFileW);
            DetourDetach(&(PVOID&)Real_GetFileAttributesW, Mine_GetFileAttributesW);
            DetourTransactionCommit();

            g_logFile.reset();
        }

        return TRUE;
    }
    catch (...)
    {
        TerminateProcess(GetCurrentProcess(), 666);
        return false;
    }
}
