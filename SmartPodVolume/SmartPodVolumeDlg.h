
// SmartPodVolumeDlg.h: 头文�?
//

#pragma once


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
	HDEVNOTIFY m_hDevNotify;

	// 生成的消息映射函�?
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnDevicechange(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnDestroy();
};
