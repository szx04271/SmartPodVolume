
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
	ON_MESSAGE(WM_NEWDEVICEDLG_CLOSED, &CSmartPodVolumeDlg::OnNewdevicedlgClosed)
	ON_MESSAGE(WM_NEW_DEVICE_NEEDS_REGISTRATION, &CSmartPodVolumeDlg::OnNewDeviceNeedsRegistration)
	ON_WM_QUERYENDSESSION()
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()


// CSmartPodVolumeDlg 消息处理程序

BOOL CSmartPodVolumeDlg::OnInitDialog() {
	CDialog::OnInitDialog();

	// 设置此对话框的图标��? 当应用程序主窗口不是对话框时，框架将自动
	//  执行此操��?
	SetIcon(m_hIcon, TRUE);			// 设置大图��?
	SetIcon(m_hIcon, FALSE);		// 设置小图��?

	if (__argc == 2 && !wcscmp(__wargv[1], L"--test-mode")) {
		m_testMode = true;

		EnableWindow(TRUE);
	}

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

	std::thread wizardCommunicationThread(&CSmartPodVolumeDlg::WizardCommunicationProc, this);
	wizardCommunicationThread.detach();

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
	auto mmDeviceCollection = utils::GetMmDeviceCollection();
	if (!mmDeviceCollection) {
		spdlog::error(L"Failed to get MMDeviceCollection");
	}
	UINT mmDeviceCount;
	HRESULT hr = mmDeviceCollection->GetCount(&mmDeviceCount);
	if (FAILED(hr)) {
		spdlog::error(L"Failed to fetch MmDevice count (hr={})", hr);
		return;
	}

	std::set<std::wstring> connectedDeviceIds;
	for (UINT i = 0; i < mmDeviceCount; ++i) {
		CComPtr<IMMDevice> device;
		hr = mmDeviceCollection->Item(i, &device);
		if (FAILED(hr)) {
			spdlog::error(L"Failed to fetch the {}th MmDevice.", i);
			continue;
		}
		LPWSTR id;
		hr = device->GetId(&id);
		if (FAILED(hr)) {
			spdlog::error(L"Failed to get ID of the {}th MmDevice.", i);
			continue;
		}

		connectedDeviceIds.emplace(utils::WStringLower(id));

		CoTaskMemFree(id);
	}

	for (auto it = m_registeredCallbacks.begin(); it != m_registeredCallbacks.end();) {
		if (!connectedDeviceIds.contains(it->first)) {
			it->second.endpointVolume->UnregisterControlChangeNotify(it->second.callback);
			spdlog::info(L"Unregistered registered device {} because it's removed from computer.", it->first);

			it = m_registeredCallbacks.erase(it);
		}
		else ++it;
	}
	
	for (auto it = m_newMmDeviceWindows.begin(); it != m_newMmDeviceWindows.end();) {
		if (!connectedDeviceIds.contains(it->first)) {
			it->second->m_dontNotifyMainWndOnDestroy = true;
			it->second->DestroyWindow();
			spdlog::info(L"The NewDeviceDlg of new MMDevice {} is closed because it's removed from computer.", it->first);

			it = m_newMmDeviceWindows.erase(it);
		}
		else ++it;
	}

	if (!m_volumesToBeSaved.empty()) {
		spdlog::info(L"There are {} volumes to be saved till now. Saving them all...", m_volumesToBeSaved.size());
		SaveAllVolumes();
		KillTimer(AUTO_SAVE_CONFIG_TIMER_ID);
	}
}

void CSmartPodVolumeDlg::OnDestroy() {
	CDialog::OnDestroy();

	if (m_hDevNotify) {
		UnregisterDeviceNotification(m_hDevNotify);
		m_hDevNotify = nullptr;
		spdlog::info(L"Unregistered all device notifications because the program is being closed.");
	}

	UnregisterAllVolumeNotifications();
	spdlog::info(L"Unregistered all volume notifications because the program is being closed.");

	if (m_volumesToBeSaved.size()) {
		SaveAllVolumes();
		//KillTimer(AUTO_SAVE_CONFIG_TIMER_ID);
	}
}

void CSmartPodVolumeDlg::OnBnClickedDisplayNewDeviceDialog() {
	utils::MmDeviceInfo info;
	info.friendlyName = L"并夕夕￥9.9包邮耳机";
	info.description = L"耳机";
	info.id = L"{XXXXXXX}.{XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX}";

	CNewDeviceDlg* dlg = new CNewDeviceDlg(info, CComPtr<IMMDevice>());
	dlg->m_dontNotifyMainWndOnDestroy = true;
	dlg->DoNonModal();
}

void CSmartPodVolumeDlg::OnBnClickedDisplayVolumeSetFailDialog() {
	utils::MmDeviceInfo info;
	info.friendlyName = L"劣质耳机";
	info.description = L"耳机";
	info.id = L"{YYYYYYY}.{YYYYYYYYYYYYYYYYYYYYYYYYYYYYYY}";

	CVolumeSetFailDlg* dlg = new CVolumeSetFailDlg(E_NOTIMPL, info);
	dlg->DoNonModal();
}

CSmartPodVolumeDlg::RegisteredDevice* CSmartPodVolumeDlg::RegisterVolumeNotification(IMMDevice* device) {
	LPWSTR _rawId;
	HRESULT hr = device->GetId(&_rawId);
	if (FAILED(hr)) {
		spdlog::error(L"Error getting mmDevice id");
		return nullptr;
	}

	// automatically frees COM string when return
	std::unique_ptr<WCHAR, std::function<void(WCHAR*)>> id(_rawId, [](WCHAR* ptr) {CoTaskMemFree(ptr); });

	auto itToPrevious = m_registeredCallbacks.find(utils::WStringLower(id.get()));
	if (itToPrevious != m_registeredCallbacks.end()) {
		return &itToPrevious->second;
	}

	// only register if not alerady registered

	CComPtr<IAudioEndpointVolume> endpointVolume;
	hr = utils::QueryVolumeController(device, &endpointVolume);
	if (FAILED(hr)) {
		spdlog::error(L"Error obtaining IAudioEndpointVolume interface pointer (hr={})", hr);
		return nullptr;
	}

	// CComPtr constructor `CComPtr<T>(T* p)` DOES AddRef!!!!
	// So don't use `CComPtr<T> ptr(new T)` which will cause ptr's initial ref count is 2
	// and will only be `Release`d once!
	CComPtr<MyVolumeChangeCallback> callback;
	callback.Attach(new MyVolumeChangeCallback(id.get())); // `Attach` does NOT AddRef

	hr = endpointVolume->RegisterControlChangeNotify(callback);
	if (FAILED(hr)) {
		spdlog::error(L"RegisterControlChangeNotify (for device with id={}) failed (hr={})",
			id.get(), hr);
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
		endpointVolume,callback
	};
	auto ret = &(m_registeredCallbacks[callback->GetLowercaseDeviceId()] = std::move(registeredDevice));

	spdlog::info(L"Successfully registered volume change notify for id={}", id.get());

	m_volumesToBeSaved[callback->GetLowercaseDeviceId()] = callback->m_volumeInfo;
	SaveAllVolumes();
	spdlog::debug(L"saved vol {}%", callback->m_volumeInfo.volumePercent);
	//SendMessageW(WM_REGISTERED_DEVICE_VOLUME_CHANGED, (WPARAM)&callback, 0);

	return ret;
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

		connectedMmDevices.emplace_back(std::move(device), utils::UniqueCoTaskPtr<WCHAR>(_rawId));
	}

	try {
		auto j = utils::GetConfigJson();
		if (!j.is_object()) {
			return;
		}

		auto& whiteListJson = j[conf_key::WHITELIST];
		for (auto& [deviceId, deviceInfo] : whiteListJson.items()) {
			for (auto& connectedMmDevice : connectedMmDevices) {
				if (utils::ConfKeyizeId(connectedMmDevice.id.get()) == deviceId) {
					// found dest device
					// register here
					auto registeredDevice = RegisterVolumeNotification(connectedMmDevice.device);
					if (registeredDevice) {
						spdlog::info(L"Successfully registered volume notification for mmdevice {}", connectedMmDevice.id.get());
					}
					else {
						spdlog::info(L"Failed to register volume notification for mmdevice {}", connectedMmDevice.id.get());
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
	for (auto& [_, deviceInfo] : m_registeredCallbacks) {
		deviceInfo.endpointVolume->UnregisterControlChangeNotify(deviceInfo.callback);
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
					OnNewMmDevice(*mmDeviceInfo, mmDevice);
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

					RegisterVolumeNotification(mmDevice);

					if (!failBecauseInvalidConfig && FAILED(hr)) {
						CVolumeSetFailDlg* setFailDlg = new CVolumeSetFailDlg(hr, *mmDeviceInfo);
						setFailDlg->DoNonModal();
					}
				}
				else {
					// this is a new device
					OnNewMmDevice(*mmDeviceInfo, mmDevice);
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

void CSmartPodVolumeDlg::OnNewMmDevice(const utils::MmDeviceInfo& info, const CComPtr<IMMDevice>& device) {
	auto lowerId = utils::WStringLower(info.id);
	if (m_newMmDeviceWindows.contains(lowerId)) {
		return;
	}

	spdlog::info(L"This is a new device. Asking user for choice.");
	auto newDeviceDlg = new CNewDeviceDlg(info, device);
	m_newMmDeviceWindows[lowerId] = newDeviceDlg;
	newDeviceDlg->DoNonModal();
}

afx_msg LRESULT CSmartPodVolumeDlg::OnRegisteredDeviceVolumeChanged(WPARAM wParam, LPARAM lParam) {
	if (!wParam) {
		spdlog::error(L"OnRegisteredDeviceVolumeChanged finds wParam is zero... BUG!");
		return 114514;
	}

	auto callback = (MyVolumeChangeCallback*)wParam;
	m_volumesToBeSaved[callback->GetLowercaseDeviceId()] = callback->m_volumeInfo;
	SetTimer(AUTO_SAVE_CONFIG_TIMER_ID, 5000, nullptr); // this resets the timer before sets it

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

		auto writeSuccess = utils::WriteConfigJson(configJson);
		if (!writeSuccess) {
			spdlog::error(L"Auto save config failed. Try again 10 seconds later.");
		}
		else {
			m_volumesToBeSaved.clear();
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

void CSmartPodVolumeDlg::WizardCommunicationProc() noexcept {
	int retryRemining = 5;
	bool toStop = false;

	constexpr BYTE aliveSignalBuf[1] = { BKGND_PROCESS_ALIVE_SIGNAL };
	
	// in sub thread, dont operate UI directly
	while (!toStop) {
		WaitNamedPipeW(BKGND_PROCESS_PIPE_NAME, NMPWAIT_WAIT_FOREVER);
		HANDLE hPipe = CreateFileW(BKGND_PROCESS_PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, nullptr,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hPipe == INVALID_HANDLE_VALUE) {
			auto err = GetLastError();
			if (err == ERROR_FILE_NOT_FOUND) {
				// server(wizard) not running
				Sleep(1000);
				continue;
			}
			else {
				spdlog::warn(L"CreateFileW (for pipe) failed, lasterror={}. Retry times remaining: {}", err,
					retryRemining);
				if (retryRemining == 0) {
					break;
				}
				--retryRemining;
				continue;
			}
		}

		spdlog::info(L"Successfully connected to wizard.");
		// connected to wizard
		while (true) {
			DWORD bytesAvailable = 0;
			// see if stop signal is sent
			auto peekSuccess = PeekNamedPipe(hPipe, nullptr, 0, nullptr, &bytesAvailable, nullptr);
			if (!peekSuccess) {
				spdlog::warn(L"PeekNamedPipe failed. Assuming disconnection. Error: {}", GetLastError());
				break;
			}

			if (bytesAvailable) {
				BYTE buf[1]{};
				DWORD bytesRead = 0;
				if (ReadFile(hPipe, buf, 1, &bytesRead, nullptr)) {
					if (buf[0] == BKGND_PROCESS_STOP_SIGNAL) {
						spdlog::info(L"Wizard sent \'stop service\' signal. Exiting.");
						toStop = true; // exit external loop
						break;
					}
				}
				else {
					spdlog::warn(L"ReadFile failed. Assuming disconnection. Error: {}", GetLastError());
					break;
				}
			}

			// send alive signal
			DWORD _;
			auto writeSuccess = WriteFile(hPipe, aliveSignalBuf, 1, &_, nullptr);
			// According to Google Gemini 3 Pro,
			// there can be many possibilities of error code on disconnection.
			// So once this fails we assume disconnection without checking error code.
			if (!writeSuccess) {
				spdlog::warn(L"ReadFile failed. Assuming disconnection. Error: {}", GetLastError());
				break;
			}

			Sleep(1000);
		}

		// server(wizard) died (at least in assumption)
		spdlog::info(L"Wizard is closed. Waiting for next server connection.");

		CloseHandle(hPipe);
	}

	PostMessageW(WM_CLOSE);
}

afx_msg LRESULT CSmartPodVolumeDlg::OnNewDeviceNeedsRegistration(WPARAM wParam, LPARAM lParam) {
	RegisterVolumeNotification((IMMDevice*)wParam);
	return 0;
}

BOOL CSmartPodVolumeDlg::OnQueryEndSession() {
	if (!CDialog::OnQueryEndSession())
		return FALSE;

	SendMessageW(WM_CLOSE);
	
	return TRUE;
}

void CSmartPodVolumeDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos) {
	CDialog::OnWindowPosChanging(lpwndpos);

	if (!m_testMode) {
		// hide main window
		lpwndpos->flags &= ~SWP_SHOWWINDOW;
	}
}
