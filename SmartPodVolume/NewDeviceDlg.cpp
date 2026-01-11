// NewDeviceDlg.cpp: 实现文件
//

#include "pch.h"
#include "SmartPodVolume.h"
#include "NewDeviceDlg.h"
#include "utils.h"
#include "constants.h"


// CNewDeviceDlg 对话框

IMPLEMENT_DYNAMIC(CNewDeviceDlg, CDialog)

CNewDeviceDlg::CNewDeviceDlg(const utils::MmDeviceInfo& info, const CComPtr<IMMDevice>& device, CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_NEW_DEVICE, pParent), 
    m_mmDeviceInfo(info),
	m_device(device),
	m_dontNotifyMainWndOnDestroy(false)
{

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
	ON_BN_CLICKED(IDYES, &CNewDeviceDlg::OnBnClickedYes)
	ON_BN_CLICKED(IDNO, &CNewDeviceDlg::OnBnClickedNo)
	ON_WM_DESTROY()
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
	// intentionally kept empty
}

HBRUSH CNewDeviceDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_TIP_STATIC) {
		pDC->SetTextColor(RGB(100, 100, 100));
	}

	// 如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

void CNewDeviceDlg::OnClose() {
	// intentionally kept empty
}

BOOL CNewDeviceDlg::PreTranslateMessage(MSG* pMsg) {
	if (pMsg->message == WM_KEYDOWN) {
		if (pMsg->wParam == VK_ESCAPE) {
			return TRUE; // block this message
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CNewDeviceDlg::OnBnClickedYes() {
	if (!m_device) {
		// this should only occur in the test window (opened from main UI)
		DestroyWindow();
		return;
	}

	try {
		auto configJson = utils::GetConfigJson();
		if (!configJson.is_object()) {
			configJson = {};
		}
		auto& whiteListJson = configJson[conf_key::WHITELIST];
		if (!whiteListJson.is_object()) {
			whiteListJson = {};
		}
		auto& deviceJson = whiteListJson[utils::ConfKeyizeId(m_mmDeviceInfo.id)];
		if (!deviceJson.is_object()) {
			deviceJson = {};
		}
		deviceJson[conf_key::FRIENDLY_NAME] = utils::WcToU8(m_mmDeviceInfo.friendlyName);
		deviceJson[conf_key::DESCRIPTION] = utils::WcToU8(m_mmDeviceInfo.description);

		CComPtr<IAudioEndpointVolume> endpointVolume;
		HRESULT hr = utils::QueryVolumeController(m_device, &endpointVolume);
		if (SUCCEEDED(hr)) {
			float volume;
			hr = endpointVolume->GetMasterVolumeLevelScalar(&volume);
			if (SUCCEEDED(hr)) {
				volume *= 100.f;
				deviceJson[conf_key::EXPECTED_VOLUME] = volume;
			}
			else {
				spdlog::warn(L"Error getting initial EXPECTED VOLUME for device {} (hr={})", m_mmDeviceInfo.id, hr);
			}

			BOOL mute;
			hr = endpointVolume->GetMute(&mute);
			if (SUCCEEDED(hr)) {
				deviceJson[conf_key::MUTE] = (bool)mute;
			}
			else {
				spdlog::warn(L"Error getting initial EXPECTED MUTE for device {} (hr={})", m_mmDeviceInfo.id, hr);
			}
		}
		else {
			spdlog::warn(L"Error querying IAudioEndpointVolume for device {} to get initial volume and mute (hr={})",
				m_mmDeviceInfo.id, hr);
		}

		bool writeSuccess = utils::WriteConfigFile(configJson.dump());
		if (writeSuccess) {
			spdlog::info(L"Successfully wrote initial config for new mmDevice {}", m_mmDeviceInfo.id);
		}
		else {
			spdlog::warn(L"Failed to wrote initial config for new mmDevice {}", m_mmDeviceInfo.id);
		}

		// register volume change notify
		AfxGetApp()->m_pMainWnd->SendMessageW(WM_NEW_DEVICE_NEEDS_REGISTRATION, (WPARAM)m_device.p);
	}
	catch (std::exception& e) {
		spdlog::error(L"Error collecting and dumping initial config for new mmDevice {} (err: {})",
			m_mmDeviceInfo.id, utils::AcpToWc(e.what()));
	}
	catch (...) {
		spdlog::error(L"Error collecting and dumping initial config for new mmDevice {} (unknown error)",
			m_mmDeviceInfo.id);
	}

	DestroyWindow();
}

void CNewDeviceDlg::OnBnClickedNo() {
	if (!m_device) {
		DestroyWindow();
		return;
	}

	try {
		auto configJson = utils::GetConfigJson();
		if (!configJson.is_object()) {
			configJson = {};
		}
		auto& blackListJson = configJson[conf_key::BLACKLIST];
		if (!blackListJson.is_object()) {
			blackListJson = {};
		}
		auto& deviceJson = blackListJson[utils::ConfKeyizeId(m_mmDeviceInfo.id)];
		deviceJson = {
			{conf_key::DESCRIPTION, utils::WcToU8(m_mmDeviceInfo.description)},
			{conf_key::FRIENDLY_NAME, utils::WcToU8(m_mmDeviceInfo.friendlyName)}
		};

		bool writeSuccess = utils::WriteConfigFile(deviceJson.dump());
		if (writeSuccess) {
			spdlog::info(L"Successfully added device {} into blacklist.", m_mmDeviceInfo.id);
		}
		else {
			spdlog::error(L"Failed to write config file when trying to adding device {} to blacklist.", m_mmDeviceInfo.id);
		}
	}
	catch (std::exception& e) {
		spdlog::error(L"Error adding device {} into blacklist (err: {}).", m_mmDeviceInfo.id, utils::AcpToWc(e.what()));
	}
	catch (...) {
		spdlog::error(L"Error adding device {} into blacklist (unknown error).", m_mmDeviceInfo.id);
	}

	DestroyWindow();
}

void CNewDeviceDlg::OnCancel() {
	// intentionally kept empty
}

void CNewDeviceDlg::OnDestroy() {
	CDialog::OnDestroy();

	if (!m_dontNotifyMainWndOnDestroy) {
		auto lowerId = utils::WStringLower(m_mmDeviceInfo.id);
		AfxGetApp()->m_pMainWnd->SendMessageW(WM_NEWDEVICEDLG_CLOSED, (WPARAM)&lowerId);
	}
}
