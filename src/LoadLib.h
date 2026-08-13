#ifndef LOADLIB_H
#define LOADLIB_H

#include <QMessageBox>
#include <QMutex>
#include <QObject> // 如果是 QObject 派生类
#include <QSettings>
#include "CustomControl/CustomRadioButton/NewRadioBtn.h"
#include "VideoHover.h"
#include "mainwindow.h"
#include <algorithm> // std::move

#include <QNetworkReply>

#include "data/api_global.h" ///< UserSystemSettingsConfigInfo

/** hidapi info structure */
struct hid_device_info
{
    /** 设备路径 */
    char *path;
    /** Device Vendor ID */
    unsigned short vendor_id;
    /** Device Product ID */
    unsigned short product_id;
    /** 序列号 */
    wchar_t *serial_number;
    /** Device Release Number in binary-coded decimal,
                also known as Device Version Number */
    unsigned short release_number;
    /** 制造商信息 */
    wchar_t *manufacturer_string;
    /** 产品名称 */
    wchar_t *product_string;
    /** Usage Page for this Device/Interface
                (Windows/Mac only). */
    unsigned short usage_page;
    /** Usage for this Device/Interface
                (Windows/Mac only).*/
    unsigned short usage;
    /** The USB interface which this logical device
                represents. Valid on both Linux implementations
                in all cases, and valid on the Windows implementation
                only if the device contains more than one interface. */
    int interface_number;

    /** Pointer to the next device */
    struct hid_device_info *next;
};

// 函数指针定义
typedef int (*ActHID_IniDev)(unsigned short vid,
                             unsigned short pid,
                             unsigned char *reportid,
                             unsigned char *reportidnum);
typedef int (*ActHID_ReleaseDev)();
typedef int (*ActHID_DownFW)(const char *path, int iDevIndex);
typedef int (*ActHID_Write)(unsigned char *buf, int len);
typedef int (*ActHID_Read)(unsigned char reportID, unsigned char *buf, int len, int timeout, int cnt);

//hid_init hid_enumerate hid_free_enumeration hid_exit
typedef int (*hid_init_func)();
typedef struct hid_device_info *(*hid_enumerate_func)(unsigned short, unsigned short);
typedef void (*hid_free_enumeration_func)(struct hid_device_info *);
typedef int (*hid_exit_func)();

struct DevStatus
{
    int ConnectSta = 0;
    int VolumeLevel = 0;
    int Volume = 0;
    int electricity = 0;
    int EQMode = 0;
    int MicEn = 0;
    int MicListening = 0;
    int MicSta = 0;
};

class LoadLib
{
public:
    LoadLib();
    // 导出函数(函数指针实例)
    ActHID_IniDev pfn_ActHID_IniDev;
    ActHID_ReleaseDev pfn_ActHID_ReleaseDev;
    ActHID_DownFW pfn_ActHID_DownFW;
    ActHID_Write pfn_ActHID_Write;
    ActHID_Read pfn_ActHID_Read;

    hid_init_func pfn_hid_init;
    hid_enumerate_func pfn_hid_enumerate;
    hid_free_enumeration_func pfn_hid_free_enumeration;
    hid_exit_func pfn_hid_exit;

    unsigned char reportID[20] = {0};
    unsigned char reportIDnum = 0;
    // unsigned short PID = 0x4801;
    // unsigned short VID = 0x10d6;

    QHash<QString, QHash<QString, QString>> EnumeDev(); //枚举所有包含“XIBERIA”的设备名称与VID、PID

    int openCard();
    int openANDswitchCard();
    int closeCard();

    int GetDevStatus(DevStatus &sta);

    int SetEQ(int eq30,
              int eq60,
              int eq120,
              int eq250,
              int eq500,
              int eq1k,
              int eq2k,
              int eq4k,
              int eq8k,
              int eq16k);
    int NewSetEQ(int *Freq, int *Gain, int *q);
    int NewSetMicEQ(int *Freq, int *Gain, int *q);
    int SetEQMode(int Mode);
    int GetEQMode(int *Mode);
    int SetVolumeGain(int GainVal);
    int GetVolumeGain(int *GainVal);

    int SetMicEn(bool MicOpenEn);
    int SetMicGain(int AnalogGain, int DigitalGain);
    int SetMicGainEn(bool GainEn);
    int GetMicMsg(bool *MicOpenEn, int *AnalogGain, int *DigitalGain, bool *BoostEn);
    int SetMicListening(bool en);
    int GetMicListening(bool *en);
    int SetMicNoise(bool en);
    int GetMicNoise(bool *en);

    int SetBeepEn(bool en);
    int SetBeepVolume(int level);
    int GetBeepMsg(bool *en, int *level);

    int GetElectricity(int *level);

    int SetKey(int MuteKey, int MuteAct, int PlayKey, int PlayAct, int EqKey, int EqAct);
    int SetPlayKey(int PlayKey, int PlayAct);
    int SetMuteKey(int MuteKey, int MuteAct);
    int SetEqKey(int EqKey, int EqAct);
    int KeyReset();

    int GetVersion(char *DongleVer, char *EarVer);

    int write(unsigned char *buf, int len);
    int read(unsigned char *buf, int len, int timeout, int cnt);
};

#endif // LOADLIB_H
extern LoadLib *lolib;
extern QMessageBox msgBox;
extern bool isHidRun;
extern bool isApoRun;
extern QMutex mutex;
extern unsigned short SelDev_VID;
extern unsigned short SelDev_PID;
extern QString SelDev_DeviceName;
extern QString SelDev_DeviceGuid;
extern bool devSelEn; //是否打开设备选择界面

extern QString MyPlanName;//方案名称
extern QString MyPlanDesc;//方案描述
extern QStringList MyPlanLab1;//方案标签1
extern QString MyPlanLab2;//方案标签2
extern QString MyPlanType;//方案分类
extern int MyPlanTypeIdx;//方案分类ID
// 设置按钮列表（用于与UI设计中的按钮关联）
// extern QList<NewRadioBtn*> MyPlanRadioList;//我的预设（导入+自建），目的是方便系统和我的分开保存，不做其他使用
// extern QHash<QString, NewRadioBtn*> MyPlanRadioHash;

// extern QList<NewRadioBtn*> AllPlanRadioList;//所有预设（系统+导入+自建）
// extern QList<NewRadioBtn*> AllPlanRadioList_Load;
// extern QList<PlanVal> SysPlanVal_Init;
extern QHash<QString, PlanVal> SysPlanVal_Init;
extern int SysPlanVal_Index;

extern QHash<QString, NewRadioBtn *> AllPlanRadioHash_Load;

//根据八个标签，弄哈希表

// extern QHash<QString, NewRadioBtn*> PlanHashTables[8];
// extern QList<NewRadioBtn*> PlanRadioLists[8];

//动态保存
extern QSettings *globalSettings;
extern QFile *CeShiSettings;

extern QLabel *g_shareCodeCopyHint; /// 全局提示窗口（顶部标题栏下,水平居中显示）

//主窗体
extern MainWindow *m;

//extern bool ReadTemp;

extern QScrollArea *scrollArea_listen;

extern bool loop;
extern int micLevel;
extern bool swUpdateBtnClicked; //true:上位机检测更新按钮被点击    false:弹出发现新版本弹窗（软件自动查询）
extern int LanguageIdx;



extern QString SoftWareVer;       //当前上位机版本号
extern char DongleVer[31]; // 30字节 + \0
extern char EarVer[31];

extern bool UpdateTP1;
extern bool UpdateTP1En;//是否已经在升级TP1，若为false，先弹窗让用户确认
extern bool UpdateRxDone;//是否已升级完rx
extern bool UpdateTxDone;//是否已升级完tx

extern int iDevIndex;
extern QString readlastVer_RX; //读出的版本
extern QString readlastVer_TX; //读出的版本
extern HANDLE m_hMem;
extern unsigned char *m_pMem;
extern bool UpdateEn;

extern QString DevId;   //用户反馈-设备ID(服务器返回的绑定设备的id)
extern QString DriId;   //用户反馈-驱动ID
extern QString FWId;    //用户反馈-固件ID
extern QString DevType; //用户反馈-设备类型（耳机）

extern bool RetrievePlan; //重新获取方案（机型选择后，要根据机型显示方案）


///< 用户信息 相关
extern UserInformation g_user_information; ///< 用户信息相关

///< 更多设置 相关
extern UserSystemSettingsConfigInfo
    g_user_system_settings_config_info; ///< 用户系统设置相关信息(包括界面设置)
