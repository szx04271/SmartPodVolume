#include "pch.h"
#include "PrivateDecl.h"

LRESULT CALLBACK WndCreateMonitorProc(int code, WPARAM wParam, LPARAM lParam) {
	if (/* code < 0 || */ code != HCBT_CREATEWND) {
		return CallNextHookEx(nullptr, code, wParam, lParam);
	}

	auto hwnd = (HWND)wParam;
	if (!t_subclassedWnds.contains(hwnd)) {
		auto subclassSuccess = SetWindowSubclass(hwnd, DarkThemeSubclassProc, DARK_THEME_SUBCLASS_ID, 0);
		if (subclassSuccess) {
			t_subclassedWnds.emplace(hwnd);
		}
	}

	return CallNextHookEx(nullptr, code, wParam, lParam);
}

LRESULT CALLBACK DarkThemeSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, 
	UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
	if (uMsg == WM_SHOWWINDOW) {
		auto style = GetWindowLongW(hWnd, GWL_STYLE);
		if (style & WS_CAPTION) {
			// use dark title bar
			BOOL value = TRUE;
			DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
		}

		WCHAR className[1024];
		int charsWritten = GetClassNameW(hWnd, className, 1024);
		if (charsWritten && _wcsicmp(className, L"SysListView32") == 0) {
			// special handling for list view (the other part in WM_THEMECHANGE)
			AllowWindowDarkMode(hWnd, true);
			SetWindowTheme(hWnd, L"Explorer", nullptr);
		}
		else {
			// use dark controls
			SetWindowTheme(hWnd, L"DarkMode_Explorer", nullptr);
		}
	}
	else if (uMsg == WM_CTLCOLORDLG || uMsg == WM_CTLCOLORBTN || uMsg == WM_CTLCOLORSTATIC) {
		if (uMsg == WM_CTLCOLORSTATIC) {
			DefSubclassProc(hWnd, uMsg, wParam, lParam); // original wndproc handling

			auto hdc = (HDC)wParam;
			SetBkColor(hdc, DARK_BK_COLOR);
			if (GetTextColor(hdc) == RGB(0, 0, 0)) {
				SetTextColor(hdc, RGB(255, 255, 255));
			}
		}
		// use dark background for dialogs
		return (LRESULT)t_darkBkBrush;
	}
	else if (uMsg == WM_NCDESTROY) {
		RemoveWindowSubclass(hWnd, DarkThemeSubclassProc, DARK_THEME_SUBCLASS_ID);
		t_subclassedWnds.erase(hWnd);
	}
	else if (uMsg == WM_THEMECHANGED) {
		// special handling for list view
		WCHAR className[1024];
		int charsWritten = GetClassNameW(hWnd, className, 1024);

		if (charsWritten && _wcsicmp(className, L"SysListView32") == 0) {
			HTHEME theme = OpenThemeData(nullptr, L"ItemsView");
			if (theme) {
				COLORREF color;

				HRESULT hr = GetThemeColor(theme, 0, 0, TMT_TEXTCOLOR, &color);
				if (SUCCEEDED(hr)) {
					ListView_SetTextColor(hWnd, color);
				}

				hr = GetThemeColor(theme, 0, 0, TMT_FILLCOLOR, &color);
				if (SUCCEEDED(hr)) {
					ListView_SetTextBkColor(hWnd, color);
					ListView_SetBkColor(hWnd, color);
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
