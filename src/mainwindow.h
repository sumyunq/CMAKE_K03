#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QButtonGroup>
#include <QLabel>
#include <QMainWindow>
#include <QMouseEvent>
//翻译器
#include <QTranslator>

//系统音量头文件
#include <QMessageBox>
#include <QScrollArea>
#include "SpeakerEq.h"
#include "SpeakerListen.h"
#include "SpeakerSet.h"
#include "SystemVolumeMonitor/volume_monitor.h"
#include <endpointvolume.h>
#include <mmdeviceapi.h>

#include "modules/HomePage/home_page_main_page.h"       ///< 产品主页 头文件
#include "modules/UserSetting/user_setting_main_page.h" ///< 更多设置 头文件

#include "Community.h"    ///< 社区主页面  头文件

#include <QParallelAnimationGroup>
#include <QPropertyAnimation>

#include <QElapsedTimer>
#include <QFuture>

#include "Popup/UserGuide/UserGuide.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class DataSyncCoordinator; ///< 社区 ↔ 个人中心 联动同步器

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // static MainWindow* instance;          // 用于静态回调中获取对象

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QTimer *TimerR;
    QFutureWatcher<int> *readWatcher;
    unsigned char readBuffer[64]; // 用于存储读取数据的缓冲区

    int writeIni();
    int readIni();

    void Volume(EDataFlow dataFlow);

    void RealTimeSaveIni_ModeVal();
    void RealTimeSaveIni_SysPlan();
    void RealTimeSaveIni_SysPlanValInit();

    //     // 静态回调函数（供 DLL 调用）
    //     static void __stdcall XiberiaActionCallback(int result);   // 添加 __stdcall
    // public slots:
    //     // 保持原函数名，处理回调结果
    //     void OnXiberiaAction(int result);
    void Mapping();

    void UpdateUserSettingsConfig(
        const UserSystemSettingsConfigInfo &configInfo); ///< 更新部分用户默认系统配置

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitGlobalVars();    ///< 初始化全局可用变量,严格顺序,主线程执行
    void InitMember();        ///< 初始化内部成员, 非 ui-> 前缀 的控件成员也在此初始化
    void InitConnect();       ///< 连接默认的信号槽

private:
    Ui::MainWindow *ui;

    NewCustomToolTip *tip_connect;
    QTimer *timer_connect; //定时连接
    int m_remainCount;     //连接次数
    QVariantAnimation *m_colorAnimation;
    QString m_baseStyle_connect; // 保存 lab_Reconnect 的基础样式（不含 color 部分）

    void LoginAndInit();

    UserGuide *uGuide = nullptr;
    // QLabel *lab_Shadow;

    QFuture<void> m_windowStateFuture;

    // QScrollArea *scrollArea_listen;
    // QScrollArea *scrollArea_eq;
    SpeakerListen *widget_listenSpeaker;

    HomePageMainPage *cl_widget_home_main_page_ = nullptr;      ///< 产品主页
    UserSettingMainPage *clp_user_setting_main_page_ = nullptr; ///< 更多设置 主页面
    Community* clp_community_ = nullptr;                        ///< 社区主页面
    DataSyncCoordinator *clp_data_sync_ = nullptr;              ///< 社区 ↔ 个人中心 联动同步器

    QWidget *content_listen;
    QWidget *content_eq;

    void Initialize();
    // void CreateScrollArea();
    void closeInit();

    QPoint dragPosition;     //鼠标按下时，记录偏移位置（拖动起始地址）
    bool isDragging = false; //是否可以拖动窗体

    // 无边框窗体边缘拉伸（手动拉伸状态）
    Qt::Edges m_resizeEdges = Qt::Edges(); ///< 正在拉伸的边缘，空 = 未在拉伸
    bool m_resizeNative = false;           ///< 当前拉伸是否由 WM_NCLBUTTONDOWN 原生路径启动
    QPoint m_resizeStartPos;               ///< 拉伸起始鼠标全局坐标
    QRect m_resizeStartGeometry;           ///< 拉伸起始窗口几何
    void applyResizeDelta(const QPoint &delta); ///< 按鼠标总位移计算并应用新窗口几何（含最小尺寸钳制）

    int GetDevSta();

    QButtonGroup *group_Nav;

    QVariantMap modeValToVariantMap(const ModeVal &modeVal);
    ModeVal variantMapToModeVal(const QVariantMap &map, bool ifMerge);

    QVariantMap SysValToVariantMap(const SysVal &modeVal);
    SysVal variantMapToSysVal(const QVariantMap &map);

    QVariantMap SysValInitToVariantMap(const QHash<QString, PlanVal> &SysVal_Init);
    QHash<QString, PlanVal> variantMapToSysValInit(const QVariantMap &map);

    QPropertyAnimation *scaleAnimation;
    QPropertyAnimation *moveAnimation;
    QParallelAnimationGroup *animationGroup;

    void SetSpeakerMic();

    void onDefaultOutPutDeviceChanged(QString deviceName);
    void onDefaultInPutDeviceChanged(QString deviceName);
    void onDeviceDel(QString deviceName);
    void onDeviceAdd(QString deviceName);

    void ReadAllFile(); //读取所有ini文件

    void setupEightMyPlanSync(const QList<EightMyPlan *> &plans);

    void UpdateShadowLabelSize(QLabel *&labelOut);
    void createShadowLabel(QWidget *parent, QLabel *&labelOut);

    void updateSize();

    bool RefreshDeviceSupport(const QString &deviceGUID,
                              unsigned short &vId,
                              unsigned short &pId); //判断是否获得APO支持的设备

    void showDevSta(bool en, bool OTAEn);

    // QPixmap loadAvatar();    // WBLIU:预备移除

    void OpenAPOEffects();

    void showLogin();

    // QPropertyAnimation *animation_S;
    // QPropertyAnimation *animation_M;
    QSequentialAnimationGroup *group_S;
    QSequentialAnimationGroup *group_M;

    //异步动态保存数据到本地
    // 三个防抖定时器
    QTimer *m_timerSaveModeVal;
    QTimer *m_timerSaveSysPlan;
    QTimer *m_timerSaveSysPlanInit;
    QTimer *m_timerSaveHomePageEQ = nullptr;  ///< 首页算法值保存防抖（300ms singleShot）

    // 用于保护 globalSettings 的互斥锁
    QMutex m_saveMutex;

    // 异步保存的槽
    void onTriggerAsyncSaveModeVal();
    void onTriggerAsyncSaveSysPlan();
    void onTriggerAsyncSaveSysPlanInit();

    // 原来的保存函数（需要改成数据驱动，不再直接访问 UI）
    void writeModeValToSettings(const QVariantMap &movieVal);
    void writeSysPlanToSettings(const QVariantMap &sysPlanVal);
    void writeSysPlanInitToSettings(const QVariantMap &sysPlanInitVal);

    void LoginInEn(); //判断登录状态

    void saveHomePageExtraEQValue();
    void saveHomePageExtraEQValueSync();  ///< 首页算法值同步落盘（防抖 timeout / 退出兜底共用）
    void flushHomePageEQValueSave();      ///< 首页算法值退出兜底：停防抖 + 排空在途异步保存 + 同步落盘（closeEvent/closeInit 共用）

    void applyHomePagePlansSwitch(bool checked); ///< 首页方案开关应用：开=确保命中首页四方案并应用，关=关闭当前方案通道
    void closeHomePageAlgorithmSwitch();          ///< 关闭首页左侧算法开关（若已开启）
    void setHomePagePlansSwitchOpenSilently();    ///< 仅同步首页方案开关 UI/真源为开启，不触发 toggled 链

    void setThemeAndPanelTransparency(int idx,int PValue);//切换主题
    void setPanelTransparency(int idx,int PValue);//设置面板透明度
    void setBackgroundTransparency(int PValue);//设置背景透明度
    void setPanelBlur(int PValue);//设置面板模糊度
    void restoreBackgroundFromModel();          ///< 从当前背景配置恢复运行时壁纸缓存
    void updateBackgroundCache();             ///< 预缩放背景图缓存
    void scheduleBlurRebuild();               ///< 触发面板模糊防抖重算
    void rebuildPanelBlur();                  ///< 重算面板毛玻璃模糊缓存（防抖超时执行）

    void syncSpeakerMuteUi(bool muted, bool animate);    ///< 同步首页扬声器静音状态（muted=true 静音）
    void syncMicrophoneMuteUi(bool muted, bool animate); ///< 同步首页麦克风静音状态（muted=true 静音）

    void SetMicCurrentIndexChanged(int index);

public:
    void importDownloadedPlan(const QString &filePath); ///< 导入下载的方案文件到方案库（含提示+清理）
    void applyHomePresetPlan(const QPair<QString, QString> &planKey); ///< 首页四个快捷方案点击入口

protected:
    //实现鼠标拖拽窗体
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void closeEvent(QCloseEvent *e) override;

    void resizeEvent(QResizeEvent *event) override;

    // 无边框窗体边缘拉伸：WM_NCHITTEST（原生拉伸）+ WM_SETCURSOR（悬停光标），命中边缘/四角时交给系统
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void on_pBt_mini_clicked();
    void on_pBt_max_clicked(bool checked);

    // wbliu:旧版 麦克风开关（已迁移到 HomePageMainPage 中 cl_microphone_setting_->pBt_mic_switch_ 的 lambda connect）
    // void on_pBt_mic_switch_toggled(bool checked);

    void on_pBt_spk_toggled(bool checked);

    void on_pBt_mic_toggled(bool checked);

    // void on_hSlider_mic_valueChanged(int value);///< WBLIU: 旧版

    void Nav_toggled(int id, bool checked);

    void on_proBar_BatteryLevel_valueChanged(int value);



    void on_pBt_close_clicked();

    void onAvatarDoubleClicked(); ///< 头像双击事件

    void CloseTimer();

    void on_pBt_UserGuide_clicked();

    void on_pBt_Reconnect_clicked();

    void onTimeout_connect();

public slots:
    void LanguageSet();
    void Timer_readData();
    void onReadFinished(); //异步hid读取完成后
    void SelDevSuccess(const DeSheng::DeviceInfo &deviceInfo, const QRect &sourceGeometry);
    void onBackgroundTransparencyChanged(qreal opacity); ///< 界面设置 — 背景透明度变化
    void onPanelBlurChanged(qreal radius);               ///< 界面设置 — 面板模糊度变化
    void onBackgroundChanged(const QString &path);       ///< 界面设置 — 壁纸变更
    void onDefaultBackgroundRestored();                  ///< 界面设置 — 恢复默认背景

    void refreshUserDisplay(); ///< 刷新用户头像和昵称显示
    void showPanel();
    void closeSoftWare();

signals:
    void apoReady(); // 通知 APO 加载完成
    void updateVideoHoverPosition();

private:
    Qt::Edges edgesAtPos(const QPoint &pos) const; ///< 鼠标所在的窗口边缘（6px 内），非边缘返回空

    bool m_apoReady = false;
    bool m_adsReady = false;    // 广告列表及图片全部下载完成
    bool m_loginInitScheduled = false; ///< LoginAndInit 已调度（防重入：广告兜底置位后迟到的广告成功信号不得重复调度）
    QMetaObject::Connection m_speakerDisableOnReleaseConnection_;
    void tryProceedToLoginAndInit(); ///< 检查前置条件，满足后延时 2s 进入 LoginAndInit

    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *event) override;

    // 保持宽高比居中裁剪（覆盖），适配高 DPI 缩放
    const qreal dpr = devicePixelRatioF();
    QPixmap scaled = QPixmap(":/Skin/Images/home/background.png").scaled(size() * dpr,
                                                      Qt::KeepAspectRatioByExpanding,
                                                      Qt::SmoothTransformation);
};
#endif // MAINWINDOW_H

extern QTranslator tran; //翻译器
// extern int BatteryLevel;
// extern int BatteryUpdate;
