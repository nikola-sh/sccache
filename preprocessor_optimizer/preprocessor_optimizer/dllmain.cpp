#include <windows.h>
#include <detours.h>
#include <string>

#include "../common/shared_cache.h"

std::unique_ptr<SharedCache> g_sharedCache;

std::string GetInjecteeDllPath();

typedef BOOL (__stdcall *CreateProcessWType) (
    LPCWSTR               lpApplicationName,
    LPWSTR                lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPCWSTR               lpCurrentDirectory,
    LPSTARTUPINFOW        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
);

CreateProcessWType Real_CeateProcessW = CreateProcessW;

BOOL __stdcall Mine_CreateProcessW (
    LPCWSTR               lpApplicationName,
    LPWSTR                lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPCWSTR               lpCurrentDirectory,
    LPSTARTUPINFOW        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )
{
    return DetourCreateProcessWithDllEx(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation, GetInjecteeDllPath().c_str(), Real_CeateProcessW);
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
            g_sharedCache.reset(new SharedCache(true));

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)Real_CeateProcessW, Mine_CreateProcessW);
            DetourTransactionCommit();
        }

        else if (dwReason == DLL_PROCESS_DETACH)
        {
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourDetach(&(PVOID&)Real_CeateProcessW, Mine_CreateProcessW);
            DetourTransactionCommit();
        }

        return TRUE;
    }
    catch (...)
    {
        TerminateProcess(GetCurrentProcess(), 666);
        return false;
    }
}
