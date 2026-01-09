
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

	RegisterVolumeNotification();

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
	spdlog::info("OnDevicechange eventType={} data={}", nEventType, dwData);

	if (nEventType == DBT_DEVICEARRIVAL) {
		PDEV_BROADCAST_HDR pHdr = reinterpret_cast<PDEV_BROADCAST_HDR>(dwData);
		PDEV_BROADCAST_DEVICEINTERFACE_W pDevInf = reinterpret_cast<PDEV_BROADCAST_DEVICEINTERFACE_W>(pHdr);
		DeviceArrived(pDevInf);
	}
	return TRUE;
}

void CSmartPodVolumeDlg::OnDestroy() {
	CDialog::OnDestroy();

	if (m_hDevNotify) {
		UnregisterDeviceNotification(m_hDevNotify);
		m_hDevNotify = nullptr;
		spdlog::info(L"Unregistered device notification.");
	}

	UnregisterVolumeNotification();
	spdlog::info(L"Unregistered volume notification.");
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

void CSmartPodVolumeDlg::RegisterVolumeNotification() {
	auto j = utils::GetConfigJson();
	if (j.is_null()) {
		return;
	}

	auto deviceCollection = utils::GetMmDeviceCollection();
	if (!deviceCollection) {
		return;
	}
	UINT deviceCount = 0;
	deviceCollection->GetCount(&deviceCount);
	if (deviceCount == 0) {
		return;
	}
	//                               interface             ID
	using DeviceAndId = std::pair<CComPtr<IMMDevice>, std::wstring>;
	std::list<DeviceAndId> deviceList;
	HRESULT hr = S_OK;
	for (UINT i = 0; i < deviceCount; ++i) {
		CComPtr<IMMDevice> device;
		hr = deviceCollection->Item(i, &device);
		if (FAILED(hr)) {
			spdlog::warn(L"Error getting interface of {}th(begin from 0) item of IMMDeviceCollection. Skip.", i);
			continue;
		}
		LPWSTR id;
		hr = device->GetId(&id);
		if (FAILED(hr)) {
			spdlog::warn(L"Error getting id of {}th(begin from 0) IMMDevice. Skip.", i);
			continue;
		}
		deviceList.emplace_back(std::move(device), id);
		CoTaskMemFree(id);
	}

	auto itList = j.find(conf_key::WHITELIST);
	if (itList == j.end() || !itList->is_array()) {
		return;
	}
	for (auto& deviceJson : *itList) {
		auto itId = deviceJson.find(conf_key::MMDEVICE_ID);
		if (itId == deviceJson.end() || !itId->is_string()) continue;
		for (auto& device : deviceList) {
			auto idInConfig = utils::U8ToWc(itId->get<std::string>());
			if (!_wcsicmp(device.second.c_str(), idInConfig.c_str())) {
				// found dest device
				CComPtr<IAudioEndpointVolume> endpointVolume;
				hr = utils::QueryVolumeController(device.first, &endpointVolume);
				if (FAILED(hr)) {
					spdlog::error(L"Error obtaining IAudioEndpointVolume interface pointer (hr={})", hr);
					continue;
				}

				MyVolumeChangeCallback* callback = new MyVolumeChangeCallback(device.second);
				hr = endpointVolume->RegisterControlChangeNotify(callback);
				if (FAILED(hr)) {
					spdlog::error(L"RegisterControlChangeNotify (for device with id={}) failed (hr={})",
						device.second.c_str(), hr);
					callback->Release();
					continue;
				}

				float volume = 0;
				BOOL mute = FALSE;
				hr = endpointVolume->GetMasterVolumeLevelScalar(&volume);
				if (FAILED(hr)) {
					spdlog::error(L"Error getting initial [volume] for device with id={}, hr={}. Defaulted to 0.",
						device.second.c_str(), hr);
				}
				hr = endpointVolume->GetMute(&mute);
				if (FAILED(hr)) {
					spdlog::error(L"Error getting initial [mute] for device with id={}, hr={}. Defaulted to false.",
						device.second.c_str(), hr);
				}

				// registration success
				RegisteredDevice registeredDevice = {
					endpointVolume,callback,volume / 100.f,mute
				};
				m_registeredCallbacks.emplace_back(std::move(registeredDevice));
				spdlog::info(L"Successfully registered volume change notify for id={}", device.second);
			}
		}
	}
}

void CSmartPodVolumeDlg::UnregisterVolumeNotification() {
	for (auto& device : m_registeredCallbacks) {
		device.endpointVolume->UnregisterControlChangeNotify(device.callback);
		device.callback->Release();
	}
}

void CSmartPodVolumeDlg::DeviceArrived(PDEV_BROADCAST_DEVICEINTERFACE_W devInf) {
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
				if (j.is_null()) {
					NewMmDevice(*mmDeviceInfo);
					return;
				}

				auto FindDeviceInList = [&j, &mmDeviceInfo](const char* listKey) -> json {
					auto itList = j.find(listKey);
					if (itList != j.end() && itList->is_array()) {
						for (auto& deviceInList : *itList) {
							auto itId = deviceInList.find(conf_key::MMDEVICE_ID);
							if (itId != deviceInList.end() && itId->is_string()) {
								auto id = utils::U8ToWc(itId->get<std::string>());
								if (!_wcsicmp(id.c_str(), mmDeviceInfo->id.c_str())) {
									return deviceInList;
								}
							}
						}
					}

					return json();
				};

				if (!FindDeviceInList(conf_key::BLACKLIST).is_null()) {
					spdlog::info("This device is in blacklist. Ignore.");
					return;
				}
				else {
					json deviceJson = FindDeviceInList(conf_key::WHITELIST);
					bool white = !deviceJson.is_null();

					if (white) {
						HRESULT hr = S_OK;
						CComPtr<IAudioEndpointVolume> endpointVol;
						hr = utils::QueryVolumeController(mmDevice, &endpointVol);
						if (FAILED(hr)) {
							spdlog::error(L"QueryVolumeController failed with hr={}", hr);
							return;
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
							spdlog::info(L"FAILED to set volume (invalid config)");
						}
						else if (FAILED(hr)) {
							spdlog::info(L"FAILED to set volume (hr={})", hr);
						}
						else {
							spdlog::info(L"SUCCESSFULLY set volume");
							volumeSuccess = true;
						}

						auto itMute = deviceJson.find(conf_key::MUTE);
						if (itMute != deviceJson.end() && itMute->is_boolean()) {
							muteConfigValid = true;
							auto expectedMute = itMute->get<bool>();
							hr = endpointVol->SetMute(expectedMute, nullptr);
						}
						if (!muteConfigValid) {
							spdlog::info(L"FAILED to set mute (invalid config)");
						}
						else if (FAILED(hr)) {
							spdlog::info(L"FAILED to set mute (hr={})", hr);
						}
						else {
							spdlog::info(L"SUCCESSFULLY set mute");
							muteSuccess = true;
						}

						if (!volumeSuccess || !muteSuccess) {
							CVolumeSetFailDlg* setFailDlg = new CVolumeSetFailDlg(hr, *mmDeviceInfo);
							setFailDlg->Create(IDD_VOLUME_SET_FAIL);
							setFailDlg->ShowWindow(SW_SHOWNORMAL);
						}
					}
					else {
						// this is a new device
						NewMmDevice(*mmDeviceInfo);
					}
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
