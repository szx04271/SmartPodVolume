#pragma once

class MyVolumeChangeCallback :
    public IAudioEndpointVolumeCallback
{
private:
    ULONG m_refCount;
    std::wstring m_deviceId;

public:
    MyVolumeChangeCallback(std::wstring_view deviceId)
        : m_deviceId(deviceId), m_refCount(1) { }

    // IUnknown methods
	STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override;
	STDMETHOD_(ULONG, AddRef)() override;
	STDMETHOD_(ULONG, Release)() override;

    virtual HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) override;

    std::wstring GetDeviceId() const {
        return m_deviceId;
    }
};

