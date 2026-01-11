
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
	afx_msg LRESULT OnRegisteredDeviceVolumeChanged(WPARAM wParam, LPARAM lParam);

	void OnDeviceArrived(PDEV_BROADCAST_DEVICEINTERFACE_W devInf);

	// store this because
	// (1) we want a NewDeviceDlg to close when its corresponding MmDevice is removed from computer
	// (2) we don't want two NewDeviceDlg referencing to one MmDevice to appear simultaneously
	std::map<utils::LowercaseIdType, CNewDeviceDlg*> m_newMmDeviceWindows;

	void OnNewMmDevice(const utils::MmDeviceInfo& info, const CComPtr<IMMDevice>& device);

	afx_msg LRESULT OnNewdevicedlgClosed(WPARAM wParam, LPARAM lParam) {
		m_newMmDeviceWindows.erase(*(std::wstring*)wParam);
		return 0;
	}

	void OnDeviceRemoved(PDEV_BROADCAST_DEVICEINTERFACE_W devInf);

	afx_msg void OnDestroy();
	afx_msg void OnBnClickedDisplayNewDeviceDialog();
	afx_msg void OnBnClickedDisplayVolumeSetFailDialog();

	struct RegisteredDevice 
	{
		CComPtr<IAudioEndpointVolume> endpointVolume;
		MyVolumeChangeCallback* callback;
	};
	// key is mmDevice id in LOWERCASE
	std::map<utils::LowercaseIdType, RegisteredDevice> m_registeredCallbacks;

	RegisteredDevice* RegisterVolumeNotification(IMMDevice* device);
	void RegisterVolumeNotificationsForAllKnown();
	void UnregisterAllVolumeNotifications();

	afx_msg LRESULT OnNewDeviceNeedsRegistration(WPARAM wParam, LPARAM lParam);

	std::map<utils::LowercaseIdType, MyVolumeChangeCallback::DeviceVolumeInfo> m_volumesToBeSaved;

	static constexpr inline UINT_PTR AUTO_SAVE_CONFIG_TIMER_ID = 1;
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	bool SaveAllVolumes() noexcept;
	
};
