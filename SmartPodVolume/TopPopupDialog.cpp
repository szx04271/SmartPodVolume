// TopPopupDialog.cpp: 实现文件
//

#include "pch.h"
#include "SmartPodVolume.h"
#include "TopPopupDialog.h"


// TopPopupDialog 对话框

IMPLEMENT_DYNAMIC(CTopPopupDialog, CDialog)

CTopPopupDialog::CTopPopupDialog(UINT dlgResId) : 
	CDialog(dlgResId, nullptr),
	m_dlgResId(dlgResId)
{

}

CTopPopupDialog::~CTopPopupDialog()
{
}

void CTopPopupDialog::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CTopPopupDialog, CDialog)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// TopPopupDialog 消息处理程序

void CTopPopupDialog::DoNonModal() {
	Create(m_dlgResId, GetDesktopWindow());
	ShowWindow(SW_SHOWNORMAL);

	// 搜狗输入法的dll貌似会在这个SetForegroundWindow发送的某个消息的处理程序里抛std::runtime_error，
	// 但似乎它自己给catch了。所以会在调试时看到一行输出，但并不会崩溃。
	SetForegroundWindow();
}

BOOL CTopPopupDialog::OnInitDialog() {
	CDialog::OnInitDialog();

	GetSystemMenu(FALSE)->EnableMenuItem(SC_CLOSE, MF_GRAYED | MF_BYCOMMAND);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CTopPopupDialog::PostNcDestroy() {
	CDialog::PostNcDestroy();

	delete this; // because dialogs using CTopPopupDialog are all assumed to be non-modal
}

void CTopPopupDialog::OnOK() {
	// intentionally kept empty
}

void CTopPopupDialog::OnClose() {
	// intentionally kept empty
}

BOOL CTopPopupDialog::PreTranslateMessage(MSG* pMsg) {
	if (pMsg->message == WM_KEYDOWN) {
		if (pMsg->wParam == VK_ESCAPE) {
			return TRUE; // block this message
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CTopPopupDialog::OnCancel() {
	// intentionally kept empty
}
