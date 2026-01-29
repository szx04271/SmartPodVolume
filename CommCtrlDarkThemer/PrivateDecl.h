#pragma once

inline constexpr int DARK_THEME_SUBCLASS_ID = 0x5254;
inline constexpr COLORREF DARK_BK_COLOR = RGB(0x20, 0x20, 0x20);

inline HMODULE g_hModule = nullptr;
inline thread_local HHOOK t_hHook = nullptr;
inline thread_local HBRUSH t_darkBkBrush = nullptr;
inline thread_local std::set<HWND> t_subclassedWnds;

LRESULT CALLBACK WndCreateMonitorProc(int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DarkThemeSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
// for listview's scrollbar
BOOL AllowWindowDarkMode(HWND hwnd, bool allow);
