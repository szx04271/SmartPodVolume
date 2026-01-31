#include "pch.h"
#include "PrivateDecl.h"
#include "CommCtrlDarkThemer.h"

LRESULT CALLBACK WndCreateMonitorProc(int code, WPARAM wParam, LPARAM lParam) {
	if (/* code < 0 || */ code != HCBT_CREATEWND) {
		return CallNextHookEx(nullptr, code, wParam, lParam);
	}

	TrySubclassWindow((HWND)wParam);

	return CallNextHookEx(nullptr, code, wParam, lParam);
}

BOOL CALLBACK OuterWndEnumProc(HWND hwnd, LPARAM lParam) {
	TrySubclassWindow(hwnd);
	InitializeForWindow(hwnd);
	InvalidateRect(hwnd, nullptr, TRUE);
	UpdateWindow(hwnd);

	EnumChildWindows(hwnd,
		[](HWND child, LPARAM) -> BOOL {
		TrySubclassWindow(child);
		InitializeForWindow(child);
		InvalidateRect(child, nullptr, TRUE);
		UpdateWindow(child);
		return TRUE;
	}, 0);

	return TRUE;
}

LRESULT CALLBACK DarkThemeSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, 
	UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
	if (uMsg == WM_SHOWWINDOW) {
		InitializeForWindow(hWnd);
	}
	else if (uMsg == WM_CTLCOLORDLG || uMsg == WM_CTLCOLORBTN || uMsg == WM_CTLCOLORSTATIC) {
		if (uMsg == WM_CTLCOLORSTATIC) {
			DefSubclassProc(hWnd, uMsg, wParam, lParam); // original wndproc handling

			auto hdc = (HDC)wParam;
			SetBkColor(hdc, DARK_BK_COLOR);
			if (GetTextColor(hdc) == RGB(0, 0, 0)) {
				SetTextColor(hdc, RGB(255, 255, 255));
				t_toRepaintWnds.emplace(hWnd);
			}
		}
		// use dark background for dialogs
		return (LRESULT)t_darkBkBrush;
	}
	else if (uMsg == WM_NCDESTROY) {
		RemoveWindowSubclass(hWnd, DarkThemeSubclassProc, DARK_THEME_SUBCLASS_ID);
		t_subclassedWnds.erase(hWnd);
		t_darkTitleBarWnds.erase(hWnd);
		t_setThemeListCtrls.erase(hWnd);
		t_setThemeWnds.erase(hWnd);
		t_toRepaintWnds.erase(hWnd);
		t_setBkColorListCtrls.erase(hWnd);
		t_setTextColorListCtrls.erase(hWnd);
	}
	else if (uMsg == WM_THEMECHANGED) {
		// special handling for list view
		WCHAR className[1024];
		int charsWritten = GetClassNameW(hWnd, className, 1024);

		if (charsWritten && _wcsicmp(className, L"SysListView32") == 0) {
			HTHEME theme = OpenThemeData(nullptr, L"ItemsView");
			if (theme) {
				COLORREF color;

				HRESULT hr;
				if (!t_setTextColorListCtrls.contains(hWnd)) {
					hr = GetThemeColor(theme, 0, 0, TMT_TEXTCOLOR, &color);
					if (SUCCEEDED(hr)) {
						auto oriTextColor = ListView_GetTextColor(hWnd);
						ListView_SetTextColor(hWnd, color);
						t_setTextColorListCtrls.emplace(hWnd, oriTextColor);
					}
				}

				if (!t_setBkColorListCtrls.contains(hWnd)) {
					hr = GetThemeColor(theme, 0, 0, TMT_FILLCOLOR, &color);
					if (SUCCEEDED(hr)) {
						ListControlBkColor originalColor{};
						originalColor.textBkColor = ListView_GetTextBkColor(hWnd);
						originalColor.bkColor = ListView_GetBkColor(hWnd);

						ListView_SetTextBkColor(hWnd, color);
						ListView_SetBkColor(hWnd, color);
						
						t_setBkColorListCtrls.emplace(hWnd, originalColor);
					}
				}

				CloseThemeData(theme);
			}
		}
	}
	else if (uMsg == WM_PAINT) {
		WCHAR className[1024];
		int charsWritten = GetClassNameW(hWnd, className, 1024);
		
		if (charsWritten && wcscmp(className, L"#32770") == 0 && !IsIconic(hWnd)) {
			// [HACK]
			// Take over WM_PAINT of dialogs to ensure MessageBox's background is completely dark.
			// Should have only taken over that of MessageBoxes instead of all dialogs but
			// a way to tell MessageBoxes from other dialogs here is lacked.

			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			FillRect(hdc, &ps.rcPaint, t_darkBkBrush);
			EndPaint(hWnd, &ps);

			t_toRepaintWnds.emplace(hWnd);

			return 0;
		}
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

BOOL AllowWindowDarkMode(HWND hwnd, bool allow) {
	auto hUxTheme = LoadLibraryW(L"uxtheme.dll");
	if (!hUxTheme) {
		return FALSE;
	}

	typedef DWORD(WINAPI* AllowDarkModeForWindow_t)(HWND, BOOL);
	auto _AllowDarkModeForWindow = (AllowDarkModeForWindow_t)GetProcAddress(hUxTheme, MAKEINTRESOURCEA(133));
	if (!_AllowDarkModeForWindow) {
		FreeLibrary(hUxTheme);
		return FALSE;
	}
	auto ret = _AllowDarkModeForWindow(hwnd, allow);

	FreeLibrary(hUxTheme);
	return ret;
}

void TrySubclassWindow(HWND hwnd) {
	if (!t_subclassedWnds.contains(hwnd)) {
		WCHAR className[1024];
		GetClassNameW(hwnd, className, 1024);

		const auto WPF_CLASS_PREFIX = L"HwndWrapper[";
		if (wcsncmp(className, WPF_CLASS_PREFIX, wcslen(WPF_CLASS_PREFIX)) == 0) {
			// dont subclass WPF windows
			return;
		}

		auto subclassSuccess = SetWindowSubclass(hwnd, DarkThemeSubclassProc, DARK_THEME_SUBCLASS_ID, 0);
		if (subclassSuccess) {
			t_subclassedWnds.emplace(hwnd);
		}
	}
}

void InitializeForWindow(HWND hwnd) {
	auto style = GetWindowLongW(hwnd, GWL_STYLE);
	if ((style & WS_CAPTION) && !t_darkTitleBarWnds.contains(hwnd)) {
		// use dark title bar
		BOOL value = TRUE;
		DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
		t_darkTitleBarWnds.emplace(hwnd);
	}

	WCHAR className[1024];
	int charsWritten = GetClassNameW(hwnd, className, 1024);
	if (charsWritten && _wcsicmp(className, L"SysListView32") == 0 &&
		!t_setThemeListCtrls.contains(hwnd)) {
		// special handling for list view (the other part in WM_THEMECHANGE)
		AllowWindowDarkMode(hwnd, true);
		t_setThemeListCtrls.emplace(hwnd);
		SetWindowTheme(hwnd, L"Explorer", nullptr);
	}
	else if (!t_setThemeWnds.contains(hwnd)) {
		// use dark controls
		t_setThemeWnds.emplace(hwnd);
		HRESULT hr = SetWindowTheme(hwnd, L"DarkMode_Explorer", nullptr);
	}
}
