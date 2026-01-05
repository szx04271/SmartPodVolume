// NewDeviceDlg.cpp: 实现文件
//

#include "pch.h"
#include "SmartPodVolume.h"
#include "afxdialogex.h"
#include "NewDeviceDlg.h"


// CNewDeviceDlg 对话框

IMPLEMENT_DYNAMIC(CNewDeviceDlg, CDialog)

CNewDeviceDlg::CNewDeviceDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_NEW_DEVICE, pParent) {

}

CNewDeviceDlg::~CNewDeviceDlg() {
}

void CNewDeviceDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NEW_DEVICE_INFO, m_deviceInfoReport);
}


BEGIN_MESSAGE_MAP(CNewDeviceDlg, CDialog)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CNewDeviceDlg 消息处理程序

BOOL CNewDeviceDlg::OnInitDialog() {
	CDialog::OnInitDialog();

	//::SetWindowTheme(m_deviceInfoReport.GetSafeHwnd(), L"Explorer", nullptr);
	m_deviceInfoReport.SetExtendedStyle(m_deviceInfoReport.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	CRect rc; m_deviceInfoReport.GetClientRect(&rc);
	m_deviceInfoReport.InsertColumn(0, L"属性项", LVCFMT_LEFT, rc.Width() * 1 / 3.5);
	m_deviceInfoReport.InsertColumn(1, L"属性值", LVCFMT_LEFT, rc.Width() * 2.5 / 3.5);
	//LVCOLUMNW col{};
	//col.mask = LVCF_FMT;
	//col.fmt = LVCFMT_FIXED_WIDTH; // 这属性在插入列时设置不管用，只有插入之后再设管用，莫名其妙
	//m_deviceInfoReport.SetColumn(0, &col);
	//m_deviceInfoReport.SetColumn(1, &col);

	// 后来发现可以隐藏header，我操，前面白搞了

	m_deviceInfoReport.InsertItem(0, L"友好名称");
	m_deviceInfoReport.InsertItem(1, L"描述");
	m_deviceInfoReport.InsertItem(2, L"MM设备ID");

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
