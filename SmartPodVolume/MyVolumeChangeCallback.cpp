#include "pch.h"
#include "MyVolumeChangeCallback.h"
#include "constants.h"

STDMETHODIMP MyVolumeChangeCallback::QueryInterface(REFIID riid, void** ppv) {
	if (riid == IID_IUnknown || riid == __uuidof(IAudioEndpointVolumeCallback)) {
		*ppv = this;
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) MyVolumeChangeCallback::AddRef() {
	return (ULONG)InterlockedIncrement(&m_refCount);
}

STDMETHODIMP_(ULONG) MyVolumeChangeCallback::Release() {
	auto ref_count = InterlockedDecrement(&m_refCount);
	if (ref_count == 0) {
		spdlog::info(L"MyVolumeChangeCallback of id={} is being destructed.", m_deviceId);
		delete this;
	}
	return (ULONG)ref_count;
}

/*
* 2026/01/21 (after commit 2e774913) 注
* 把耳机和电脑断开连接时，出于未知原因，系统在发送DeviceRemoved前，会先发送一些DeviceArrived，
* 这些arrived的devices里就可能包含我们已注册的MM设备自身或祖宗。由于这些设备在白名单，主线程
* 收到DeviceArrived后会视为白名单设备新接入，于是将其音量设为配置文件里的预期音量。注意，此时
* 未收到DeviceRemoved，因此还没反注册下面这个音量改变通知。但实测下来，这时收到的三个左右的
* DeviceArrived中虽然皆设置了音量，但并非每次设置都触发了这个音量改变通知。原因不明。猜测是因为
* 此时系统正准备移除这些设备，因此对这些通知的支持不完整。
* ===========================
* 注后打算修改：加uuid判断，排除一切自身修改音量引起的回调通知。
* ===========================
* 2026/1/21 17:08 已加
*/
HRESULT STDMETHODCALLTYPE MyVolumeChangeCallback::OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) noexcept {
	// in subthread
	
	// `==` between GUIDs is overloaded by Microsoft
	if (pNotify->guidEventContext == VOLUME_SETTER_GUID) {
		return S_OK;
	}

	spdlog::info(L"[VOL or MUTE CHANGED] id={} vol={} mute={}", m_deviceId, pNotify->fMasterVolume * 100.0f,
		pNotify->bMuted ? true : false);

	// In view that the user is likely to adjust volume many times in a few seconds,
	// we don't directly save the new volume to disk here. 
	m_volumeInfo.volumePercent = pNotify->fMasterVolume * 100.f;
	m_volumeInfo.mute = pNotify->bMuted ? true : false;

	AfxGetApp()->m_pMainWnd->SendMessageW(WM_REGISTERED_DEVICE_VOLUME_CHANGED, (WPARAM)this, 0);

	return S_OK;
}
