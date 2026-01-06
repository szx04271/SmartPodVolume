// VolumeSetFailDlg.cpp: 实现文件
//

#include "pch.h"
#include "SmartPodVolume.h"
#include "VolumeSetFailDlg.h"


// CVolumeSetFailDlg 对话框

IMPLEMENT_DYNAMIC(CVolumeSetFailDlg, CDialog)

CVolumeSetFailDlg::CVolumeSetFailDlg(HRESULT hr, const utils::MmDeviceInfo& deviceInfo, CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_VOLUME_SET_FAIL, pParent),
      m_mmDeviceInfo(deviceInfo) {
	m_errCodeString.Format(L"0x%08x", hr);
}

CVolumeSetFailDlg::~CVolumeSetFailDlg()
{
}

void CVolumeSetFailDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DEVICE_INFO_REPORT, m_deviceInfoReport);
	DDX_Text(pDX, IDC_ERR_CODE_DISPLAY, m_errCodeString);
}


BEGIN_MESSAGE_MAP(CVolumeSetFailDlg, CDialog)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CVolumeSetFailDlg 消息处理程序

void CVolumeSetFailDlg::PostNcDestroy() {
	CDialog::PostNcDestroy();

	delete this;
}

void CVolumeSetFailDlg::OnOK() {
}

HBRUSH CVolumeSetFailDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_TIP_STATIC) {
		pDC->SetTextColor(RGB(100, 100, 100));
	}

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

BOOL CVolumeSetFailDlg::OnInitDialog() {
	CDialog::OnInitDialog();

	m_deviceInfoReport.SetDeviceInfo(m_mmDeviceInfo);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}
