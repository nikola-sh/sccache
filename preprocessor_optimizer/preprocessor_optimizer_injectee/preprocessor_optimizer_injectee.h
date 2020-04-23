#include <windows.h>

typedef HANDLE (__stdcall * CreateFileWType)(
    LPCWSTR               lpFileName,
    DWORD                 dwDesiredAccess,
    DWORD                 dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD                 dwCreationDisposition,
    DWORD                 dwFlagsAndAttributes,
    HANDLE                hTemplateFile);

extern CreateFileWType Real_CreateFileW;

HANDLE __stdcall Mine_CreateFileW(
    LPCWSTR               lpFileName,
    DWORD                 dwDesiredAccess,
    DWORD                 dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD                 dwCreationDisposition,
    DWORD                 dwFlagsAndAttributes,
    HANDLE                hTemplateFile);


typedef DWORD (__stdcall * GetFileAttributesWType)(
    LPCWSTR lpFileName
);

extern GetFileAttributesWType Real_GetFileAttributesW;

DWORD __stdcall Mine_GetFileAttributesW(
    LPCWSTR lpFileName
);
