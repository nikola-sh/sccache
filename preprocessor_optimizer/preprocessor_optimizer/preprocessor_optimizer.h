#pragma once

#include <windows.h>

#ifdef PREPROCESSOROPTIMIZER_EXPORTS
#define PREPROCESSOROPTIMIZER_API __declspec(dllexport)
#else
#define PREPROCESSOROPTIMIZER_API __declspec(dllimport)
#endif

extern "C" PREPROCESSOROPTIMIZER_API HANDLE RunCompilerProcess(
    wchar_t const* imagePath,
    wchar_t const* cmdline
);
