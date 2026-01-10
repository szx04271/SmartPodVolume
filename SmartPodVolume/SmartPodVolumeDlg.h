
// SmartPodVolumeDlg.h: 头文�?
//

#pragma once
#include "utils.h"
#include "NewDeviceDlg.h"
#include "MyVolumeChangeCallback.h"

// CSmartPodVolumeDlg 对话�?
class CSmartPodVolumeDlg : public CDialog
{
// 构�?
public:
	CSmartPodVolumeDlg(CWnd* pParent = nullptr);	// 标准构造函�?

// 对话框数�?
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SMARTPODVOLUME_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	HDEVNOTIFY m_hDevNotify = nullptr;

	// 生成的消息映射函�?
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	
public:
	afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);

	void OnDeviceArrived(PDEV_BROADCAST_DEVICEINTERFACE_W devInf);
	void OnNewMmDevice(const utils::MmDeviceInfo& info) {
		spdlog::info(L"This is a new device. Asking user for choice.");
		auto newDeviceDlg = new CNewDeviceDlg(info);
		newDeviceDlg->Create(IDD_NEW_DEVICE);
		newDeviceDlg->ShowWindow(SW_SHOWNORMAL);
	}

	void OnDeviceRemoved(PDEV_BROADCAST_DEVICEINTERFACE_W devInf);

	afx_msg void OnDestroy();
	afx_msg void OnBnClickedDisplayNewDeviceDialog();
	afx_msg void OnBnClickedDisplayVolumeSetFailDialog();

	struct RegisteredDevice 
	{
		CComPtr<IAudioEndpointVolume> endpointVolume;
		MyVolumeChangeCallback* callback;
		float volumePercent;
		bool mute;
		std::wstring mmDeviceId;
		std::list<std::wstring> fromDevInfIds;
	};
	std::list<RegisteredDevice> m_registeredCallbacks;
	RegisteredDevice* RegisterVolumeNotification(IMMDevice* device, std::wstring_view fromDevInfId);
	void RegisterVolumeNotificationsForAllKnown();
	void UnregisterAllVolumeNotifications();
};
