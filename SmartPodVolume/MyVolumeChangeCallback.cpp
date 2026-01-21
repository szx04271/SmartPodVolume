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

HRESULT STDMETHODCALLTYPE MyVolumeChangeCallback::OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) noexcept {
	// in subthread

	spdlog::info(L"thrid={} [VOL or MUTE CHANGED] id={} vol={} mute={}", GetCurrentThreadId(), m_deviceId, pNotify->fMasterVolume * 100.0f,
		pNotify->bMuted ? true : false);

	// In view that the user is likely to adjust volume many times in a few seconds,
	// we don't directly save the new volume to disk here. 
	m_volumeInfo.volumePercent = pNotify->fMasterVolume * 100.f;
	m_volumeInfo.mute = pNotify->bMuted ? true : false;

	AfxGetApp()->m_pMainWnd->SendMessageW(WM_REGISTERED_DEVICE_VOLUME_CHANGED, (WPARAM)this, 0);

	return S_OK;
}
