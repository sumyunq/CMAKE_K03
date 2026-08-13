#ifndef DEFAULTOUTPUT_H
#define DEFAULTOUTPUT_H

#include <QString>
#include <QHash>
#include "Mmdeviceapi.h"
#include <QRegularExpression>

class DefaultOutput
{

private:
    IMMDeviceEnumerator* m_pEnumerator;

public:
    //static QHash<QString, QString>  enumDevices(EDataFlow dataFlow);//获取所有设备
    static QHash<QString,  QHash<QString, QString>>  enumDevices(EDataFlow dataFlow);//获取所有设备
    static QHash<QString,  QHash<QString, QString>>  enumDevices2(EDataFlow dataFlow);//获取所有设备(优化版)
    static QString DefaultOutput::getDefaultDevice(EDataFlow dataFlow);//获取默认设备
    static bool                     changeDevice( QString id );//更改默认设备
    //static QString DefaultOutput::extractVidPidFromInstanceId(const QString& instanceId);
    static QString extractVidPidFromInstanceId(const QString& instanceId,QString& vid, QString& pid);
};

#endif
