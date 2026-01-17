#pragma once

inline constexpr const wchar_t* CONFIG_FILE_NAME = L"config.json";

// all in utf-8
namespace conf_key {
	inline constexpr const char* BLACKLIST = "blacklist"; // array
	inline constexpr const char* WHITELIST = "whitelist"; // array

	inline constexpr const char* FRIENDLY_NAME = "friendly_name"; // string
	inline constexpr const char* EXPECTED_VOLUME = "expected_vol"; // float
	inline constexpr const char* DESCRIPTION = "description"; // string
	inline constexpr const char* MUTE = "mute"; // bool
}

enum CustomWindowMessages {
	// wParam: MyVolumeChangeCallback*
	// lParam: unused
	// return value: unused
	WM_REGISTERED_DEVICE_VOLUME_CHANGED = WM_USER + 67,

	// wParam: pointer to std::wstring of its lowercase MmDevice ID
	// lParam: unused
	// return value: unused
	WM_NEWDEVICEDLG_CLOSED,

	// wParam: pointer to IMMDevice
	// lParam: unused
	// return value: unused
	WM_NEW_DEVICE_NEEDS_REGISTRATION
};

inline constexpr LPCWSTR BKGND_PROCESS_PIPE_NAME = LR"(\\.\pipe\D9227EEB_62EB_4903_B4A1_5ACB5D97FCBC)";
inline constexpr BYTE BKGND_PROCESS_ALIVE_SIGNAL = 0x84;
inline constexpr BYTE BKGND_PROCESS_STOP_SIGNAL = 0xfe;

inline constexpr LPCWSTR INSTANCE_MUTEX_NAME = L"Global\\{FB8FB32A-6FA7-4287-BAFE-2DD1DD76DDA1}";