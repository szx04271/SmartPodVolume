#pragma once


// TopPopupDialog 对话框

// base class for a topmost popup dialog (and with other features)
class CTopPopupDialog : public CDialog
{
	DECLARE_DYNAMIC(CTopPopupDialog)

public:
	CTopPopupDialog(UINT dlgResId);   // 标准构造函数
	virtual ~CTopPopupDialog();

private:
	UINT m_dlgResId;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	void DoNonModal();

	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	virtual void OnOK();
	afx_msg void OnClose();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnCancel();
};
