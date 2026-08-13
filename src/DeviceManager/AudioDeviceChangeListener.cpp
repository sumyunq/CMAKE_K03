/*
//法1
#include "AudioDeviceChangeListener.h"

#include <mmdeviceapi.h>

// 如果连接器找不到 PKEY_AudioEndpoint_FormFactor，请添加以下手动定义
#ifndef PKEY_AudioEndpoint_FormFactor
EXTERN_C const PROPERTYKEY DECLSPEC_SELECTANY PKEY_AudioEndpoint_FormFactor =
    { {0x1da5d803, 0xd492, 0x4edd, {0x8c, 0x23, 0xe0, 0xc0, 0xff, 0xee, 0x7f, 0x0e}}, 0 };
#endif


AudioDeviceChangeListener::AudioDeviceChangeListener(QObject* parent)
    : QObject(parent), m_refCount(1), m_pEnumerator(nullptr)
{
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&m_pEnumerator);
    if (SUCCEEDED(hr)) {
        hr = m_pEnumerator->RegisterEndpointNotificationCallback(this);
    }
}

AudioDeviceChangeListener::~AudioDeviceChangeListener()
{
    if (m_pEnumerator) {
        m_pEnumerator->UnregisterEndpointNotificationCallback(this);
        m_pEnumerator->Release();
    }
}

// IUnknown methods
ULONG AudioDeviceChangeListener::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}

ULONG AudioDeviceChangeListener::Release()
{
    ULONG ulRef = InterlockedDecrement(&m_refCount);
    if (ulRef == 0) {
        delete this;
    }
    return ulRef;
}

HRESULT AudioDeviceChangeListener::QueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IUnknown || riid == __uuidof(IMMNotificationClient)) {
        *ppv = static_cast<IMMNotificationClient*>(this);
        AddRef();
        return S_OK;
    }
    *ppv = NULL;
    return E_NOINTERFACE;
}
bool AudioDeviceChangeListener::IsSpeakerDevice(LPCWSTR pwstrDeviceId)
{
    IMMDeviceEnumerator *pEnumerator = nullptr;
    IMMDevice *pDevice = nullptr;
    bool isSpeaker = false;

    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        reinterpret_cast<void**>(&pEnumerator));

    if (FAILED(hr))
        return false;

    hr = pEnumerator->GetDevice(pwstrDeviceId, &pDevice);
    if (FAILED(hr))
    {
        pEnumerator->Release();
        return false;
    }

    IPropertyStore *pProps = nullptr;
    hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
    if (SUCCEEDED(hr))
    {
        PROPVARIANT varForm;
        PropVariantInit(&varForm);
        hr = pProps->GetValue(PKEY_AudioEndpoint_FormFactor, &varForm);
        if (SUCCEEDED(hr) && varForm.vt == VT_UI4)
        {
            // 扬声器类型：Speakers、Headphones 等，根据需求可进一步限定
            EndpointFormFactor formFactor = static_cast<EndpointFormFactor>(varForm.ulVal);
            isSpeaker = (formFactor == Speakers ||
                         formFactor == Headphones ||
                         formFactor == Headset);
        }
        PropVariantClear(&varForm);
        pProps->Release();
    }

    pDevice->Release();
    pEnumerator->Release();
    return isSpeaker;
}
//当默认设备被拔出时，回调触发顺序：OnDeviceStateChanged、OnDefaultDeviceChanged、OnDeviceRemoved
//当插入新的设备时，回调触发顺序：OnDeviceAdded、OnDeviceStateChanged、OnDefaultDeviceChanged
//当设备状态发生变化时触发。例如，设备被禁用、启用、拔出等。
HRESULT AudioDeviceChangeListener::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
{
    // 仅处理扬声器设备
    if (!IsSpeakerDevice(pwstrDeviceId))
        return S_OK;
    //插入设备：0x00000001: DEVICE_STATE_ACTIVE
    //设备被禁用：0x00000002: DEVICE_STATE_DISABLED
    //设备不存在：0x00000004: DEVICE_STATE_NOTPRESENT
    //拔出设备：0x00000008: DEVICE_STATE_UNPLUGGED
    if(dwNewState == DEVICE_STATE_NOTPRESENT)
    {
        //设备拔出
        QString deviceName = getDeviceNameFromId(pwstrDeviceId);
        emit defaultDeviceDel(deviceName);
    }else if(dwNewState == DEVICE_STATE_ACTIVE)
    {
        //插入设备
        QString deviceName = getDeviceNameFromId(pwstrDeviceId);
        emit defaultDeviceAdd(deviceName);
    }
    return S_OK;
}
//当新设备被添加到系统时触发（例如插入USB耳机）。并没有进入回调，故使用OnDeviceStateChanged替代
HRESULT AudioDeviceChangeListener::OnDeviceAdded(LPCWSTR pwstrDeviceId)
{
    return S_OK;
}
//当设备被移除时触发（例如拔出USB耳机）。并没有进入回调，故使用OnDeviceStateChanged替代
HRESULT AudioDeviceChangeListener::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
{
    QString deviceName = getDeviceNameFromId(pwstrDeviceId);
    emit defaultDeviceDel(deviceName);
    return S_OK;
}
//当默认设备改变时触发（例如用户从扬声器切换到耳机）。
HRESULT AudioDeviceChangeListener::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId)
{
    // 我们只关心输出设备（eRender）并且是默认设备（eConsole）的变化
    if (flow == eRender && role == eConsole) {
        // 注意：pwstrDefaultDeviceId可能是NULL，表示没有默认设备
        //QString deviceId = QString::fromWCharArray(pwstrDefaultDeviceId);
        QString deviceName = getDeviceNameFromId(pwstrDefaultDeviceId);
        emit defaultOutPutDeviceChanged(deviceName);
    }else if(flow == eCapture && role == eConsole)
    {
        QString deviceName = getDeviceNameFromId(pwstrDefaultDeviceId);
        emit defaultInPutDeviceChanged(deviceName);
    }

    return S_OK;
}
//当设备的某个属性值发生变化时触发（例如设备名称、属性键改变）。
HRESULT AudioDeviceChangeListener::OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
{
    return S_OK;
}

QString AudioDeviceChangeListener::getDeviceNameFromId(LPCWSTR deviceId) {
    HRESULT hr;
    QString deviceName;

    // 初始化 COM 库
    CoInitialize(nullptr);

    // 创建设备枚举器
    IMMDeviceEnumerator* pEnumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
                          CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                          (void**)&pEnumerator);

    if (SUCCEEDED(hr)) {
        // 通过设备 ID 获取设备对象
        IMMDevice* pDevice = nullptr;
        hr = pEnumerator->GetDevice(deviceId, &pDevice);

        if (SUCCEEDED(hr)) {
            // 打开设备属性存储
            IPropertyStore* pProps = nullptr;
            hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);

            if (SUCCEEDED(hr)) {
                // 获取设备友好名称
                PROPVARIANT varName;
                PropVariantInit(&varName);
                hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);

                if (SUCCEEDED(hr)) {
                    deviceName = QString::fromWCharArray(varName.pwszVal);
                }
                PropVariantClear(&varName);
                pProps->Release();
            }
            pDevice->Release();
        }
        pEnumerator->Release();
    }

    CoUninitialize();
    return deviceName;
}
*/




#include "DeviceManager/AudioDeviceChangeListener.h"
#include "qdebug.h"
#include <QCoreApplication>
#include <QTimer>
#include <dbt.h>
#include <initguid.h>   // 可选，确保 GUID 定义

#ifndef PKEY_AudioEndpoint_FormFactor
EXTERN_C const PROPERTYKEY DECLSPEC_SELECTANY PKEY_AudioEndpoint_FormFactor =
    { {0x1da5d803, 0xd492, 0x4edd, {0x8c, 0x23, 0xe0, 0xc0, 0xff, 0xee, 0x7f, 0x0e}}, 0 };
#endif

// 音频渲染设备接口 GUID {e6327cad-dcec-4949-ae8a-991e976a79d2}
static const GUID GUID_DEVINTERFACE_AUDIO_RENDER =
    { 0xe6327cad, 0xdcec, 0x4949, { 0xae, 0x8a, 0x99, 0x1e, 0x97, 0x6a, 0x79, 0xd2 } };

// 新增捕获 GUID
static const GUID GUID_AUDIO_CAPTURE =
    { 0xe6327cad, 0xdcec, 0x4949, { 0xae, 0x8a, 0x99, 0x1e, 0x97, 0x6a, 0x79, 0xd3 } };

// 构造函数
AudioDeviceChangeListener::AudioDeviceChangeListener(QObject* parent)
    : QObject(parent), m_refCount(1), m_pEnumerator(nullptr)
{
    CoInitialize(nullptr);  // 确保 COM 已初始化
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                  __uuidof(IMMDeviceEnumerator), (void**)&m_pEnumerator);
    if (SUCCEEDED(hr)) {
        m_pEnumerator->RegisterEndpointNotificationCallback(this);
    }
    // 安全安装过滤器
    if (QCoreApplication::instance()) {
        QCoreApplication::instance()->installNativeEventFilter(this);
    } else {
        qFatal("AudioDeviceChangeListener must be created after QApplication!");
    }
    // WM_DEVICECHANGE 防抖定时器：连续事件合并为一次检查
    m_debounceTimer.setSingleShot(true);
    connect(&m_debounceTimer, &QTimer::timeout, this, &AudioDeviceChangeListener::checkDeviceChanges);
    // 初始化已知列表
    QSet<QString> ids;
    QMap<QString, QString> idToName;
    enumerateSpeakerDevices(ids, idToName);
    m_knownSpeakerIds = ids;
    m_knownSpeakerNames = idToName;
}


AudioDeviceChangeListener::~AudioDeviceChangeListener()
{
    qApp->removeNativeEventFilter(this);

    if (m_pEnumerator) {
        m_pEnumerator->UnregisterEndpointNotificationCallback(this);
        m_pEnumerator->Release();
    }
}

// ---------- IUnknown ----------
ULONG AudioDeviceChangeListener::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}

ULONG AudioDeviceChangeListener::Release()
{
    ULONG ulRef = InterlockedDecrement(&m_refCount);
    if (ulRef == 0) {
        delete this;
    }
    return ulRef;
}

HRESULT AudioDeviceChangeListener::QueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IUnknown || riid == __uuidof(IMMNotificationClient)) {
        *ppv = static_cast<IMMNotificationClient*>(this);
        AddRef();
        return S_OK;
    }
    *ppv = nullptr;
    return E_NOINTERFACE;
}

// ---------- WM_DEVICECHANGE 处理 ----------
// 事件过滤（同时检测输入/输出）
bool AudioDeviceChangeListener::nativeEventFilter(const QByteArray &eventType, void *message, long *result) {
    Q_UNUSED(eventType)
    Q_UNUSED(result)
    MSG* msg = static_cast<MSG*>(message);
    if (msg->message == WM_DEVICECHANGE) {
        if (msg->wParam == DBT_DEVICEARRIVAL || msg->wParam == DBT_DEVICEREMOVECOMPLETE || msg->wParam == DBT_DEVNODES_CHANGED){
            // 防抖：驱动安装期间 WM_DEVICECHANGE 会连续触发多次（设备节点创建、端点创建、
            // 属性写入等各阶段各来一次），每次 start() 都重置定时器，合并为最后一次检查。
            // 延迟 300ms 而不是 1ms：此时驱动/属性往往还没加载完，立即枚举会得到"半成品"设备。
            m_debounceTimer.start(300);
        }
    }
    return false;
}
//检测设备改变
void AudioDeviceChangeListener::checkDeviceChanges()
{
    QSet<QString> currentIds;
    QMap<QString, QString> idToName;
    enumerateSpeakerDevices(currentIds, idToName);

    // qDebug() << "====== Check Device Changes ======";
    // qDebug() << "Current active speaker IDs:" << currentIds;
    // qDebug() << "Known speaker IDs:" << m_knownSpeakerIds;

    // 对比找出新增和移除的设备
    QSet<QString> added = currentIds - m_knownSpeakerIds;
    QSet<QString> removed = m_knownSpeakerIds - currentIds;

    // ---- 新增设备就绪检查 ----
    // 设备刚插入时，WASAPI 端点可能已出现，但友好名称等属性还没写入（驱动未加载完全）。
    // 此时直接上报，上层 enumDevices2 / APO 会因取不到完整信息而无法正确识别设备。
    // 处理：延迟重试直到新增设备的名称可读；重试期间不更新已知列表，
    //       否则空名设备会被记入 known，之后就永远不再被当作"新增"（设备永久丢失）。
    //设备若名称不可读（= 属性未加载完）→ 不发射信号、不更新已知列表，300ms 后重试（前 3 次 300ms，之后 1000ms，最多 8次 ≈ 6 秒窗口）

    bool hasNotReadyDevice = false;
    for (const QString& id : added) {
        if (idToName.value(id).isEmpty()) {
            hasNotReadyDevice = true;
            qDebug() << "新增设备尚未加载完全，等待就绪:" << id;
            break;
        }
    }

    if (hasNotReadyDevice) {
        if (m_pendingRetries < kMaxRetries) {
            m_pendingRetries++;
            // 前几次快速重试，之后放慢，给慢驱动（如蓝牙）更多时间
            int delayMs = (m_pendingRetries <= 3) ? 300 : 1000;
            QTimer::singleShot(delayMs, this, &AudioDeviceChangeListener::checkDeviceChanges);
        } else {
            // 重试耗尽：保持设备"未确认"状态（不记入已知列表），
            // 后续 WM_DEVICECHANGE / OnDeviceStateChanged(ACTIVE) 事件会再次触发检查
            qWarning() << "新增设备加载超时（友好名称始终不可读），等待后续事件重试";
            m_pendingRetries = 0;
        }
        return;   // 设备未就绪：不发射信号、不更新已知列表
    }
    m_pendingRetries = 0;

    // 发出移除信号（使用之前保存的名称）
    for (const QString& id : removed) {
        QString name = m_knownSpeakerNames.value(id, QString());
        if (!name.isEmpty()) {
            qDebug() << "emit defaultDeviceDel";
            emit defaultDeviceDel(name);
        }
    }

    // 发出新增信号
    for (const QString& id : added) {
        QString name = idToName.value(id, QString());
        if (!name.isEmpty()) {
            qDebug() << "emit defaultDeviceAdd";
            emit defaultDeviceAdd(name);
        }
    }

    // 额外清理：如果 known 中的设备在 system 中仍存在但状态已变为 UNPLUGGED/NOTPRESENT，
    // 主动将其移出已知列表并发射信号（避免永远等不到 removed）
    for (const QString& knownId : m_knownSpeakerIds) {
        if (!currentIds.contains(knownId)) {
            continue; // 已处理
        }
        // 进一步检查该设备是否实际已不可用
        IMMDevice* pDevice = nullptr;
        HRESULT hr = m_pEnumerator->GetDevice(knownId.toStdWString().c_str(), &pDevice);
        if (SUCCEEDED(hr)) {
            DWORD state = 0;
            pDevice->GetState(&state);
            pDevice->Release();
            if (state & (DEVICE_STATE_UNPLUGGED | DEVICE_STATE_NOTPRESENT)) {
                // 强制移除并发射信号
                QString name = m_knownSpeakerNames.value(knownId);
                emit defaultDeviceDel(name);
                currentIds.remove(knownId); // 确保更新时不包含它
            }
        }
    }




    // 更新已知列表
    updateKnownDevices(currentIds, idToName);
}

// ---------- 扬声器设备枚举 ----------
void AudioDeviceChangeListener::enumerateSpeakerDevices(QSet<QString>& ids, QMap<QString, QString>& idToName)
{
    ids.clear();
    idToName.clear();

    IMMDeviceCollection* pCollection = nullptr;
    if (!m_pEnumerator) return;

    HRESULT hr = m_pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection);
    if (FAILED(hr)) return;

    UINT count = 0;
    pCollection->GetCount(&count);
    for (UINT i = 0; i < count; ++i) {
        IMMDevice* pDevice = nullptr;
        hr = pCollection->Item(i, &pDevice);
        if (FAILED(hr)) continue;

        LPWSTR pwszId = nullptr;
        hr = pDevice->GetId(&pwszId);
        if (SUCCEEDED(hr)) {
            QString id = QString::fromWCharArray(pwszId);
            CoTaskMemFree(pwszId);

            if (IsSpeakerDevice(id.toStdWString().c_str())) {
                ids.insert(id);
                // 获取友好名称
                IPropertyStore* pProps = nullptr;
                if (SUCCEEDED(pDevice->OpenPropertyStore(STGM_READ, &pProps))) {
                    PROPVARIANT varName;
                    PropVariantInit(&varName);
                    if (SUCCEEDED(pProps->GetValue(PKEY_Device_FriendlyName, &varName))) {
                        idToName.insert(id, QString::fromWCharArray(varName.pwszVal));
                    }
                    PropVariantClear(&varName);
                    pProps->Release();
                }
            }
        }
        pDevice->Release();
    }
    pCollection->Release();
}

void AudioDeviceChangeListener::updateKnownDevices(const QSet<QString>& currentIds,
                                                   const QMap<QString, QString>& idToName)
{
    m_knownSpeakerIds = currentIds;
    m_knownSpeakerNames = idToName;
}

// ---------- IsSpeakerDevice（保留原逻辑）----------
bool AudioDeviceChangeListener::IsSpeakerDevice(LPCWSTR pwstrDeviceId)
{
    IMMDevice* pDevice = nullptr;
    bool isSpeaker = false;

    if (!m_pEnumerator) return false;

    HRESULT hr = m_pEnumerator->GetDevice(pwstrDeviceId, &pDevice);
    if (FAILED(hr)) return false;

    IPropertyStore* pProps = nullptr;
    hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
    if (SUCCEEDED(hr)) {
        PROPVARIANT varForm;
        PropVariantInit(&varForm);
        hr = pProps->GetValue(PKEY_AudioEndpoint_FormFactor, &varForm);
        if (SUCCEEDED(hr) && varForm.vt == VT_UI4) {
            EndpointFormFactor formFactor = static_cast<EndpointFormFactor>(varForm.ulVal);
            isSpeaker = (formFactor == Speakers ||
                         formFactor == Headphones ||
                         formFactor == Headset);
        }
        PropVariantClear(&varForm);
        pProps->Release();
    }
    pDevice->Release();
    return isSpeaker;
}

// ---------- IMMNotificationClient 回调（仅默认设备变更使用）----------
HRESULT AudioDeviceChangeListener::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
{
    // 只关心扬声器/耳机类设备
    if (!IsSpeakerDevice(pwstrDeviceId))
        return S_OK;

    if (dwNewState & DEVICE_STATE_ACTIVE) {
        // 设备已就绪（ACTIVE），获取名称并通知
        QString id = QString::fromWCharArray(pwstrDeviceId);
        // 已知列表中没有该设备、或名称仍为空（说明 WM_DEVICECHANGE 路径尚未上报成功）时，重新上报
        if (!m_knownSpeakerIds.contains(id) || m_knownSpeakerNames.value(id).isEmpty()) {
            QString name = getDeviceNameFromId(pwstrDeviceId);
            if (!name.isEmpty()) {
                emit defaultDeviceAdd(name);
                // 同时加入已知列表，避免重复信号
                m_knownSpeakerIds.insert(id);
                m_knownSpeakerNames.insert(id, name);
            }
        }
    } else if (dwNewState & (DEVICE_STATE_UNPLUGGED | DEVICE_STATE_NOTPRESENT)) {
        // 设备被拔出或不存在，删除
        QString deviceName = getDeviceNameFromId(pwstrDeviceId);
        // 如果设备已不可访问，尝试从已知名称缓存获取
        if (deviceName.isEmpty()) {
            QString id = QString::fromWCharArray(pwstrDeviceId);
            deviceName = m_knownSpeakerNames.value(id, QString());
        }
        if (!deviceName.isEmpty()) {
            emit defaultDeviceDel(deviceName);
            m_knownSpeakerIds.remove(QString::fromWCharArray(pwstrDeviceId));
            m_knownSpeakerNames.remove(QString::fromWCharArray(pwstrDeviceId));
        }
    }
    return S_OK;
    // // 设备状态变化交由 WM_DEVICECHANGE 处理，此处不发射信号
    // return S_OK;
}

HRESULT AudioDeviceChangeListener::OnDeviceAdded(LPCWSTR /*pwstrDeviceId*/)
{
    return S_OK;
}

HRESULT AudioDeviceChangeListener::OnDeviceRemoved(LPCWSTR /*pwstrDeviceId*/)
{
    return S_OK;
}

HRESULT AudioDeviceChangeListener::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId)
{
    if (flow == eRender && role == eConsole) {
        QString deviceName = getDeviceNameFromId(pwstrDefaultDeviceId);
        emit defaultOutPutDeviceChanged(deviceName);
    } else if (flow == eCapture && role == eConsole) {
        QString deviceName = getDeviceNameFromId(pwstrDefaultDeviceId);
        emit defaultInPutDeviceChanged(deviceName);
    }
    return S_OK;
}

HRESULT AudioDeviceChangeListener::OnPropertyValueChanged(LPCWSTR /*pwstrDeviceId*/, const PROPERTYKEY /*key*/)
{
    return S_OK;
}

// ---------- 工具函数 ----------
QString AudioDeviceChangeListener::getDeviceNameFromId(LPCWSTR deviceId)
{
    QString deviceName;
    if (!m_pEnumerator) return deviceName;

    IMMDevice* pDevice = nullptr;
    HRESULT hr = m_pEnumerator->GetDevice(deviceId, &pDevice);
    if (SUCCEEDED(hr)) {
        IPropertyStore* pProps = nullptr;
        hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
        if (SUCCEEDED(hr)) {
            PROPVARIANT varName;
            PropVariantInit(&varName);
            hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
            if (SUCCEEDED(hr)) {
                deviceName = QString::fromWCharArray(varName.pwszVal);
            }
            PropVariantClear(&varName);
            pProps->Release();
        }
        pDevice->Release();
    }
    return deviceName;
}
