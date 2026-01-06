#pragma once

namespace utils {
	std::wstring GuidToStringW(const GUID& guid) noexcept;

	// aka. "audio adapter" / "audio input & output (in devmgmt.msc)"
	struct DeviceInterfaceInfo {
		std::wstring classGuid;
		std::wstring classDescription;
		std::wstring deviceFriendlyName;
		std::wstring deviceInstanceId; // unique identifier
	};
	DeviceInterfaceInfo GetDeviceInterfaceInfoFromPath(LPCWSTR devicePath) noexcept;

	bool IsDeviceSameAsOrDescendantOf(LPCWSTR deviceInstanceId, LPCWSTR targetAncestorId) noexcept;

	std::vector<CComPtr<IMMDevice>> FindAssociatedMmDevices(LPCWSTR deviceInstanceId) noexcept;

	HRESULT SetDeviceVolume(CComPtr<IMMDevice> mmDevice, int volumePercent) noexcept;

	/* There can be more than one IMMDevice attached to one Device Interface (definition as mentioned above) */

	// aka. "endpoint device"
	struct MmDeviceInfo {
		std::wstring friendlyName;
		std::wstring id; // from IMMDevice->GetId(); without "SWD\\MMDEVAPI\\" prefix
		std::wstring description; // The device description of the endpoint device (for example, "Speakers").
		//std::wstring deviceInterfaceId; // ID of the audio adapter to which this MMDevice is attached to
	};
	std::optional<MmDeviceInfo> GetMmDeviceInfo(CComPtr<IMMDevice> mmDevice) noexcept;

	std::string WcToU8(std::wstring_view wstr) noexcept;

	std::wstring U8ToWc(std::string_view u8str) noexcept;

	std::wstring AcpToWc(std::string_view acpStr) noexcept;

	// with '\\'
	std::wstring GetRealCurrentDirectory() noexcept;

	std::string ReadConfigFile() noexcept;

}