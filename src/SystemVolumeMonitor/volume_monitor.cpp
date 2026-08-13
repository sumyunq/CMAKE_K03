#include "SystemVolumeMonitor/volume_monitor.h"

#include <QDebug>
#include <comdef.h>

//扬声器音量条定义一次，麦克风音量条定义一次，分别注册了回调函数（变化时只会进入到对应的回调函数中），而对应的回调函数所在的定义中m_dataFlow不同
VolumeMonitor::VolumeMonitor(EDataFlow dataFlow, QObject* parent): QObject(parent), m_refCount(1), m_dataFlow(dataFlow)
{

    IMMDeviceEnumerator* pEnumerator = nullptr;
    IMMDevice* pDevice = nullptr;
    HRESULT hr = NULL;


    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),nullptr,CLSCTX_INPROC_SERVER,__uuidof(IMMDeviceEnumerator),(void**)&pEnumerator);

    if (SUCCEEDED(hr)) {
        hr = pEnumerator->GetDefaultAudioEndpoint(dataFlow, eConsole, &pDevice);
        pEnumerator->Release();
    }

    if (SUCCEEDED(hr)) {
        hr = pDevice->Activate(__uuidof(IAudioEndpointVolume),CLSCTX_INPROC_SERVER,nullptr,(void**)&m_pEndpointVolume);
        pDevice->Release();
    }


    if (SUCCEEDED(hr)) {
        hr = m_pEndpointVolume->RegisterControlChangeNotify(this);//注册音量变化通知回调
    }


    if (FAILED(hr)) {
        qWarning() << "Volume monitor init failed:" << _com_error(hr).ErrorMessage();
    }

}

VolumeMonitor::~VolumeMonitor()
{
    if (m_pEndpointVolume) {
        m_pEndpointVolume->UnregisterControlChangeNotify(this);
        m_pEndpointVolume->Release();
    }
}
//增加引用计数
ULONG VolumeMonitor::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}
//减少引用计数
ULONG VolumeMonitor::Release()
{
    LONG ref = InterlockedDecrement(&m_refCount);
    if (ref == 0)
        delete this;
    return ref;
}
//查询支持的接口
HRESULT VolumeMonitor::QueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IUnknown || riid == __uuidof(IAudioEndpointVolumeCallback))
    {
        *ppv = static_cast<IAudioEndpointVolumeCallback*>(this);
        AddRef();
        return S_OK;
    }
    *ppv = nullptr;
    return E_NOINTERFACE;
}
//当音频音量变化时接收通知
HRESULT VolumeMonitor::OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify)
{
    bool MuteEn = false;
    if(pNotify->bMuted == 1)
    {
        MuteEn = true;
    }
    if(m_dataFlow == eRender)
    {
        //qDebug("OnNotify eRender,MuteEn:%d\n",MuteEn);
        emit volumeChanged(pNotify->fMasterVolume,MuteEn);
    }else if(m_dataFlow == eCapture)
    {
        //qDebug("OnNotify eCapture,MuteEn:%d\n",MuteEn);
        emit MicvolumeChanged(pNotify->fMasterVolume,MuteEn);
    }
    return S_OK;
}
