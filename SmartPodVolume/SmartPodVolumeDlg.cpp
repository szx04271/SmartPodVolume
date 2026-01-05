
// SmartPodVolumeDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "SmartPodVolume.h"
#include "SmartPodVolumeDlg.h"
#include "utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const GUID BLUETOOTH_CLASS_GUID = { 0xe0cbf06c,0xcd8b,0x4647,0xbb,0x8a,0x26,0x3b,0x43,0xf0,0xf9,0x74 };

// CSmartPodVolumeDlg 对话��?



CSmartPodVolumeDlg::CSmartPodVolumeDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_SMARTPODVOLUME_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSmartPodVolumeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSmartPodVolumeDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_DEVICECHANGE, &CSmartPodVolumeDlg::OnDevicechange)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CSmartPodVolumeDlg 消息处理程序

BOOL CSmartPodVolumeDlg::OnInitDialog()
{
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
		spdlog::info(L"RegisterDeviceNotificationW succeeded. Listening for Bluetooth device changes.");
	}

	return TRUE;  // 除非将焦点设置到控件，否则返��?TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标��? 对于使用文档/视图模型��?MFC 应用程序��?
//  这将由框架自动完成��?

void CSmartPodVolumeDlg::OnPaint()
{
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
HCURSOR CSmartPodVolumeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// 1.“耳机已开机，但电脑蓝牙没开，打开蓝牙，电脑连上耳机”
// 2.“电脑蓝牙已打开，但耳机没开机，打开耳机，电脑连上耳机”
// 以上两种方式会使OnDevicechange收到不同的信息。第一种会同时有“蓝牙”、
// “网络适配器”、“系统设备”、“声音、视频和游戏控制器”、“音频输入和输出”设备的DBT_DEVICEARRIVAL消息，
// 但第二种就只收得到后两种设备的该消息。
// 这两种方式似乎都有概率触发“默认音量90%多甚至100%”这一bug。暂未发现
// 这两种方式在此方面的差异。
afx_msg LRESULT CSmartPodVolumeDlg::OnDevicechange(WPARAM wParam, LPARAM lParam) {
	spdlog::info("OnDevicechange wp={} lp={}", wParam, lParam);

	if (wParam == DBT_DEVICEARRIVAL) {
		PDEV_BROADCAST_HDR pHdr = reinterpret_cast<PDEV_BROADCAST_HDR>(lParam);
		PDEV_BROADCAST_DEVICEINTERFACE_W pDevInf = reinterpret_cast<PDEV_BROADCAST_DEVICEINTERFACE_W>(pHdr);
		// pDevInf->dbcc_classguid是设备接口类GUID，不是设备安装类GUID，不能直接传给SetupDiGetClassDescriptionW

		auto diInfo = utils::GetDeviceInterfaceInfoFromPath(pDevInf->dbcc_name);
		spdlog::info(L"Device arrived, id={}, name= {}, setup class guid = {}, setup class name = {}, device name = {}", diInfo.deviceInstanceId, pDevInf->dbcc_name, diInfo.classGuid,
			diInfo.classDescription, diInfo.deviceFriendlyName);

		auto mmDevices = utils::FindAssociatedMmDevices(diInfo.deviceInstanceId.c_str());
		if (mmDevices.empty()) {
			return TRUE;
		}

		for (auto& mmDevice : mmDevices) {
			auto mmDeviceInfo = utils::GetMmDeviceInfo(mmDevice);
			if (mmDeviceInfo.has_value()) {
				spdlog::info(L"Discovered MMDevice (friendlyName={}, id={}, desc={})", mmDeviceInfo->friendlyName,
					mmDeviceInfo->id, mmDeviceInfo->description);
			}
			else {
				spdlog::warn(L"Discovered MMDevice but no info. WTF?");
			}
			HRESULT hr = utils::SetDeviceVolume(mmDevice, 14);
			if (SUCCEEDED(hr)) {
				spdlog::info(L"SUCCESSFULLY set volume");
			}
			else {
				spdlog::warn(L"FAILED to set volume");
			}
		}
		//if (true) { // TODO: 此处从配置读取判断是否为目标设备
		//	const float targetVolumePercent = 30.0f; // TODO: 从配置读取
		//	if (utils::SetDeviceVolume(info.deviceInstanceId.c_str(), targetVolumePercent)) {
		//		spdlog::info(L"Successfully set volume of device {} to {}%", info.deviceInstanceId, targetVolumePercent);
		//	}
		//	else {
		//		spdlog::warn(L"Failed to set volume of device {}", info.deviceInstanceId);
		//	}
		//}
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
}
