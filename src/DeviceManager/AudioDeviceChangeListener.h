/*#ifndef AUDIODEVICEMONITOR_H
#define AUDIODEVICEMONITOR_H

#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <QObject>

class AudioDeviceChangeListener : public QObject, public IMMNotificationClient
{
    Q_OBJECT
public:
    AudioDeviceChangeListener(QObject* parent = nullptr);
    virtual ~AudioDeviceChangeListener();

    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv);

    // IMMNotificationClient methods
    STDMETHOD(OnDeviceStateChanged)(LPCWSTR pwstrDeviceId, DWORD dwNewState);
    STDMETHOD(OnDeviceAdded)(LPCWSTR pwstrDeviceId);
    STDMETHOD(OnDeviceRemoved)(LPCWSTR pwstrDeviceId);
    STDMETHOD(OnDefaultDeviceChanged)(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId);
    STDMETHOD(OnPropertyValueChanged)(LPCWSTR pwstrDeviceId, const PROPERTYKEY key);


    QString getDeviceNameFromId(LPCWSTR deviceId);

    bool IsSpeakerDevice(LPCWSTR pwstrDeviceId);

signals:
    void defaultDeviceDel(QString deviceName);
    void defaultDeviceAdd(QString deviceName);
    void defaultOutPutDeviceChanged(QString deviceName);
    void defaultInPutDeviceChanged(QString deviceName);
    //void defaultDeviceChanged(QString deviceName);

private:
    LONG m_refCount;
    IMMDeviceEnumerator* m_pEnumerator;
};

#endif // AUDIODEVICEMONITOR_H
*/


#ifndef AUDIODEVICEMONITOR_H
#define AUDIODEVICEMONITOR_H

#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <QObject>
#include <QAbstractNativeEventFilter>
#include <QSet>
#include <QMap>
#include <QTimer>

class AudioDeviceChangeListener : public QObject,
                                  public QAbstractNativeEventFilter,
                                  public IMMNotificationClient
{
    Q_OBJECT
public:
    AudioDeviceChangeListener(QObject* parent = nullptr);
    virtual ~AudioDeviceChangeListener();

    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv);

    // IMMNotificationClient methods
    STDMETHOD(OnDeviceStateChanged)(LPCWSTR pwstrDeviceId, DWORD dwNewState);
    STDMETHOD(OnDeviceAdded)(LPCWSTR pwstrDeviceId);
    STDMETHOD(OnDeviceRemoved)(LPCWSTR pwstrDeviceId);
    STDMETHOD(OnDefaultDeviceChanged)(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId);
    STDMETHOD(OnPropertyValueChanged)(LPCWSTR pwstrDeviceId, const PROPERTYKEY key);

    // QAbstractNativeEventFilter
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;

    QString getDeviceNameFromId(LPCWSTR deviceId);
    bool IsSpeakerDevice(LPCWSTR pwstrDeviceId);

signals:
    void defaultDeviceDel(QString deviceName);
    void defaultDeviceAdd(QString deviceName);
    void defaultOutPutDeviceChanged(QString deviceName);
    void defaultInPutDeviceChanged(QString deviceName);

private slots:
    void checkDeviceChanges();

private:
    void enumerateSpeakerDevices(QSet<QString> &ids, QMap<QString, QString> &idToName);
    void updateKnownDevices(const QSet<QString> &currentIds, const QMap<QString, QString> &idToName);

    LONG m_refCount;
    IMMDeviceEnumerator* m_pEnumerator;

    // 已知的扬声器设备 ID 集合
    QSet<QString> m_knownSpeakerIds;
    // 已知设备 ID 到名称的映射，用于拔出时获取名称
    QMap<QString, QString> m_knownSpeakerNames;

    // WM_DEVICECHANGE 防抖定时器（驱动安装期间事件会连续触发多次，合并为一次检查）
    QTimer m_debounceTimer;
    // 新增设备就绪检查的重试计数
    int m_pendingRetries = 0;
    // 单次事件的最大重试次数（超时后保持"未确认"状态，等后续事件再触发）
    static const int kMaxRetries = 8;
};

#endif // AUDIODEVICEMONITOR_H
