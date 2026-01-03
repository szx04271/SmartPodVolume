#include "pch.h"
#include "utils.h"

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

	utils::DeviceInfo GetDeviceInfoFromPath(LPCWSTR devicePath) noexcept {
		DeviceInfo ret;
		PSP_DEVICE_INTERFACE_DETAIL_DATA_W detail = nullptr;
		SP_DEVINFO_DATA devInfoData = {};
		HDEVINFO hDevInfo = SetupDiCreateDeviceInfoList(nullptr, nullptr);
		if (hDevInfo == INVALID_HANDLE_VALUE) {
			return ret;
		}

		SP_DEVICE_INTERFACE_DATA deviceInterfaceData = {};
		deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

		if (!SetupDiOpenDeviceInterfaceW(hDevInfo, devicePath, 0, &deviceInterfaceData)) {
			goto cleanup;
		}

		devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		DWORD requiredSize = 0;
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

		constexpr size_t BUF_CCH_LEN = 1024;
		WCHAR classDesc[BUF_CCH_LEN] = {}, friendlyName[BUF_CCH_LEN], instanceId[BUF_CCH_LEN];
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

	bool IsDeviceSameAsOrDescendantOf(LPCWSTR deviceInstanceId, LPCWSTR targetAncestorId) noexcept {
		if (!_wcsicmp(deviceInstanceId, targetAncestorId)) {
			return true;
		}

		DEVINST devInst = 0, parentInst = 0;
		CONFIGRET cr = CM_Locate_DevNodeW(&devInst, (LPWSTR)deviceInstanceId, CM_LOCATE_DEVNODE_NORMAL);
		if (cr != CR_SUCCESS) {
			spdlog::error(L"CM_Locate_DevNodeW failed {}", cr);
			return false;
		}

		while (CM_Get_Parent(&parentInst, devInst, 0) == CR_SUCCESS) {
			WCHAR parentId[512];
			if (CM_Get_Device_IDW(parentInst, parentId, sizeof(parentId) / sizeof(WCHAR), 0) == CR_SUCCESS) {
				spdlog::debug(L"parent id:{}", parentId);
				if (_wcsicmp(parentId, targetAncestorId) == 0) {
					return true;
				}
			}

			devInst = parentInst;
		}

		return false;
	}

	bool SetDeviceVolume(LPCWSTR deviceInstanceId, float volumePercent) noexcept {
		HRESULT hr = S_OK;
		CComPtr<IMMDeviceEnumerator> enumerator;
		CComPtr<IMMDeviceCollection> deviceCollection;
		CComPtr<IMMDevice> device;
		CComPtr<IAudioEndpointVolume> endpointVolume;
		bool foundAny = false;

		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&enumerator);
		if (FAILED(hr)) {
			return foundAny;
		}

		hr = enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &deviceCollection);
		if (FAILED(hr)) {
			return foundAny;
		}

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
			CStringW fullPnpId = L"SWD\\MMDEVAPI\\";
			fullPnpId += id;
			if (IsDeviceSameAsOrDescendantOf(fullPnpId, deviceInstanceId)) {
				spdlog::info(L"Matched.");
				hr = device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)&endpointVolume);
				if (SUCCEEDED(hr)) {
					hr = endpointVolume->SetMasterVolumeLevelScalar(volumePercent / 100.0f, nullptr);
					if (SUCCEEDED(hr)) {
						foundAny = true;
					}
				}
			}
			else {
				spdlog::info(L"Not matched.");
			}

			CoTaskMemFree(id);
		}

		return foundAny;
	}

}