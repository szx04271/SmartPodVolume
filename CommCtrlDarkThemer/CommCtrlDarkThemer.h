#pragma once

#ifdef CCDT_EXPORT
#define CCDT_API
#else
#define CCDT_API __declspec(dllimport)
#endif

CCDT_API LSTATUS DarkThemer_InstallForCurrentThread();

CCDT_API LSTATUS DarkThemer_UninstallForCurrentThread();

// ListView needs this.
// `ifDark` is a boolean intentionally decleared as uchar to
// clearly show its size for other languages to call it.
CCDT_API DWORD DarkThemer_ForceAppDark(unsigned char ifDark);

// Interthread calling is forbidden!
CCDT_API std::pair<HRESULT, std::wstring> DarkThemer_SafeSetWindowTheme(HWND hwnd, LPCWSTR subAppName);