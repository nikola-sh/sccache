#include "preprocessor_optimizer_injectee.h"
#include "log_file.h"
#include "../common/shared_cache.h"

#include <map>
#include <string>

extern std::unique_ptr<SharedCache> g_notFoundFiles;

bool IsPathMatched(std::wstring const& path)
{
    std::wstring const& prefix = L"F:\\Projects\\Monorepo\\src\\";
    if (path.size() > prefix.size() && std::equal(prefix.begin(), prefix.end(), path.begin(), [](wchar_t a, wchar_t b){ return towlower(a) == towlower(b); }))
        return true;

    return false;
}

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

    bool const cacheConditions = dwCreationDisposition == OPEN_EXISTING && IsPathMatched(lpFileName);

    /*if (cacheConditions)
    {
        if (g_notFoundFiles->Check(lpFileName))
        {
            g_logFile->Write(std::wstring() + L"Got from cache!");

            ::SetLastError(ERROR_FILE_NOT_FOUND);
            return INVALID_HANDLE_VALUE;
        }
    }*/

    HANDLE const handle = Real_CreateFileW(
        lpFileName,
        dwDesiredAccess,
        dwShareMode,
        lpSecurityAttributes,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        hTemplateFile);

    DWORD const error = GetLastError();

    /*if (cacheConditions && handle == INVALID_HANDLE_VALUE && error == ERROR_FILE_NOT_FOUND)
    {
        g_notFoundFiles->Add(lpFileName, error);
        g_logFile->Write(std::wstring() + L"Stored in cache: " + lpFileName);
    }*/

    return handle;
}
