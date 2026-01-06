#pragma once

inline constexpr const wchar_t* CONFIG_FILE_NAME = L"config.json";

namespace conf_key {
	inline constexpr const char* BLACKLIST = "blacklist";
	inline constexpr const char* WHITELIST = "whitelist";
	inline constexpr const char* RETRYLIST = "retrylist";

	// all in utf-8
	inline constexpr const char* FRIENDLY_NAME = "friendly_name";
	inline constexpr const char* EXPECTED_VOLUME = "expected_vol";
	inline constexpr const char* DESCRIPTION = "description";
	inline constexpr const char* MMDEVICE_ID = "mmdevice_id";
}
