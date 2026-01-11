#pragma once
#include "DeviceInfoListCtrl.h"


// CNewDeviceDlg 对话框

class CNewDeviceDlg : public CDialog
{
	DECLARE_DYNAMIC(CNewDeviceDlg)

public:
	CNewDeviceDlg(const utils::MmDeviceInfo& info, const CComPtr<IMMDevice>& device, CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CNewDeviceDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_NEW_DEVICE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	bool m_dontNotifyMainWndOnDestroy;

	CComPtr<IMMDevice> m_device;
	utils::MmDeviceInfo m_mmDeviceInfo;
	virtual BOOL OnInitDialog();
	CDeviceInfoListCtrl m_deviceInfoReport;
	virtual void PostNcDestroy();
	virtual void OnOK();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnClose();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedYes();
	afx_msg void OnBnClickedNo();
	virtual void OnCancel();
	afx_msg void OnDestroy();
};
