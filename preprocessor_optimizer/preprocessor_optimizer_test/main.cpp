#include <windows.h>

#include <iostream>
#include <string>

typedef HANDLE (*RunCompilerProcessType)(
    wchar_t const* imagePath,
    wchar_t const* cmdline
);

int wmain(int argc, wchar_t* argv)
{
    HMODULE module = ::LoadLibraryW(L"preprocessor_optimizer.dll");
    if (!module)
    {
        std::cerr << "failed to load preprocessor_optimizer.dll\n";
        return 1;
    }

    RunCompilerProcessType runCompilerProcess = (RunCompilerProcessType)GetProcAddress(module, "RunCompilerProcess");
    if (!runCompilerProcess)
    {
        std::cerr << "failed to load function RunCompilerProcess\n";
        return 1;
    }
   
#if 0 
    HANDLE notepadProcess = runCompilerProcess(L"C:\\Windows\\Notepad.exe", L"C:\\Windows\\Notepad.exe M:\\3333.txt");
    if (!notepadProcess)
    {
        std::cerr << "failed to start Notepad process\n";
        return 1;
    }
#else
    std::wstring cmdlineBuf(L"C:\\Windows\\Notepad.exe M:\\3333.txt");

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    BOOL ok = CreateProcessW(L"C:\\Windows\\Notepad.exe", &cmdlineBuf[0], nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
    if (!ok)
    {
        std::cerr << "failed to start Notepad process\n";
        return 1;
    }

    ::CloseHandle(pi.hThread);

    HANDLE notepadProcess = pi.hProcess;
#endif

    WaitForSingleObject(notepadProcess, INFINITE);
    
    DWORD exitCode;
    if (!GetExitCodeProcess(notepadProcess, &exitCode))
    {
        std::cerr << "failed to get Notepad process exit code\n";
        return 1;
    }

    std::cout << "Exit code: " << exitCode << "\n";

    return 0;
}