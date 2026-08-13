#pragma once
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <QObject>

class VolumeMonitor : public QObject, public IAudioEndpointVolumeCallback {
    Q_OBJECT
public:
    explicit VolumeMonitor(EDataFlow dataFlow, QObject* parent = nullptr);
    ~VolumeMonitor() override;


    // IUnknown接口
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override;
    // 音量回调接口
    HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) override;

signals:
    void volumeChanged(float newVolume,bool Muted); // 0.0-1.0范围,是否静音

    void MicvolumeChanged(float newVolume,bool Muted); // 0.0-1.0范围,是否静音

private:
    LONG m_refCount;
    EDataFlow m_dataFlow;
    IAudioEndpointVolume* m_pEndpointVolume;
};
