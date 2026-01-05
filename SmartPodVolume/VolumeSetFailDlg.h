#pragma once
#include "DeviceInfoListCtrl.h"


// CVolumeSetFailDlg 对话框

class CVolumeSetFailDlg : public CDialog
{
	DECLARE_DYNAMIC(CVolumeSetFailDlg)

public:
	CVolumeSetFailDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CVolumeSetFailDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VOLUME_SET_FAIL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CDeviceInfoListCtrl m_deviceInfoReport;
	virtual void PostNcDestroy();
	virtual void OnOK();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
