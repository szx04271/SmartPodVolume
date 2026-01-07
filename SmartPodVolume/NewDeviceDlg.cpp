// NewDeviceDlg.cpp: 实现文件
//

#include "pch.h"
#include "SmartPodVolume.h"
#include "NewDeviceDlg.h"


// CNewDeviceDlg 对话框

IMPLEMENT_DYNAMIC(CNewDeviceDlg, CDialog)

CNewDeviceDlg::CNewDeviceDlg(const utils::MmDeviceInfo& info, CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_NEW_DEVICE, pParent), 
      m_mmDeviceInfo(info) {

}

CNewDeviceDlg::~CNewDeviceDlg() {
}

void CNewDeviceDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NEW_DEVICE_INFO, m_deviceInfoReport);
}


BEGIN_MESSAGE_MAP(CNewDeviceDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CNewDeviceDlg 消息处理程序

BOOL CNewDeviceDlg::OnInitDialog() {
	CDialog::OnInitDialog();

	GetSystemMenu(FALSE)->EnableMenuItem(SC_CLOSE, MF_GRAYED | MF_BYCOMMAND);

	m_deviceInfoReport.SetDeviceInfo(m_mmDeviceInfo);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CNewDeviceDlg::PostNcDestroy() {
	CDialog::PostNcDestroy();

	delete this;
}

void CNewDeviceDlg::OnOK() {
}

HBRUSH CNewDeviceDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_TIP_STATIC) {
		pDC->SetTextColor(RGB(100, 100, 100));
	}

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

void CNewDeviceDlg::OnClose() {
}

BOOL CNewDeviceDlg::PreTranslateMessage(MSG* pMsg) {
	if (pMsg->message == WM_KEYDOWN) {
		if (pMsg->wParam == VK_ESCAPE) {
			return TRUE; // block this message
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}
