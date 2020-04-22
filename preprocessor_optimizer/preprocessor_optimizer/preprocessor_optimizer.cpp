#include <windows.h>
#include <detours.h>

#include <string>
#include <stdexcept>

#include "preprocessor_optimizer.h"

std::string GetInjecteeDllPath()
{
    HMODULE thisModule;
    if (!GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCWSTR)&GetInjecteeDllPath, &thisModule))
    {
        throw std::runtime_error("failed to get current module handle");
    }

    char dllPath[4096];
    DWORD n = GetModuleFileNameA(thisModule, dllPath, _countof(dllPath));
    if (n >= _countof(dllPath))
    {
        throw std::runtime_error("failed to get DLL path");
    }

    char* p = strrchr(dllPath, '\\');
    if (p == 0)
    {
        throw std::runtime_error("failed to get DLL path");
    }

    std::string injecteePath(dllPath, p + 1 - dllPath);
    injecteePath += "preprocessor_optimizer_injectee.dll";

    return injecteePath;
}

HANDLE RunCompilerProcess(
    wchar_t const* imagePath,
    wchar_t const* cmdline
)
try
{
    std::wstring cmdlineBuf(cmdline);

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    
    BOOL ok = DetourCreateProcessWithDllEx(imagePath, &cmdlineBuf[0], nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi, GetInjecteeDllPath().c_str(), nullptr);
    if (!ok)
    {
        throw std::runtime_error("failed to create process");
    }

    ::CloseHandle(pi.hThread);

    return pi.hProcess;
}
catch (std::exception const&)
{
    return nullptr;
}
