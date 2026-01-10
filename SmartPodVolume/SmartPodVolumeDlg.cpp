
// SmartPodVolumeDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "SmartPodVolume.h"
#include "SmartPodVolumeDlg.h"
#include "utils.h"
#include "NewDeviceDlg.h"
#include "VolumeSetFailDlg.h"
#include "constants.h"


// MFC's DEBUG_NEW itself causes bugs. 

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// CSmartPodVolumeDlg 对话��?



CSmartPodVolumeDlg::CSmartPodVolumeDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_SMARTPODVOLUME_DIALOG, pParent) {
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSmartPodVolumeDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSmartPodVolumeDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_DEVICECHANGE()
	ON_BN_CLICKED(IDC_DISPLAY_NEW_DEVICE_DIALOG, &CSmartPodVolumeDlg::OnBnClickedDisplayNewDeviceDialog)
	ON_BN_CLICKED(IDC_DISPLAY_VOLUME_SET_FAIL_DIALOG, &CSmartPodVolumeDlg::OnBnClickedDisplayVolumeSetFailDialog)
	ON_MESSAGE(WM_REGISTERED_DEVICE_VOLUME_CHANGED, &CSmartPodVolumeDlg::OnRegisteredDeviceVolumeChanged)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CSmartPodVolumeDlg 消息处理程序

BOOL CSmartPodVolumeDlg::OnInitDialog() {
	CDialog::OnInitDialog();

	// 设置此对话框的图标��? 当应用程序主窗口不是对话框时，框架将自动
	//  执行此操��?
	SetIcon(m_hIcon, TRUE);			// 设置大图��?
	SetIcon(m_hIcon, FALSE);		// 设置小图��?

	DEV_BROADCAST_DEVICEINTERFACE_W notificationFilter = {};
	notificationFilter.dbcc_size = sizeof(notificationFilter);
	notificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	m_hDevNotify = RegisterDeviceNotificationW(
		this->m_hWnd,
		&notificationFilter,
		DEVICE_NOTIFY_WINDOW_HANDLE | DEVICE_NOTIFY_ALL_INTERFACE_CLASSES
	);
	if (!m_hDevNotify) {
		spdlog::error(L"RegisterDeviceNotificationW failed (lastError={}). Exiting.", GetLastError());
		::MessageBoxW(nullptr, L"RegisterDeviceNotificationW 失败，无法监测新音频设备接入。程序将退出。", L"SmartPodVolume 错误", MB_ICONERROR);
		PostMessageW(WM_CLOSE);
	}
	else {
		spdlog::info(L"RegisterDeviceNotificationW succeeded. Listening for device changes.");
	}

	RegisterVolumeNotificationsForAllKnown();

	return TRUE;  // 除非将焦点设置到控件，否则返��?TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标��? 对于使用文档/视图模型��?MFC 应用程序��?
//  这将由框架自动完成��?

void CSmartPodVolumeDlg::OnPaint() {
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示��?
HCURSOR CSmartPodVolumeDlg::OnQueryDragIcon() {
	return static_cast<HCURSOR>(m_hIcon);
}

// 1.“耳机已开机，但电脑蓝牙没开，打开蓝牙，电脑连上耳机”
// 2.“电脑蓝牙已打开，但耳机没开机，打开耳机，电脑连上耳机”
// 以上两种方式会使OnDevicechange收到不同的信息。第一种会同时有“蓝牙”、
// “网络适配器”、“系统设备”、“声音、视频和游戏控制器”、“音频输入和输出”设备的DBT_DEVICEARRIVAL消息，
// 但第二种就只收得到后两种设备的该消息。
// 这两种方式似乎都有概率触发“默认音量90%多甚至100%”这一bug。暂未发现
// 这两种方式在此方面的差异。

BOOL CSmartPodVolumeDlg::OnDeviceChange(UINT nEventType, DWORD_PTR dwData) {
	spdlog::debug("OnDevicechange eventType={} data={}", nEventType, dwData);

	PDEV_BROADCAST_HDR pHdr = reinterpret_cast<PDEV_BROADCAST_HDR>(dwData);
	if (pHdr && pHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
		PDEV_BROADCAST_DEVICEINTERFACE_W pDevInf = reinterpret_cast<PDEV_BROADCAST_DEVICEINTERFACE_W>(pHdr);
		if (nEventType == DBT_DEVICEARRIVAL) {
			OnDeviceArrived(pDevInf);
		}
		else if (nEventType == DBT_DEVICEREMOVECOMPLETE) {
			OnDeviceRemoved(pDevInf);
		}
	}

	return TRUE;
}

void CSmartPodVolumeDlg::OnDeviceRemoved(PDEV_BROADCAST_DEVICEINTERFACE_W devInf) {
	auto info = utils::GetDeviceInterfaceInfoFromPath(devInf->dbcc_name);
	if (!info.deviceInstanceId.length()) {
		spdlog::warn(L"A device with unknown id is removed. Ignored.");
		return;
	}

	std::list<decltype(m_registeredCallbacks)::iterator> toBeRemoved;
	for (auto it = m_registeredCallbacks.begin(); it != m_registeredCallbacks.end(); ++it) {
		for (auto& derivedFromId : it->fromDevInfIds) {
			if (!_wcsicmp(derivedFromId.c_str(), info.deviceInstanceId.c_str())) {
				it->endpointVolume->UnregisterControlChangeNotify(it->callback);
				// dont remove here, otherwise `it` becomes invalid and `++it` crushes
				toBeRemoved.emplace_back(it);
				spdlog::info(L"Registered device (id={}) is being unregistered because it or its ancestor is removed from computer.",
					it->mmDeviceId);
			}
		}
	}

	for (auto& it : toBeRemoved) {
		m_registeredCallbacks.erase(it); // this automatically calls Release() of any COM pointer inside
	}
}

void CSmartPodVolumeDlg::OnDestroy() {
	CDialog::OnDestroy();

	if (m_hDevNotify) {
		UnregisterDeviceNotification(m_hDevNotify);
		m_hDevNotify = nullptr;
		spdlog::info(L"Unregistered device notification.");
	}

	UnregisterAllVolumeNotifications();
	spdlog::info(L"Unregistered volume notification.");

	SaveAllVolumes();
}

void CSmartPodVolumeDlg::OnBnClickedDisplayNewDeviceDialog() {
	utils::MmDeviceInfo info;
	info.friendlyName = L"并夕夕￥9.9包邮耳机";
	info.description = L"耳机";
	info.id = L"{XXXXXXX}.{XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX}";

	CNewDeviceDlg* dlg = new CNewDeviceDlg(info);
	dlg->Create(IDD_NEW_DEVICE);
	dlg->ShowWindow(SW_SHOWNORMAL);
}

void CSmartPodVolumeDlg::OnBnClickedDisplayVolumeSetFailDialog() {
	utils::MmDeviceInfo info;
	info.friendlyName = L"劣质耳机";
	info.description = L"耳机";
	info.id = L"{XXXXXXX}.{XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX}";

	CVolumeSetFailDlg* dlg = new CVolumeSetFailDlg(E_NOTIMPL, info);
	dlg->Create(IDD_VOLUME_SET_FAIL);
	dlg->ShowWindow(SW_SHOWNORMAL);
}

CSmartPodVolumeDlg::RegisteredDevice* CSmartPodVolumeDlg::RegisterVolumeNotification(IMMDevice* device, std::wstring_view fromDevInfId) {
	LPWSTR _rawId;
	HRESULT hr = device->GetId(&_rawId);
	if (FAILED(hr)) {
		spdlog::error(L"Error getting mmDevice id");
		return nullptr;
	}

	// automatically frees COM string when return
	std::unique_ptr<WCHAR, std::function<void(WCHAR*)>> id(_rawId, [](WCHAR* ptr) {CoTaskMemFree(ptr); });

	RegisteredDevice* thisDevice = nullptr;
	for (auto& registeredDevice : m_registeredCallbacks) {
		if (_wcsicmp(registeredDevice.mmDeviceId.c_str(), id.get()) == 0) {
			// already registered, only append fromDevInfIds and return
			thisDevice = &registeredDevice;
			break;
		}
	}

	if (!thisDevice) {
		// only register if not alerady registered

		CComPtr<IAudioEndpointVolume> endpointVolume;
		hr = utils::QueryVolumeController(device, &endpointVolume);
		if (FAILED(hr)) {
			spdlog::error(L"Error obtaining IAudioEndpointVolume interface pointer (hr={})", hr);
			return nullptr;
		}

		MyVolumeChangeCallback* callback = new MyVolumeChangeCallback(id.get());
		hr = endpointVolume->RegisterControlChangeNotify(callback);
		if (FAILED(hr)) {
			spdlog::error(L"RegisterControlChangeNotify (for device with id={}) failed (hr={})",
				id.get(), hr);
			callback->Release();
			return nullptr;
		}

		// registration success

		float volume = 0;
		BOOL mute = FALSE;
		hr = endpointVolume->GetMasterVolumeLevelScalar(&volume);
		if (FAILED(hr)) {
			spdlog::error(L"Error getting initial [volume] for device with id={}, hr={}. Defaulted to 0.",
				id.get(), hr);
		}
		hr = endpointVolume->GetMute(&mute);
		if (FAILED(hr)) {
			spdlog::error(L"Error getting initial [mute] for device with id={}, hr={}. Defaulted to false.",
				id.get(), hr);
		}

		callback->m_volumeInfo.volumePercent = volume * 100.f;
		callback->m_volumeInfo.mute = mute ? true : false;

		RegisteredDevice registeredDevice = {
			endpointVolume,callback,id.get()
		};
		thisDevice = &m_registeredCallbacks.emplace_back(std::move(registeredDevice));
		spdlog::info(L"Successfully registered volume change notify for id={}", id.get());

		m_volumesToBeSaved[callback->GetLowercaseDeviceId()] = callback->m_volumeInfo;
		SaveAllVolumes();
		//SendMessageW(WM_REGISTERED_DEVICE_VOLUME_CHANGED, (WPARAM)&callback, 0);
	}

	thisDevice->fromDevInfIds.emplace_back(fromDevInfId);
	//spdlog::info(L"Added devInfId {} to registered device whose mmDeviceId is {}", fromDevInfId.data(), id.get());

	return thisDevice;
}

void CSmartPodVolumeDlg::RegisterVolumeNotificationsForAllKnown() {
	auto deviceCollection = utils::GetMmDeviceCollection();
	if (!deviceCollection) {
		return;
	}
	UINT deviceCount = 0;
	deviceCollection->GetCount(&deviceCount);
	if (deviceCount == 0) {
		return;
	}

	struct MmDeviceAndId {
		CComPtr<IMMDevice> device;
		utils::UniqueCoTaskPtr<WCHAR> id;
	};
	std::list<MmDeviceAndId> connectedMmDevices;

	HRESULT hr = S_OK;
	for (UINT i = 0; i < deviceCount; ++i) {
		CComPtr<IMMDevice> device;
		hr = deviceCollection->Item(i, &device);
		if (FAILED(hr)) {
			spdlog::warn(L"Error getting interface of {}th(begin from 0) item of IMMDeviceCollection. Skip.", i);
			continue;
		}

		LPWSTR _rawId;
		hr = device->GetId(&_rawId);
		if (FAILED(hr)) {
			spdlog::warn(L"Error getting id of mmdevice interface {}", (uintptr_t)device.p);
			continue;
		}

		CComPtr<IPropertyStore> propStore;
		device->OpenPropertyStore(STGM_READ, &propStore);
		PROPVARIANT pv;
		hr = propStore->GetValue(PKEY_Device_ContainerId, &pv);
		if (SUCCEEDED(hr) && pv.vt == VT_CLSID) {
			spdlog::info(L"[MYDEBUG] CONTAINER ID = {}", utils::GuidToStringW(*pv.puuid));
		}
		else {
			spdlog::info(L"[MYDEBUG] CONTAINER ID FAIL");
		}
		PropVariantClear(&pv);

		connectedMmDevices.emplace_back(std::move(device), utils::UniqueCoTaskPtr<WCHAR>(_rawId));
	}

	try {
		auto j = utils::GetConfigJson(); // TODO: optimize this to avoid reading disk too frequently
		if (!j.is_object()) {
			return;
		}

		auto& whiteListJson = j[conf_key::WHITELIST];
		for (auto& [deviceId, deviceInfo] : whiteListJson.items()) {
			for (auto& connectedMmDevice : connectedMmDevices) {
				if (utils::ConfKeyizeId(connectedMmDevice.id.get()) == deviceId) {
					// found dest device
					// register here
					auto fullId = utils::MmDeviceIdToFullPnpId(connectedMmDevice.id.get());
					auto registeredDevice = RegisterVolumeNotification(connectedMmDevice.device, fullId);
					if (registeredDevice) {
						auto ancestors = utils::GetAncestorDeviceIds(fullId);
						registeredDevice->fromDevInfIds.splice(registeredDevice->fromDevInfIds.end(), ancestors);
					}
				}
			}
		}
	}
	catch (std::exception& e) {
		spdlog::error(L"RegisterVolumeNotificationsForAllKnown error ({})", utils::AcpToWc(e.what()));
	}
	catch (...) {
		spdlog::error(L"RegisterVolumeNotificationsForAllKnown error (unknown error)");
	}
}

void CSmartPodVolumeDlg::UnregisterAllVolumeNotifications() {
	for (auto& device : m_registeredCallbacks) {
		device.endpointVolume->UnregisterControlChangeNotify(device.callback);
		device.callback->Release();
	}
}

void CSmartPodVolumeDlg::OnDeviceArrived(PDEV_BROADCAST_DEVICEINTERFACE_W devInf) {
	// devInf->dbcc_classguid是设备接口类GUID，不是设备安装类GUID，不能直接传给SetupDiGetClassDescriptionW

	auto diInfo = utils::GetDeviceInterfaceInfoFromPath(devInf->dbcc_name);
	spdlog::info(L"Device arrived, id={}, name= {}, setup class guid = {}, setup class name = {}, device name = {}", diInfo.deviceInstanceId, devInf->dbcc_name, diInfo.classGuid,
		diInfo.classDescription, diInfo.deviceFriendlyName);

	auto mmDevices = utils::FindAssociatedMmDevices(diInfo.deviceInstanceId.c_str());
	if (mmDevices.empty()) {
		// no mmDevice attached to it
		return;
	}

	for (auto& mmDevice : mmDevices) {
		auto mmDeviceInfo = utils::GetMmDeviceInfo(mmDevice);
		if (mmDeviceInfo.has_value()) {
			spdlog::info(L"Discovered MMDevice (friendlyName={}, id={}, desc={})", mmDeviceInfo->friendlyName,
				mmDeviceInfo->id, mmDeviceInfo->description);

			// lookup all known mmDevices
			try {
				auto j = utils::GetConfigJson();
				if (!j.is_object()) {
					OnNewMmDevice(*mmDeviceInfo);
					return;
				}

				auto FindDeviceInList = [&j, &mmDeviceInfo](const char* listKey) -> json {
					auto& listJson = j[listKey];
					if (!listJson.is_object()) {
						return json();
					}
					auto& deviceJson = listJson[utils::ConfKeyizeId(mmDeviceInfo->id)];
					return deviceJson.is_object() ? deviceJson : json();
				};

				if (!FindDeviceInList(conf_key::BLACKLIST).is_null()) {
					spdlog::info("This device is in blacklist. Ignore.");
					return;
				}

				json deviceJson = FindDeviceInList(conf_key::WHITELIST);
				bool white = !deviceJson.is_null();

				if (white) {
					bool failBecauseInvalidConfig;
					HRESULT hr = utils::ApplyConfiguredVolume(deviceJson, mmDevice, failBecauseInvalidConfig);

					RegisterVolumeNotification(mmDevice, diInfo.deviceInstanceId);

					if (!failBecauseInvalidConfig && FAILED(hr)) {
						CVolumeSetFailDlg* setFailDlg = new CVolumeSetFailDlg(hr, *mmDeviceInfo);
						setFailDlg->Create(IDD_VOLUME_SET_FAIL);
						setFailDlg->ShowWindow(SW_SHOWNORMAL);
					}
				}
				else {
					// this is a new device
					OnNewMmDevice(*mmDeviceInfo);
				}
			}
			catch (std::exception& e) {
				spdlog::error(L"Error looking up known devices ({})", utils::AcpToWc(e.what()));
			}
			catch (...) {
				spdlog::error(L"Error looking up known devices (unknown error)");
			}
		}
		else {
			spdlog::warn(L"Discovered MMDevice but no info. WTF?");
		}
	}
}

afx_msg LRESULT CSmartPodVolumeDlg::OnRegisteredDeviceVolumeChanged(WPARAM wParam, LPARAM lParam) {
	if (!wParam) {
		spdlog::error(L"OnRegisteredDeviceVolumeChanged finds wParam is zero... BUG!");
		return 114514;
	}

	auto callback = (MyVolumeChangeCallback*)wParam;
	m_volumesToBeSaved[callback->GetLowercaseDeviceId()] = callback->m_volumeInfo;
	SetTimer(AUTO_SAVE_CONFIG_TIMER_ID, 10000, nullptr); // this resets the timer before sets it

	return 0;
}

void CSmartPodVolumeDlg::OnTimer(UINT_PTR nIDEvent) {
	if (nIDEvent == AUTO_SAVE_CONFIG_TIMER_ID) {
		if (SaveAllVolumes()) {
			// if all successfully saved, kill the timer
			KillTimer(AUTO_SAVE_CONFIG_TIMER_ID);
		}
	}

	CDialog::OnTimer(nIDEvent);
}

bool CSmartPodVolumeDlg::SaveAllVolumes() noexcept {
	try {
		auto configJson = utils::GetConfigJson();

		auto& whiteList = configJson[conf_key::WHITELIST];
		if (whiteList.is_object() == false) {
			whiteList = {}; // empty object
		}

		for (auto& [wcId, info] : m_volumesToBeSaved) {
			auto id = utils::ConfKeyizeId(wcId);
			auto& deviceJson = whiteList[id];
			if (!deviceJson.is_object()) {
				deviceJson = {};
			}
			deviceJson[conf_key::EXPECTED_VOLUME] = info.volumePercent;
			deviceJson[conf_key::MUTE] = info.mute;
		}

		auto configString = configJson.dump();
		auto writeSuccess = utils::WriteConfigFile(configString);
		if (!writeSuccess) {
			spdlog::error(L"Auto save config failed. Try again 10 seconds later.");
		}
		return writeSuccess;
	}
	catch (std::exception& e) {
		spdlog::error(L"SaveAllVolumes error: {}", utils::AcpToWc(e.what()));
		return false;
	}
	catch (...) {
		spdlog::error(L"SaveAllVolumes error (unknown error)");
		return false;
	}
}
