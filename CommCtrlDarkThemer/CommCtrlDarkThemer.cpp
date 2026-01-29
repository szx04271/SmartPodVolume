#include "pch.h"
#include "PrivateDecl.h"
#include "CommCtrlDarkThemer.h"

CCDT_API LSTATUS DarkThemer_InstallForCurrentThread()
{
	if (t_hHook) {
		return ERROR_ALREADY_EXISTS;
	}

	auto hhook = SetWindowsHookExW(WH_CBT, WndCreateMonitorProc, g_hModule, GetCurrentThreadId());
	if (!hhook) {
		return GetLastError();
	}
	t_darkBkBrush = CreateSolidBrush(DARK_BK_COLOR);
	t_hHook = hhook;
	return ERROR_SUCCESS;
}

CCDT_API LSTATUS DarkThemer_UninstallForCurrentThread()
{
	if (!t_hHook) {
		return ERROR_FILE_NOT_FOUND;
	}

	bool success = UnhookWindowsHookEx(t_hHook);
	if (!success) {
		return GetLastError();
	}
	t_hHook = nullptr;

	if (t_darkBkBrush) {
		DeleteObject(t_darkBkBrush);
		t_darkBkBrush = nullptr;
	}

	if (!t_subclassedWnds.empty()) {
		for (auto hwnd : t_subclassedWnds) {
			RemoveWindowSubclass(hwnd, DarkThemeSubclassProc, DARK_THEME_SUBCLASS_ID);
		}
		t_subclassedWnds.clear();
	}

	return ERROR_SUCCESS;
}

CCDT_API DWORD DarkThemer_ForceAppDark(unsigned char ifDark) {
	auto hUxTheme = LoadLibraryW(L"uxtheme.dll");
	if (!hUxTheme) {
		return -1;
	}

	typedef DWORD(WINAPI* SetPreferredAppMode_t)(DWORD);
	auto _SetPreferredAppMode = (SetPreferredAppMode_t)GetProcAddress(hUxTheme, MAKEINTRESOURCEA(135));
	if (!_SetPreferredAppMode) {
		FreeLibrary(hUxTheme);
		return -2;
	}
	// 2 for ForceDark, 1 for AllowDark, 0 for Default
	auto ret = _SetPreferredAppMode(ifDark ? 2 : 0);

	FreeLibrary(hUxTheme);
	return ret;
}


