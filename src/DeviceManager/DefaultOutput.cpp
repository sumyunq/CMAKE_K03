#include "DeviceManager/DefaultOutput.h"
#include <stdio.h>
#include <wchar.h>
#include <tchar.h>
#include "./Popup/RestartPrompt/RestartPrompt.h"
#include "windows.h"
#include <comdef.h>

#include "DeviceManager/PolicyConfig.h"
#include "Propidl.h"
#include "Functiondiscoverykeys_devpkey.h"

#include <QMessageBox>

#include "LoadLib.h"
#include "LoadApoDLL.h"
#include "APOThread/ApoManager.h"

#include <setupapi.h>
#include <devguid.h>
#include <QHash>
#include <QString>
#pragma comment(lib, "Setupapi.lib")
#pragma comment(lib, "Ole32.lib")
// #pragma comment(lib, "ksuser.lib")


// MinGW特化
#if defined(__MINGW32__)
template<>
const GUID& __mingw_uuidof<IPolicyConfigVista>() {
    static const GUID IID_IPolicyConfigVista = {
        0x568b9108, 0x44bf, 0x40b4, {0x90, 0x06, 0x86, 0xaf, 0xe5, 0xb5, 0xa6, 0x20}
    };
    return IID_IPolicyConfigVista;
}
template<>
const GUID& __mingw_uuidof<CPolicyConfigVistaClient>() {
    static const GUID IID_IPolicyConfigVista = {
        0x294935CE, 0xF637, 0x4E7C, {0xA4, 0x1B, 0xAB, 0x25, 0x54, 0x60, 0xB8, 0x62}
    };
    return IID_IPolicyConfigVista;
}
#endif


#include <memory>  // 添加这行
#include <functional>  // 用于std::function

// 全局或类的成员变量
QHash<QString, QHash<QString, QString>> audioDeviceInfoMap;

// 解析VID和PID的辅助函数示例
void ParseVIDPID(const wchar_t* hardwareId, QString& vid, QString& pid)
{
    // 查找 "VID_" 和 "PID_" 子串
    const wchar_t* vidPos = wcsstr(hardwareId, L"VID_");
    const wchar_t* pidPos = wcsstr(hardwareId, L"PID_");

    if (vidPos && pidPos) {
        // 提取4位十六进制VID和PID
        wchar_t vidBuf[5] = {0};
        wchar_t pidBuf[5] = {0};

        wcsncpy(vidBuf, vidPos + 4, 4);
        wcsncpy(pidBuf, pidPos + 4, 4);

        vid = QString::fromWCharArray(vidBuf).toUpper();
        pid = QString::fromWCharArray(pidBuf).toUpper();


        //qDebug("  VID: %ls, PID: %ls\n", vidBuf, pidBuf);
    }
}
void setup()
{
    audioDeviceInfoMap.clear();
    // 1. 获取音频设备信息集
    HDEVINFO hDevInfo = SetupDiGetClassDevs(
        &GUID_DEVCLASS_MEDIA,
        NULL,
        NULL,
        DIGCF_PRESENT
        );

    if (hDevInfo == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        // Qt环境下可以使用：//qDebug() << "SetupDiGetClassDevs 失败，错误代码:" << error;
        //qDebug("SetupDiGetClassDevs 失败，错误代码: %lu\n", error);
        return;
    }

    SP_DEVINFO_DATA deviceInfoData = { 0 };
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    // 2. 遍历设备
    for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &deviceInfoData); i++)
    {

        QString friendlyName;
        QString vid = "";
        QString pid = "";
        QString instancePath;//设备实例路径
        QString HardId;//设备硬件ID（蓝牙无后缀可通过）


        // 3. 使用兼容性更好的方式获取【设备实例路径】
        WCHAR deviceInstanceId[256];
        if (SetupDiGetDeviceInstanceIdW(
                hDevInfo,
                &deviceInfoData,
                deviceInstanceId,
                sizeof(deviceInstanceId) / sizeof(WCHAR),
                NULL
                )) {
            //qDebug("设备实例路径: %ls\n", deviceInstanceId);
            instancePath = QString::fromWCharArray(deviceInstanceId);
        } else {
            //qDebug("无法获取设备实例路径\n");
            continue;
        }

        // 4. 获取【硬件ID】（从中解析VID/PID）
        WCHAR hardwareIds[1024];
        if (SetupDiGetDeviceRegistryPropertyW(
                hDevInfo,
                &deviceInfoData,
                SPDRP_HARDWAREID,
                NULL,
                (PBYTE)hardwareIds,
                sizeof(hardwareIds),
                NULL
                )) {
            //qDebug("硬件ID: %ls\n", hardwareIds);

            // 解析VID和PID
            ParseVIDPID(hardwareIds,vid,pid);
            HardId = QString::fromWCharArray(hardwareIds);
        }

        // 5. 获取设备【友好名称】
        WCHAR friendlyNameBuffer[256] = {0};
        if (SetupDiGetDeviceRegistryPropertyW(
                hDevInfo,
                &deviceInfoData,
                SPDRP_FRIENDLYNAME,
                NULL,
                (PBYTE)friendlyNameBuffer,
                sizeof(friendlyNameBuffer),
                NULL
                )) {
            //qDebug("友好名称: %ls\n", friendlyNameBuffer);
            friendlyName = QString::fromWCharArray(friendlyNameBuffer);
        }

        // 6. 额外：检查是否为蓝牙设备
        //CheckIfBluetoothDevice(hDevInfo, &deviceInfoData);

        QHash<QString, QString> deviceInfo;
        deviceInfo["vid"] = vid;
        deviceInfo["pid"] = pid;
        deviceInfo["instance_path"] = instancePath;
        deviceInfo["hardware_id"] = HardId;
        deviceInfo["friendlyName"] = friendlyName;
        if(HardId.contains("BTHENUM", Qt::CaseInsensitive))
        {
            deviceInfo["IsBluetooth"] = "1";
        }else
        {
            deviceInfo["IsBluetooth"] = "0";
        }

        // 以友好名称为key存储
        // 注意：如果友好名称重复，后面的会覆盖前面的
        // 如果需要处理重复名称，可以添加序号
        QString uniqueKey = friendlyName;
        int counter = 1;
        while (audioDeviceInfoMap.contains(uniqueKey)) {
            const QHash<QString, QString>& existingDevice = audioDeviceInfoMap[uniqueKey];
            QString existingHardwareId = existingDevice["hardware_id"];

            // 如果已有设备的 hardware_id 与当前相同 → 覆盖或视为同一设备（可选择 break 或更新）
            if (existingHardwareId.compare(HardId, Qt::CaseInsensitive) == 0) {
                // 是同一个设备，允许覆盖或直接使用此 key
                break;
            } else {
                // 不是同一个设备，必须重命名
                uniqueKey = QString("%1 (%2)").arg(friendlyName).arg(counter++);
            }
        }

        audioDeviceInfoMap[uniqueKey] = deviceInfo;

        // //qDebug("存储音频设备: %s", uniqueKey.toUtf8().constData());
        // //qDebug("  VID: %s", vid.toUtf8().constData());
        // //qDebug("  PID: %s", pid.toUtf8().constData());
        // //qDebug("  实例路径: %s", instancePath.toUtf8().constData());
        // //qDebug("\n");
    }

    // 7. 清理资源
    SetupDiDestroyDeviceInfoList(hDevInfo);
}
//重启电脑
bool rebootWindows()
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        qWarning() << "OpenProcessToken failed:" << GetLastError();
        return false;
    }
    LookupPrivilegeValue(nullptr, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, nullptr, nullptr);
    if (GetLastError() != ERROR_SUCCESS) {
        qWarning() << "AdjustTokenPrivileges failed:" << GetLastError();
        return false;
    }

    // 强制重启，设为 0 表示立即关机/重启，不显示倒计时
    if (!InitiateSystemShutdownExW(nullptr, nullptr, 0, TRUE, TRUE,
                                   SHTDN_REASON_MAJOR_OTHER))
    {
        qWarning() << "InitiateSystemShutdownEx failed:" << GetLastError();
        return false;
    }
    return true;
}
QHash<QString, QHash<QString, QString>> DefaultOutput::enumDevices(EDataFlow dataFlow)
{
    QHash<QString, QHash<QString, QString>> list;
    QHash<QString, QHash<QString, QString>> list_dis;
    QHash<QString, QString> Msg_dis;
    Msg_dis.reserve(8);  // 预分配足够容量
    QHash<QString, QString> Msg;
    Msg.reserve(8);  // 预分配足够容量

    int supportedDeviceCount = 0;//支持APO的设备数量
    int NeedRebootCount = 0;//需要纠正APO优先级的设备数量

    setup();

    // 使用RAII管理COM初始化
    class COMInitializer {
    public:
        COMInitializer() : m_initialized(SUCCEEDED(CoInitialize(NULL))) {}
        ~COMInitializer() { if (m_initialized) CoUninitialize(); }
        bool isInitialized() const { return m_initialized; }
    private:
        bool m_initialized;
    };

    COMInitializer comInit;
    if (!comInit.isInitialized()) {
        return list;
    }

    // 使用智能指针管理COM接口（使用ComPtr或自定义删除器）
    IMMDeviceEnumerator* pEnum = nullptr;
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                  __uuidof(IMMDeviceEnumerator), (void**)&pEnum);
    if (FAILED(hr)) {
        return list;
    }

    // 自定义删除器
    auto comDeleter = [](IUnknown* p) { if (p) p->Release(); };
    std::unique_ptr<IMMDeviceEnumerator, decltype(comDeleter)> enumPtr(pEnum, comDeleter);

    IMMDeviceCollection* pDevices = nullptr;
    hr = enumPtr->EnumAudioEndpoints(dataFlow, DEVICE_STATE_ACTIVE, &pDevices);
    if (FAILED(hr)) {
        return list;
    }

    std::unique_ptr<IMMDeviceCollection, decltype(comDeleter)> devicesPtr(pDevices, comDeleter);

    UINT count = 0;
    hr = pDevices->GetCount(&count);
    if (FAILED(hr)) {
        return list;
    }

    // 预分配结果容量
    list.reserve(static_cast<int>(count));
    list_dis.reserve(static_cast<int>(count));

    // 预分类设备信息（只在需要时）
    bool needDeviceMatching = (dataFlow == eRender && retB && apo != nullptr);
    QVector<QHash<QString, QString>> bluetoothDevices;
    QVector<QHash<QString, QString>> normalDevices;

    if (needDeviceMatching && !audioDeviceInfoMap.isEmpty()) {
        bluetoothDevices.reserve(audioDeviceInfoMap.size());
        normalDevices.reserve(audioDeviceInfoMap.size());

        for (auto it = audioDeviceInfoMap.constBegin(); it != audioDeviceInfoMap.constEnd(); ++it) {
            const QHash<QString, QString>& deviceInfo = it.value();
            const QString& hardwareId = deviceInfo.value("hardware_id");
            if (deviceInfo.value("IsBluetooth") == "1" || hardwareId.contains("BTHENUM", Qt::CaseInsensitive))
            {
                bluetoothDevices.append(deviceInfo);
            } else {
                normalDevices.append(deviceInfo);
            }
        }
    }
    // 输出蓝牙设备名称
    qDebug() << "蓝牙设备列表:";
    for (const auto &dev : bluetoothDevices) {
        qDebug()<<"蓝牙" << dev.value("friendlyName");
    }

    // 输出普通设备名称
    // qDebug() << "普通设备列表:";
    for (const auto &dev : normalDevices) {
        qDebug() <<"普通" << dev.value("friendlyName");
        emit ApoManager::instance()->requestlogWithTime("=== 普通设备列表 ===");
        emit ApoManager::instance()->requestlogWithTime(QString("friendlyName:%1").arg(dev.value("friendlyName")));
    }



    // 循环枚举设备
    for (UINT i = 0; i < count; ++i)
    {
        IMMDevice* pDevice = nullptr;
        hr = pDevices->Item(i, &pDevice);
        if (FAILED(hr)) {
            _com_error err(hr);
            LPCTSTR errMsg = err.ErrorMessage();  // 获取可读错误信息
            emit ApoManager::instance()->requestlogWithTime(QString("Item 获取设备 %1 失败，HRESULT: %2，描述: %3").arg(i).arg(hr).arg(errMsg));
            continue;
        }

        std::unique_ptr<IMMDevice, decltype(comDeleter)> devicePtr(pDevice, comDeleter);

        LPWSTR wstrID = nullptr;
        hr = pDevice->GetId(&wstrID);
        if (FAILED(hr)) {
            _com_error err(hr);
            LPCTSTR errMsg = err.ErrorMessage();  // 获取可读错误信息
            emit ApoManager::instance()->requestlogWithTime(QString("GetId 获取设备 %1 失败，HRESULT: %2，描述: %3").arg(i).arg(hr).arg(errMsg));
            continue;
        }

        // RAII管理字符串内存
        auto coTaskMemFreeDeleter = [](void* p) { if (p) CoTaskMemFree(p); };
        std::unique_ptr<WCHAR[], decltype(coTaskMemFreeDeleter)> idPtr(wstrID, coTaskMemFreeDeleter);

        IPropertyStore* pStore = nullptr;
        hr = pDevice->OpenPropertyStore(STGM_READ, &pStore);
        if (FAILED(hr)) {
            _com_error err(hr);
            LPCTSTR errMsg = err.ErrorMessage();  // 获取可读错误信息
            emit ApoManager::instance()->requestlogWithTime(QString("OpenPropertyStore 获取设备 %1 失败，HRESULT: %2，描述: %3").arg(i).arg(hr).arg(errMsg));
            continue;
        }

        std::unique_ptr<IPropertyStore, decltype(comDeleter)> storePtr(pStore, comDeleter);

        PROPVARIANT friendlyName;
        PropVariantInit(&friendlyName);

        // RAII管理PROPVARIANT
        auto propVariantClearDeleter = [](PROPVARIANT* p) { PropVariantClear(p); };
        std::unique_ptr<PROPVARIANT, decltype(propVariantClearDeleter)>
            namePtr(&friendlyName, propVariantClearDeleter);

        hr = pStore->GetValue(PKEY_Device_FriendlyName, &friendlyName);
        if (FAILED(hr) || friendlyName.vt != VT_LPWSTR) {
            _com_error err(hr);
            LPCTSTR errMsg = err.ErrorMessage();  // 获取可读错误信息
            emit ApoManager::instance()->requestlogWithTime(QString("OpenPropertyStore 获取设备 %1 失败，HRESULT: %2，描述: %3").arg(i).arg(hr).arg(errMsg));
            continue;
        }

        // 快速字符串转换
        QString qid = QString::fromWCharArray(wstrID);
        QString qname = QString::fromWCharArray(friendlyName.pwszVal);

        //麦克风
        if (dataFlow == eCapture || General) {
            // 简单情况：只需要基本信息
            Msg.insert("Name", qname);
            list.insert(qid, Msg);

            emit ApoManager::instance()->requestlogWithTime("=== enumDevices Msg General 麦克风设备信息 ===");
            emit ApoManager::instance()->requestlogWithTime(QString("Name:%1").arg(qname));

            continue;
        }

        if (dataFlow == eRender && retB && apo)
        {
            // 在 normalDevices 中查找匹配的 friendlyName
            auto matchedInfo = std::find_if(normalDevices.begin(), normalDevices.end(),
                                            [&qname](const QHash<QString, QString>& devInfo) {
                                                QString friendly = devInfo.value("friendlyName");
                                                // 忽略大小写，且只比较基础名称（去掉可能的后缀）
                                                if (friendly.isEmpty()) return false;
                                                // 简单做法：qname 包含 friendly 或 friendly 包含 qname（视情况而定）
                                                return qname.contains(friendly, Qt::CaseInsensitive) ||
                                                       friendly.contains(qname, Qt::CaseInsensitive);
                                            });

            if (matchedInfo == normalDevices.end()) {
                // 不是普通设备（或找不到对应信息），跳过
                continue;
            }


            // 检查是否支持LHDC
            QString VidPid;
            int isSupported = 0;
            int isSuccessed = apo->IsLhdcDeviceSupport(qid, VidPid,isSupported);
            emit ApoManager::instance()->requestlogWithTime(QString("=== apo IsLhdcDeviceSupport返回 ===:%1").arg(isSuccessed));
            //true代表支持，false代表不支持，把不支持的设备信息都存起来，若最后没有支持的设备，判断不支持设备中有没有T10有线无线，K03S超竞版，K06S，t7，若存在，则代表APO被吃掉了
            if(isSuccessed == 1)
            {
                emit ApoManager::instance()->requestlogWithTime(QString("=== apo IsLhdcDeviceSupport isSupported返回 ===:%1").arg(isSupported));
                if (isSupported == -1)
                {
                    emit ApoManager::instance()->requestlogWithTime("=== apo不支持的设备信息 ===");
                    emit ApoManager::instance()->requestlogWithTime(QString("Name:%1").arg(qname));
                    emit ApoManager::instance()->requestlogWithTime(QString("qid:%1").arg(qid));
                    emit ApoManager::instance()->requestlogWithTime(QString("VidPid:%1").arg(VidPid));


                    qDebug() << "=== apo不支持的设备信息 ===";
                    qDebug() << "Name:" << qname;
                    qDebug() << "qid:" << qid;
                    qDebug() << "VidPid:" << VidPid;


                    // 需要修正 APO Registry key/value -> 修正後需重新開機才會生效
                    int setRet = apo->SetDeviceApoClsid(qid,0);
                    if (1 == setRet) NeedRebootCount++;

                    continue;

                    // QString vid, pid;
                    // // 提取 VID_... 和 PID_... 部分
                    // // 方法：按 '&' 分割
                    // QStringList pairs = VidPid.split('&');
                    // for (const QString& pair : pairs) {
                    //     // pair 可能是 "VidPid:VID_36F9" 或 "PID_F001"
                    //     if (pair.contains("VID_")) {
                    //         QString vidStr = pair.mid(pair.indexOf("VID_"));
                    //         vid = vidStr.mid(4); // 去掉 "VID_"
                    //     } else if (pair.contains("PID_")) {
                    //         QString pidStr = pair.mid(pair.indexOf("PID_"));
                    //         pid = pidStr.mid(4); // 去掉 "PID_"
                    //     }
                    // }

                    // Msg_dis.insert("Name",qname);
                    // Msg_dis.insert("vid", vid);
                    // Msg_dis.insert("pid", pid);
                    // list_dis.insert(qid, Msg_dis);

                    // continue;
                }else if(isSupported == 1)
                {
                    supportedDeviceCount++;
                }else
                {
                    continue;
                }

                Msg.insert("Name", qname);


                // 设备匹配逻辑，不能通过是否带有"耳机"判断是否蓝牙模式
                bool isBluetooth = 0;
                const QVector<QHash<QString, QString>>& searchList = normalDevices;

                emit ApoManager::instance()->requestlogWithTime("=== apo支持的设备信息 ===");
                emit ApoManager::instance()->requestlogWithTime(QString("Name:%1").arg(qname));
                emit ApoManager::instance()->requestlogWithTime(QString("qid:%1").arg(qid));
                emit ApoManager::instance()->requestlogWithTime(QString("VidPid:%1").arg(VidPid));
                // emit ApoManager::instance()->requestlogWithTime(QString("DO设备isBluetooth:%1").arg(isBluetooth));

                // 从 VidPid 中提取当前设备的 PID
                QString currentPid;
                if (!VidPid.isEmpty()) {
                    int pidIndex = VidPid.indexOf("PID_");
                    if (pidIndex != -1) {
                        int start = pidIndex + 4;          // 跳过 "PID_"
                        int end = VidPid.indexOf('&', start);
                        if (end == -1) end = VidPid.length();
                        currentPid = VidPid.mid(start, end - start);
                    }
                }

                int cnt = 0;
                bool matched = false;
                for (const auto& deviceInfo : searchList)
                {

                    QString friendlyNameInMap = deviceInfo.value("friendlyName");
                    qDebug()<<"DO设备名称111"<<friendlyNameInMap;
                    if (friendlyNameInMap.isEmpty()) {
                        emit ApoManager::instance()->requestlogWithTime("=== friendlyNameInMap名称为空 ===");
                        continue;
                    }

                    cnt++;
                    qDebug("DOcnt:%d\n",cnt);

                    // 使用局部变量避免重复调用value()
                    const QString& vid = deviceInfo.value("vid");
                    const QString& pid = deviceInfo.value("pid");
                    const QString& hardwareId = deviceInfo.value("hardware_id");

                    // qDebug() << "=== enumDevices Msg设备信息 ===";
                    // qDebug()<<"DO设备硬件ID" <<hardwareId;
                    // qDebug()<<"DO设备VID" <<vid;
                    // qDebug()<<"DO设备PID" <<pid;
                    // qDebug()<<"DO设备friendlyNameInMap" <<friendlyNameInMap;
                    // qDebug()<<"DO设备qname" <<qname;

                    emit ApoManager::instance()->requestlogWithTime("=== enumDevices Msg设备信息 ===");
                    emit ApoManager::instance()->requestlogWithTime(QString("DO设备硬件ID:%1").arg(hardwareId));
                    emit ApoManager::instance()->requestlogWithTime(QString("DO设备VID:%1").arg(vid));
                    emit ApoManager::instance()->requestlogWithTime(QString("DO设备PID:%1").arg(pid));
                    emit ApoManager::instance()->requestlogWithTime(QString("DO设备friendlyNameInMap:%1").arg(friendlyNameInMap));
                    emit ApoManager::instance()->requestlogWithTime(QString("DO设备qname:%1").arg(qname));

                    //蓝牙模式
                    if(hardwareId.contains("BTHENUM", Qt::CaseInsensitive))
                    {
                        isBluetooth = 1;
                        emit ApoManager::instance()->requestlogWithTime("蓝牙模式");
                        // emit ApoManager::instance()->requestlogWithTime("蓝牙模式，不可进入");
                        // continue;
                    }else
                    {
                        isBluetooth = 0;
                    }



                    if (qname.contains(friendlyNameInMap, Qt::CaseInsensitive) && currentPid == pid) {
                        if(qname.contains("Wireless", Qt::CaseInsensitive))
                        {
                            if (!friendlyNameInMap.contains("Wireless", Qt::CaseInsensitive))
                            {
                                continue;
                            }
                        }
                        Msg.insert("vid", vid);
                        Msg.insert("pid", pid);
                        Msg.insert("vid&pid", isBluetooth ? hardwareId :
                                                  QString("VID_%1&PID_%2").arg(vid).arg(pid));
                        Msg.insert("IsBluetooth", isBluetooth ? "1" : "0");
                        Msg.insert("friendlyName", friendlyNameInMap);
                        matched = true;

                        emit ApoManager::instance()->requestlogWithTime("添加设备");
                        list.insert(qid, Msg);

                        break;
                    }
                }
            }

        }
    }

    /*// 沒有任何支援裝置時：只顯示一次彈窗（直到之後有支援裝置出現才重置）
    if (supportedDeviceCount == 0 && (!list_dis.isEmpty()))
    {
        bool showMgx = false;
        QStringList unsupportedDeviceReports;
        for(QHash<QString, QHash<QString, QString>>::iterator iter = list_dis.begin(); iter != list_dis.end(); iter++ )
        {
            QHash<QString, QString> deviceInfo = iter.value();
            // 比较设备信息的每个字段
            int vid = deviceInfo.value("vid").toUInt();
            int pid = deviceInfo.value("pid").toUInt();
            QString qName = deviceInfo.value("Name");
            if( qName.contains("T10",Qt::CaseInsensitive)
                || (qName.contains("K03S",Qt::CaseInsensitive) && (pid == 0xF016 || pid == 0xF017))
                || qName.contains("K06S",Qt::CaseInsensitive)
                || (qName.contains("T7",Qt::CaseInsensitive) && (pid == 0xF014 || pid == 0xF008))
                )
            {
                showMgx = true;
                QString qid = iter.key();
                uint eFlow = 0; // 0 = RENDER, 1 = CAPTURE
                uint apoCount = 0;
                int ret = apo->GetDeviceApoCount(qid,eFlow,apoCount);
                emit ApoManager::instance()->requestlogWithTime(QString("GetDeviceApoCount Ret:%1,apoCount:%2").arg(ret).arg(apoCount));

                QString report;
                QTextStream stream(&report);
                stream << QStringLiteral("装置名称: ") << qName << "\n";
                stream << QStringLiteral("裝置 ID: ") << qid << "\n";
                stream << QStringLiteral("其他APO信息: ") << "\n";
                if(apoCount == 0)
                {
                    stream << QStringLiteral("无") << "\n";
                }
                for (uint apoIndex = 0; apoIndex < apoCount; apoIndex++)
                {
                    QString friendlyName, copyrightInfo;
                    int retInfo = apo->GetDeviceApoInfo(eFlow, apoIndex, friendlyName, copyrightInfo);
                    stream << "  [" << apoIndex << "] Ret=" << retInfo
                           << QStringLiteral(", APO名称=") << friendlyName
                           << QStringLiteral(", 公司=") << copyrightInfo << "\n";
                }
                unsupportedDeviceReports.append(report);
            }
        }
        if(showMgx)
        {
            QString separator =
                QString(50, '-') +   // 生成 50 个连字符
                QString("\n");
            QString detail = unsupportedDeviceReports.join(separator);

            QString message =
                "本机声卡支持 APO 音效，但组件运行异常。可能是其他音效软件冲突，或是驱动安装不完整。下述为当前所支持的设备挂载的所有 APO 信息，请卸载其他APO。\n\n" +
                detail;

            // 直接显示消息框
            QMessageBox::information(nullptr, "APO 裝置资讯", message);
        }
    }*/


    // 有任何装置的 APO 设定被修正 -> 需要重新开机 APO机制才會生效
    if (NeedRebootCount > 0)
    {
        RestartPrompt *restart = new RestartPrompt(m);
        restart->setModal(true);
        int ret = restart->exec();
        if(ret == QDialog::Accepted)
        {
            //重启
            if (rebootWindows()) {
                QApplication::quit();
            } else {
                QMessageBox::warning(m, "错误", "重启失败，请手动重启。");
            }
        }
        restart->deleteLater();
        restart = nullptr;
    }

    return list;

}

// enumDevices 的优化版，行为与 enumDevices 一致：
// 1. setup()（SetupAPI 全量枚举）只在"渲染 + APO 匹配"路径按需执行，eCapture/General 不再白扫
// 2. 内层 O(端点×设备) 的日志刷屏移除，每台设备只保留关键审计日志（日志文案未改动）
// 3. find_if + 内层 for 双重扫描合并为单次扫描，顺带省掉无谓的 APO 调用
// 4. Msg 局部化，避免跨设备字段残留；删除死代码（list_dis/supportedDeviceCount/蓝牙预分类）
QHash<QString, QHash<QString, QString>> DefaultOutput::enumDevices2(EDataFlow dataFlow)
{
    QHash<QString, QHash<QString, QString>> list;

    // 需要修正 APO 优先级的设备数量（修正后需重启生效）
    int NeedRebootCount = 0;

    // 设备表只在"渲染 + APO 匹配"路径才用得上，SetupAPI 全量枚举开销大，按需执行
    const bool needDeviceMatching = (dataFlow == eRender && retB && apo != nullptr);
    if (needDeviceMatching)
        setup();

    // 普通设备表（蓝牙设备排除在外，与 enumDevices 行为一致）
    QVector<QHash<QString, QString>> normalDevices;
    if (needDeviceMatching && !audioDeviceInfoMap.isEmpty()) {
        normalDevices.reserve(audioDeviceInfoMap.size());
        for (auto it = audioDeviceInfoMap.constBegin(); it != audioDeviceInfoMap.constEnd(); ++it) {
            const QHash<QString, QString>& deviceInfo = it.value();
            if (deviceInfo.value("IsBluetooth") == "1"
                || deviceInfo.value("hardware_id").contains("BTHENUM", Qt::CaseInsensitive)) {
                continue;   // 蓝牙设备不参与匹配
            }
            normalDevices.append(deviceInfo);
        }
    }

    // 缓存单例，避免循环内反复查询
    ApoManager* apoMgr = ApoManager::instance();

    // 调试输出：普通设备列表
    if (!normalDevices.isEmpty()) {
        emit apoMgr->requestlogWithTime("=== 普通设备列表 ===");
        for (const auto& dev : normalDevices)
            emit apoMgr->requestlogWithTime(QString("friendlyName:%1").arg(dev.value("friendlyName")));
    }

    // 使用RAII管理COM初始化
    class COMInitializer {
    public:
        COMInitializer() : m_initialized(SUCCEEDED(CoInitialize(NULL))) {}
        ~COMInitializer() { if (m_initialized) CoUninitialize(); }
        bool isInitialized() const { return m_initialized; }
    private:
        bool m_initialized;
    };

    COMInitializer comInit;
    if (!comInit.isInitialized())
        return list;

    // 使用智能指针管理COM接口
    IMMDeviceEnumerator* pEnum = nullptr;
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                  __uuidof(IMMDeviceEnumerator), (void**)&pEnum);
    if (FAILED(hr))
        return list;

    // 自定义删除器
    auto comDeleter = [](IUnknown* p) { if (p) p->Release(); };
    std::unique_ptr<IMMDeviceEnumerator, decltype(comDeleter)> enumPtr(pEnum, comDeleter);

    IMMDeviceCollection* pDevices = nullptr;
    hr = enumPtr->EnumAudioEndpoints(dataFlow, DEVICE_STATE_ACTIVE, &pDevices);
    if (FAILED(hr))
        return list;

    std::unique_ptr<IMMDeviceCollection, decltype(comDeleter)> devicesPtr(pDevices, comDeleter);

    UINT count = 0;
    hr = pDevices->GetCount(&count);
    if (FAILED(hr))
        return list;

    // 预分配结果容量
    list.reserve(static_cast<int>(count));

    // 循环枚举设备
    for (UINT i = 0; i < count; ++i)
    {
        IMMDevice* pDevice = nullptr;
        hr = pDevices->Item(i, &pDevice);
        if (FAILED(hr)) {
            _com_error err(hr);
            emit apoMgr->requestlogWithTime(QString("Item 获取设备 %1 失败，HRESULT: %2，描述: %3").arg(i).arg(hr).arg(err.ErrorMessage()));
            continue;
        }

        std::unique_ptr<IMMDevice, decltype(comDeleter)> devicePtr(pDevice, comDeleter);

        LPWSTR wstrID = nullptr;
        hr = pDevice->GetId(&wstrID);
        if (FAILED(hr)) {
            _com_error err(hr);
            emit apoMgr->requestlogWithTime(QString("GetId 获取设备 %1 失败，HRESULT: %2，描述: %3").arg(i).arg(hr).arg(err.ErrorMessage()));
            continue;
        }

        // RAII管理字符串内存
        auto coTaskMemFreeDeleter = [](void* p) { if (p) CoTaskMemFree(p); };
        std::unique_ptr<WCHAR[], decltype(coTaskMemFreeDeleter)> idPtr(wstrID, coTaskMemFreeDeleter);

        IPropertyStore* pStore = nullptr;
        hr = pDevice->OpenPropertyStore(STGM_READ, &pStore);
        if (FAILED(hr)) {
            _com_error err(hr);
            emit apoMgr->requestlogWithTime(QString("OpenPropertyStore 获取设备 %1 失败，HRESULT: %2，描述: %3").arg(i).arg(hr).arg(err.ErrorMessage()));
            continue;
        }

        std::unique_ptr<IPropertyStore, decltype(comDeleter)> storePtr(pStore, comDeleter);

        PROPVARIANT friendlyName;
        PropVariantInit(&friendlyName);

        // RAII管理PROPVARIANT
        auto propVariantClearDeleter = [](PROPVARIANT* p) { PropVariantClear(p); };
        std::unique_ptr<PROPVARIANT, decltype(propVariantClearDeleter)>
            namePtr(&friendlyName, propVariantClearDeleter);

        hr = pStore->GetValue(PKEY_Device_FriendlyName, &friendlyName);
        if (FAILED(hr) || friendlyName.vt != VT_LPWSTR) {
            _com_error err(hr);
            emit apoMgr->requestlogWithTime(QString("OpenPropertyStore 获取设备 %1 失败，HRESULT: %2，描述: %3").arg(i).arg(hr).arg(err.ErrorMessage()));
            continue;
        }

        // 快速字符串转换
        const QString qid = QString::fromWCharArray(wstrID);
        const QString qname = QString::fromWCharArray(friendlyName.pwszVal);

        // 麦克风 / 通用模式：只需要基本信息
        if (dataFlow == eCapture || General) {
            QHash<QString, QString> msg;
            msg.insert("Name", qname);
            list.insert(qid, msg);

            emit apoMgr->requestlogWithTime("=== enumDevices Msg General 麦克风设备信息 ===");
            emit apoMgr->requestlogWithTime(QString("Name:%1").arg(qname));

            continue;
        }

        if (dataFlow == eRender && retB && apo)
        {
            // 单次扫描：收集友好名匹配的候选（含 Wireless 约束），通常 0~1 个
            QVector<const QHash<QString, QString>*> candidates;
            for (const auto& devInfo : normalDevices) {
                const QString friendly = devInfo.value("friendlyName");
                if (friendly.isEmpty()) continue;
                if (!qname.contains(friendly, Qt::CaseInsensitive)) continue;
                if (qname.contains("Wireless", Qt::CaseInsensitive)
                    && !friendly.contains("Wireless", Qt::CaseInsensitive)) continue;
                candidates.append(&devInfo);
            }
            if (candidates.isEmpty())
                continue;   // 不是普通设备（或找不到对应信息），跳过，省掉 APO 调用

            // 检查是否支持LHDC
            QString VidPid;
            int isSupported = 0;
            // again:
            int isSuccessed = apo->IsLhdcDeviceSupport(qid, VidPid, isSupported);
            emit apoMgr->requestlogWithTime(QString("=== apo IsLhdcDeviceSupport返回 ===:%1").arg(isSuccessed));
            // true代表支持，false代表不支持，把不支持的设备信息都存起来，若最后没有支持的设备，
            // 判断不支持设备中有没有T10有线无线，K03S超竞版，K06S，t7，若存在，则代表APO被吃掉了
            if (isSuccessed != 1)
                continue;

            emit apoMgr->requestlogWithTime(QString("=== apo IsLhdcDeviceSupport isSupported返回 ===:%1,%2").arg(isSupported).arg(VidPid));

            if (isSupported == -1)
            {
                // 不支持的设备：需要修正 APO Registry key/value -> 修正後需重新開機才會生效
                emit apoMgr->requestlogWithTime("=== apo不支持的设备信息 ===");
                emit apoMgr->requestlogWithTime(QString("Name:%1").arg(qname));
                emit apoMgr->requestlogWithTime(QString("qid:%1").arg(qid));
                emit apoMgr->requestlogWithTime(QString("VidPid:%1").arg(VidPid));

                // 需要修正 APO Registry key/value -> 修正後需重新開機才會生效
                int setRet = apo->SetDeviceApoClsid(qid, 0);
                if (1 == setRet) NeedRebootCount++;

                continue;
            }
            // if (isSupported == 0
            //     && (qname.contains("T10",Qt::CaseInsensitive)
            //                          || qname.contains("K03S",Qt::CaseInsensitive)
            //                          || qname.contains("K03",Qt::CaseInsensitive)
            //                          || qname.contains("K06S",Qt::CaseInsensitive)
            //                          || qname.contains("T7 GT",Qt::CaseInsensitive)
            //                          || qname.contains("T7",Qt::CaseInsensitive)))
            // {
            //     emit apoMgr->requestlogWithTime("=== 执行again ===");
            //     goto again;
            // }
            if (isSupported != 1)
                continue;   // 未知状态，跳过

            emit apoMgr->requestlogWithTime("=== apo支持的设备信息 ===");
            emit apoMgr->requestlogWithTime(QString("Name:%1").arg(qname));
            emit apoMgr->requestlogWithTime(QString("qid:%1").arg(qid));
            emit apoMgr->requestlogWithTime(QString("VidPid:%1").arg(VidPid));

            // 从 VidPid 中提取当前设备的 PID
            QString currentPid;
            if (!VidPid.isEmpty()) {
                int pidIndex = VidPid.indexOf("PID_");
                if (pidIndex != -1) {
                    int start = pidIndex + 4;          // 跳过 "PID_"
                    int end = VidPid.indexOf('&', start);
                    if (end == -1) end = VidPid.length();
                    currentPid = VidPid.mid(start, end - start);
                }
            }

            // 在候选中找 PID 匹配的设备（蓝牙模式由硬件ID判定，与 enumDevices 一致）
            const QHash<QString, QString>* matchedInfo = nullptr;
            for (const auto* cand : candidates) {
                if (cand->value("pid") == currentPid) {
                    matchedInfo = cand;
                    break;
                }
            }
            if (!matchedInfo)
                continue;

            const QString vid = matchedInfo->value("vid");
            const QString pid = matchedInfo->value("pid");
            const QString hardwareId = matchedInfo->value("hardware_id");
            const bool isBluetooth = hardwareId.contains("BTHENUM", Qt::CaseInsensitive);

            if (isBluetooth)
                emit apoMgr->requestlogWithTime("蓝牙模式");

            QHash<QString, QString> msg;
            msg.reserve(6);
            msg.insert("Name", qname);
            msg.insert("vid", vid);
            msg.insert("pid", pid);
            msg.insert("vid&pid", isBluetooth ? hardwareId :
                                              QString("VID_%1&PID_%2").arg(vid).arg(pid));
            msg.insert("IsBluetooth", isBluetooth ? "1" : "0");
            msg.insert("friendlyName", matchedInfo->value("friendlyName"));
            list.insert(qid, msg);

            emit apoMgr->requestlogWithTime("添加设备");
        }
    }

    // 有任何装置的 APO 设定被修正 -> 需要重新开机 APO机制才會生效
    if (NeedRebootCount > 0)
    {
        RestartPrompt *restart = new RestartPrompt(m);
        restart->setModal(true);
        int ret = restart->exec();
        if(ret == QDialog::Accepted)
        {
            //重启
            if (rebootWindows()) {
                QApplication::quit();
            } else {
                QMessageBox::warning(m, "错误", "重启失败，请手动重启。");
            }
        }
        restart->deleteLater();
        restart = nullptr;
    }

    return list;
}

// 辅助函数：从设备实例ID中提取VID和PID   删
QString DefaultOutput::extractVidPidFromInstanceId(const QString& instanceId,QString& vid, QString& pid)
{
    QString result;

    // 常见的格式：
    // USB\VID_xxxx&PID_xxxx&...
    // HDAUDIO\FUNC_01&VEN_xxxx&DEV_xxxx&...
    // BTHENUM\{0000110b-0000-1000-8000-00805f9b34fb}_VID&0001000e_PID&0000

    // 查找VID和PID
    QString upperId = instanceId.toUpper();

    // 对于USB设备
    if (upperId.contains("USB\\")) {
        QRegularExpression vidRegex("VID_([0-9A-F]{4})");
        QRegularExpression pidRegex("PID_([0-9A-F]{4})");

        QRegularExpressionMatch vidMatch = vidRegex.match(upperId);
        QRegularExpressionMatch pidMatch = pidRegex.match(upperId);

        if (vidMatch.hasMatch() && pidMatch.hasMatch()) {
            vid = vidMatch.captured(1);
            pid = pidMatch.captured(1);
            result = QString("VID_%1&PID_%2").arg(vid).arg(pid);
        }
    }
    // 对于HD Audio设备（使用VEN和DEV）
    else if (upperId.contains("HDAUDIO\\")) {
        /* QRegularExpression venRegex("VEN_([0-9A-F]{4})");
        QRegularExpression devRegex("DEV_([0-9A-F]{4})");

        QRegularExpressionMatch venMatch = venRegex.match(upperId);
        QRegularExpressionMatch devMatch = devRegex.match(upperId);

        if (venMatch.hasMatch() && devMatch.hasMatch()) {
            QString ven = venMatch.captured(1);
            QString dev = devMatch.captured(1);
            result = QString("VEN_%1&DEV_%2").arg(ven).arg(dev);
        }*/
    }
    // 对于蓝牙设备
    else if (upperId.contains("BTHENUM\\")) {
        QRegularExpression vidRegex("VID&([0-9A-F]+)");
        QRegularExpression pidRegex("PID&([0-9A-F]+)");

        QRegularExpressionMatch vidMatch = vidRegex.match(upperId);
        QRegularExpressionMatch pidMatch = pidRegex.match(upperId);

        if (vidMatch.hasMatch() && pidMatch.hasMatch()) {
            vid = vidMatch.captured(1);
            pid = pidMatch.captured(1);

            // 确保长度正确（通常为4位，不足则补0）
            vid = QString("%1").arg(vid.toUInt(nullptr, 16), 4, 16, QChar('0')).toUpper();
            pid = QString("%1").arg(pid.toUInt(nullptr, 16), 4, 16, QChar('0')).toUpper();

            result = QString("VID_%1&PID_%2").arg(vid).arg(pid);
        }
    }

    return result;
}

QString DefaultOutput::getDefaultDevice(EDataFlow dataFlow)
{
    QString defaultId;

    HRESULT hr = CoInitialize(nullptr); // 可能已初始化，重复调用无副作用
    bool wasInitialized = SUCCEEDED(hr);

    IMMDeviceEnumerator* pEnum = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                          __uuidof(IMMDeviceEnumerator), (void**)&pEnum);
    if (FAILED(hr))
        return defaultId;

    IMMDevice* pDefaultDevice = nullptr;
    hr = pEnum->GetDefaultAudioEndpoint(dataFlow, eConsole, &pDefaultDevice);
    if (SUCCEEDED(hr)) {
        LPWSTR wstrID = nullptr;
        hr = pDefaultDevice->GetId(&wstrID);
        if (SUCCEEDED(hr)) {
            defaultId = QString::fromStdU16String((char16_t*)wstrID);
            CoTaskMemFree(wstrID);
        }
        pDefaultDevice->Release();
    }

    pEnum->Release();

    if (!wasInitialized) {
        CoUninitialize();
    }

    return defaultId;
}

bool DefaultOutput::changeDevice( QString id )
{
    try
    {
        IPolicyConfigVista* pPolicyConfig;
        //eConsole:控制台(系统通知音、传统应用音频)  eMultimedia:多媒体(媒体播放器、游戏等非实时音频)  eCommunications:通信(通话、会议等实时音频流)
        //eConsole、eMultimedia都设置为音频默认值，eCommunications设置为通信默认值
        //可以把三种都设置到同一个设备上，以防需各种切换
        HRESULT hr = CoCreateInstance( __uuidof( CPolicyConfigVistaClient ), NULL, CLSCTX_ALL, __uuidof( IPolicyConfigVista ), (LPVOID*)&pPolicyConfig );
        if( SUCCEEDED( hr ) )
        {
            hr = pPolicyConfig->SetDefaultEndpoint( (PCWSTR)id.toStdU16String().data(), eConsole );
            hr = pPolicyConfig->SetDefaultEndpoint( (PCWSTR)id.toStdU16String().data(), eMultimedia );
            hr = pPolicyConfig->SetDefaultEndpoint( (PCWSTR)id.toStdU16String().data(), eCommunications );
            pPolicyConfig->Release();
            return SUCCEEDED( hr );
        }
        else
            return false;
    }
    catch( ... )
    {
        return false;
    }
}
