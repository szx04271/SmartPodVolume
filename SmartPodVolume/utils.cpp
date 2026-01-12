#include "pch.h"
#include "utils.h"
#include "constants.h"

namespace utils {
	std::wstring GuidToStringW(const GUID& guid) noexcept {
		RPC_WSTR rpcStr;
		auto status = UuidToStringW(&guid, &rpcStr);
		if (status != RPC_S_OK) {
			spdlog::error("UuidToStringW failed! Not enough memory?");
			return std::wstring();
		}
		try {
			std::wstring ret = (const wchar_t*)rpcStr;
			RpcStringFreeW(&rpcStr);
			return ret;
		}
		catch (...) {
			RpcStringFreeW(&rpcStr);
			return std::wstring();
		}
	}

	utils::DeviceInterfaceInfo GetDeviceInterfaceInfoFromPath(LPCWSTR devicePath) noexcept {
		DeviceInterfaceInfo ret;
		PSP_DEVICE_INTERFACE_DETAIL_DATA_W detail = nullptr;
		SP_DEVINFO_DATA devInfoData = {};
		SP_DEVICE_INTERFACE_DATA deviceInterfaceData = {};
		DWORD requiredSize = 0;
		constexpr size_t BUF_CCH_LEN = 1024;
		WCHAR classDesc[BUF_CCH_LEN] = {}, friendlyName[BUF_CCH_LEN], instanceId[BUF_CCH_LEN];

		HDEVINFO hDevInfo = SetupDiCreateDeviceInfoList(nullptr, nullptr);
		if (hDevInfo == INVALID_HANDLE_VALUE) {
			return ret;
		}

		deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

		if (!SetupDiOpenDeviceInterfaceW(hDevInfo, devicePath, 0, &deviceInterfaceData)) {
			goto cleanup;
		}

		devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		SetupDiGetDeviceInterfaceDetailW(hDevInfo, &deviceInterfaceData, nullptr, 0, &requiredSize, nullptr);
		detail = (PSP_DEVICE_INTERFACE_DETAIL_DATA_W)malloc(requiredSize);
		if (!detail) {
			spdlog::error("malloc failed! Not enough memory?");
			goto cleanup;
		}
		detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W); // 确定是填sizeof这个而不是requiredSize?
		if (!SetupDiGetDeviceInterfaceDetailW(hDevInfo, &deviceInterfaceData, detail, requiredSize, nullptr, &devInfoData)) {
			goto cleanup;
		}
		ret.classGuid = GuidToStringW(devInfoData.ClassGuid);

		if (SetupDiGetClassDescriptionW(&devInfoData.ClassGuid, classDesc, BUF_CCH_LEN, nullptr)) {
			ret.classDescription = classDesc;
		}
		if (SetupDiGetDeviceRegistryPropertyW(hDevInfo, &devInfoData, SPDRP_FRIENDLYNAME, nullptr, (PBYTE)friendlyName, sizeof(friendlyName), nullptr)) {
			ret.deviceFriendlyName = friendlyName;
		}
		if (SetupDiGetDeviceInstanceIdW(hDevInfo, &devInfoData, instanceId, BUF_CCH_LEN, nullptr)) {
			ret.deviceInstanceId = instanceId;
		}

	cleanup:
		free(detail);
		detail = nullptr;
		SetupDiDestroyDeviceInfoList(hDevInfo);

		return ret;
	}

	bool IsDeviceSameAsOrDescendantOf(std::wstring_view deviceInstanceId, std::wstring_view targetAncestorId) noexcept {
		if (!_wcsicmp(deviceInstanceId.data(), targetAncestorId.data())) {
			return true;
		}

		DEVINST devInst = 0, parentInst = 0;
		CONFIGRET cr = CM_Locate_DevNodeW(&devInst, (LPWSTR)deviceInstanceId.data(), CM_LOCATE_DEVNODE_NORMAL);
		if (cr != CR_SUCCESS) {
			spdlog::error(L"CM_Locate_DevNodeW failed {}", cr);
			return false;
		}

		while (CM_Get_Parent(&parentInst, devInst, 0) == CR_SUCCESS) {
			WCHAR parentId[512];
			if (CM_Get_Device_IDW(parentInst, parentId, sizeof(parentId) / sizeof(WCHAR), 0) == CR_SUCCESS) {
				spdlog::debug(L"parent id:{}", parentId);
				if (_wcsicmp(parentId, targetAncestorId.data()) == 0) {
					return true;
				}
			}

			devInst = parentInst;
		}

		return false;
	}

	std::list<ATL::CComPtr<IMMDevice>> FindAssociatedMmDevices(LPCWSTR deviceInstanceId) noexcept {
		std::list<CComPtr<IMMDevice>> ret;

		CComPtr<IMMDeviceCollection> deviceCollection = GetMmDeviceCollection();
		if (!deviceCollection) {
			return ret;
		}

		HRESULT hr = S_OK;
		CComPtr<IMMDevice> device;
		CComPtr<IAudioEndpointVolume> endpointVolume;

		UINT count;
		deviceCollection->GetCount(&count);
		for (UINT i = 0; i < count; ++i) {
			device.Release();

			hr = deviceCollection->Item(i, &device);
			if (FAILED(hr)) {
				continue;
			}

			LPWSTR id;
			hr = device->GetId(&id);
			if (FAILED(hr) || id == nullptr) {
				spdlog::debug("5 hr={} id=null", hr);
				continue;
			}

			spdlog::info(L"Checking: Its CoreAudio ID: {}; The PnP(SetupAPI) ID to match: {}", id, deviceInstanceId);
			auto fullPnpId = MmDeviceIdToFullPnpId(id);
			if (IsDeviceSameAsOrDescendantOf(fullPnpId, deviceInstanceId)) {
				spdlog::info(L"Matched.");
				ret.emplace_back(device);				
			}
			else {
				spdlog::info(L"Not matched.");
			}

			CoTaskMemFree(id);
		}

		return ret;
	}

	std::list<std::wstring> GetAncestorDeviceIds(std::wstring_view fullPnpId) noexcept {
		std::list<std::wstring> ret;

		DEVINST devInst = 0, parentInst = 0;
		CONFIGRET cr = CM_Locate_DevNodeW(&devInst, (LPWSTR)fullPnpId.data(), CM_LOCATE_DEVNODE_NORMAL);
		if (cr != CR_SUCCESS) {
			spdlog::error(L"CM_Locate_DevNodeW failed {}", cr);
			return ret;
		}

		while (CM_Get_Parent(&parentInst, devInst, 0) == CR_SUCCESS) {
			WCHAR parentId[512];
			if (CM_Get_Device_IDW(parentInst, parentId, sizeof(parentId) / sizeof(WCHAR), 0) == CR_SUCCESS) {
				ret.emplace_back(parentId);
			}

			devInst = parentInst;
		}

		return ret;
	}

	std::optional<utils::MmDeviceInfo> GetMmDeviceInfo(IMMDevice* mmDevice) noexcept {
		assert(mmDevice);

		MmDeviceInfo ret;
		HRESULT hr = S_OK;
		LPWSTR id;
		hr = mmDevice->GetId(&id);
		if (SUCCEEDED(hr) && id) {
			ret.id.assign(id);
			CoTaskMemFree(id);
		}
		else {
			spdlog::error(L"Failed to query the ID of an MmDevice (interface pointer={:#x}). (What the fuck?)", (uintptr_t)mmDevice);
			return std::nullopt; // if we can't get id, then other attributes are useless. 大败而归。
		}

		CComPtr<IPropertyStore> propertyStore;
		hr = mmDevice->OpenPropertyStore(STGM_READ, &propertyStore);
		if (FAILED(hr)) {
			return ret;
		}

		PROPVARIANT pv;
		hr = propertyStore->GetValue(PKEY_Device_FriendlyName, &pv);
		if (SUCCEEDED(hr) && pv.vt == VT_LPWSTR) {
			ret.friendlyName.assign(pv.pwszVal);
			PropVariantClear(&pv);
		}

		hr = propertyStore->GetValue(PKEY_Device_DeviceDesc, &pv);
		if (SUCCEEDED(hr) && pv.vt == VT_LPWSTR) {
			ret.description.assign(pv.pwszVal);
			PropVariantClear(&pv);
		}

		return ret;
	}

	std::string WcToU8(std::wstring_view wstr) noexcept {
		std::string ret;
		auto bufferSize = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.length() + 1,
			nullptr, 0, nullptr, nullptr);
		if (bufferSize == 0 || bufferSize == 1) {
			return ret;
		}
		ret.resize(bufferSize - 1);
		WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.length() + 1,
			&ret[0], bufferSize, nullptr, nullptr);
		return ret;
	}

	std::wstring U8ToWc(std::string_view u8str) noexcept {
		std::wstring ret;
		auto cchBufferSize = MultiByteToWideChar(CP_UTF8, 0, u8str.data(), u8str.length() + 1,
			nullptr, 0);
		if (cchBufferSize == 0 || cchBufferSize == 1) {
			return ret;
		}
		ret.resize(cchBufferSize - 1);
		MultiByteToWideChar(CP_UTF8, 0, u8str.data(), u8str.length() + 1,
			&ret[0], cchBufferSize);
		
		return ret;
	}

	std::wstring AcpToWc(std::string_view acpStr) noexcept {
		std::wstring ret;
		auto cchBufferSize = MultiByteToWideChar(CP_ACP, 0, acpStr.data(), acpStr.length() + 1,
			nullptr, 0);
		if (cchBufferSize == 0 || cchBufferSize == 1) {
			return ret;
		}
		ret.resize(cchBufferSize - 1);
		MultiByteToWideChar(CP_ACP, 0, acpStr.data(), acpStr.length() + 1,
			&ret[0], cchBufferSize);

		return ret;
	}

	std::wstring GetRealCurrentDirectory() noexcept {
		std::unique_ptr<WCHAR[]> buffer = std::make_unique<WCHAR[]>(1024);
		std::wstring ret;
		auto len = GetModuleFileNameW(nullptr, buffer.get(), 1024);
		if (!len) {
			return ret;
		}
		
		auto lastBackslashPointer = wcsrchr(buffer.get(), L'\\');
		if (!lastBackslashPointer) {
			return ret;
		}
		lastBackslashPointer[1] = 0;
		ret.assign(buffer.get());
		return ret;
	}

	std::string ReadConfigFile() noexcept {
		std::string ret;

		auto filePath = GetRealCurrentDirectory();
		if (filePath.empty()) {
			spdlog::error(L"Error opening config file (can't get current dir)");
			return ret;
		}
		filePath += CONFIG_FILE_NAME;

		FILE* file = nullptr;
		auto err = _wfopen_s(&file, filePath.c_str(), L"rb, ccs=UTF-8");
		if (err != 0) {
			if (err != ENOENT) {
				spdlog::error(L"Error opening config file (errno={}).", err);
			}
			return ret;
		}

		fseek(file, 0, SEEK_END);
		auto fileSize = ftell(file);
		if (fileSize == 0) {
			fclose(file);
			return ret;
		}
		rewind(file);

		ret.resize(fileSize);
		fread(&ret[0], fileSize, 1, file);
		fclose(file);

		return ret;
	}

	json GetConfigJson() noexcept {
		auto configJsonString = utils::ReadConfigFile();
		json j;

		if (configJsonString.size()) {
			try {
				j = json::parse(configJsonString);
			}
			catch (std::exception& e) {
				spdlog::warn(L"Bad config content ({}). Deleting...", utils::AcpToWc(e.what()));
				DeleteFileW((utils::GetRealCurrentDirectory() + CONFIG_FILE_NAME).c_str());
				j = json();
			}
		}
		else {
			spdlog::info("Config is empty.");
		}

		return j;
	}

#if 0
	ATL::CComPtr<IMMDevice> GetIMmDeviceById() noexcept {
		// not implemented
		assert(false);
		return CComPtr<IMMDevice>();
	}
#endif

	HRESULT ApplyConfiguredVolume(const json& deviceJson, IMMDevice* device, /* out */ bool& isConfigInvalid) noexcept {
		isConfigInvalid = false;

		HRESULT hr = S_OK;
		CComPtr<IAudioEndpointVolume> endpointVol;
		hr = utils::QueryVolumeController(device, &endpointVol);
		if (FAILED(hr)) {
			spdlog::error(L"QueryVolumeController failed with hr={}", hr);
			return hr;
		}

		bool volumeConfigValid = false, muteConfigValid = false;
		bool volumeSuccess = false, muteSuccess = false;

		auto itVolume = deviceJson.find(conf_key::EXPECTED_VOLUME);
		if (itVolume != deviceJson.end() && itVolume->is_number()) {
			auto expectedVol = itVolume->get<float>();
			if (0.f <= expectedVol && expectedVol <= 100.f) {
				spdlog::info(L"Expected vol: {}%", expectedVol);
				volumeConfigValid = true;
				hr = endpointVol->SetMasterVolumeLevelScalar(expectedVol / 100.f, nullptr);
			}
		}
		if (!volumeConfigValid) {
			isConfigInvalid = true;
			spdlog::info(L"FAILED to set volume (invalid config)");
		}
		else if (FAILED(hr)) {
			spdlog::info(L"FAILED to set volume (hr={})", hr);
		}
		else {
			spdlog::info(L"SUCCESSFULLY set volume");
			volumeSuccess = true;
		}

		HRESULT hr2 = S_OK;
		auto itMute = deviceJson.find(conf_key::MUTE);
		if (itMute != deviceJson.end() && itMute->is_boolean()) {
			muteConfigValid = true;
			auto expectedMute = itMute->get<bool>();
			hr2 = endpointVol->SetMute(expectedMute, nullptr);
		}
		if (!muteConfigValid) {
			isConfigInvalid = true;
			spdlog::info(L"FAILED to set mute (invalid config)");
		}
		else if (FAILED(hr)) {
			spdlog::info(L"FAILED to set mute (hr={})", hr);
		}
		else {
			spdlog::info(L"SUCCESSFULLY set mute");
			muteSuccess = true;
		}

		return volumeSuccess && muteSuccess ? S_OK : (SUCCEEDED(hr) ? hr2 : hr);
	}

	bool WriteConfigFile(std::string_view configString) noexcept {
		FILE* file = nullptr;
		// TODO: modify this to support BOM
		auto err = _wfopen_s(&file, (GetRealCurrentDirectory() + CONFIG_FILE_NAME).c_str(), L"wb, ccs=UTF-8");
		if (err) {
			spdlog::error(L"Error opening config file for writing (errno={}).", err);
			return false;
		}

		auto elemWritten = fwrite(configString.data(), configString.size(), 1, file);
		if (elemWritten != 1) {
			spdlog::error(L"Error writing config file ");
			fclose(file);
			return false;
		}

		spdlog::info(L"Successfully wrote config file.");
		fclose(file);
		return true;
	}

}