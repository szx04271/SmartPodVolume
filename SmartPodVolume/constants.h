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