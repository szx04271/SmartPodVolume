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

	EnumThreadWindows(GetCurrentThreadId(), OuterWndEnumProc, 0); // apply for existing windows (if any)

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

	// remove subclass first
	for (auto hwnd : t_subclassedWnds) {
		RemoveWindowSubclass(hwnd, DarkThemeSubclassProc, DARK_THEME_SUBCLASS_ID);
	}
	t_subclassedWnds.clear();

	for (auto hwnd : t_darkTitleBarWnds) {
		BOOL value = FALSE;
		DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
	}
	t_darkTitleBarWnds.clear();

	for (auto hwnd : t_setThemeListCtrls) {
		AllowWindowDarkMode(hwnd, false);

		auto externallySetTheme = t_externallySetTheme.find(hwnd);
		if (externallySetTheme != t_externallySetTheme.end()) {
			SetWindowTheme(hwnd, externallySetTheme->second.c_str(), nullptr);
		}
		else {
			SetWindowTheme(hwnd, nullptr, nullptr);
		}
	}
	t_setThemeListCtrls.clear();

	for (auto hwnd : t_setThemeWnds) {
		auto externallySetTheme = t_externallySetTheme.find(hwnd);
		if (externallySetTheme != t_externallySetTheme.end()) {
			SetWindowTheme(hwnd, externallySetTheme->second.c_str(), nullptr);
		}
		else {
			SetWindowTheme(hwnd, nullptr, nullptr);
		}
	}
	t_setThemeWnds.clear();

	for (auto& [hwnd, clrs] : t_setBkColorListCtrls) {
		ListView_SetTextBkColor(hwnd, clrs.textBkColor);
		ListView_SetBkColor(hwnd, clrs.bkColor);
	}
	t_setBkColorListCtrls.clear();

	for (auto& [hwnd, clr] : t_setTextColorListCtrls) {
		ListView_SetTextColor(hwnd, clr);
	}
	t_setTextColorListCtrls.clear();

	for (auto hwnd : t_toRepaintWnds) {
		InvalidateRect(hwnd, nullptr, TRUE);
		UpdateWindow(hwnd);
	}
	t_toRepaintWnds.clear();
	
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

CCDT_API std::pair<HRESULT, std::wstring> DarkThemer_SafeSetWindowTheme(HWND hwnd, LPCWSTR subAppName)
{
	std::pair<HRESULT, std::wstring> ret;
	if (!subAppName) {
		ret.first = E_INVALIDARG;
		return ret;
	}

	HRESULT hr = SetWindowTheme(hwnd, subAppName, nullptr);
	ret.first = hr;
	if (FAILED(hr)) {
		return ret;
	}
	if (t_externallySetTheme.contains(hwnd)) {
		ret.second = std::move(t_externallySetTheme.at(hwnd));
	}
	t_externallySetTheme[hwnd] = subAppName;
	return ret;
}

