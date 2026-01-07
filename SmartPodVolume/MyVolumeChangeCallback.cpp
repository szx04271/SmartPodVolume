#include "pch.h"
#include "MyVolumeChangeCallback.h"

STDMETHODIMP MyVolumeChangeCallback::QueryInterface(REFIID riid, void** ppv) {
	if (riid == IID_IUnknown || riid == __uuidof(IAudioEndpointVolumeCallback)) {
		*ppv = this;
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) MyVolumeChangeCallback::AddRef() {
	return InterlockedIncrement(&m_refCount);
}

STDMETHODIMP_(ULONG) MyVolumeChangeCallback::Release() {
	auto ref_count = InterlockedDecrement(&m_refCount);
	if (ref_count == 0) {
		delete this;
	}
	return ref_count;
}

HRESULT STDMETHODCALLTYPE MyVolumeChangeCallback::OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) {
	spdlog::info(L"[VOLCHANGED] id={} vol={}", m_deviceId, pNotify->fMasterVolume * 100.0f);

	return S_OK;
}
