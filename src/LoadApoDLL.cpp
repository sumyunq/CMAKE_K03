#include "LoadApoDLL.h"
#include <tchar.h>
#include <QCoreApplication>
#include <QTimer>
#include <QMutex>
#include <QDebug>
#define NOMINMAX
#include <algorithm>
#include <windows.h>
#include <QMessageBox>
static const char* STR_CERTIFICATE = "TbwjQA%%Se";
bool IsSetLdcDev = false;
const int EqualizerBandArraySize = 20;


LoadApoDLL::LoadApoDLL()
{

}
LoadApoDLL::~LoadApoDLL()
{
    FreeLibrary(m_hAct);
    m_hAct = NULL;

    if (g_hSavi)
    {
        g_pSaviDeinit(g_hSavi);
        g_hSavi = NULL;
    }
    FreeLibrary(g_hSaviCTL);
    g_hSaviCTL = NULL;
}
void LoadApoDLL::LoadApoLibrary()
{
    TCHAR  szModule[512] = { 0 };
    TCHAR  szModule_Down[512] = { 0 };

    //wcscat_s(szModule, L"Savitech3darEngineApo.dll");//下行驱动接口
    wcscat_s(szModule, L"LHDCX_XIBERIA_CLI.dll");//下行驱动接口
    wcscat_s(szModule_Down, L"SaviUIControl.dll");//上行驱动接口
    SetLastError(0);

    //下行库
    m_hAct = LoadLibrary(szModule);
    if(m_hAct == NULL)
    {
        QMessageBox::critical(nullptr, QObject::tr("错误"), QObject::tr("加载下行库失败，缺少插件，请重新安装驱动"));
        qDebug("LoadLibrary failed:%d\n",GetLastError());
        return;
    }else
    {
        qDebug("APO LoadLibrary success\n");

        //导出库里面的函数地址
        pIsLhdcDeviceSupport = (Savitech_IsLhdcDeviceSupport)GetProcAddress(m_hAct, "_Savitech_IsLhdcDeviceSupport@16");
        // pSetLhdcDevicePrivate = (Savitech_SetLhdcDevicePrivate)GetProcAddress(m_hAct, "_Savitech_SetLhdcDevicePrivate@4");
        pSetLhdcDevice = (Savitech_SetLhdcDevice)GetProcAddress(m_hAct, "_Savitech_SetLhdcDevice@4");
        // pInitialEngine = (Savitech_InitialEngine)GetProcAddress(m_hAct, "_Savitech_InitialEngine@4");

        // pGetDeviceApoCount = (Savitech_GetDeviceApoCount)GetProcAddress(m_hAct, "_Savitech_GetDeviceApoCount@12");
        // pGetDeviceApoInfo = (Savitech_GetDeviceApoInfo)GetProcAddress(m_hAct, "_Savitech_GetDeviceApoInfo@24");
        pSetDeviceApoClsid = (Savitech_SetDeviceApoClsid)GetProcAddress(m_hAct, "_Savitech_SetDeviceApoClsid@");

        pSetProcessEffectOption = (Savitech_SetProcessEffectOption)GetProcAddress(m_hAct, "_Savitech_SetProcessEffectOption@4");
        pGetProcessEffectOption = (Savitech_GetProcessEffectOption)GetProcAddress(m_hAct, "_Savitech_GetProcessEffectOption@4");
        /*pSetEqualizerBandQualityEx = (Savitech_SetEqualizerBandQualityEx)GetProcAddress(m_hAct, "_Savitech_SetEqualizerBandQualityEx@8");
        pGetEqualizerBandQualityEx = (Savitech_GetEqualizerBandQualityEx)GetProcAddress(m_hAct, "_Savitech_GetEqualizerBandQualityEx@8");
        pSetEqualizerCenterFrequencyEx = (Savitech_SetEqualizerCenterFrequencyEx)GetProcAddress(m_hAct, "_Savitech_SetEqualizerCenterFrequencyEx@8");
        pGetEqualizerCenterFrequencyEx = (Savitech_GetEqualizerCenterFrequencyEx)GetProcAddress(m_hAct, "_Savitech_GetEqualizerCenterFrequencyEx@8");
        pSetEqualizerBandEnableEx = (Savitech_SetEqualizerBandEnableEx)GetProcAddress(m_hAct, "_Savitech_SetEqualizerBandEnableEx@8");;
        pGetEqualizerBandEnableEx = (Savitech_GetEqualizerBandEnableEx)GetProcAddress(m_hAct, " _Savitech_GetEqualizerBandEnableEx@8");;
        pSetEqualizerGain = (Savitech_SetEqualizerGain)GetProcAddress(m_hAct, "_Savitech_SetEqualizerGain@12");
        pGetEqualizerGain = (Savitech_GetEqualizerGain)GetProcAddress(m_hAct, "_Savitech_GetEqualizerGain@8");
        pSetEqualizerGains = (Savitech_SetEqualizerGains)GetProcAddress(m_hAct, "_Savitech_SetEqualizerGains@8");
        pGetEqualizerGains = (Savitech_GetEqualizerGains)GetProcAddress(m_hAct, "_Savitech_GetEqualizerGains@8");*/
        // pSetEqualizerGainEx = (Savitech_SetEqualizerGainEx)GetProcAddress(m_hAct, "_Savitech_SetEqualizerGainEx@4");
        // pGetEqualizerGainEx = (Savitech_GetEqualizerGainEx)GetProcAddress(m_hAct, "_Savitech_GetEqualizerGainEx@4");
        pSetReverbActivateRoomType = (Savitech_SetReverbActivateRoomType)GetProcAddress(m_hAct, "_Savitech_SetReverbActivateRoomType@4");
        pGetReverbActivateRoomType = (Savitech_GetReverbActivateRoomType)GetProcAddress(m_hAct, "_Savitech_GetReverbActivateRoomType@4");
        pSetArReverbRatio = (Savitech_SetArReverbRatio)GetProcAddress(m_hAct, "_Savitech_SetArReverbRatio@8");
        pGetArReverbRatio = (Savitech_GetArReverbRatio)GetProcAddress(m_hAct, "_Savitech_GetArReverbRatio@4");
        pSetCompBassGain = (Savitech_SetCompBassGain)GetProcAddress(m_hAct, "_Savitech_SetCompBassGain@4");
        pGetCompBassGain = (Savitech_GetCompBassGain)GetProcAddress(m_hAct, "_Savitech_GetCompBassGain@4");
        pSetCompBassCenterFrequency = (Savitech_SetCompBassCenterFrequency)GetProcAddress(m_hAct, "_Savitech_SetCompBassCenterFrequency@8");
        pGetCompBassCenterFrequency = (Savitech_GetCompBassCenterFrequency)GetProcAddress(m_hAct, "_Savitech_GetCompBassCenterFrequency@4");

        pInitialDependencyResource = (Savitech_InitialDependencyResource)GetProcAddress(m_hAct, "_Savitech_InitialDependencyResource@0");
        pIsInitialDependencyResource = (Savitech_IsInitialDependencyResource)GetProcAddress(m_hAct, "_Savitech_IsInitialDependencyResource@4");
        pReleaseDependencyResource = (Savitech_ReleaseDependencyResource)GetProcAddress(m_hAct, "_Savitech_ReleaseDependencyResource@0");
        /*pSetVoiceClarityLevel = (Savitech_SetVoiceClarityLevel)GetProcAddress(m_hAct, "_Savitech_SetVoiceClarityLevel@8");
        pGetVoiceClarityLevel = (Savitech_GetVoiceClarityLevel)GetProcAddress(m_hAct, "_Savitech_GetVoiceClarityLevel@4");
        pSetUserDrcCompressor = (Savitech_SetUserDrcCompressor)GetProcAddress(m_hAct, "_Savitech_SetUserDrcCompressor@4");
        pGetUserDrcCompressor = (Savitech_GetUserDrcCompressor)GetProcAddress(m_hAct, "_Savitech_GetUserDrcCompressor@4");*/

        pSetDistance = (Savitech_SetDistance)GetProcAddress(m_hAct, "_Savitech_SetDistance@8");
        pGetDistance = (Savitech_GetDistance)GetProcAddress(m_hAct, "_Savitech_GetDistance@4");

        pSetGlobalInputRatio = (Savitech_SetGlobalInputRatio)GetProcAddress(m_hAct, "_Savitech_SetGlobalInputRatio@8");
        pGetGlobalInputRatio = (Savitech_GetGlobalInputRatio)GetProcAddress(m_hAct, "_Savitech_GetGlobalInputRatio@4");

        //1.0.11.0APO新增
        p_GetDrcThreshold = (Savitech_GetDrcThreshold)GetProcAddress(m_hAct, "_Savitech_GetDrcThreshold@4");
        p_SetDrcThreshold = (Savitech_SetDrcThreshold)GetProcAddress(m_hAct, "_Savitech_SetDrcThreshold@8");
        p_GetDrcRatio = (Savitech_GetDrcRatio)GetProcAddress(m_hAct, "_Savitech_GetDrcRatio@4");
        p_SetDrcRatio = (Savitech_SetDrcRatio)GetProcAddress(m_hAct, "_Savitech_SetDrcRatio@8");
        p_GetDrcAttackTime = (Savitech_GetDrcAttackTime)GetProcAddress(m_hAct, "_Savitech_GetDrcAttackTime@4");
        p_SetDrcAttackTime = (Savitech_SetDrcAttackTime)GetProcAddress(m_hAct, "_Savitech_SetDrcAttackTime@8");
        p_GetDrcReleaseTime = (Savitech_GetDrcReleaseTime)GetProcAddress(m_hAct, "_Savitech_GetDrcReleaseTime@4");
        p_SetDrcReleaseTime = (Savitech_SetDrcReleaseTime)GetProcAddress(m_hAct, "_Savitech_SetDrcReleaseTime@8");
        p_GetDrcMakeupEnable = (Savitech_GetDrcMakeupEnable)GetProcAddress(m_hAct, "_Savitech_GetDrcMakeupEnable@4");
        p_SetDrcMakeupEnable = (Savitech_SetDrcMakeupEnable)GetProcAddress(m_hAct, "_Savitech_SetDrcMakeupEnable@4");
        p_GetDrcInputGain = (Savitech_GetDrcInputGain)GetProcAddress(m_hAct, "_Savitech_GetDrcInputGain@4");
        p_SetDrcInputGain = (Savitech_SetDrcInputGain)GetProcAddress(m_hAct, "_Savitech_SetDrcInputGain@8");
        p_GetDrcOutputGain = (Savitech_GetDrcOutputGain)GetProcAddress(m_hAct, "_Savitech_GetDrcOutputGain@4");
        p_SetDrcOutputGain = (Savitech_SetDrcOutputGain)GetProcAddress(m_hAct, "_Savitech_SetDrcOutputGain@8");
        p_GetDrcLimiterEnable = (Savitech_GetDrcLimiterEnable)GetProcAddress(m_hAct, "_Savitech_GetDrcLimiterEnable@4");
        p_SetDrcLimiterEnable = (Savitech_SetDrcLimiterEnable)GetProcAddress(m_hAct, "_Savitech_SetDrcLimiterEnable@4");
        p_GetDrcLimiterThreshold = (Savitech_GetDrcLimiterThreshold)GetProcAddress(m_hAct, "_Savitech_GetDrcLimiterThreshold@4");
        p_SetDrcLimiterThreshold = (Savitech_SetDrcLimiterThreshold)GetProcAddress(m_hAct, "_Savitech_SetDrcLimiterThreshold@8");
        p_IsSupportExtendEqualizerEffect = (Savitech_IsSupportExtendEqualizerEffect)GetProcAddress(m_hAct, "_Savitech_IsSupportExtendEqualizerEffect@4");
        p_GetExtendEqualizerBandCount = (Savitech_GetExtendEqualizerBandCount)GetProcAddress(m_hAct, "_Savitech_GetExtendEqualizerBandCount@0");
        p_ResetExtendEqualizerSetting = (Savitech_ResetExtendEqualizerSetting)GetProcAddress(m_hAct, "_Savitech_ResetExtendEqualizerSetting@4");
        p_SetExtendEqualizerGain = (Savitech_SetExtendEqualizerGain)GetProcAddress(m_hAct, "_Savitech_SetExtendEqualizerGain@16");
        p_GetExtendEqualizerGain = (Savitech_GetExtendEqualizerGain)GetProcAddress(m_hAct, "_Savitech_GetExtendEqualizerGain@8");
        p_SetExtendEqualizerGainEx = (Savitech_SetExtendEqualizerGainEx)GetProcAddress(m_hAct, "_Savitech_SetExtendEqualizerGainEx@12");
        p_GetExtendEqualizerGainEx = (Savitech_GetExtendEqualizerGainEx)GetProcAddress(m_hAct, "_Savitech_GetExtendEqualizerGainEx@12");
        p_SetExtendEqualizerCenterFrequency = (Savitech_SetExtendEqualizerCenterFrequency)GetProcAddress(m_hAct, "_Savitech_SetExtendEqualizerCenterFrequency@16");
        p_GetExtendEqualizerCenterFrequency = (Savitech_GetExtendEqualizerCenterFrequency)GetProcAddress(m_hAct, "_Savitech_GetExtendEqualizerCenterFrequency@8");
        p_SetExtendEqualizerCenterFrequencyEx = (Savitech_SetExtendEqualizerCenterFrequencyEx)GetProcAddress(m_hAct, "_Savitech_SetExtendEqualizerCenterFrequencyEx@12");
        p_GetExtendEqualizerCenterFrequencyEx = (Savitech_GetExtendEqualizerCenterFrequencyEx)GetProcAddress(m_hAct, "_Savitech_GetExtendEqualizerCenterFrequencyEx@12");
        p_SetExtendEqualizerBandEnable = (Savitech_SetExtendEqualizerBandEnable)GetProcAddress(m_hAct, "_Savitech_SetExtendEqualizerBandEnable@12");
        p_GetExtendEqualizerBandEnable = (Savitech_GetExtendEqualizerBandEnable)GetProcAddress(m_hAct, "_Savitech_GetExtendEqualizerBandEnable@8");
        p_SetExtendEqualizerBandEnableEx = (Savitech_SetExtendEqualizerBandEnableEx)GetProcAddress(m_hAct, "_Savitech_SetExtendEqualizerBandEnableEx@12");
        p_GetExtendEqualizerBandEnableEx = (Savitech_GetExtendEqualizerBandEnableEx)GetProcAddress(m_hAct, "_Savitech_GetExtendEqualizerBandEnableEx@12");
        p_SetExtendEqualizerBandQuality = (Savitech_SetExtendEqualizerBandQuality)GetProcAddress(m_hAct, "_Savitech_SetExtendEqualizerBandQuality@16");
        p_GetExtendEqualizerBandQuality = (Savitech_GetExtendEqualizerBandQuality)GetProcAddress(m_hAct, "_Savitech_GetExtendEqualizerBandQuality@8");
        p_SetExtendEqualizerBandQualityEx = (Savitech_SetExtendEqualizerBandQualityEx)GetProcAddress(m_hAct, "_Savitech_SetExtendEqualizerBandQualityEx@12");
        p_GetExtendEqualizerBandQualityEx = (Savitech_GetExtendEqualizerBandQualityEx)GetProcAddress(m_hAct, "_Savitech_GetExtendEqualizerBandQualityEx@12");
        p_SetExtendEqualizerBandFilter = (Savitech_SetExtendEqualizerBandFilter)GetProcAddress(m_hAct, "_Savitech_SetExtendEqualizerBandFilter@12");
        p_GetExtendEqualizerBandFilter = (Savitech_GetExtendEqualizerBandFilter)GetProcAddress(m_hAct, "_Savitech_GetExtendEqualizerBandFilter@8");
        p_SetExtendEqualizerBandFilterEx = (Savitech_SetExtendEqualizerBandFilterEx)GetProcAddress(m_hAct, "_Savitech_SetExtendEqualizerBandFilterEx@12");
        p_GetExtendEqualizerBandFilterEx = (Savitech_GetExtendEqualizerBandFilterEx)GetProcAddress(m_hAct, "_Savitech_GetExtendEqualizerBandFilterEx@12");

        if (pIsLhdcDeviceSupport == NULL) {
            DWORD error = GetLastError();
            qDebug() << "GetProcAddress failed, error code:" << error;
            FreeLibrary(m_hAct);
            m_hAct = NULL;
            return;
        }
    }


    //上行库
    g_hSaviCTL = LoadLibrary(szModule_Down);
    if(g_hSaviCTL == NULL)
    {
        QMessageBox::critical(nullptr, QObject::tr("错误"), QObject::tr("加载上行库失败，缺少插件，请重新安装驱动"));
        qDebug("LoadLibrary failed:%d\n",GetLastError());
        return;
    }else
    {
        qDebug("APO LoadLibrary success\n");
        g_pSaviInit = (pSAVIINIT)GetProcAddress(g_hSaviCTL, "SaviInit");
        g_pSaviDeinit = (pSAVIDEINIT)GetProcAddress(g_hSaviCTL, "SaviDeinit");
        g_pSaviIsSupportEP = (pSAVIISSUPPORTEP)GetProcAddress(g_hSaviCTL, "SaviIsSupportEP");
        g_pSaviGetNoiseGateEnable = (pSAVIGETNOISEGATEENABLE)GetProcAddress(g_hSaviCTL, "SaviGetNoiseGateEnable");
        g_pSaviSetNoiseGateEnable = (pSAVISETNOISEGATEENABLE)GetProcAddress(g_hSaviCTL, "SaviSetNoiseGateEnable");
        g_pSaviGetNoiseGateLevel = (pSAVIGETNOISEGATELEVEL)GetProcAddress(g_hSaviCTL, "SaviGetNoiseGateLevel");
        g_pSaviSetNoiseGateLevel = (pSAVISETNOISEGATELEVEL)GetProcAddress(g_hSaviCTL, "SaviSetNoiseGateLevel");
        g_pSaviGetAINSEnable = (pSAVIGETAINSENABLE)GetProcAddress(g_hSaviCTL, "SaviGetAINSEnable");
        g_pSaviSetAINSEnable = (pSAVISETAINSENABLE)GetProcAddress(g_hSaviCTL, "SaviSetAINSEnable");
        g_pSaviGetAINSLevel = (pSAVIGETAINSLEVEL)GetProcAddress(g_hSaviCTL, "SaviGetAINSLevel");
        g_pSaviSetAINSLevel = (pSAVISETAINSLEVEL)GetProcAddress(g_hSaviCTL, "SaviSetAINSLevel");
        g_pSaviGetVocalEffectsEnable = (pSAVIGETVOCALEFFECTSENABLE)GetProcAddress(g_hSaviCTL, "SaviGetVocalEffectsEnable");
        g_pSaviSetVocalEffectsEnable = (pSAVISETVOCALEFFECTSENABLE)GetProcAddress(g_hSaviCTL, "SaviSetVocalEffectsEnable");
        g_pSaviGetRichVocalsEnable = (pSAVIGETRICHVOCALSENABLE)GetProcAddress(g_hSaviCTL, "SaviGetRichVocalsEnable");
        g_pSaviSetRichVocalsEnable = (pSAVISETRICHVOCALSENABLE)GetProcAddress(g_hSaviCTL, "SaviSetRichVocalsEnable");
        g_pSavi_func2_get_ep_registry_API = (pSAVI_FUNC2_GET_EP_REGISTRY_API)GetProcAddress(g_hSaviCTL, "Savi_func2_get_ep_registry_API");
        g_pSavi_func2_set_ep_registry_API = (pSAVI_FUNC2_SET_EP_REGISTRY_API)GetProcAddress(g_hSaviCTL, "Savi_func2_set_ep_registry_API");


    }

}


//初始化资源
int LoadApoDLL::InitialDependencyResource()
{
    try {
        m_errorCode = pInitialDependencyResource();
    } catch (...) {
        logWithTime("Exception in InitialDependencyResource");
    }

    logWithTime(QString("111APO DLL InitialDependencyResource returnCode:%1").arg(m_errorCode));

    return m_errorCode;
}
//初始化是否成功
bool LoadApoDLL::IsInitialDependencyResource()
{
    int isInit = 0;


    try {
        m_errorCode = pIsInitialDependencyResource(&isInit);
    } catch (...) {
        logWithTime("Exception in IsInitialDependencyResource");
    }

    //qDebug("[LHDCX_TECSUN_DLL][Savitech_IsInitialDependencyResource] isInit:{isInit}, return:{m_errorCode}");
    return (0 != isInit);
}
//释放资源
int LoadApoDLL::ReleaseDependencyResource()
{
    try {
       m_errorCode = pReleaseDependencyResource();
    } catch (...) {
        logWithTime("Exception in ReleaseDependencyResource");
    }

    return m_errorCode;
}
//检测枚举的耳机设备是否为APO支持的设备
bool LoadApoDLL::IsLhdcDeviceSupport(const QString& deviceGUID, QString& vId_pId, int& isSupported)
{
    // int isSupported = 0, npType = 0;

    // 输出缓冲区：vId_pId 最大长度
    const int BUFFER_SIZE = 250;//250
    char vidPidBuffer[BUFFER_SIZE+1] = {0};

    QByteArray utf8Data = deviceGUID.toUtf8();
    const char* cstr = utf8Data.constData();  // 安全！生命周期由 utf8Data 持有
    qDebug("IsLhdcDeviceSupport GUID: %s", cstr);
    //std::wstring guidW = deviceGUID.toStdWString();
    //qDebug("GUID: %ls", guidW.c_str());
    //m_errorCode = pIsLhdcDeviceSupport(guidW.c_str(), &isSupported, &npType, vidPidBuffer, BUFFER_SIZE);

    // m_errorCode = pIsLhdcDeviceSupport(cstr, &isSupported, &npType, vidPidBuffer, BUFFER_SIZE);


    try {
        m_errorCode = pIsLhdcDeviceSupport(cstr, &isSupported, vidPidBuffer, BUFFER_SIZE+1);
    } catch (...) {
        logWithTime("Exception in IsLhdcDeviceSupport");
    }

    if (m_errorCode == 1)
    {
        qDebug("pIsLhdcDeviceSupport返回1");
        // 将输出字符串转为 QString
        //vId_pId = QString::fromWCharArray(vidPidBuffer);
        vId_pId = QString::fromUtf8(vidPidBuffer);

        // 如果支持，去掉 "USB\\" 前缀
        if (isSupported != 0) {
            vId_pId = vId_pId.remove("USB\\", Qt::CaseInsensitive);
        }
    }
    return (m_errorCode);
}

// //得到当前设备APO的数量
// int LoadApoDLL::GetDeviceApoCount(const QString& pszDeviceId, uint eFlow, uint& pCount)
// {
//     pCount = 0;
//     try {
//         // // 将 QString 转为 UTF-8字符串
//         // QByteArray deviceIdUtf8 = pszDeviceId.toUtf8();
//         // const char* cstr = deviceIdUtf8.constData();
//         // qDebug("GUID: %s", cstr);
//         // m_errorCode = pGetDeviceApoCount(deviceIdUtf8, eFlow, &pCount);


//         std::wstring guidW = pszDeviceId.toStdWString();
//         qDebug("GetDeviceApoCount GUID: %ls", guidW.c_str());
//         m_errorCode = pGetDeviceApoCount(guidW.c_str(), eFlow, &pCount);

//     } catch (const std::exception& ex) {
//         m_errorCode = -1;
//         qDebug() << "GetDeviceApoCount exception:" << ex.what();
//     } catch (...) {
//         m_errorCode = -1;
//         qDebug() << "GetDeviceApoCount unknown exception";
//     }
//     return m_errorCode;
// }
// //得到当前设备的APO信息
// int LoadApoDLL::GetDeviceApoInfo(uint eFlow, uint uIndex, QString& pszFriendlyName, QString& pszCopyrightInfo)
// {
//     pszFriendlyName.clear();
//     pszCopyrightInfo.clear();

//     const int BUF_LEN = 512;
//     // // 使用 QByteArray 作为 char 缓冲区，自动管理内存
//     // QByteArray friendly(BUF_LEN, '\0');
//     // QByteArray copyright(BUF_LEN, '\0');
//     // 使用 std::wstring 作为宽字符缓冲区
//     std::wstring friendly(BUF_LEN, L'\0');
//     std::wstring copyright(BUF_LEN, L'\0');

//     try {
//         m_errorCode = pGetDeviceApoInfo(
//             eFlow,
//             uIndex,
//             friendly.data(), static_cast<unsigned int>(friendly.size()),
//             copyright.data(), static_cast<unsigned int>(copyright.size())
//             );

//         if (m_errorCode == 1) {   //成功返回 1
//             // pszFriendlyName = QString::fromUtf8(friendly.constData());
//             // pszCopyrightInfo = QString::fromUtf8(copyright.constData());
//             // 从宽字符串转为 QString
//             pszFriendlyName = QString::fromStdWString(friendly.c_str());
//             pszCopyrightInfo = QString::fromStdWString(copyright.c_str());
//         }
//     } catch (const std::exception& ex) {
//         m_errorCode = -1;
//         qDebug() << "GetDeviceApoInfo exception:" << ex.what();
//     } catch (...) {
//         m_errorCode = -1;
//         qDebug() << "GetDeviceApoInfo unknown exception";
//     }

//     return m_errorCode;
// }

//设置APO优先级
int LoadApoDLL::SetDeviceApoClsid(const QString& pszDeviceId, uint eFlow)
{
    try {
        std::wstring guidW = pszDeviceId.toStdWString();
        qDebug("GetDeviceApoCount GUID: %ls", guidW.c_str());
        m_errorCode = pSetDeviceApoClsid(eFlow, guidW.c_str());
    } catch (const std::exception& ex) {
        m_errorCode = -1;
        qDebug() << "SetDeviceApoClsid exception:" << ex.what();
    } catch (...) {
        m_errorCode = -1;
        qDebug() << "SetDeviceApoClsid unknown exception";
    }
    return m_errorCode;
}


//设置APO作用于那个设备
int LoadApoDLL::SetLhdcDevice(const QString& deviceGUID)
{
    QByteArray utf8Data = deviceGUID.toUtf8();
    const char* cstr = utf8Data.constData();
    qDebug("SetLhdcDevice GUID: %s", cstr);

    try {
        m_errorCode = pSetLhdcDevice(cstr);//1代表成功
    } catch (...) {
        logWithTime("Exception in SetLhdcDevice");
    }
    if(m_errorCode == 1)
    {
        IsSetLdcDev = true;
    }else
    {
        IsSetLdcDev = false;
    }
    logWithTime(QString("111APO DLL SetLhdcDevice m_errorCode: %1").arg(m_errorCode));
    return m_errorCode;
}
unsigned int LoadApoDLL::GetProcessEffectOption()
{
    unsigned int rtnOption = 0;

    try {
        m_errorCode = pGetProcessEffectOption(&rtnOption);
    } catch (...) {
        logWithTime("Exception in GetProcessEffectOption");
    }

    logWithTime(QString("111APO DLL GetProcessEffectOption value:%1 returnCode:%2").arg(QString::number(rtnOption, 16)).arg(m_errorCode));

    return rtnOption;
}
unsigned int LoadApoDLL::SetProcessEffectOption(unsigned int option)
{
    logWithTime(QString("111APO DLL SetProcessEffectOption value: %1").arg(QString::number(option, 16)));

    try {
        m_errorCode = pSetProcessEffectOption(option);
    } catch (...) {
        logWithTime("Exception in SetProcessEffectOption");
    }
    logWithTime(QString("111APO DLL SetProcessEffectOption value:%1 returnCode:%2").arg(QString::number(option, 16)).arg(m_errorCode));
    return GetProcessEffectOption();
}
//获得空间环绕使能
bool LoadApoDLL::GetSurroundState()
{
    if(IsSetLdcDev)
    {
        unsigned int currentOption = GetProcessEffectOption();

        // 检查是否设置了 OptionEnableDownmixEffect 标志位
        bool isEnabled = (currentOption & static_cast<unsigned int>(EffectProcessOption::OptionEnableDownmixEffect)) != 0;

        qDebug() << "[GetSurroundState]" << (isEnabled ? "Enabled" : "Disabled");
        return isEnabled;
    }else
    {
        return 0;
    }
}
//设置空间环绕使能
int LoadApoDLL::SetSurroundState(bool enable)
{
    if(IsSetLdcDev)
    {
        logWithTime(QString("111APO DLL SetSurroundState enable:%1").arg(enable));
        unsigned int curOption = GetProcessEffectOption();

        if (enable)
        {
            // 启用三项：Downmix、StereoToMultiple、ArEffect
            curOption |= static_cast<unsigned int>(EffectProcessOption::OptionEnableDownmixEffect);
            curOption |= static_cast<unsigned int>(EffectProcessOption::OptionEnableStereoToMultiple);
            // curOption |= static_cast<unsigned int>(EffectProcessOption::OptionEnableArEffect);
        }
        else
        {
            // 仅关闭 Downmix 和 StereoToMultiple（保留 ArEffect）
            curOption &= ~static_cast<unsigned int>(EffectProcessOption::OptionEnableDownmixEffect);
            // curOption &= ~static_cast<unsigned int>(EffectProcessOption::OptionEnableStereoToMultiple);
            // 注意：ArEffect 不受影响
        }

        // 应用新的配置
        m_errorCode = SetProcessEffectOption(curOption);

        // 日志输出
        qDebug().noquote() << QString("[SetSurroundState] enable=%1 -> New Option: 0x%2, ErrorCode: %3")
                                  .arg(enable ? "true" : "false")
                                  .arg(curOption, 0, 16)
                                  .arg(m_errorCode);

        return m_errorCode;
    }else
    {
        return 0;
    }
}
//获得空间环绕强度
int LoadApoDLL::GetDistance()
{
    if(IsSetLdcDev)
    {
        // double minValue = 0.1, maxValue = 2.0; // meter
        // int minScale = 0, maxScale = 7;
        // const int scaleRange = maxScale - minScale;

        double distance = 0.0;

        try {
            pGetDistance(&distance);
        } catch (...) {
            logWithTime("Exception in GetDistance");
        }

        // // Clamp 实际读取的距离到有效范围 [minValue, maxValue]
        // distance = qBound(minValue, distance, maxValue);

        // // 正向归一化后反向映射到 scale：距离越大，scale 越小
        // double normalized = (distance - minValue) / (maxValue - minValue); // [0,1]
        // double scaleValue = maxScale - normalized * scaleRange;            // [7,0] → [0,7] 反向

        // // 四舍五入并裁剪确保在合法整数范围
        // int scale = static_cast<int>(qRound(scaleValue));
        // scale = qBound(minScale, scale, maxScale);   // 再次保险性 clamp
        // return scale;
        return distance;
    }else
    {
        return 0;
    }
}
//设置空间环绕强度
int LoadApoDLL::SetDistance(int scale)
{
    if(IsSetLdcDev)
    {
        // double minValue = 0.1, maxValue = 2.0; // meter(其中1.2meter为正常音量)
        // int minScale = 0, maxScale = 7;

        // // clamp: 将 scale 限制在 [minScale, maxScale]
        // scale = qBound(minScale, scale, maxScale);

        // // 反向线性映射：scale 越大 -> distance 越小
        // double distance = minValue + ((maxScale - scale) / static_cast<double>(maxScale - minScale)) * (maxValue - minValue);


        try {
            pSetDistance(scale);
        } catch (...) {
            logWithTime("Exception in SetDistance");
        }

        logWithTime(QString("111APO DLL SetDistance value:%1").arg(scale));

        return GetDistance();
    }else
    {
        return 0;
    }
}


//空间
//设置房间类型
ReverbRoomType LoadApoDLL::SetReverbFilter(ReverbRoomType nRoomIndex)
{
    return (ReverbRoomType)SetReverbActivateRoomType((int)nRoomIndex);
}
ReverbRoomType LoadApoDLL::GetReverbFilter()
{
    return (ReverbRoomType)GetReverbActivateRoomType();
}
int LoadApoDLL::SetReverbActivateRoomType(int nRoomType)
{

    try {
        m_errorCode = pSetReverbActivateRoomType(nRoomType);
    } catch (...) {
        logWithTime("Exception in SetReverbActivateRoomType");
    }
    logWithTime(QString("111APO DLL ReverbCode SetReverbActivateRoomType nRoomType: %1 returnCode:%2").arg(nRoomType).arg(m_errorCode));

    return GetReverbActivateRoomType();
}
int LoadApoDLL::GetReverbActivateRoomType()
{
    int roomType = 0;

    try {
        m_errorCode = pGetReverbActivateRoomType(&roomType);
    } catch (...) {
        logWithTime("Exception in GetReverbActivateRoomType");
    }
    logWithTime(QString("111APO DLL ReverbCode GetReverbActivateRoomType roomType: %1 returnCode:%2").arg(roomType).arg(m_errorCode));

    return roomType;
}
//比例
double LoadApoDLL::SetArReverbRatio(double dbRatioReverb)
{

    try {
       m_errorCode = pSetArReverbRatio(dbRatioReverb*10.0);
    } catch (...) {
        logWithTime("Exception in SetArReverbRatio");
    }
    logWithTime(QString("111APO DLL ReverbCode SetArReverbRatio dbRatioReverb: %1 returnCode:%2").arg(dbRatioReverb).arg(m_errorCode));

    return GetArReverbRatio();

}
double LoadApoDLL::GetArReverbRatio()
{
    double rtnReverbRatio[1] = {0.0};


    try {
        m_errorCode = pGetArReverbRatio(rtnReverbRatio);
    } catch (...) {
        logWithTime("Exception in GetArReverbRatio");
    }

    logWithTime(QString("111APO DLL ReverbCode GetArReverbRatio returnCode:%1").arg(m_errorCode));

    return rtnReverbRatio[0];
}
//设置Reverb的使能
int LoadApoDLL::SetReverbState(bool enable)
{
    logWithTime(QString("111APO DLL ReverbCode SetReverbState enable:%1").arg(enable));
    uint curOption = GetProcessEffectOption();

    // 获取目标标志位
    const uint32_t flag = static_cast<uint32_t>(EffectProcessOption::OptionEnableReverbEffect);
    // const uint32_t flag2 = static_cast<uint32_t>(EffectProcessOption::OptionEnableArEffect);

    if (enable)
    {
        curOption |=  flag;
        // curOption |=  flag2;
    }
    else
    {
        curOption &= ~flag;
    }

    logWithTime(QString("111APO DLL ReverbCode SetReverbState curOption:%1").arg(curOption));

    SetProcessEffectOption(curOption);

    return m_errorCode;
}
//获得混响使能
bool LoadApoDLL::GetReverbState()
{
    if(IsSetLdcDev)
    {
        unsigned int reverbOption = GetProcessEffectOption() & static_cast<unsigned int>(EffectProcessOption::OptionEnableReverbEffect);

        bool isEnabled = (reverbOption == static_cast<unsigned int>(EffectProcessOption::OptionEnableReverbEffect));

        qDebug() << "[GetReverbState]" << (isEnabled ? "Enabled" : "Disabled");

        return isEnabled;
    }else
    {
        return 0;
    }
}


int LoadApoDLL::GetCompBassGain()
{
    // double value = 0.0; // 初始化输出值
    int value = 0; // 初始化输出值


    try {
        // 调用底层函数（传入地址）
        m_errorCode = pGetCompBassGain(&value);
    } catch (...) {
        logWithTime("Exception in GetCompBassGain");
    }

    logWithTime(QString("111APO DLL int GetCompBassGain value:%1 returnCode:%2").arg(value).arg(m_errorCode));



    // 输出调试日志（等价于 C# 的 Console.WriteLine）
    // qDebug().noquote() << QString("[LHDCX_TECSUN_DLL][Savitech_GetCompBassGain] dbGainScalar:%1, return:%2")
    //                           //.arg(value, 0, 'g', 6)  // 通用格式，保留6位有效数字
    //                           .arg(value)
    //                           .arg(m_errorCode);

    return value;
}
int LoadApoDLL::SetCompBassGain(int value)
{

    logWithTime("111APO DLL int SetCompBassGain");

    try {
        // 调用底层函数设置增益
        m_errorCode = pSetCompBassGain(value);
    } catch (...) {
        logWithTime("Exception in SetCompBassGain");
    }

    logWithTime(QString("111APO DLL int SetCompBassGain value: %1 returnCode:%2").arg(value).arg(m_errorCode));


    // 写后读：获取设备当前真实值以确认设置生效
    return GetCompBassGain();
}

//获得低音增强增益值（0-100）
int LoadApoDLL::GetBassBoostGain()
{
    if(IsSetLdcDev)
    {
        int d = GetCompBassGain();
        return d;
    }else
    {
        return 0;
    }
}
//设置低音增强增益值（0-100）
int LoadApoDLL::SetBassBoostGain(int value)
{

    if(IsSetLdcDev)
    {
        logWithTime(QString("111APO DLL int SetBassBoostGain IsSetLdcDev true value: %1").arg(value*10));
        SetCompBassGain(value*10);
        return m_errorCode;
    }else
    {
        logWithTime("111APO DLL int SetBassBoostGain IsSetLdcDev false");
        return 0;
    }
}
//获得低音增强使能
bool LoadApoDLL::GetBassBoostState()
{
    if(IsSetLdcDev)
    {
        // 获取当前所有效果选项的位掩码
        uint32_t currentOptions = GetProcessEffectOption();

        // 提取“OptionEnableCompBass”位
        const uint32_t bassBoostBit = static_cast<uint32_t>(EffectProcessOption::OptionEnableCompBass);

        // 检查该位是否被置起（使用按位与）
        return (currentOptions & bassBoostBit) == bassBoostBit;
    }else
    {
        return 0;
    }
}
//设置低音增强使能
int LoadApoDLL::SetBassBoostState(bool enable)
{
    if(IsSetLdcDev)
    {
        logWithTime(QString("111APO DLL SetBassBoostState enable:%1").arg(enable));
        // 获取当前所有效果的状态（位掩码）
        uint32_t curOption = GetProcessEffectOption();

        // 获取目标标志位
        const uint32_t bassBoostFlag = static_cast<uint32_t>(EffectProcessOption::OptionEnableCompBass);
        // const uint32_t bassBoostFlag = static_cast<uint32_t>(EffectProcessOption::OptionEnableCompBass) | static_cast<uint32_t>(EffectProcessOption::OptionEnableArEffect);

        if (enable)
        {
            // 启用：使用按位或设置标志位
            curOption |= bassBoostFlag;
        }
        else
        {
            // 禁用：使用按位与 + 取反清除标志位
            curOption &= ~bassBoostFlag;
        }

        // 将修改后的位掩码写回系统
        SetProcessEffectOption(curOption);

        // 返回错误码（假定 SetProcessEffectOption 内部会更新 m_errorCode）
        return m_errorCode;
    }else
    {
        return 0;
    }
}
//获得低音中心频率
int LoadApoDLL::GetBassBoostCenterFrequency()
{
    return GetCompBassCenterFrequency();
}


int LoadApoDLL::GetCompBassCenterFrequency()
{
    double value = 0.0;
    try {
        m_errorCode = pGetCompBassCenterFrequency(&value);
    } catch (...) {
        logWithTime("Exception in GetCompBassCenterFrequency");
    }
    int ret = qRound(value); // 或 static_cast<int>(std::round(value))
    logWithTime(QString("GetCompBassCenterFrequency centerFrequency:%1, return:%2")
                    .arg(ret).arg(m_errorCode));
    return ret;
}
//设置低音中心频率
int LoadApoDLL::SetCompBassCenterFrequency(double value)
{
    try {
        m_errorCode = pSetCompBassCenterFrequency(value);
    } catch (...) {
        logWithTime("Exception in SetCompBassCenterFrequency");
    }

    logWithTime(QString("SetCompBassCenterFrequency centerFrequency:%1, return:%2")
                              .arg(value).arg(m_errorCode));
    return GetCompBassCenterFrequency();
}


/*//获得声音清晰增益值
int LoadApoDLL::GetVoiceClarityGain()
{
    if(IsSetLdcDev)
    {
        double value = 0;


        try {
            m_errorCode = pGetVoiceClarityLevel(&value);
        } catch (...) {
            logWithTime("Exception in GetVoiceClarityGain");
        }

        if (m_errorCode != 0) {
            return m_errorCode; // 表示获取失败
        }

        // 对返回的浮点值进行四舍五入 -> 等价于 Math.Round(value)
        return static_cast<int>(std::round(value));
    }else
    {
        return 0;
    }
}
//设置声音清晰增益值
int LoadApoDLL::SetVoiceClarityGain(int value)
{
    if(IsSetLdcDev)
    {

        try {
            m_errorCode = pSetVoiceClarityLevel(value);
        } catch (...) {
            logWithTime("Exception in SetVoiceClarityGain");
        }
        return m_errorCode;
    }else
    {
        return 0;
    }
}
//获得声音清晰使能
bool LoadApoDLL::GetVoiceClarityState()
{
    if(IsSetLdcDev)
    {
        // 获取当前所有音效选项的位掩码
        uint32_t currentOptions = GetProcessEffectOption();

        // 提取“语音清晰度”标志位
        const uint32_t flag = static_cast<uint32_t>(EffectProcessOption::OptionEnableVoiceClarity);

        // 判断该位是否被设置（使用按位与）
        return (currentOptions & flag) == flag;
        // 等价写法：return (currentOptions & flag) != 0;
    }else
    {
        return 0;
    }
}
//设置声音清晰使能
int LoadApoDLL::SetVoiceClarityState(bool enable)
{
    if(IsSetLdcDev)
    {
        // 获取当前所有音效的状态（位掩码）
        uint32_t curOption = GetProcessEffectOption();

        // 定义目标标志位
        const uint32_t flag = static_cast<uint32_t>(EffectProcessOption::OptionEnableVoiceClarity);

        if (enable)
        {
            // 启用：使用按位或设置标志位
            curOption |= flag;
        }
        else
        {
            // 禁用：使用按位与 + 取反清除标志位
            curOption &= ~flag;
        }

        // 将更新后的位掩码写入系统
        SetProcessEffectOption(curOption);

        // 返回错误码（假设此变量在 SetProcessEffectOption 调用后被更新）
        return m_errorCode;
    }else
    {
        return 0;
    }
}

//设置均衡器的频点（可设置20个,上位机为20Hz~20KHz，接口为20Hz~无穷限）
// int LoadApoDLL::SetEqualizerCenterFrequencyEx(double* EqBandFrequencyList)
// {
//     // 注意：QVector 的数据在内存中是连续的，data() 返回 double* 可直接传递
//     m_errorCode = pSetEqualizerCenterFrequencyEx(EqBandFrequencyList);
//     return m_errorCode;
// }
int LoadApoDLL::SetEqualizerCenterFrequencyEx(const QVector<double>& EqBandFrequencyList)
{
    if(IsSetLdcDev)
    {
        if (EqBandFrequencyList.isEmpty()) {
            m_errorCode = -1; // 自定义错误：空数组
            logWithTime(QString("111APO DLL EqBandFrequencyList returnCode: %1").arg(m_errorCode));
            return m_errorCode;
        }


        // QString vectorStr;
        // for (int i = 0; i < EqBandFrequencyList.size(); ++i) {
        //     if (i > 0) vectorStr += ", ";
        //     vectorStr += QString::number(EqBandFrequencyList[i]);
        // }
        // logWithTime(QString("111APO DLL SetEqualizerCenterFrequencyEx value: [%1]").arg(vectorStr));


        try {
            // 注意：QVector 的数据在内存中是连续的，data() 返回 double* 可直接传递
            m_errorCode = pSetEqualizerCenterFrequencyEx(const_cast<double*>(EqBandFrequencyList.data()),EqBandFrequencyList.length());

        } catch (...) {
            logWithTime("Exception in SetEqualizerCenterFrequencyEx");
        }


        logWithTime(QString("111APO DLL SetEqualizerCenterFrequencyEx returnCode: %1").arg(m_errorCode));

        return m_errorCode;
    }else
    {
        return 0;
    }
}
//获得均衡器的频点（上位机为20Hz~20KHz，接口为20Hz~无穷限）
QVector<double> LoadApoDLL::GetEqualizerCenterFrequencyEx()
{
    if(IsSetLdcDev)
    {
        QVector<double> centerFreq(EqualizerBandArraySize);
        centerFreq[0] = 111;

        try {
            // 调用 DLL 函数
            m_errorCode = pGetEqualizerCenterFrequencyEx(centerFreq.data(),centerFreq.length());
        } catch (...) {
            logWithTime("Exception in GetEqualizerCenterFrequencyEx");
        }



        // 构建调试信息字符串
        QStringList freqListStr;
        for (double freq : centerFreq) {
            freqListStr.append(QString::number(freq, 'f', 1)); // 保留1位小数
        }

        // qDebug() << "[LHDCX_TECSUN_DLL][Savitech_GetEqualizerCenterFrequencyEx] CenterFreq:"
        //          << freqListStr.join(" , ")
        //          << "return:" << m_errorCode;

        return centerFreq;
    }else
    {
        return {0};
    }
}

//设置均衡器的Q值（可设置20个,上位机为0.1~10,接口为0.1~100）
int LoadApoDLL::SetEqualizerBandQualityEx(const QVector<double>& EqBandQualityList)
{
    if(IsSetLdcDev)
    {

        // QString vectorStr;
        // for (int i = 0; i < EqBandQualityList.size(); ++i) {
        //     if (i > 0) vectorStr += ", ";
        //     vectorStr += QString::number(EqBandQualityList[i]);
        // }
        // logWithTime(QString("111APO DLL SetEqualizerBandQualityEx value: [%1]").arg(vectorStr));



        try {
            // 调用 DLL 函数
            m_errorCode = pSetEqualizerBandQualityEx(EqBandQualityList.data(),EqBandQualityList.length());
        } catch (...) {
            logWithTime("Exception in SetEqualizerBandQualityEx");
        }

        logWithTime(QString("111APO DLL SetEqualizerBandQualityEx returnCode: %1").arg(m_errorCode));


        return m_errorCode;
    }else
    {
        return 0;
    }
}

//获得均衡器使能
bool LoadApoDLL::GetEqualizerState()
{
    if(IsSetLdcDev)
    {
        uint equalizerOption = GetProcessEffectOption();
        // 提取“均衡器使能”标志位
        const uint32_t flag = static_cast<uint32_t>(EffectProcessOption::OptionEnableEqualizer);

        // 判断该位是否被设置（使用按位与）
        return (equalizerOption & flag) == flag;
        // 等价写法：return (currentOptions & flag) != 0;
    }else
    {
        return 0;
    }
}
//设置均衡器使能
int LoadApoDLL::SetEqualizerState(bool enable)
{
    if(IsSetLdcDev)
    {
        logWithTime(QString("111APO DLL SetEqualizerState enable:%1").arg(enable));
        uint curOption = GetProcessEffectOption();

        if (enable) {
            // 启用：使用按位或设置标志位
            curOption |= static_cast<uint32_t>(EffectProcessOption::OptionEnableEqualizer);
            // 可扩展其他功能，如 AR 效果（注释中提到）：
            // curOption |= static_cast<uint32_t>(EffectProcessOption::OptionEnableArEffect);
        } else {
            // 禁用：使用按位与和取反清除特定位
            curOption &= ~static_cast<uint32_t>(EffectProcessOption::OptionEnableEqualizer);
        }
        SetProcessEffectOption(curOption);
        return m_errorCode;
    }else
    {
        return 0;
    }
}
//获得所有频段的增益值(可设置20个)
QVector<double> LoadApoDLL::GetEqualizerGainEx()
{
    if(IsSetLdcDev)
    {
        // 初始化大小为20的 QVector，用于接收增益数据（单位：dB）
        QVector<double> rtnListGainInDB(EqualizerBandArraySize);

        try {
            m_errorCode = pGetEqualizerGains(rtnListGainInDB.data(),rtnListGainInDB.length());
        } catch (...) {
            logWithTime("Exception in GetEqualizerGainEx");
        }

        return rtnListGainInDB;
    }else
    {
        return {0};
    }
}
//设置所有频段的增益值
int LoadApoDLL::SetEqualizerGainEx(const QVector<double>& EqGainList)
{
    if(IsSetLdcDev)
    {
        double gainArray[20] = {0};
        for (int i = 0; i < qMin(EqGainList.size(), 20); ++i) {
            gainArray[i] = EqGainList.at(i);
        }


        try {
            m_errorCode = pSetEqualizerGains(gainArray,EqGainList.length());
        } catch (...) {
            logWithTime("Exception in SetEqualizerGainEx");
        }


        // QString vectorStr;
        // for (int i = 0; i < EqGainList.size(); ++i) {
        //     if (i > 0) vectorStr += ", ";
        //     vectorStr += QString::number(EqGainList[i]);
        // }
        // logWithTime(QString("111APO DLL SetEqualizerGainEx value: [%1]").arg(vectorStr));
        logWithTime(QString("111APO DLL SetEqualizerGainEx returnCode: [%1]").arg(m_errorCode));

        // // 构建日志字符串：打印前13个值
        // QStringList values;
        // for (int i = 0; i < 13; ++i) {
        //     values.append(QString::number(gainArray[i], 'f', 2)); // 保留两位小数
        // }

        // QString logMessage = QString("[LHDCX_TECSUN_DLL][Savitech_SetEqualizerGainEx] listGainInDB: %1 , return: %2")
        //                          .arg(values.join(" , "))
        //                          .arg(m_errorCode);

        // qDebug().noquote() << logMessage;

        return m_errorCode;
    }else
    {
        return 0;
    }
}
//获得对应频段的增益值
double LoadApoDLL::GetEqualizerGain(uint32_t index_band)
{
    if(IsSetLdcDev)
    {
        double rtn_gain[1] = {0.0}; // 等价于 C# 中 new double[1]

        try {
            // 调用底层 SDK 函数获取增益值
            m_errorCode = pGetEqualizerGain(index_band, rtn_gain);
        } catch (...) {
            logWithTime("Exception in GetEqualizerGain");
        }
        return rtn_gain[0];
    }else
    {
        return 0;
    }
}

//设置对应频段的增益值
double LoadApoDLL::SetEqualizerGain(uint32_t index_band, double dbValue)
{
    logWithTime("111APO DLL int SetEqualizerGain");
    if(IsSetLdcDev)
    {
        logWithTime("111APO DLL int SetEqualizerGain IsSetLdcDev true");

        try {
            // 调用底层函数设置指定频段的增益
            m_errorCode = pSetEqualizerGain(index_band, dbValue);
        } catch (...) {
            logWithTime("Exception in SetEqualizerGain");
        }

        logWithTime(QString("111APO DLL pSetEqualizerGain index_band:%1 dbValue:%2 returnCode%3").arg(index_band).arg(dbValue, 0, 'f', 2).arg(m_errorCode));


        // 调用 Get 函数获取实际写入的值（用于验证或反馈）
        double actualValue = GetEqualizerGain(index_band);

        logWithTime(QString("111APO DLL GetEqualizerGain actualValue: %1").arg(actualValue));

        return actualValue;
    }else
    {
        logWithTime("111APO DLL int SetEqualizerGain IsSetLdcDev false");
        return 0;
    }
}

int LoadApoDLL::SetEqualizerBandEnableEx(int* EqBandEnableList)
{
    // QString vectorStr;
    // for (int i = 0; i < 20; ++i) {
    //     if (i > 0) vectorStr += ", ";
    //     vectorStr += QString::number(EqBandEnableList[i]);
    // }
    // logWithTime(QString("111APO DLL SetEqualizerBandEnableEx value: [%1]").arg(vectorStr));


    try {
      m_errorCode = pSetEqualizerBandEnableEx(EqBandEnableList,20);
    } catch (...) {
        logWithTime("Exception in SetEqualizerBandEnableEx");
    }

    logWithTime(QString("111APO DLL SetEqualizerBandEnableEx returnCode: [%1]").arg(m_errorCode));
    return m_errorCode;
}

int* LoadApoDLL::GetEqualizerBandEnableEx()
{
    int BandEnable[EqualizerBandArraySize] = {0};

    try {
      m_errorCode = pGetEqualizerBandEnableEx(BandEnable,EqualizerBandArraySize);
    } catch (...) {
        logWithTime("Exception in GetEqualizerBandEnableEx");
    }
    return BandEnable;
}
*/
//均衡器
bool LoadApoDLL::IsSupportExtendEqualizerEffect()
{
    int rtn = 0;
    p_IsSupportExtendEqualizerEffect(&rtn);
    return rtn != 0;
}

unsigned int LoadApoDLL::GetExtendEqualizerBandCount()
{
    return p_GetExtendEqualizerBandCount();
}

int LoadApoDLL::ResetExtendEqualizerSetting(unsigned int index_EQ)
{
    return p_ResetExtendEqualizerSetting(index_EQ);
}

int LoadApoDLL::SetExtendEqualizerGain(unsigned int index_EQ, unsigned int index_band, double dbValue)
{
    return p_SetExtendEqualizerGain(index_EQ, index_band, dbValue);
}

double LoadApoDLL::GetExtendEqualizerGain(unsigned int index_EQ, unsigned int index_band)
{
    return p_GetExtendEqualizerGain(index_EQ, index_band);
}

int LoadApoDLL::SetExtendEqualizerGainEx(unsigned int index_EQ, QVector<double>& gains)
{
    return p_SetExtendEqualizerGainEx(index_EQ, gains.data(), gains.size());
}

QVector<double> LoadApoDLL::GetExtendEqualizerGainEx(unsigned int index_EQ, int maxsize)
{
    QVector<double> gains(maxsize);
    p_GetExtendEqualizerGainEx(index_EQ, gains.data(), maxsize);
    return gains;
}

int LoadApoDLL::SetExtendEqualizerCenterFrequency(unsigned int index_EQ, unsigned int index_band, double freq)
{
    return p_SetExtendEqualizerCenterFrequency(index_EQ, index_band, freq);
}

double LoadApoDLL::GetExtendEqualizerCenterFrequency(unsigned int index_EQ, unsigned int index_band)
{
    return p_GetExtendEqualizerCenterFrequency(index_EQ, index_band);
}

int LoadApoDLL::SetExtendEqualizerCenterFrequencyEx(unsigned int index_EQ,  QVector<double>& freqs)
{
    return p_SetExtendEqualizerCenterFrequencyEx(index_EQ, freqs.data(), freqs.size());
}

QVector<double> LoadApoDLL::GetExtendEqualizerCenterFrequencyEx(unsigned int index_EQ, int maxsize)
{
    QVector<double> freqs(maxsize);
    p_GetExtendEqualizerCenterFrequencyEx(index_EQ, freqs.data(), maxsize);
    return freqs;
}

int LoadApoDLL::SetExtendEqualizerBandEnable(unsigned int index_EQ, unsigned int index_band, bool enable)
{
    return p_SetExtendEqualizerBandEnable(index_EQ, index_band, enable ? 1 : 0);
}

bool LoadApoDLL::GetExtendEqualizerBandEnable(unsigned int index_EQ, unsigned int index_band)
{
    return p_GetExtendEqualizerBandEnable(index_EQ, index_band) != 0;
}

int LoadApoDLL::SetExtendEqualizerBandEnableEx(unsigned int index_EQ,  QVector<bool>& enables)
{
    QVector<unsigned int> intArr(enables.size());
    for (int i = 0; i < enables.size(); ++i)
        intArr[i] = enables.at(i) ? 1u : 0u;
    return p_SetExtendEqualizerBandEnableEx(index_EQ, intArr.data(), intArr.size());
}

QVector<bool> LoadApoDLL::GetExtendEqualizerBandEnableEx(unsigned int index_EQ, int maxsize)
{
    QVector<unsigned int> intArr(maxsize);
    p_GetExtendEqualizerBandEnableEx(index_EQ, intArr.data(), maxsize);
    QVector<bool> result;
    result.reserve(maxsize);
    for (unsigned int v : intArr)
        result.append(v != 0);
    return result;
}

int LoadApoDLL::SetExtendEqualizerBandQuality(unsigned int index_EQ, unsigned int index_band, double quality)
{
    return p_SetExtendEqualizerBandQuality(index_EQ, index_band, quality);
}

double LoadApoDLL::GetExtendEqualizerBandQuality(unsigned int index_EQ, unsigned int index_band)
{
    return p_GetExtendEqualizerBandQuality(index_EQ, index_band);
}

int LoadApoDLL::SetExtendEqualizerBandQualityEx(unsigned int index_EQ,  QVector<double>& qualities)
{
    return p_SetExtendEqualizerBandQualityEx(index_EQ, qualities.data(), qualities.size());
}

QVector<double> LoadApoDLL::GetExtendEqualizerBandQualityEx(unsigned int index_EQ, int maxsize)
{
    QVector<double> qualities(maxsize);
    p_GetExtendEqualizerBandQualityEx(index_EQ, qualities.data(), maxsize);
    return qualities;
}

int LoadApoDLL::SetExtendEqState(uint index, bool enable)
{
    // 基础位：OptionEnableExtendEuqalizer00（注意原拼写）
    uint32_t option = static_cast<uint32_t>(EffectProcessOption::OptionEnableExtendEuqalizer00)
                      << static_cast<int>(index);
    uint32_t curOption = GetProcessEffectOption();

    if (enable)
        curOption |= option;       // 置位
        // curOption = curOption | option | static_cast<uint32_t>(EffectProcessOption::OptionEnableArEffect);
    else
        curOption &= ~option;      // 清零

    SetProcessEffectOption(curOption);
    return m_errorCode;
}

bool LoadApoDLL::GetExtendEqState(uint index)
{
    uint32_t option = static_cast<uint32_t>(EffectProcessOption::OptionEnableExtendEuqalizer00)
    << static_cast<int>(index);
    return (GetProcessEffectOption() & option) == option;
}

//滤波器(只有十组eq中的第一和第二组（0,1）的需要切换，其他组的不用修改，默认值都为Peaking EQ)
// 实现单个设置
int LoadApoDLL::SetExtendEqualizerBandFilter(uint index_EQ, uint index_band, EqualizerFilter filter)
{
    return p_SetExtendEqualizerBandFilter(index_EQ, index_band, static_cast<int>(filter));
}

// 实现单个获取
EqualizerFilter LoadApoDLL::GetExtendEqualizerBandFilter(uint index_EQ, uint index_band)
{
    int value = p_GetExtendEqualizerBandFilter(index_EQ, index_band);
    return static_cast<EqualizerFilter>(value);
}

// 设置整个 EQ 的滤波器数组
int LoadApoDLL::SetExtendEqualizerBandFilterEx(uint index_EQ, const QVector<EqualizerFilter>& filters)
{
    // 转换为 int 数组（C风格）
    int size = filters.size();
    if (size == 0)
        return 0; // 或错误码

    // 动态分配临时数组
    QVector<int> intArr(size);
    for (int i = 0; i < size; ++i) {
        intArr[i] = static_cast<int>(filters[i]);
    }

    return p_SetExtendEqualizerBandFilterEx(index_EQ, intArr.data(), size);
}
// 获取整个 EQ 的滤波器数组
QVector<EqualizerFilter> LoadApoDLL::GetExtendEqualizerBandFilterEx(uint index_EQ, int maxsize)
{
    QVector<int> intArr(maxsize);
    int actualCount = p_GetExtendEqualizerBandFilterEx(index_EQ, intArr.data(), maxsize);
    if (actualCount <= 0)
        return QVector<EqualizerFilter>();

    // 如果实际返回数量小于 maxsize，截取有效部分
    if (actualCount < maxsize)
        intArr.resize(actualCount);

    QVector<EqualizerFilter> result(intArr.size());
    for (int i = 0; i < intArr.size(); ++i) {
        result[i] = static_cast<EqualizerFilter>(intArr[i]);
    }
    return result;
}

//DRC
int LoadApoDLL::SetDrcState(bool enable)
{
    if(IsSetLdcDev)
    {
        logWithTime(QString("111APO DLL SetArEffectState enable: %1").arg(enable));
        uint curOption = GetProcessEffectOption();

        if (enable) {
            // 启用：使用按位或设置标志位
            // curOption |= static_cast<uint32_t>(EffectProcessOption::OptionEnableDrcEffect);
            curOption = curOption | static_cast<uint32_t>(EffectProcessOption::OptionEnableDrcEffect)
                        | static_cast<uint32_t>(EffectProcessOption::OptionEnableStereoToMultiple);
                        // | static_cast<uint32_t>(EffectProcessOption::OptionEnableArEffect);
        } else {
            // 禁用：使用按位与和取反清除特定位
            curOption &= ~static_cast<uint32_t>(EffectProcessOption::OptionEnableDrcEffect);
        }
        SetProcessEffectOption(curOption);
        return m_errorCode;
    }else
    {
        return 0;
    }
}
bool LoadApoDLL::GetDrcState()
{
    uint drcOption = GetProcessEffectOption() & static_cast<uint>(EffectProcessOption::OptionEnableDrcEffect);
    return (drcOption == static_cast<uint>(EffectProcessOption::OptionEnableDrcEffect));
}

double LoadApoDLL::GetDrcThreshold()
{
    double value = 0.0;
    m_errorCode = p_GetDrcThreshold(&value);
    return value;
}

int LoadApoDLL::SetDrcThreshold(double threshold)
{
    m_errorCode = p_SetDrcThreshold(threshold);
    return m_errorCode;
}

double LoadApoDLL::GetDrcRatio()
{
    double value = 0.0;
    m_errorCode = p_GetDrcRatio(&value);
    return value;
}

int LoadApoDLL::SetDrcRatio(double ratio)
{
    m_errorCode = p_SetDrcRatio(ratio);
    return m_errorCode;
}

double LoadApoDLL::GetDrcAttackTime()
{
    double value = 0.0;
    m_errorCode = p_GetDrcAttackTime(&value);
    return value;
}

int LoadApoDLL::SetDrcAttackTime(double attackTime)
{
    m_errorCode = p_SetDrcAttackTime(attackTime);
    return m_errorCode;
}

double LoadApoDLL::GetDrcReleaseTime()
{
    double value = 0.0;
    m_errorCode = p_GetDrcReleaseTime(&value);
    return value;
}

int LoadApoDLL::SetDrcReleaseTime(double releaseTime)
{
    m_errorCode = p_SetDrcReleaseTime(releaseTime);
    return m_errorCode;
}

unsigned int LoadApoDLL::GetDrcMakeupEnable()
{
    unsigned int value = 0;
    m_errorCode = p_GetDrcMakeupEnable(&value);
    return value;
}

int LoadApoDLL::SetDrcMakeupEnable(unsigned int makeupEnable)
{
    m_errorCode = p_SetDrcMakeupEnable(makeupEnable);
    return m_errorCode;
}

double LoadApoDLL::GetDrcInputGain()
{
    double value = 0.0;
    m_errorCode = p_GetDrcInputGain(&value);
    return value;
}

int LoadApoDLL::SetDrcInputGain(double inputGain)
{
    m_errorCode = p_SetDrcInputGain(inputGain);
    return m_errorCode;
}

double LoadApoDLL::GetDrcOutputGain()
{
    double value = 0.0;
    m_errorCode = p_GetDrcOutputGain(&value);
    return value;
}

int LoadApoDLL::SetDrcOutputGain(double outputGain)
{
    m_errorCode = p_SetDrcOutputGain(outputGain);
    return m_errorCode;
}

unsigned int LoadApoDLL::GetDrcLimiterEnable()
{
    unsigned int value = 0;
    m_errorCode = p_GetDrcLimiterEnable(&value);
    return value;
}

int LoadApoDLL::SetDrcLimiterEnable(unsigned int limiterEnable)
{
    m_errorCode = p_SetDrcLimiterEnable(limiterEnable);
    return m_errorCode;
}

double LoadApoDLL::GetDrcLimiterThreshold()
{
    double value = 0.0;
    m_errorCode = p_GetDrcLimiterThreshold(&value);
    return value;
}

int LoadApoDLL::SetDrcLimiterThreshold(double limiterThreshold)
{
    m_errorCode = p_SetDrcLimiterThreshold(limiterThreshold);
    return m_errorCode;
}


// Ratio → dB: 使用公式 20 * log10(ratio)
double LoadApoDLL::RatioToDb(double ratio)
{
    if (ratio <= 0.0) {
        return -std::numeric_limits<double>::infinity(); // 防止负数或零
    }
    return 20.0 * std::log10(ratio);
}

// dB → Ratio: 使用公式 10^(dB / 20)
double LoadApoDLL::DbToRatio(int db)
{
    double a = std::pow(10.0, db / 20.0);
    return std::pow(10.0, db / 20.0);
}

//获得扬声器的额外音量增益
int LoadApoDLL::GetGlobalInputGainDb()
{
    double value = 0;

    try {
      m_errorCode = pGetGlobalInputRatio(&value);
    } catch (...) {
        logWithTime("Exception in GetGlobalInputGainDb");
    }
    double dbValue = value;//RatioToDb(value);

    return static_cast<int>(std::round(dbValue));
}
//设置扬声器的额外音量增益
int LoadApoDLL::SetGlobalInputGainDb(int value)
{
    logWithTime(QString("111APO DLL SetGlobalInputGainDb value: [%1]").arg(value));
    // qDebug("111APO DLL SetGlobalInputGainDb value: [%d]", value);

    try {
        m_errorCode = pSetGlobalInputRatio(value*2);
    } catch (...) {
        logWithTime("Exception in SetGlobalInputGainDb");
    }

    logWithTime(QString("111APO DLL SetGlobalInputGainDb returnCode: [%1]").arg(m_errorCode));


    return m_errorCode;
}

//所有
int LoadApoDLL::SetRenderState(bool enable)
{

    if(IsSetLdcDev)
    {
        uint curOption = GetProcessEffectOption();        
        if (enable)
        {
            uint option = /*static_cast<uint32_t>(EffectProcessOption::OptionEnableArEffect) |*/
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableUpmix) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableDownmixEffect) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableReverbEffect) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableCompBass) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableDrcEffect) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableStereoToMultiple) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableExtendEuqalizer00) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableExtendEuqalizer01) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableExtendEuqalizer02) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableExtendEuqalizer03) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableExtendEuqalizer04) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableExtendEuqalizer05) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableExtendEuqalizer06) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableExtendEuqalizer07) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableExtendEuqalizer08) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableExtendEuqalizer09);

            curOption = curOption | option;
        }
        else
        {
            uint option = /*static_cast<uint32_t>(EffectProcessOption::OptionEnableArEffect) |*/
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableDownmixEffect) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableReverbEffect) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableCompBass) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableDrcEffect) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableExtendEuqalizer00) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableExtendEuqalizer01) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableExtendEuqalizer02) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableExtendEuqalizer03) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableExtendEuqalizer04) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableExtendEuqalizer05) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableExtendEuqalizer06) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableExtendEuqalizer07) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableExtendEuqalizer08) |
                          static_cast<uint32_t>(EffectProcessOption::OptionEnableExtendEuqalizer09);

            curOption = curOption & (~option);
        }




        SetProcessEffectOption(curOption);

        return m_errorCode;
    }
    else
    {
        return 0;
    }
}
int LoadApoDLL::SetArEffectState(bool enable)
{
    if(IsSetLdcDev)
    {
        logWithTime(QString("111APO DLL SetArEffectState enable: %1").arg(enable));
        uint curOption = GetProcessEffectOption();

        if (enable) {
            // 启用：使用按位或设置标志位
            curOption |= static_cast<uint32_t>(EffectProcessOption::OptionEnableArEffect);
            // 可扩展其他功能，如 AR 效果（注释中提到）：
            // curOption |= static_cast<uint32_t>(EffectProcessOption::OptionEnableArEffect);
        } else {
            // 禁用：使用按位与和取反清除特定位
            curOption &= ~static_cast<uint32_t>(EffectProcessOption::OptionEnableArEffect);
        }
        SetProcessEffectOption(curOption);
        return m_errorCode;
    }else
    {
        return 0;
    }
}

/*//Compressor
//设置DRC(Compressor/灵犀算法)的使能
int LoadApoDLL::SetSmartVolumeState(bool enable)
{
    logWithTime(QString("111APO DLL SetSmartVolumeState enable:%1").arg(enable));
    uint curOption = GetProcessEffectOption();

    // 获取目标标志位
    const uint32_t flag = static_cast<uint32_t>(EffectProcessOption::OptionEnableUserDrcEffect);

    if (enable)
    {
        curOption |=  flag;
    }
    else
    {
        curOption &= ~flag;
    }
    SetProcessEffectOption(curOption);
    return m_errorCode;
}
bool LoadApoDLL::GetSmartVolumeState()
{
    unsigned int userDrcOption = GetProcessEffectOption() & static_cast<unsigned int>(EffectProcessOption::OptionEnableUserDrcEffect);
    return (userDrcOption == static_cast<unsigned int>(EffectProcessOption::OptionEnableUserDrcEffect));
}

int LoadApoDLL::GetSmartVolumeGain()
{
    unsigned int value = 0;

    try {
        // 调用底层 API，假设其返回 int 类型错误码（0 表示成功）
        m_errorCode = pGetUserDrcCompressor(&value);
    } catch (...) {
        logWithTime("Exception in GetSmartVolumeGain");
    }

    logWithTime(QString("Savitech_GetUserDrcCompressor with return:%1").arg(m_errorCode));

    return static_cast<int>(value);

}
//DRC
int LoadApoDLL::SetSmartVolumeGain(int value)
{
    // 将传入的 int 转换为 unsigned int（注意：如果 value 可能为负，需要处理）
    unsigned int uValue = static_cast<unsigned int>(value);

    try {
        m_errorCode = pSetUserDrcCompressor(uValue);
    } catch (...) {
        logWithTime("Exception in SetSmartVolumeGain");
    }

    logWithTime(QString("Savitech_SetUserDrcCompressor with return:%1").arg(m_errorCode));

    // 设置后重新获取当前值并返回
    return GetSmartVolumeGain();
}
*/

//输出log
void LoadApoDLL::logWithTime(const QString &msg)
{
    static QMutex mutex;
    QMutexLocker lock(&mutex);

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    // 加上当前线程ID，这是诊断最关键信息
    QString threadId = QString("0x%1").arg((quintptr)QThread::currentThread(), 0, 16);
    stream << timestamp << " [" << threadId << "] " << msg << Qt::endl;
    // stream << timestamp << " " << msg << Qt::endl;
}


//上行
//上行初始化（放到机型选择后）
void LoadApoDLL::InitialUpApo()
{
    if (g_hSavi == NULL)
    {
        // sdk init
        g_hSavi = g_pSaviInit(1);  // The input valuse is always 1.

        if (g_hSavi == NULL)
        {
            QMessageBox::critical(nullptr, QObject::tr("错误"), QObject::tr("上行初始化失败"));
            qDebug("Init SDK Fail");
            return;
        }
    }
}
//获得初始化是否成功
int LoadApoDLL::GetInitEn()
{
    if (g_hSavi == NULL)
    {
        return 0;
    }else
    {
        return 1;
    }
}
//是否支持该设备
int LoadApoDLL::IsSupportEP()
{
    if (g_hSavi == NULL)
    {
        return 0;
    }else
    {
        return g_pSaviIsSupportEP(g_hSavi,idSaved);
    }
}
//获得人声清晰的使能
int LoadApoDLL::GetVocalEffectsEnable()
{
    int en = 0;
    if (g_hSavi)
    {
        en = (int)g_pSaviGetVocalEffectsEnable(g_hSavi, idSaved);
    }
    return en;
}
//设置人声清晰使能
void LoadApoDLL::SetVocalEffectsEnable(int en)
{
    if (g_hSavi)
    {
        bool res = g_pSaviSetVocalEffectsEnable(g_hSavi, idSaved, en);
        logWithTime(QString("SetVocalEffectsEnable with return:%1").arg(res));
    }
}

//获得人声浑厚使能
int LoadApoDLL::GetRichVocalsEnable()
{
    int en = 0;
    if (g_hSavi)
    {
        en = (int)g_pSaviGetRichVocalsEnable(g_hSavi, idSaved);
    }
    return en;
}
//设置人声浑厚使能
void LoadApoDLL::SetRichVocalsEnable(int en)
{
    if (g_hSavi)
    {
        bool res = g_pSaviSetRichVocalsEnable(g_hSavi, idSaved, en);
        logWithTime(QString("SetRichVocalsEnable with return:%1").arg(res));
    }
}

//获得麦克风降噪使能
int LoadApoDLL::GetAINSEnable()
{
    int en = 0;
    if (g_hSavi)
    {
        en = (int)g_pSaviGetAINSEnable(g_hSavi, idSaved);
    }

    return en;
}
//设置麦克风降噪使能
void LoadApoDLL::SetAINSEnable(int en)
{
    if (g_hSavi)
    {
        g_pSaviSetAINSEnable(g_hSavi, idSaved, en);
    }

}
//设置麦克风降噪等级
void LoadApoDLL::SetAINSLevel(int percent_level)
{
    if (g_hSavi)
    {
        g_pSaviSetAINSLevel(g_hSavi, idSaved, percent_level);
    }

}



