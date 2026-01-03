#pragma once

namespace utils {
	std::wstring GuidToStringW(const GUID& guid) noexcept;

	struct DeviceInfo {
		std::wstring classGuid;
		std::wstring classDescription;
		std::wstring deviceFriendlyName;
		std::wstring deviceInstanceId; // unique identifier
	};
	DeviceInfo GetDeviceInfoFromPath(LPCWSTR devicePath) noexcept;

	bool IsDeviceSameAsOrDescendantOf(LPCWSTR deviceInstanceId, LPCWSTR targetAncestorId) noexcept;

	bool SetDeviceVolume(LPCWSTR deviceInstanceId, float volumePercent) noexcept;
}