#pragma once
#include "utils.h"

class MyVolumeChangeCallback :
    public IAudioEndpointVolumeCallback
{
public:
	struct DeviceVolumeInfo {
		float volumePercent;
		bool mute;
	} m_volumeInfo;

private:
    long volatile m_refCount;

    // always lowercase
    utils::LowercaseIdType m_deviceId;

public:
    MyVolumeChangeCallback(std::wstring_view deviceId)
        : m_deviceId(utils::WStringLower(deviceId)), m_refCount(1) { }

    // IUnknown methods
	STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override;
	STDMETHOD_(ULONG, AddRef)() override;
	STDMETHOD_(ULONG, Release)() override;

    virtual HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) noexcept override;

    utils::LowercaseIdType GetLowercaseDeviceId() const {
        // if new is defined as DEBUG_NEW,
        // app crashes here when RegisterVolumeNotification calls SendMessageW(WM_REGISTERED_DEVICE_VOLUME_CHANGED)
        return m_deviceId;
    }
};

