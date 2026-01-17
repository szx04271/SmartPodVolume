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

	bool IsDeviceSameAsOrDescendantOf(std::wstring_view deviceInstanceId, std::wstring_view targetAncestorId) noexcept;

	std::list<CComPtr<IMMDevice>> FindAssociatedMmDevices(LPCWSTR deviceInstanceId) noexcept;

	std::list<std::wstring> GetAncestorDeviceIds(std::wstring_view fullPnpId) noexcept;

	/* There can be more than one IMMDevice attached to one Device Interface (definition as mentioned above) */

	// aka. "endpoint device"
	struct MmDeviceInfo {
		std::wstring friendlyName;
		std::wstring id; // from IMMDevice->GetId(); without "SWD\\MMDEVAPI\\" prefix
		std::wstring description; // The device description of the endpoint device (for example, "Speakers").
		//std::wstring deviceInterfaceId; // ID of the audio adapter to which this MMDevice is attached to
	};
	std::optional<MmDeviceInfo> GetMmDeviceInfo(IMMDevice* mmDevice) noexcept;

	std::string WcToU8(std::wstring_view wstr) noexcept;

	std::wstring U8ToWc(std::string_view u8str) noexcept;

	std::wstring AcpToWc(std::string_view acpStr) noexcept;

	// with '\\'
	std::wstring GetRealCurrentDirectory() noexcept;

	std::string ReadConfigFile() noexcept;

	json GetConfigJson() noexcept;

#if 0
	CComPtr<IMMDevice> GetIMmDeviceById() noexcept;
#endif

	inline CComPtr<IMMDeviceCollection> GetMmDeviceCollection() noexcept {
		HRESULT hr = S_OK;
		CComPtr<IMMDeviceEnumerator> enumerator;
		CComPtr<IMMDeviceCollection> deviceCollection;

		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&enumerator);
		if (FAILED(hr)) {
			spdlog::error(L"GetMmDeviceCollection error (CoCreateInstance error hr={})", hr);
			return deviceCollection;
		}

		hr = enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &deviceCollection);
		if (FAILED(hr)) {
			spdlog::error(L"GetMmDeviceCollection error (EnumAudioEndpoints error hr={})", hr);
		}

		return deviceCollection;
	}

	inline HRESULT QueryVolumeController(IMMDevice* device, IAudioEndpointVolume **controller) noexcept {
		return device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)controller);
	}

	// If this fails, it directly writing error messages into log.
	// If this sets isConfigInvalid to true, don't check the returned HRESULT.
	HRESULT ApplyConfiguredVolume(const json& deviceJson, IMMDevice* device, /* out */ bool& isConfigInvalid) noexcept;

	struct CoTaskMemDeleter {
		void operator()(LPVOID ptr) const noexcept {
			CoTaskMemFree(ptr);
		}
	};
	template<typename T> using UniqueCoTaskPtr = std::unique_ptr<T, CoTaskMemDeleter>;

	inline std::wstring MmDeviceIdToFullPnpId(std::wstring_view mmDeviceId) noexcept {
		return L"SWD\\MMDEVAPI\\" + std::wstring(mmDeviceId);
	}

	inline std::wstring WStringLower(std::wstring_view str) noexcept {
		if (str.empty()) {
			return std::wstring();
		}
		std::wstring ret(str.length(), L'\0');
		std::transform(str.begin(), str.end(), ret.begin(), towlower);
		return ret;
	}

	using LowercaseIdType = std::wstring;

	// 把UTF-16大小写混合ID转换成UTF-8全小写ID (配置文件中的存储格式)
	inline std::string ConfKeyizeId(std::wstring_view rawId) noexcept {
		return WcToU8(WStringLower(rawId));
	}

	bool WriteConfigFile(std::string_view configString) noexcept;

	// Set working dir to where the exe is actually located.
	bool SetWorkingDirToExeDir() noexcept;
}

