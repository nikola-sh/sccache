#include "preprocessor_optimizer_injectee.h"
#include "log_file.h"
#include "../common/shared_cache.h"

#include <map>
#include <string>

extern std::unique_ptr<SharedCache> g_notFoundFiles;

template <typename CharType>
CharType ToLowerCase(CharType ch)
{
    if (ch >= CharType('A') && ch <= CharType('Z'))
        return ch + CharType('a' - 'A');
    else
        return ch;
}

template <size_t N>
bool IsPathMatched1(std::wstring const& path, wchar_t const (&prefix)[N])
{
    size_t const pathLen = N - 1;

    if (path.size() > pathLen && std::equal(prefix, prefix + pathLen, path.begin()))
        return true;

    return false;
}

bool IsPathMatched(std::wstring path)
{
    for (wchar_t& ch: path)
    {
        ch = ToLowerCase(ch);
        if (ch == L'/')
            ch = '\\';
    }

    return IsPathMatched1(path, L"f:\\projects\\monorepo\\src\\") ||
        IsPathMatched1(path, L"f:\\projects\\monorepo\\am_delivery_build\\.delivery\\") ||
        IsPathMatched1(path, L"c:\\program files");
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
    try
    {
        g_logFile->Write(lpFileName);

        bool const cacheConditions = dwCreationDisposition == OPEN_EXISTING && IsPathMatched(lpFileName);

        if (cacheConditions)
        {
            DWORD err;
            if (g_notFoundFiles->Get(lpFileName, err))
            {
                g_logFile->Write(L"Got from cache!");

                SetLastError(err);
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

        if (cacheConditions && handle == INVALID_HANDLE_VALUE && (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND))
        {
            g_notFoundFiles->Add(lpFileName, error);
            g_logFile->Write(L"Stored in cache: ", lpFileName);
        }

        SetLastError(error);
        return handle;
    }
    catch(std::exception& ex)
    {
        std::string err = ex.what();
        std::wstring werr;
        for (char ch : err)
            werr.push_back(ch);
        
        g_logFile->Write(werr.c_str());

        TerminateProcess(GetCurrentProcess(), 666);
        SetLastError(ERROR_FUNCTION_FAILED);
        return INVALID_HANDLE_VALUE;
    }
    catch(...)
    {
        TerminateProcess(GetCurrentProcess(), 666);
        SetLastError(ERROR_FUNCTION_FAILED);
        return INVALID_HANDLE_VALUE;
    }
}

DWORD __stdcall Mine_GetFileAttributesW(
    LPCWSTR               lpFileName
    )
{
    try
    {
        g_logFile->Write(lpFileName);

        bool const cacheConditions = IsPathMatched(lpFileName);

        if (cacheConditions)
        {
            DWORD err;
            if (g_notFoundFiles->Get(lpFileName, err))
            {
                g_logFile->Write(L"Got from cache!");

                SetLastError(err);
                return INVALID_FILE_ATTRIBUTES;
            }
        }

        DWORD const attr = Real_GetFileAttributesW(lpFileName);
        DWORD const error = GetLastError();

        if (cacheConditions && attr == INVALID_FILE_ATTRIBUTES && (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND))
        {
            g_notFoundFiles->Add(lpFileName, error);
            g_logFile->Write(L"Stored in cache: ", lpFileName);
        }

        SetLastError(error);
        return attr;
    }
    catch(std::exception& ex)
    {
        std::string err = ex.what();
        std::wstring werr;
        for (char ch : err)
            werr.push_back(ch);

        g_logFile->Write(werr.c_str());

        TerminateProcess(GetCurrentProcess(), 666);
        SetLastError(ERROR_FUNCTION_FAILED);
        return INVALID_FILE_ATTRIBUTES;
    }
    catch(...)
    {
        TerminateProcess(GetCurrentProcess(), 666);
        SetLastError(ERROR_FUNCTION_FAILED);
        return INVALID_FILE_ATTRIBUTES;
    }
}