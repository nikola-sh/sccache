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
   
    HANDLE notepadProcess = runCompilerProcess(L"C:\\Windows\\Notepad.exe", L"C:\\Windows\\Notepad.exe M:\\3333.txt");
    if (!notepadProcess)
    {
        std::cerr << "failed to start Notepad process\n";
        return 1;
    }

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