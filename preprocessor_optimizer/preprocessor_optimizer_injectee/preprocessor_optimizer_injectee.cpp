#include "preprocessor_optimizer_injectee.h"
#include "log_file.h"

#include <map>
#include <string>

std::map<std::wstring, DWORD> g_notFoundFiles;

HANDLE __stdcall Mine_CreateFileW(
    LPCWSTR               lpFileName,
    DWORD                 dwDesiredAccess,
    DWORD                 dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD                 dwCreationDisposition,
    DWORD                 dwFlagsAndAttributes,
    HANDLE                hTemplateFile)
{
    g_logFile->Write(lpFileName);

    bool const cacheConditions = dwCreationDisposition == OPEN_EXISTING;

    if (cacheConditions)
    {
        auto it = g_notFoundFiles.find(lpFileName);
        if (it != g_notFoundFiles.end())
        {
            g_logFile->Write(std::wstring() + L"Got from cache!");

            ::SetLastError(it->second);
            return INVALID_HANDLE_VALUE;
        }
    }

    HANDLE const handle = Real_CreateFileW(
        lpFileName,
        dwDesiredAccess,
        dwShareMode,
        lpSecurityAttributes,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        hTemplateFile);

    DWORD const error = GetLastError();

    if (cacheConditions && handle == INVALID_HANDLE_VALUE && error == ERROR_FILE_NOT_FOUND)
    {
        g_notFoundFiles[lpFileName] = error;
        g_logFile->Write(std::wstring() + L"Stored in cache: " + lpFileName);
    }

    return handle;
}
