// VolumeSetFailDlg.cpp: 实现文件
//

#include "pch.h"
#include "SmartPodVolume.h"
#include "VolumeSetFailDlg.h"
#include "constants.h"


// CVolumeSetFailDlg 对话框

IMPLEMENT_DYNAMIC(CVolumeSetFailDlg, CTopPopupDialog)

CVolumeSetFailDlg::CVolumeSetFailDlg(HRESULT hr, const utils::MmDeviceInfo& deviceInfo)
	: CTopPopupDialog(IDD_VOLUME_SET_FAIL),
	m_forTestPurpose(false),
    m_mmDeviceInfo(deviceInfo) {
	m_errCodeString.Format(L"0x%08x", hr);
}

CVolumeSetFailDlg::~CVolumeSetFailDlg()
{
}

void CVolumeSetFailDlg::DoDataExchange(CDataExchange* pDX)
{
	CTopPopupDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DEVICE_INFO_REPORT, m_deviceInfoReport);
	DDX_Text(pDX, IDC_ERR_CODE_DISPLAY, m_errCodeString);
}


BEGIN_MESSAGE_MAP(CVolumeSetFailDlg, CTopPopupDialog)
	ON_WM_CTLCOLOR()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDYES, &CVolumeSetFailDlg::OnBnClickedYes)
	ON_BN_CLICKED(IDNO, &CVolumeSetFailDlg::OnBnClickedNo)
END_MESSAGE_MAP()


// CVolumeSetFailDlg 消息处理程序

HBRUSH CVolumeSetFailDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	HBRUSH hbr = CTopPopupDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_TIP_STATIC) {
		pDC->SetTextColor(RGB(100, 100, 100));
	}

	// 如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

BOOL CVolumeSetFailDlg::OnInitDialog() {
	CTopPopupDialog::OnInitDialog();

	m_deviceInfoReport.SetDeviceInfo(m_mmDeviceInfo);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CVolumeSetFailDlg::OnBnClickedYes() {
	if (m_forTestPurpose) {
		DestroyWindow();
		return;
	}
	
	try {
		auto configJson = utils::GetConfigJson();
		if (!configJson.is_object()) {
			configJson = {};
		}
		auto id = utils::ConfKeyizeId(m_mmDeviceInfo.id);

		auto& whiteList = configJson[conf_key::WHITELIST];
		if (whiteList.contains(id)) {
			whiteList.erase(id);
		}

		auto& blacklist = configJson[conf_key::BLACKLIST];
		if (!blacklist.is_object()) {
			blacklist = {};
		}
		blacklist[id] = {
			{conf_key::FRIENDLY_NAME, utils::WcToU8(m_mmDeviceInfo.friendlyName)},
			{conf_key::DESCRIPTION, utils::WcToU8(m_mmDeviceInfo.description)}
		};

		bool writeSuccess = utils::WriteConfigJson(configJson);
		if (writeSuccess) {
			spdlog::info(L"Successfully moved device {} from whitelist to blacklist", m_mmDeviceInfo.id);
		}
		else {
			spdlog::error(L"Failed to write config file when trying to move device {} from whitelist to blacklist",
				m_mmDeviceInfo.id);
		}
	}
	catch (std::exception& e) {
		spdlog::error(L"Error moving device {} from white to blacklist ({})", m_mmDeviceInfo.id,
			utils::AcpToWc(e.what()));
	}
	catch (...) {
		spdlog::error(L"Error moving device {} from white to blacklist (unknown error)", m_mmDeviceInfo.id);
	}

	DestroyWindow();
}

void CVolumeSetFailDlg::OnBnClickedNo() {
	// do nothing because when this window pops up, the device is in whitelist
	DestroyWindow();
}
