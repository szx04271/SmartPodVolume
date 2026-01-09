#pragma once

inline constexpr const wchar_t* CONFIG_FILE_NAME = L"config.json";

// all in utf-8
namespace conf_key {
	inline constexpr const char* BLACKLIST = "blacklist"; // array
	inline constexpr const char* WHITELIST = "whitelist"; // array
	inline constexpr const char* RETRYLIST = "retrylist"; // array

	inline constexpr const char* FRIENDLY_NAME = "friendly_name"; // string
	inline constexpr const char* EXPECTED_VOLUME = "expected_vol"; // float
	inline constexpr const char* DESCRIPTION = "description"; // string
	inline constexpr const char* MMDEVICE_ID = "mmdevice_id"; // string
	inline constexpr const char* MUTE = "mute"; // bool
}
