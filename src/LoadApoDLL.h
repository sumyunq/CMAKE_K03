#ifndef LOADAPODLL_H
#define LOADAPODLL_H
#include <QString>
#include <QMovie>

#include <QSharedMemory>

#include <QThread>

// 结果常量
constexpr int Result_Success = 1;
constexpr int Result_Failed = 0;
typedef void *HANDLE;

enum class EffectProcessOption : unsigned int
{
    OptionEnableEuqalizer = 0x1,
    OptionEnableArEffect = 0x1 << 1,
    OptionEnableRaceEffect = 0x1 << 2,
    OptionEnableUpmix = 0x1 << 3,
    OptionEnableBassEnhancement = 0x1 << 4,
    OptionEnableStereoToMultiple = 0x1 << 5,
    OptionEnableDownmixEffect = 0x1 << 6,
    OptionEnableReverbEffect = 0x1 << 7,
    OptionEnableCompBass = 0x1 << 8,
    OptionEnableDrcEffect = 0x1 << 9,
    OptionEnableWdrcEffect = 0x1 << 10,
    OptionEnableVoiceClarity = 0x1 << 11,
    OptionEnableUserDrcEffect = 0x1 << 12,
    OptionEnableLfeMixedEffect = 0x1 << 13,
    OptionEnableExtendEuqalizer00 = 0x1 << 14,
    OptionEnableExtendEuqalizer01 = 0x1 << 15,
    OptionEnableExtendEuqalizer02 = 0x1 << 16,
    OptionEnableExtendEuqalizer03 = 0x1 << 17,
    OptionEnableExtendEuqalizer04 = 0x1 << 18,
    OptionEnableExtendEuqalizer05 = 0x1 << 19,
    OptionEnableExtendEuqalizer06 = 0x1 << 20,
    OptionEnableExtendEuqalizer07 = 0x1 << 21,
    OptionEnableExtendEuqalizer08 = 0x1 << 22,
    OptionEnableExtendEuqalizer09 = 0x1 << 23,
};
enum AudioSettingOption
{
    //Error = -1,
    Movie = 0,
    Gaming = 1,
    Music = 2,
    //Speaker,
    Voice = 4,
    Balanced,
    Shooter,
    Racing,
    Roleplay,
    Customize,
    ModeCount,
};
enum ReverbRoomType
{
    E_REVERB_ROOM_STUDIO = 4,
    E_REVERB_ROOM_THEATER,
    E_REVERB_ROOM_CONCERT,
    E_REVERB_ROOM_CAVE,
    E_REVERB_ROOM_CHURCH,
    E_REVERB_ROOM_STADIUM,
    E_REVERB_ROOM_COUNT
};
//滤波器
enum EqualizerFilter : int
{
    PeakFilter,
    HighPassFilter,
    LowPassFilter,
    HighShelvingFilter,
    LowShelvingFilter,
    NotchFilter
};

// 定义函数指针类型，匹配 StdCall 调用约定
//设备是否被APO支持
//typedef int (__stdcall *Savitech_IsLhdcDeviceSupport)(const wchar_t* strSpeakerModuleName, int* isSupport, int* type, wchar_t* vId_pId, int size);
// typedef int (__stdcall *Savitech_IsLhdcDeviceSupport)(const char* strSpeakerModuleName, int* isSupport, int* type, char* vId_pId, int size);
typedef int (__stdcall *Savitech_IsLhdcDeviceSupport)(const char* strSpeakerModuleName, int* isSupport, char* vId_pId, int bufSize);
// //设置APO所要支持的设备
// typedef int (__stdcall*Savitech_SetLhdcDevicePrivate)(const char* SpeakerModuleName);
typedef int (__stdcall *Savitech_SetLhdcDevice)(const char* SpeakerModuleName);

// typedef int (__stdcall *Savitech_GetDeviceApoCount)(const wchar_t* pszDeviceId, unsigned int eFlow, unsigned int* pCount);//const char* pszDeviceId
//设置APO优先级
typedef int (__stdcall *Savitech_SetDeviceApoClsid)(uint eFlow, const wchar_t* pszDeviceId);
// typedef int (__stdcall *Savitech_GetDeviceApoInfo)(unsigned int eFlow, unsigned int uIndex,
//                                                     wchar_t* pszFriendlyName, unsigned int cchFriendlyName,
//                                                     wchar_t* pszCopyrightInfo, unsigned int cchCopyrightInfo);

// 获取单例对象
// typedef int (__stdcall *Savitech_InitialEngine)(const char* strCertificatedStr);
// 设置处理效果选项（所有功能的使能）
typedef int (__stdcall *Savitech_SetProcessEffectOption)(unsigned int option);
// 获取处理效果选项（所有功能的使能）
typedef int (__stdcall *Savitech_GetProcessEffectOption)(unsigned int* rtnOption);

/*
// 设置 EQ 带宽质量（Q值）数组
typedef int (__stdcall *Savitech_SetEqualizerBandQualityEx)(const double* listBandQuality, int size);
// 获得 EQ 带宽质量（Q值）数组
typedef int (__stdcall *Savitech_GetEqualizerBandQualityEx)(double* rtnListBandQuality, int maxsize);

// 设置单个 EQ 波段增益（dB）
typedef int (__stdcall *Savitech_SetEqualizerGain)(unsigned int index_band, double dbValue);

// 获取单个 EQ 波段增益
typedef int (__stdcall *Savitech_GetEqualizerGain)(unsigned int index_band, double* rtn_gain);
// 设置单个 EQ 波段增益（dB）
typedef int (__stdcall *Savitech_SetEqualizerGains)(const double* gains, int size);

// 获取单个 EQ 波段增益
typedef int (__stdcall *Savitech_GetEqualizerGains)(double* gains, int maxsize);

// // 设置所有 EQ 波段增益数组（单位：dB）
// typedef int (__stdcall *Savitech_SetEqualizerGainEx)(const double* listGainInDB);

// // 获取所有 EQ 波段增益数组
// typedef int (__stdcall *Savitech_GetEqualizerGainEx)(double* rtnListGainInDB);

//设置EQ频点
typedef int (__stdcall *Savitech_SetEqualizerCenterFrequencyEx)(const double* listCenterFrequency, int size);
//获得EQ频点
//typedef int (__stdcall *Savitech_GetEqualizerCenterFrequencyEx)(double* rtnListCenterFrequency);

typedef int (__stdcall *Savitech_GetEqualizerCenterFrequencyEx)(double rtnListCenterFrequency[20], int size);

//设置EQ频段使能
typedef int (__stdcall *Savitech_SetEqualizerBandEnableEx)(int* listBandEnable, int size);
//获得EQ频段使能
typedef int (__stdcall *Savitech_GetEqualizerBandEnableEx)(int* rtnListBandEnable, int size);

//设置/获得EQ过滤器（不用）
typedef int (__stdcall *Savitech_SetEqualizerBandFilterEx)(EqualizerFilter* listBandFilter, int size);
typedef int (__stdcall *Savitech_GetEqualizerBandFilterEx)(EqualizerFilter* rtnListBandFilter, int size);
*/



// 设置混响房间类型
typedef int (__stdcall *Savitech_SetReverbActivateRoomType)(int nRoomTypeId);

// 获取当前混响房间类型
typedef int (__stdcall *Savitech_GetReverbActivateRoomType)(int* rtnRoomId);

// 设置混响比例/增益（AR 混响）
typedef int (__stdcall *Savitech_SetArReverbRatio)(double dbRatioReverb);

// 获取混响比例/增益
typedef int (__stdcall *Savitech_GetArReverbRatio)(double* rtnReverbRatio);

// 设置低音增强增益
// typedef int (__stdcall *Savitech_SetCompBassGain)(double dbGainScalar);
typedef int (__stdcall *Savitech_SetCompBassGain)(int dbGainScalar);

// 获取低音增强增益
// typedef int (__stdcall *Savitech_GetCompBassGain)(double* rtnGainScalar);
typedef int (__stdcall *Savitech_GetCompBassGain)(int* rtnGainScalar);

//设置低音中心频率
typedef int (__stdcall *Savitech_SetCompBassCenterFrequency)(double centerFrequency);

//获取低音中心频率
typedef int (__stdcall *Savitech_GetCompBassCenterFrequency)(double* rtnCenterFrequency);

// 检查是否已初始化依赖资源
typedef int (__stdcall *Savitech_IsInitialDependencyResource)(int* isInit);

// 初始化依赖资源
typedef int (__stdcall *Savitech_InitialDependencyResource)();

// 释放依赖资源
typedef int (__stdcall *Savitech_ReleaseDependencyResource)();

// 获取音频设置选项（如游戏、音乐等模式）(不用)
typedef int (__stdcall *Savitech_GetAudioSettingOption)(int* rtnOption);

// 设置音频设置选项(不用)
typedef int (__stdcall *Savitech_SetAudioSettingOption)(int option);

// 恢复音频设置为默认(不用)
typedef int (__stdcall *Savitech_ResetAudioSettingToDefault)(int option);
/*
// 获取均衡器预设名称(不用)
typedef int (__stdcall *Savitech_GetEqualizerPresetName)(char* rtn_preset_name, unsigned int uiNameSizeInBytes);

// 设置均衡器预设名称(不用)
typedef int (__stdcall *Savitech_SetEqualizerPresetName)(const char* preset_name, unsigned int uiNameSizeInBytes);

// 设置声音清晰度等级(不用)
typedef int (__stdcall *Savitech_SetVoiceClarityLevel)(double Level);

// 获取声音清晰度等级(不用)
typedef int (__stdcall *Savitech_GetVoiceClarityLevel)(double* rtnLevel);

// 设置用户 DRC 压缩器比率
typedef int (__stdcall *Savitech_SetUserDrcCompressor)(unsigned int ratioScale);
// 获得用户 DRC 压缩器比率
typedef int (__stdcall *Savitech_GetUserDrcCompressor)(unsigned int* rtnRatioScale);
*/
// 设置空间强度
typedef int (__stdcall *Savitech_SetDistance)(double Level);
// 获得空间强度
typedef int (__stdcall *Savitech_GetDistance)(double* rtnLevel);

//设置扬声器的额外增益
typedef int (__stdcall *Savitech_SetGlobalInputRatio)(double Level);
//获得扬声器的额外增益
typedef int (__stdcall *Savitech_GetGlobalInputRatio)(double* rtnLevel);


//1.0.11.0APO新增
//DRC(ui中的灵晰算法)
//获取DRC(动态范围)阈值
typedef int (__stdcall *Savitech_GetDrcThreshold)(double* rtnThreshold);
//设置DRC(动态范围)阈值（任何超过阈值的信号都将被压缩）（-50~0db，无小数点）
typedef int (__stdcall *Savitech_SetDrcThreshold)(double threshold);
//获取DRC(动态范围)压缩比率
typedef int (__stdcall *Savitech_GetDrcRatio)(double* rtnRatio);
//设置DRC(动态范围)压缩比率（1-30，小数点两位）
typedef int (__stdcall *Savitech_SetDrcRatio)(double ratio);
//获取DRC(动态范围)反应速度
typedef int (__stdcall *Savitech_GetDrcAttackTime)(double* rtnAttackTime);
//设置DRC(动态范围)反应速度（0-2s,小数点三位）
typedef int (__stdcall *Savitech_SetDrcAttackTime)(double attackTime);
//获取DRC(动态范围)恢复速度
typedef int (__stdcall *Savitech_GetDrcReleaseTime)(double* rtnReleaseTime);
//设置DRC(动态范围)恢复速度（0-2s,小数点三位）
typedef int (__stdcall *Savitech_SetDrcReleaseTime)(double releaseTime);
//获取DRC(动态范围)自动（补偿）增益使能
typedef int (__stdcall *Savitech_GetDrcMakeupEnable)(unsigned int* rtnMakeupEnable);
//设置DRC(动态范围)自动（补偿）增益使能
typedef int (__stdcall *Savitech_SetDrcMakeupEnable)(unsigned int makeupEnable);
//获取DRC(动态范围)输入增益
typedef int (__stdcall *Savitech_GetDrcInputGain)(double* rtnInputGain);
//设置DRC(动态范围)输入增益（-30-30db,小数点两位）
typedef int (__stdcall *Savitech_SetDrcInputGain)(double inputGain);
//获取DRC(动态范围)输出增益
typedef int (__stdcall *Savitech_GetDrcOutputGain)(double* rtnOutputGain);
//设置DRC(动态范围)输出增益（-30-30db,小数点两位）
typedef int (__stdcall *Savitech_SetDrcOutputGain)(double outputGain);
//获取DRC(动态范围)限制器使能
typedef int (__stdcall *Savitech_GetDrcLimiterEnable)(unsigned int* rtnLimiterEnable);
//设置DRC(动态范围)限制器使能（控制音频信号的最大音量）
typedef int (__stdcall *Savitech_SetDrcLimiterEnable)(unsigned int limiterEnable);
//获取DRC(动态范围)限制器阈值
typedef int (__stdcall *Savitech_GetDrcLimiterThreshold)(double* rtnLimiterThreshold);
//设置DRC(动态范围)限制器阈值（任何超过阈值的信号都将被压缩）（-50~0db，无小数点）
typedef int (__stdcall *Savitech_SetDrcLimiterThreshold)(double limiterThreshold);

//均衡器
//设置是否支持扩展均衡器效果
typedef int (__stdcall *Savitech_IsSupportExtendEqualizerEffect)(int* isSupport);
//获取扩展均衡器频段数量
typedef uint (__stdcall *Savitech_GetExtendEqualizerBandCount)();
//重置扩展均衡器设置
typedef int (__stdcall *Savitech_ResetExtendEqualizerSetting)(uint index_EQ);
//设置某个扩展均衡器单个EQ波段增益
typedef int (__stdcall *Savitech_SetExtendEqualizerGain)(uint index_EQ,uint index_band, double dbValue);
//获得某个扩展均衡器单个EQ波段增益
typedef double (__stdcall *Savitech_GetExtendEqualizerGain)(uint index_EQ,uint index_band);
//设置某个扩展均衡器EQ波段增益
typedef int (__stdcall *Savitech_SetExtendEqualizerGainEx)(uint index_EQ,double* gains,int size);
//获得某个扩展均衡器EQ波段增益
typedef int (__stdcall *Savitech_GetExtendEqualizerGainEx)(uint index_EQ,double* gains,int maxsize);

//设置某个扩展均衡器单个EQ频点
typedef int (__stdcall *Savitech_SetExtendEqualizerCenterFrequency)(uint index_EQ,uint index_band, double centerFrequency);
//获得某个扩展均衡器单个EQ频点
typedef double (__stdcall *Savitech_GetExtendEqualizerCenterFrequency)(uint index_EQ,uint index_band);
//设置某个扩展均衡器EQ频点
typedef int (__stdcall *Savitech_SetExtendEqualizerCenterFrequencyEx)(uint index_EQ,double* gains,int size);
//获得某个扩展均衡器EQ频点
typedef int (__stdcall *Savitech_GetExtendEqualizerCenterFrequencyEx)(uint index_EQ,double* gains,int maxsize);

//设置某个扩展均衡器某个EQ频段使能
typedef int (__stdcall *Savitech_SetExtendEqualizerBandEnable)(uint index_EQ,uint index_band, int enable);
//获得某个扩展均衡器某个EQ频段使能
typedef int (__stdcall *Savitech_GetExtendEqualizerBandEnable)(uint index_EQ, uint index_band);

//设置某个扩展均衡器EQ频段使能
typedef int (__stdcall *Savitech_SetExtendEqualizerBandEnableEx)(uint index_EQ,uint* enables,int enable);
//获得某个扩展均衡器EQ频段使能
typedef int (__stdcall *Savitech_GetExtendEqualizerBandEnableEx)(uint index_EQ, uint* enables,int maxsize);


//设置某个扩展均衡器单个EQ Q值
typedef int (__stdcall *Savitech_SetExtendEqualizerBandQuality)(uint index_EQ,uint index_band, double quality);
//获得某个扩展均衡器单个EQ Q值
typedef double (__stdcall *Savitech_GetExtendEqualizerBandQuality)(uint index_EQ,uint index_band);
//设置某个扩展均衡器EQ Q值
typedef int (__stdcall *Savitech_SetExtendEqualizerBandQualityEx)(uint index_EQ,double* qualities,int size);
//获得某个扩展均衡器EQ Q值
typedef int (__stdcall *Savitech_GetExtendEqualizerBandQualityEx)(uint index_EQ,double* qualities,int maxsize);

//设置某个扩展均衡器单个EQ过滤器
typedef int (__stdcall *Savitech_SetExtendEqualizerBandFilter)(uint index_EQ,uint index_band, int filter);
//获得某个扩展均衡器单个EQ过滤器
typedef double (__stdcall *Savitech_GetExtendEqualizerBandFilter)(uint index_EQ,uint index_band);
//设置某个扩展均衡器EQ过滤器
typedef int (__stdcall *Savitech_SetExtendEqualizerBandFilterEx)(uint index_EQ,int* filters,int size);
//获得某个扩展均衡器EQ过滤器
typedef int (__stdcall *Savitech_GetExtendEqualizerBandFilterEx)(uint index_EQ,int* filters,int maxsize);


//上行
//初始化
typedef HANDLE(__cdecl* pSAVIINIT)(int);
typedef void(__cdecl* pSAVIDEINIT)(HANDLE);
typedef int(__cdecl* pSAVIISSUPPORTEP)(HANDLE, wchar_t*);
typedef int(__cdecl* pSAVIGETNOISEGATEENABLE)(HANDLE, wchar_t*);
typedef int(__cdecl* pSAVISETNOISEGATEENABLE)(HANDLE, wchar_t*, bool);
typedef int(__cdecl* pSAVIGETNOISEGATELEVEL)(HANDLE, wchar_t*, int*);
typedef int(__cdecl* pSAVISETNOISEGATELEVEL)(HANDLE, wchar_t*, int);
typedef int(__cdecl* pSAVIGETAINSENABLE)(HANDLE, wchar_t*);
typedef int(__cdecl* pSAVISETAINSENABLE)(HANDLE, wchar_t*, bool);
typedef int(__cdecl* pSAVIGETAINSLEVEL)(HANDLE, wchar_t*, int*);
typedef int(__cdecl* pSAVISETAINSLEVEL)(HANDLE, wchar_t*, int);
typedef int(__cdecl* pSAVIGETVOCALEFFECTSENABLE)(HANDLE, wchar_t*);
typedef int(__cdecl* pSAVISETVOCALEFFECTSENABLE)(HANDLE, wchar_t*, bool);
typedef int(__cdecl* pSAVIGETRICHVOCALSENABLE)(HANDLE, wchar_t*);
typedef int(__cdecl* pSAVISETRICHVOCALSENABLE)(HANDLE, wchar_t*, bool);
typedef int(__cdecl* pSAVI_FUNC2_GET_EP_REGISTRY_API) (HANDLE, wchar_t*, wchar_t*, wchar_t*, wchar_t*, wchar_t*, wchar_t*, int);
typedef int(__cdecl* pSAVI_FUNC2_SET_EP_REGISTRY_API) (HANDLE, wchar_t*, wchar_t*, wchar_t*, wchar_t*, wchar_t*, wchar_t*);


class LoadApoDLL
{
public:
    LoadApoDLL();
    ~LoadApoDLL();
    void LoadApoLibrary();
    //下行导出函数(函数指针实例)
    Savitech_IsLhdcDeviceSupport pIsLhdcDeviceSupport;
    // Savitech_SetLhdcDevicePrivate pSetLhdcDevicePrivate;
    Savitech_SetLhdcDevice pSetLhdcDevice;

    // Savitech_GetDeviceApoCount pGetDeviceApoCount;
    // Savitech_GetDeviceApoInfo pGetDeviceApoInfo;
    Savitech_SetDeviceApoClsid pSetDeviceApoClsid;

    // Savitech_InitialEngine pInitialEngine;
    Savitech_SetProcessEffectOption pSetProcessEffectOption;
    Savitech_GetProcessEffectOption pGetProcessEffectOption;
    /*Savitech_SetEqualizerBandQualityEx pSetEqualizerBandQualityEx;
    Savitech_GetEqualizerBandQualityEx pGetEqualizerBandQualityEx;
    Savitech_SetEqualizerCenterFrequencyEx pSetEqualizerCenterFrequencyEx;
    Savitech_GetEqualizerCenterFrequencyEx pGetEqualizerCenterFrequencyEx;

    Savitech_SetEqualizerBandEnableEx pSetEqualizerBandEnableEx;
    Savitech_GetEqualizerBandEnableEx pGetEqualizerBandEnableEx;

    Savitech_SetEqualizerGain pSetEqualizerGain;
    Savitech_GetEqualizerGain pGetEqualizerGain;
    Savitech_SetEqualizerGains pSetEqualizerGains;
    Savitech_GetEqualizerGains pGetEqualizerGains;*/
    // Savitech_SetEqualizerGainEx pSetEqualizerGainEx;
    // Savitech_GetEqualizerGainEx pGetEqualizerGainEx;
    Savitech_SetReverbActivateRoomType pSetReverbActivateRoomType;
    Savitech_GetReverbActivateRoomType pGetReverbActivateRoomType;
    Savitech_SetArReverbRatio pSetArReverbRatio;
    Savitech_GetArReverbRatio pGetArReverbRatio;
    Savitech_SetCompBassGain pSetCompBassGain;
    Savitech_GetCompBassGain pGetCompBassGain;
    Savitech_SetCompBassCenterFrequency pSetCompBassCenterFrequency;
    Savitech_GetCompBassCenterFrequency pGetCompBassCenterFrequency;


    Savitech_InitialDependencyResource pInitialDependencyResource;
    Savitech_IsInitialDependencyResource pIsInitialDependencyResource;
    Savitech_ReleaseDependencyResource pReleaseDependencyResource;

    /*Savitech_SetVoiceClarityLevel pSetVoiceClarityLevel;
    Savitech_GetVoiceClarityLevel pGetVoiceClarityLevel;
    Savitech_SetUserDrcCompressor pSetUserDrcCompressor;
    Savitech_GetUserDrcCompressor pGetUserDrcCompressor;*/

    Savitech_SetDistance pSetDistance;
    Savitech_GetDistance pGetDistance;

    Savitech_SetGlobalInputRatio pSetGlobalInputRatio;
    Savitech_GetGlobalInputRatio pGetGlobalInputRatio;

    //1.0.11.0APO新增
    Savitech_GetDrcThreshold p_GetDrcThreshold;
    Savitech_SetDrcThreshold p_SetDrcThreshold;
    Savitech_GetDrcRatio p_GetDrcRatio;
    Savitech_SetDrcRatio p_SetDrcRatio;
    Savitech_GetDrcAttackTime p_GetDrcAttackTime;
    Savitech_SetDrcAttackTime p_SetDrcAttackTime;
    Savitech_GetDrcReleaseTime p_GetDrcReleaseTime;
    Savitech_SetDrcReleaseTime p_SetDrcReleaseTime;
    Savitech_GetDrcMakeupEnable p_GetDrcMakeupEnable;
    Savitech_SetDrcMakeupEnable p_SetDrcMakeupEnable;
    Savitech_GetDrcInputGain p_GetDrcInputGain;
    Savitech_SetDrcInputGain p_SetDrcInputGain;
    Savitech_GetDrcOutputGain p_GetDrcOutputGain;
    Savitech_SetDrcOutputGain p_SetDrcOutputGain;
    Savitech_GetDrcLimiterEnable p_GetDrcLimiterEnable;
    Savitech_SetDrcLimiterEnable p_SetDrcLimiterEnable;
    Savitech_GetDrcLimiterThreshold p_GetDrcLimiterThreshold;
    Savitech_SetDrcLimiterThreshold p_SetDrcLimiterThreshold;
    Savitech_IsSupportExtendEqualizerEffect p_IsSupportExtendEqualizerEffect;
    Savitech_GetExtendEqualizerBandCount p_GetExtendEqualizerBandCount;
    Savitech_ResetExtendEqualizerSetting p_ResetExtendEqualizerSetting;
    Savitech_SetExtendEqualizerGain p_SetExtendEqualizerGain;
    Savitech_GetExtendEqualizerGain p_GetExtendEqualizerGain;
    Savitech_SetExtendEqualizerGainEx p_SetExtendEqualizerGainEx;
    Savitech_GetExtendEqualizerGainEx p_GetExtendEqualizerGainEx;
    Savitech_SetExtendEqualizerCenterFrequency p_SetExtendEqualizerCenterFrequency;
    Savitech_GetExtendEqualizerCenterFrequency p_GetExtendEqualizerCenterFrequency;
    Savitech_SetExtendEqualizerCenterFrequencyEx p_SetExtendEqualizerCenterFrequencyEx;
    Savitech_GetExtendEqualizerCenterFrequencyEx p_GetExtendEqualizerCenterFrequencyEx;
    Savitech_SetExtendEqualizerBandEnable p_SetExtendEqualizerBandEnable;
    Savitech_GetExtendEqualizerBandEnable p_GetExtendEqualizerBandEnable;
    Savitech_SetExtendEqualizerBandEnableEx p_SetExtendEqualizerBandEnableEx;
    Savitech_GetExtendEqualizerBandEnableEx p_GetExtendEqualizerBandEnableEx;
    Savitech_SetExtendEqualizerBandQuality p_SetExtendEqualizerBandQuality;
    Savitech_GetExtendEqualizerBandQuality p_GetExtendEqualizerBandQuality;
    Savitech_SetExtendEqualizerBandQualityEx p_SetExtendEqualizerBandQualityEx;
    Savitech_GetExtendEqualizerBandQualityEx p_GetExtendEqualizerBandQualityEx;
    Savitech_SetExtendEqualizerBandFilter p_SetExtendEqualizerBandFilter;
    Savitech_GetExtendEqualizerBandFilter p_GetExtendEqualizerBandFilter;
    Savitech_SetExtendEqualizerBandFilterEx p_SetExtendEqualizerBandFilterEx;
    Savitech_GetExtendEqualizerBandFilterEx p_GetExtendEqualizerBandFilterEx;


    //上行导出函数(函数指针实例)
    HANDLE g_hSavi = NULL;
    pSAVIINIT g_pSaviInit = NULL;
    pSAVIDEINIT g_pSaviDeinit = NULL;
    pSAVIISSUPPORTEP g_pSaviIsSupportEP = NULL;
    pSAVIGETNOISEGATEENABLE g_pSaviGetNoiseGateEnable = NULL;
    pSAVISETNOISEGATEENABLE g_pSaviSetNoiseGateEnable = NULL;
    pSAVIGETNOISEGATELEVEL g_pSaviGetNoiseGateLevel = NULL;
    pSAVISETNOISEGATELEVEL g_pSaviSetNoiseGateLevel = NULL;
    pSAVIGETAINSENABLE g_pSaviGetAINSEnable = NULL;
    pSAVISETAINSENABLE g_pSaviSetAINSEnable = NULL;
    pSAVIGETAINSLEVEL g_pSaviGetAINSLevel = NULL;
    pSAVISETAINSLEVEL g_pSaviSetAINSLevel = NULL;
    pSAVIGETVOCALEFFECTSENABLE g_pSaviGetVocalEffectsEnable = NULL;
    pSAVISETVOCALEFFECTSENABLE g_pSaviSetVocalEffectsEnable = NULL;
    pSAVIGETRICHVOCALSENABLE g_pSaviGetRichVocalsEnable = NULL;
    pSAVISETRICHVOCALSENABLE g_pSaviSetRichVocalsEnable = NULL;
    pSAVI_FUNC2_GET_EP_REGISTRY_API g_pSavi_func2_get_ep_registry_API = NULL;
    pSAVI_FUNC2_SET_EP_REGISTRY_API g_pSavi_func2_set_ep_registry_API = NULL;


    int m_errorCode;

    unsigned int SetProcessEffectOption(unsigned int option);
    unsigned int GetProcessEffectOption();

    void InitialUpApo();
    int InitialDependencyResource();
    bool IsInitialDependencyResource();
    int ReleaseDependencyResource();
    bool IsLhdcDeviceSupport(const QString& deviceGUID, QString& vId_pId, int& isSupported);
    int SetLhdcDevice(const QString& deviceGUID);
    // int GetDeviceApoCount(const QString& pszDeviceId, uint eFlow, uint& pCount);
    // int GetDeviceApoInfo(uint eFlow, uint uIndex, QString& pszFriendlyName, QString& pszCopyrightInfo);
    int SetDeviceApoClsid(const QString& pszDeviceId, uint eFlow);//设置APO优先级


    //空间
    bool GetSurroundState();
    int SetSurroundState(bool enable);
    int SetDistance(int scale);
    int GetDistance();

    bool GetReverbState();
    int SetReverbState(bool enable);
    // int GetReverbGain();
    // int SetReverbGain(int value);
    ReverbRoomType SetReverbFilter(ReverbRoomType nRoomIndex);
    ReverbRoomType GetReverbFilter();

    int SetReverbActivateRoomType(int nRoomType);
    int GetReverbActivateRoomType();
    double SetArReverbRatio(double dbRatioReverb);
    double GetArReverbRatio();


    //低音
    bool GetBassBoostState();
    int SetBassBoostState(bool enable);

    int SetCompBassGain(int value);
    int GetCompBassGain();
    int SetCompBassCenterFrequency(double value);
    int GetCompBassCenterFrequency();

    int GetBassBoostGain();
    int SetBassBoostGain(int value);
    int GetBassBoostCenterFrequency();
    // int SetCompBassCenterFrequency(int value);
    // int SetBassBoostCenterFrequency(int value);


    //DRC
    int SetDrcState(bool enable);
    bool GetDrcState();
    double GetDrcThreshold();
    int SetDrcThreshold(double threshold);
    double GetDrcRatio();
    int SetDrcRatio(double ratio);
    double GetDrcAttackTime();
    int SetDrcAttackTime(double attackTime);
    double GetDrcReleaseTime();
    int SetDrcReleaseTime(double releaseTime);
    uint GetDrcMakeupEnable();
    int SetDrcMakeupEnable(uint makeupEnable);
    double GetDrcInputGain();
    int SetDrcInputGain(double inputGain);
    double GetDrcOutputGain();
    int SetDrcOutputGain(double outputGain);
    uint GetDrcLimiterEnable();
    int SetDrcLimiterEnable(uint limiterEnable);
    double GetDrcLimiterThreshold();
    int SetDrcLimiterThreshold(double limiterThreshold);

    //FPS（减少fps的影响），此次不用


    double RatioToDb(double ratio);
    double DbToRatio(int db);



   /* int GetVoiceClarityGain();
    int SetVoiceClarityGain(int value);
    bool GetVoiceClarityState();
    int SetVoiceClarityState(bool enable);

    bool GetEqualizerState();
    int SetEqualizerState(bool enable);
    QVector<double> GetEqualizerGainEx();
    int SetEqualizerGainEx(const QVector<double>& EqGainList);
    double SetEqualizerGain(uint32_t index_band, double dbValue);
    double GetEqualizerGain(uint32_t index_band);
    int SetEqualizerCenterFrequencyEx(const QVector<double>& EqBandFrequencyList);
    //int SetEqualizerCenterFrequencyEx(double* EqBandFrequencyList);

    QVector<double> GetEqualizerCenterFrequencyEx();
    //double* GetEqualizerCenterFrequencyEx();

    int SetEqualizerBandQualityEx(const QVector<double>& EqBandQualityList);

    int SetEqualizerBandEnableEx(int* EqBandEnableList);
    int* GetEqualizerBandEnableEx();
    */



    //扬声器增益
    int GetGlobalInputGainDb();
    int SetGlobalInputGainDb(int value);

    int SetRenderState(bool enable);
    int SetArEffectState(bool enable);

    /*void ActivateAsync(const QString &licenseKey);
    void RemovePermission();
    void VerifyPermissionAsync();
    void RegisterXiberiaActionCallback(PFN_XIBERIA_ACTION_CALLBACK bc_);*/

    /*bool GetSmartVolumeState();
    int SetSmartVolumeState(bool enable);
    int GetSmartVolumeGain();
    int SetSmartVolumeGain(int value);*/


    //均衡器
    bool IsSupportExtendEqualizerEffect();
    uint GetExtendEqualizerBandCount();
    int ResetExtendEqualizerSetting(uint index_EQ);
    int SetExtendEqState(uint index, bool enable);
    bool GetExtendEqState(uint index);
    int SetExtendEqualizerGain(uint index_EQ, uint index_band, double dbValue);
    double GetExtendEqualizerGain(uint index_EQ, uint index_band);
    int SetExtendEqualizerGainEx(uint index_EQ,  QVector<double>& gains);
    QVector<double> GetExtendEqualizerGainEx(uint index_EQ, int maxsize);
    int SetExtendEqualizerCenterFrequency(uint index_EQ, uint index_band, double freq);
    double GetExtendEqualizerCenterFrequency(uint index_EQ, uint index_band);
    int SetExtendEqualizerCenterFrequencyEx(uint index_EQ,  QVector<double>& freqs);
    QVector<double> GetExtendEqualizerCenterFrequencyEx(uint index_EQ, int maxsize);
    int SetExtendEqualizerBandEnable(uint index_EQ, uint index_band, bool enable);
    bool GetExtendEqualizerBandEnable(uint index_EQ, uint index_band);
    int SetExtendEqualizerBandEnableEx(uint index_EQ,  QVector<bool>& enables);
    QVector<bool> GetExtendEqualizerBandEnableEx(uint index_EQ, int maxsize);
    int SetExtendEqualizerBandQuality(uint index_EQ, uint index_band, double quality);
    double GetExtendEqualizerBandQuality(uint index_EQ, uint index_band);
    int SetExtendEqualizerBandQualityEx(uint index_EQ,  QVector<double>& qualities);
    QVector<double> GetExtendEqualizerBandQualityEx(uint index_EQ, int maxsize);

    //滤波器
    // 设置单个频段滤波器
    int SetExtendEqualizerBandFilter(uint index_EQ, uint index_band, EqualizerFilter filter);
    // 获取单个频段滤波器
    EqualizerFilter GetExtendEqualizerBandFilter(uint index_EQ, uint index_band);
    // 设置整个 EQ 的所有频段滤波器（数组版本）
    int SetExtendEqualizerBandFilterEx(uint index_EQ, const QVector<EqualizerFilter>& filters);
    // 获取整个 EQ 的所有频段滤波器（返回 QVector）
    QVector<EqualizerFilter> GetExtendEqualizerBandFilterEx(uint index_EQ, int maxsize);




    //上行
    int GetInitEn();
    int IsSupportEP();
    int GetVocalEffectsEnable();
    void SetVocalEffectsEnable(int en);
    int GetRichVocalsEnable();
    void SetRichVocalsEnable(int en);
    int GetAINSEnable();
    void SetAINSEnable(int en);
    void SetAINSLevel(int percent_level);


    void logWithTime(const QString &msg);

private:
    HINSTANCE m_hAct = NULL;
    HINSTANCE g_hSaviCTL = NULL;

};

#endif // LOADAPODLL_H
extern LoadApoDLL* apo;
extern const int RESULT_SUCCEED;
extern bool retB;
extern QHash<QString, QHash<QString, QString>> ERdevs;
extern bool IsSetLdcDev;
extern bool General;
extern wchar_t* idSaved;

// extern bool IsActivated;//是否激活

extern bool isLogin;//是否登录
extern QMovie *movie;
extern QTextStream stream;


extern QSharedMemory* m_sharedMemory;
