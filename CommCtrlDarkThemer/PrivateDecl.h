#pragma once

inline constexpr int DARK_THEME_SUBCLASS_ID = 0x5254;
inline constexpr COLORREF DARK_BK_COLOR = RGB(0x20, 0x20, 0x20);

inline HMODULE g_hModule = nullptr;
inline thread_local HHOOK t_hHook = nullptr;
inline thread_local HBRUSH t_darkBkBrush = nullptr;

inline thread_local std::map<HWND, std::wstring> t_externallySetTheme;

// storage these so that we can revert all changes
// when the dark theme loader is uninstalled from thread
inline thread_local std::set<HWND> t_subclassedWnds;
inline thread_local std::set<HWND> t_darkTitleBarWnds;
inline thread_local std::set<HWND> t_setThemeListCtrls;
inline thread_local std::set<HWND> t_setThemeWnds; // 已设置主题的非ListControl控件
inline thread_local std::set<HWND> t_toRepaintWnds;

struct ListControlBkColor
{
	COLORREF textBkColor;
	COLORREF bkColor;
};
inline thread_local std::map<HWND, ListControlBkColor> t_setBkColorListCtrls;
inline thread_local std::map<HWND, COLORREF> t_setTextColorListCtrls;

LRESULT CALLBACK WndCreateMonitorProc(int code, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK OuterWndEnumProc(HWND hwnd, LPARAM lParam);

LRESULT CALLBACK DarkThemeSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

// for listview's scrollbar
BOOL AllowWindowDarkMode(HWND hwnd, bool allow);

void TrySubclassWindow(HWND hwnd);

void InitializeForWindow(HWND hwnd);