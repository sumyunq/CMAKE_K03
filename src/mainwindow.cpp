#include "mainwindow.h"
#include "modules/UserSetting/UserSettingSubModule/InterfaceSettings/InterfaceSettingCustomUI/custom_QScrollArea_background_images_component.h" ///< refreshList
#include <QDebug>
#include <QDir>
#include <QThreadPool>
#include <QSettings>
#include <QTimer>
#include "OnlineTime/OnlineDurationTracker.h"
#include "ui_mainwindow.h"
#include "modules/Common/AppImageCache.h"
#include "network/auth_store.h"
#include "modules/CommunityModule/ui/data_sync_coordinator.h" ///< 社区 ↔ 个人中心 联动同步
#include "modules/CommunityModule/ui/community/community_model.h" ///< CommunityModel
#include "modules/CommunityModule/ui/community_page_widget.h" ///< CommunityPageWidget（leftModel 完整类型）
#include "modules/CommunityModule/infrastructure/logger/logger.h" ///< LOG_INFO 关键操作埋点
//#include <QThread>

#include "APOThread/ApoManager.h"
#include "LoadApoDLL.h"
#include "LoadLib.h"
#include "SpeakerSet.h"

#include <QList>
#include <QMessageBox>
#include <QObject>

#include <QListView>
#include "DeviceManager/AudioDeviceChangeListener.h"
#include "DeviceManager/DefaultOutput.h"

#include <QFuture>
#include <QtConcurrent>

#include "network/http_client.h"
#include "network/request_options.h"

#include "Popup/bExitDirectly.h"
#include "SSL/SslCertManager.h"
#include <QFontDatabase>
#include <QPainter>
#include <QSignalBlocker>
#include <string>
#include <tchar.h>
#include <windows.h>
// 前向声明：resizeCursorId 定义在 nativeEvent 附近，mouseMoveEvent 先于定义使用
static LPCTSTR resizeCursorId(Qt::Edges t_edges);
//先选择机型，再登录，读取文件，上行初始化

LoadApoDLL *apo = new LoadApoDLL();
wchar_t *idSaved = nullptr;
const int RESULT_SUCCEED = 1;
LoadLib *lolib = new LoadLib();
bool isApoRun = false;
bool isHidRun = false;
//bool EQSetEn = false;

QMutex mutex;

bool MicOpenEn = true; //麦克风是否开启

// 调用Windows API设置系统扬声器音量
IAudioEndpointVolume *pEndpointVolume[2] = {NULL};
IMMDeviceEnumerator *pEnumerator = NULL;
IMMDevice *pDevice = NULL;

namespace {
constexpr int kDefaultAudibleVolume = 100;

int audioEndpointIndex(EDataFlow dataFlow)
{
    if (dataFlow == eRender) {
        return 0;
    }
    if (dataFlow == eCapture) {
        return 1;
    }
    return -1;
}

bool refreshDefaultEndpointVolume(EDataFlow dataFlow)
{
    const int index = audioEndpointIndex(dataFlow);
    if (index < 0) {
        return false;
    }

    CoInitialize(NULL);

    IMMDeviceEnumerator *enumerator = nullptr;
    IMMDevice *device = nullptr;
    IAudioEndpointVolume *endpointVolume = nullptr;

    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  __uuidof(IMMDeviceEnumerator),
                                  (void **) &enumerator);
    if (SUCCEEDED(hr)) {
        hr = enumerator->GetDefaultAudioEndpoint(dataFlow, eConsole, &device);
    }
    if (SUCCEEDED(hr)) {
        hr = device->Activate(__uuidof(IAudioEndpointVolume),
                              CLSCTX_INPROC_SERVER,
                              NULL,
                              (void **) &endpointVolume);
    }

    if (device) {
        device->Release();
    }
    if (enumerator) {
        enumerator->Release();
    }

    if (FAILED(hr) || !endpointVolume) {
        if (endpointVolume) {
            endpointVolume->Release();
        }
        qWarning() << "Failed to refresh default endpoint volume:" << hr;
        return false;
    }

    if (pEndpointVolume[index]) {
        pEndpointVolume[index]->Release();
    }
    pEndpointVolume[index] = endpointVolume;
    return true;
}

bool setEndpointMuteState(EDataFlow dataFlow, bool muted)
{
    const int index = audioEndpointIndex(dataFlow);
    if (index < 0) {
        return false;
    }
    if (!pEndpointVolume[index] && !refreshDefaultEndpointVolume(dataFlow)) {
        return false;
    }

    const HRESULT hr = pEndpointVolume[index]->SetMute(muted ? TRUE : FALSE, NULL);
    if (FAILED(hr)) {
        qWarning() << "Failed to set endpoint mute state:" << hr;
        return false;
    }
    return true;
}

bool setEndpointVolumeScalar(EDataFlow dataFlow, float volume)
{
    const int index = audioEndpointIndex(dataFlow);
    if (index < 0) {
        return false;
    }
    if (!pEndpointVolume[index] && !refreshDefaultEndpointVolume(dataFlow)) {
        return false;
    }

    const HRESULT hr = pEndpointVolume[index]->SetMasterVolumeLevelScalar(volume, NULL);
    if (FAILED(hr)) {
        qWarning() << "Failed to set endpoint volume:" << hr;
        return false;
    }
    return true;
}

std::wstring g_savedEndpointId;

void updateSavedEndpointId(const QString &deviceId)
{
    g_savedEndpointId = deviceId.toStdWString();
    idSaved = g_savedEndpointId.empty() ? nullptr
                                        : const_cast<wchar_t *>(g_savedEndpointId.c_str());
}
} // namespace

QString filePath; //,filePath_Read;

QSettings *globalSettings = NULL;

QFile *CeShiSettings = NULL;
QTextStream stream;

QSize normalSize = {0, 0};

AudioDeviceChangeListener *m_listener;

//bool ReadTemp = false;


QLabel *lab_Speakershadow_Top;    //扬声器关闭时，覆盖ui->widget_STop 的阴影
QLabel *lab_Speakershadow_Buttom; //扬声器关闭时，覆盖ui->widget_Speaker 的阴影
QLabel *lab_Micshadow_Top;        //麦克风关闭时，覆盖ui->widget_MTop 的阴影
QLabel *lab_Micshadow_Buttom;     //麦克风关闭时，覆盖ui->widget_Mic 的阴影

QScrollArea *scrollArea_listen = nullptr;

UserSystemSettingsConfigInfo g_user_system_settings_config_info;    ///< 部分用户配置信息

int micLevel = 0;

QLabel *g_shareCodeCopyHint = nullptr; ///< 用户分享码提示框

// bool IsActivated = false;//是否激活
// bool IsOnline = false;//是否联网
bool isLogin = false; //是否登录
bool swUpdateBtnClicked = false;
int LanguageIdx = 0;
bool DSevlBtnEn = false; //点击了主页的机型选择按钮

HANDLE m_hMem = NULL;
unsigned char *m_pMem = NULL;
bool UpdateEn = false;
bool UserGuideEn = true;//点击耳机时，是否显示使用指南
bool EqPage_FirstClicked = true;//第一次点击耳机页面

int SysPlanAdd_New = 0;   //存在新增时，判断。每次有新增数值都要累加1
int exists = 0;           //文件是否存在
bool AlreadyRead = false; //已经读取本地文件获得方案

QMovie *movie2;

// 设置圆形头像（QSS border-radius 不能裁剪 QLabel pixmap 内容，需手动裁剪）
static void setCircularAvatar(QLabel *t_label, const QPixmap &t_pixmap, int t_size)
{
    if (t_pixmap.isNull()) return;
    int t_src = qMin(t_pixmap.width(), t_pixmap.height());
    QPixmap t_square = t_pixmap.copy((t_pixmap.width() - t_src) / 2,
                                      (t_pixmap.height() - t_src) / 2, t_src, t_src);
    t_square = t_square.scaled(t_size, t_size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QPixmap t_circular(t_size, t_size);
    t_circular.fill(Qt::transparent);
    QPainter t_painter(&t_circular);
    t_painter.setRenderHint(QPainter::Antialiasing);
    t_painter.setBrush(t_square);
    t_painter.setPen(Qt::NoPen);
    t_painter.drawEllipse(0, 0, t_size, t_size);
    t_painter.end();
    t_label->setStyleSheet("");
    t_label->setFixedSize(t_size, t_size);
    t_label->setAlignment(Qt::AlignCenter);
    t_label->setPixmap(t_circular);
}

// // 静态实例初始化
// MainWindow* MainWindow::instance = nullptr;

void MainWindow::Mapping()
{
    int nSize = 128;
    TCHAR szName[520] = {0};
    _stprintf(szName, _T("ACTIONS_EXE_HID"));
    //用于创建或打开一个文件映射对象，0xFFFFFFFF代表创建一个基于内存的文件映射对象而非实际文件，可读可写
    m_hMem = ::CreateFileMapping((HANDLE) 0xFFFFFFFF, NULL, PAGE_READWRITE, 0, nSize, szName);
    if (m_hMem) {
        m_pMem = (unsigned char *)
                MapViewOfFile(m_hMem, FILE_MAP_ALL_ACCESS, 0, 0, 0); //映射文件到进程地址空间
        //qDebug("CreateFileMapping: %p; %p; %d\n", m_hMem, m_pMem, nSize);
        if (m_pMem) {
            memset(m_pMem, 0, nSize);
        } else
            CloseHandle(m_hMem);
    }
}

void MainWindow::UpdateUserSettingsConfig(const UserSystemSettingsConfigInfo &configInfo)
{
    g_user_system_settings_config_info = configInfo;
    if (clp_user_setting_main_page_) {
        clp_user_setting_main_page_->UpdateAllSubPageUIInformation();
    }
}

void MainWindow::InitUIInformation()
{
    {
        // 自身属性
        // setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
        setAttribute(Qt::WA_DeleteOnClose); // 添加这行
        //隐藏标题栏（无边框）| 运行程序时在任务栏上不显示程序图标 | 始终位于当前屏幕的最前面
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint
                       | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
        // setFixedSize(1200, 780);//固定大小
        resize(1200, 780);//自绘窗口缩小放大，拉伸
        setMinimumSize(1200, 780);// 无边框自绘窗口，保持初始尺寸，允许边缘拉伸放大（最小 1200x780）
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);// 解除 .ui 中 maximumSize(1211, ...) 对宽度的锁定，否则窗口无法拉伸
        setMouseTracking(true);// 边缘 NC 按下启动的拉伸需要无按钮状态的鼠标移动事件
    }

    {
        //隐藏上位机使用指南
        ui->pBt_UserGuide->hide();
    }

    {
        // 默认显示 启动动画界面
        ui->stackedWidget_SelMain->setCurrentWidget(ui->page_OpeningAnimation);
    }
    {
        // 安装事件过滤器到按钮
        ui->pBt_mini->setFocusPolicy(Qt::NoFocus);
        ui->pBt_max->setFocusPolicy(Qt::NoFocus);
        ui->pBt_close->setFocusPolicy(Qt::NoFocus);
        ui->pBt_max->installEventFilter(this);
    }
    {
        ui->lab_user_Avatar->setCursor(Qt::PointingHandCursor); //手型
    }
}

void MainWindow::InitGlobalVars()
{
    {
        m_listener = new AudioDeviceChangeListener(this);
        //pEnumerator->RegisterEndpointNotificationCallback(m_listener);
        connect(m_listener,
                &AudioDeviceChangeListener::defaultOutPutDeviceChanged,
                this,
                &MainWindow::onDefaultOutPutDeviceChanged);
        connect(m_listener,
                &AudioDeviceChangeListener::defaultInPutDeviceChanged,
                this,
                &MainWindow::onDefaultInPutDeviceChanged);
        connect(m_listener,
                &AudioDeviceChangeListener::defaultDeviceDel,
                this,
                &MainWindow::onDeviceDel);
        connect(m_listener,
                &AudioDeviceChangeListener::defaultDeviceAdd,
                this,
                &MainWindow::onDeviceAdd);
    }

    {
        /// 提示框
        g_shareCodeCopyHint = new QLabel(this);
        g_shareCodeCopyHint->setStyleSheet(R"(
    QLabel {
        font-family: "Noto Sans S Chinese";
                font-weight: 500;
        font-size: 14px;
        background: #0091DA;
        border-radius: 18px;
        color: white;
    }
)");
        g_shareCodeCopyHint->setAlignment(Qt::AlignCenter | Qt::AlignTop);
        g_shareCodeCopyHint->adjustSize();

        // 居中显示在父窗口
        QPoint center = this->rect().center();
        g_shareCodeCopyHint->setFixedSize(164, 36);
        g_shareCodeCopyHint->move(center.x() - g_shareCodeCopyHint->width() / 2, 69);
        g_shareCodeCopyHint->hide();
    }

    {
        //试听界面，整体界面可移动
        scrollArea_listen = new QScrollArea(ui->widget_listen);
        scrollArea_listen->setWidgetResizable(true); //不启用内容自适应
        // scrollArea_listen->setStyleSheet("border:0px; background-color: #202632;");
        // 隐藏滚动条
        scrollArea_listen->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 隐藏水平滚动条
        scrollArea_listen->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 隐藏垂直滚动条
    }
}

void MainWindow::InitMember()
{
    /// 产品主页相关
    {
        cl_widget_home_main_page_ = new HomePageMainPage(ui->page_main); // 产品主页
        ui->page_main->layout()->addWidget(cl_widget_home_main_page_);

        // 首页算法值保存防抖定时器：创建于构造函数（不依赖 Initialize——
        // 广告下载失败等导致 Initialize 挂起时，算法值修改仍能防抖落盘）
        m_timerSaveHomePageEQ = new QTimer(this);
        m_timerSaveHomePageEQ->setSingleShot(true);
        m_timerSaveHomePageEQ->setInterval(300);
        connect(m_timerSaveHomePageEQ, &QTimer::timeout, this, &MainWindow::saveHomePageExtraEQValue); // 防抖后异步落盘
        connect(cl_widget_home_main_page_,
                &HomePageMainPage::HomePageEQValueChange,
                this,
                [this]() { m_timerSaveHomePageEQ->start(); }); // 停拖后 300ms 落盘一次

        // 绑定 cl_widget_home_main_page_ 相关信号槽
        connect(cl_widget_home_main_page_->cl_product_display_->cl_pBt_devSel_,
                &QPushButton::clicked,
                this,
                [=]() {
            DSevlBtnEn = true;
            ui->stackedWidget_SelMain->setCurrentWidget(ui->page_devSel);
        }); // 点击机型选择
        connect(cl_widget_home_main_page_->cl_product_display_->cl_pBt_SysVloSet_,
                &QPushButton::clicked,
                this,
                [=]() {
            //传统控制面板
            bool success = QProcess::startDetached("control.exe", {"mmsys.cpl"});

            if (!success) {
                // Windows 10/11的URI方案
                QProcess::startDetached("cmd.exe", {"/c", "start ms-settings:sound"});
            }
        });
        //点击说明书
        //点击说明书 — 打开当前选中设备的说明书 URL（DeviceInfo.DeviceManualUrl，DeviceRegistry 按型号配置）
        connect(cl_widget_home_main_page_->cl_product_display_->cl_pBt_UserGuide_, &QPushButton::clicked, this, [this]() {
            const auto &t_info = ui->page_devSel->clp_device_selection_mainPage_
                                     ->cl_selected_device_information();
            if (t_info.DeviceManualUrl.isEmpty())
                return;  ///< 未配置（K03S 普通版 / T7 GT 正式链接未给）→ 不打开
            QUrl t_url(t_info.DeviceManualUrl);
            QDesktopServices::openUrl(QUrl::fromEncoded(t_url.toEncoded()));
        });
        // 点击声音设置
        // WBLIU: 新版 扬声器开关
        // 绑定扬声器开关 toggled 信号
        connect(cl_widget_home_main_page_->cl_speaker_setting_->pBt_spk_switch_,
                &QPushButton::toggled,
                this,
                [=](bool checked) {
            setEndpointMuteState(eRender, !checked);
            syncSpeakerMuteUi(!checked, true);
        });
        // WBLIU: 新版 麦克风开关
        // 绑定麦克风开关 toggled 信号
        connect(cl_widget_home_main_page_->cl_microphone_setting_->pBt_mic_switch_,
                &QPushButton::toggled,
                this,
                [=](bool checked) {
            auto *microphoneSlider = cl_widget_home_main_page_->cl_microphone_setting_
                    ->cl_mic_hSlider_;
            if (checked && microphoneSlider && microphoneSlider->value() <= 0) {
                microphoneSlider->setValue(kDefaultAudibleVolume);
            }
            setEndpointMuteState(eCapture, !checked);
            syncMicrophoneMuteUi(!checked, true);
        });
        // WBLIU: 麦克风变化滚动条
        connect(cl_widget_home_main_page_->cl_microphone_setting_->cl_mic_hSlider_,
                &NewHSlider::valueChanged,
                this,
                [=](int value) {
            float volume = value / 100.0f;
            setEndpointVolumeScalar(eCapture, volume);
            const bool muted = value <= 0;
            setEndpointMuteState(eCapture, muted);
            syncMicrophoneMuteUi(muted, false);
            micLevel = value;
        });

        // WBLIU: 新版 麦克风设备切换
        connect(cl_widget_home_main_page_->cl_microphone_setting_->cl_cBox_Mic_,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                this,
                [=](int index) {
            if (index != -1) {
                QString deviceId = cl_widget_home_main_page_->cl_microphone_setting_
                        ->cl_cBox_Mic_->itemData(index)
                        .toString();
                if (DefaultOutput::changeDevice(deviceId)) {
                    refreshDefaultEndpointVolume(eCapture);
                }

                updateSavedEndpointId(deviceId);
                if (apo->IsSupportEP()) {
                    qDebug("设备支持上行操作\n");
                    ui->widget_mic->setUpEn(true);
                } else {
                    qDebug("设备不支持上行操作\n");
                    //麦克风AI降噪不可用
                    ui->widget_mic->setUpEn(false);
                }
            }
        });
        // WBLIU: 音量变化滚动条
        connect(cl_widget_home_main_page_->cl_speaker_setting_->cl_HSlider_,
                &NewHSlider::valueChanged,
                this,
                [=](int value) {
            float volume = value / 100.0f;
            //qDebug("扬声器：val:%d,volume:%f\n",value,volume);
            setEndpointVolumeScalar(eRender, volume);
            const bool muted = value <= 0;
            setEndpointMuteState(eRender, muted);
            syncSpeakerMuteUi(muted, false);
            ui->label_level->setText(QString::number(value));
        });

        // WBLIU: 预留 旧版下拉框
        connect(cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_,
                static_cast<void (QComboBox::*)(int)>(
                    &QComboBox::currentIndexChanged), // 明确信号类型
                this,
                [=](int index) {
            if (index != -1) {
                QString deviceId = cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_
                        ->itemData(index)
                        .toString();

                if (deviceId.isEmpty()) {
                    deviceId = cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_
                            ->itemText(index);
                }

                SelDev_DeviceGuid = deviceId;
                if (DefaultOutput::changeDevice(deviceId)) {
                    refreshDefaultEndpointVolume(eRender);
                }
                if (SelDev_DeviceGuid != "0") {
                    emit ApoManager::instance()->requestSetLhdcDevice(SelDev_DeviceGuid);
                }
            }
        });

        // 首页 人声清晰 按键
        connect(cl_widget_home_main_page_->cl_microphone_adjustment_->cl_pushButton_clear_voices_,
                &QPushButton::toggled,
                this,
                [=](bool checked) { ui->widget_mic->change_pBt_ClearVocals(checked); });

        // 首页 人声浑厚 按键
        connect(cl_widget_home_main_page_->cl_microphone_adjustment_
                ->cl_pushButton_deepPowerful_voice_,
                &QPushButton::toggled,
                this,
                [=](bool checked) { ui->widget_mic->change_pBt_RichVocals(checked); });
        // 同步 人声清晰 按键（ widget_mic 内部非公开按键）
        connect(ui->widget_mic, &MicSet::pBt_ClearVocals_changed, this, [=](bool checked) {
            QSignalBlocker blocker(
                        cl_widget_home_main_page_->cl_microphone_adjustment_->cl_pushButton_clear_voices_);
            cl_widget_home_main_page_->cl_microphone_adjustment_->cl_pushButton_clear_voices_
                    ->setChecked(checked);
        });

        // 同步 人声浑厚 按键（widget_mic 内部非公开按键）
        connect(ui->widget_mic, &MicSet::pBt_RichVocals_changed, this, [=](bool checked) {
            // QSignalBlocker blocker(cl_widget_home_main_page_->cl_microphone_adjustment_->cl_pushButton_deepPowerful_voice_);
            // 阻塞信号但不阻塞 UI 更新
            QSignalBlocker blocker(cl_widget_home_main_page_->cl_microphone_adjustment_
                                   ->cl_pushButton_deepPowerful_voice_);
            cl_widget_home_main_page_->cl_microphone_adjustment_->cl_pushButton_deepPowerful_voice_
                    ->setChecked(checked);
        });

        // 首页 算法 开关按键
        connect(
                    cl_widget_home_main_page_->cl_algorithm_adjustment_setting->cl_customPushButton_,
                    &QPushButton::toggled,
                    this,
                    [=](bool checked) {
            if (checked) {
                if (currentPlanRadio != nullptr) {
                    // 强制关闭当前预设的 各个 控制
                    currentPlanVal.AlgoOpenEn = false;
                    currentPlanVal.eqOpenEn = false;
                    currentPlanVal.spaceOpenEn = false;
                    currentPlanVal.drcOpenEn = false;
                    currentPlanRadio->updateAllPlanValue(currentPlanVal);
                    currentPlanRadio->setChecked(false);

                    ui->widget_eq->SetEQSwitchShadow(false, 0); // 关闭均衡器
                    ui->widget_eq->SetEQSwitchShadow(false, 2); // 关闭空间
                }

                //额外EQ(总开关)
                emit ApoManager::instance()->requestSetExtendEqState(2, checked); //脚步增强
                emit ApoManager::instance()->requestSetExtendEqState(3, checked); //枪声优化
                emit ApoManager::instance()->requestSetExtendEqState(4, checked); //声场控制
                emit ApoManager::instance()->requestSetExtendEqState(5, checked); //清晰度

                // 其他算法关闭
                emit ApoManager::instance()->requestSetExtendEqState(6, false); //余音消除
                emit ApoManager::instance()->requestSetExtendEqState(7, false); //空间混响
                emit ApoManager::instance()->requestSetExtendEqState(8, false); //风声弱化

                if (cl_widget_home_main_page_->cl_plans_selection_->cl_customPushButton_
                        ->isChecked()) {
                    cl_widget_home_main_page_->cl_plans_selection_->cl_customPushButton_
                            ->setChecked(false);
                }
            } else {
                // 检测 方案库是否开启
                if (cl_widget_home_main_page_->cl_plans_selection_->cl_customPushButton_
                        ->isChecked()) {
                    // 不修改apo开关
                } else {
                    emit ApoManager::instance()->requestSetExtendEqState(2, checked); //脚步增强
                    emit ApoManager::instance()->requestSetExtendEqState(3, checked); //枪声优化
                    emit ApoManager::instance()->requestSetExtendEqState(4, checked); //声场控制
                    emit ApoManager::instance()->requestSetExtendEqState(5, checked); //清晰度
                }
            }
            HomePageExtraEQOpen = checked;
            globalSettings->setValue("HomeMainPage/ExtraOpen", HomePageExtraEQOpen);
            cl_widget_home_main_page_->cl_algorithm_adjustment_setting->setEditStatus(checked);
            // 异步写入文件
            QtConcurrent::run([this]() {
                QMutexLocker locker(&m_saveMutex); // 加锁，保证 QSettings 线程安全
                globalSettings->sync();
            });
        });

        // 首页 预设方案 开关按键（应用逻辑见 applyHomePagePlansSwitch：开=按标志应用，关=按标志关闭，不切方案）
        connect(
                    cl_widget_home_main_page_->cl_plans_selection_->cl_customPushButton_,
                    &QPushButton::toggled,
                    this,
                    [this](bool checked) {
            applyHomePagePlansSwitch(checked);

            ui->widget_eq->set_pBt_EQSwitch_hideData_checked(checked);
            HomePagePlansOpen = checked;
            globalSettings->setValue("HomeMainPage/PlansOpen", HomePagePlansOpen);
            // 异步写入文件
            QtConcurrent::run([this]() {
                QMutexLocker locker(&m_saveMutex); // 加锁，保证 QSettings 线程安全
                globalSettings->sync();
            });
        });

        // 方案按键点击 → 统一由 MainWindow 串行处理：关算法、开方案开关、选中方案。
        {
            QObject::connect(cl_widget_home_main_page_->cl_plans_selection_,
                             &CustomQWidgetPlansSelection::requestHomePlanSelected,
                             this,
                             &MainWindow::applyHomePresetPlan);
        }
    }

    /// 更多设置
    {
        clp_user_setting_main_page_ = new UserSettingMainPage(ui->widget_more);
        ui->widget_more->layout()->addWidget(clp_user_setting_main_page_);
        connect(clp_user_setting_main_page_,&UserSettingMainPage::CloseReceiveTimer_S,this,&MainWindow::CloseTimer);
    }

    /// 试听内容区域以及试听相关信号,迁移自 this->CreateScrollArea()
    {
        if (!ui->widget_listen) {
            emit ApoManager::instance()->requestlogWithTime(
                        "CreateScrollArea widget_listen 不存在");
        }

        // 创建内容区域
        content_listen = new QWidget();
        content_listen->setObjectName("content_listen");
        content_listen->setAutoFillBackground(true);
        content_listen->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        //栅格布局
        QGridLayout *l_layout = new QGridLayout(content_listen);

        //l_layout->setSpacing(0);                   // 清除控件间距
        l_layout->setContentsMargins(0, 0, 0, 0); // 清除默认边距

        /// 试听界面
        widget_listenSpeaker = new SpeakerListen(content_listen);
        widget_listenSpeaker->cl_sound_test_main_page_->cl_ffmpeg_main_page_->minView
                = std::make_shared<VideoHover>(this);
        widget_listenSpeaker->cl_sound_test_main_page_->cl_ffmpeg_main_page_->minView
                ->hide(); ///画中画-小窗口, 默认隐藏
        widget_listenSpeaker->cl_sound_test_main_page_->cl_ffmpeg_main_page_->minView
                ->move(rect().center().x(), 124); /// 中偏右

        widget_listenSpeaker->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        l_layout->addWidget(widget_listenSpeaker, 0, 0); // 明确指定行列位置

        content_listen->setLayout(l_layout);

        content_listen->setMinimumSize(widget_listenSpeaker->minimumSizeHint());
        scrollArea_listen->setWidget(content_listen);
        //content_listen->setMinimumSize(1, 1);  // 防止内容区域尺寸为0
        //垂直布局
        QGridLayout *mainLayout_listen = new QGridLayout(ui->widget_listen);
        mainLayout_listen->setContentsMargins(0, 0, 0, 0); // 可选：设置边距
        //mainLayout_listen->setSpacing(0);
        mainLayout_listen->addWidget(scrollArea_listen, 0, 0); // 添加到(0,0)
        //mainLayout_listen->setRowStretch(0, 1);  // 设置行拉伸因子
        //mainLayout_listen->setColumnStretch(0, 1); // 设置列拉伸因子
        //qDebug("111ScrollArea height: %d,content_listen height:%d\n", scrollArea_listen->height(),content_listen->height());
        // 关键修复4：确保父容器正确设置布局
        ui->widget_listen->setLayout(mainLayout_listen); // 明确设置布局
    }

    // 社区主页面
    {
        clp_community_ = new Community(ui->page_community);
        ui->page_community->layout ()->addWidget (clp_community_);
    }

    // 社区 ↔ 个人中心 联动同步（共享 Service + 5-model）
    {
        if (clp_community_ && clp_community_->mainPage() && clp_user_setting_main_page_
            && clp_user_setting_main_page_->personalCenterPage()) {
            auto *t_cmp = clp_community_->mainPage();
            auto *t_pc = clp_user_setting_main_page_->personalCenterPage();
            t_pc->injectServices(t_cmp->schemeService(), t_cmp->configRepo());
            if (!clp_data_sync_) {
                clp_data_sync_ = new DataSyncCoordinator(this);
                CommunityModel *t_left[3] = {
                    t_cmp->pageWidget()->leftModel(0),
                    t_cmp->pageWidget()->leftModel(1),
                    t_cmp->pageWidget()->leftModel(2)};
                clp_data_sync_->init(t_cmp->schemeService(), t_cmp->configRepo(),
                                     t_left,
                                     t_pc->uploadedModel(), t_pc->likedModel(),
                                     t_pc->uploadedPanel(), t_pc->likedPanel());
            }
        }
    }
}

void MainWindow::InitConnect()
{
    /// widget_listenSpeaker 相关信号
    {
        //跳转到其他界面
        connect(widget_listenSpeaker,
                &SpeakerListen::ChangeToSpeakerPage,
                this,
                [this](int index, bool ShowVH, int VHIdx, float currentPos) {
            //0:跳转page_Sperker 1：跳转到page_Sperker的方案库界面 3:均衡器界面  4:试听选择界面
            if (index == 4) {
                /// 跳转 至试听 播放界面
                ui->stackedWidget->setCurrentWidget(ui->page_listen);
            }

            if (index != 4) {
                ui->stackedWidget->setCurrentWidget(ui->page_Sperker);
            }

            if (index == 1) {
                ui->stackedWidget->setCurrentWidget(ui->page_Sperker);
                // ui->page_Sperker->on_rBt_currentPlan_clicked();
            } else if (index == 3) {
                if (currentPlanRadio == NULL) {
                    ui->stackedWidget->setCurrentWidget(ui->page_Sperker);
                } else {
                    //跳转到均衡器界面
                    // ui->page_Sperker->on_pBt_ClosePlanPage_clicked();
                    ui->widget_eq->SwitchEqPage();
                    ui->widget_eq->ShowEqVal(currentPlanVal.DataVisibleEn);
                    ui->widget_eq->PageShowPlanVal();
                    ui->stackedWidget->setCurrentWidget(ui->page_eq);
                }
            }
        });

        //一起改变IndicatorText
        QObject::connect(ui->page_Sperker->currentPlan_s,
                         &NewRadioBtnText::SetITextSignal,
                         widget_listenSpeaker->currentPlan_l,
                         &NewRadioBtnText::setIndicatorText);
        // QObject::connect(ui->page_Sperker->eightPlan_s, &EightMyPlan::ShowBtn,
        //                  widget_listenSpeaker->eightPlan_l, &EightMyPlan::ShowFirstEight);
        QObject::connect(ui->page_Sperker->eightPlan_s,
                         &EightMyPlan::ShowBtn,
                         widget_listenSpeaker->eightPlan_l,
                         &EightMyPlan::ShowEightFavorite);
        QObject::connect(ui->page_Sperker->eightPlan_s,
                         &EightMyPlan::RenameBtn,
                         widget_listenSpeaker->eightPlan_l,
                         &EightMyPlan::Rename);

        //跳转到试听界面
        connect(ui->page_Sperker, &SpeakerSet::pageChange, this, [this] {
            ui->stackedWidget->setCurrentWidget(ui->page_listen);
        });

        //跳转到预设库界面
        connect(ui->widget_eq, &SpeakerEq::CurrentpageChange, this, [this] {
            ui->stackedWidget->setCurrentWidget(ui->page_Sperker);
            // ui->page_Sperker->on_rBt_currentPlan_clicked();
        });
        //跳转到试听界面
        connect(ui->widget_eq, &SpeakerEq::ListenpageChange, this, [this] {
            ui->stackedWidget->setCurrentWidget(ui->page_listen);
        });

        // 截断信号 控制按键显示（先更新真源再同步 UI，避免显示滞后一个状态）
        connect(ui->widget_eq,
                &SpeakerEq::UpdateHomePageUIInfo,
                cl_widget_home_main_page_->cl_plans_selection_,
                [=](bool targetStatus) {
            HomePagePlansOpen = targetStatus;
            // 截断信号，只更新UI
            QSignalBlocker blocker(cl_widget_home_main_page_->cl_plans_selection_
                                       ->cl_customPushButton_);
            cl_widget_home_main_page_->cl_plans_selection_->cl_customPushButton_->setChecked(
                        HomePagePlansOpen);
            globalSettings->setValue("HomeMainPage/PlansOpen", HomePagePlansOpen);
            // 异步写入文件
            QtConcurrent::run([this]() {
                QMutexLocker locker(&m_saveMutex); // 加锁，保证 QSettings 线程安全
                globalSettings->sync();
            });
        });

        //保存
        connect(ui->widget_eq, &SpeakerEq::PlanSave_E, ui->page_Sperker, &SpeakerSet::PlanSave);
        //重置
        connect(ui->widget_eq, &SpeakerEq::PlanReset_E, ui->page_Sperker, &SpeakerSet::ResetValue);
        //创建二创方案
        connect(ui->widget_eq, &SpeakerEq::CreateDerivPlan, ui->page_Sperker, [this]() {
            ui->stackedWidget->setCurrentWidget(ui->page_Sperker);
            ui->page_Sperker->CreateDerivPlanSlot();
        });

        //一起改变IndicatorText
        QObject::connect(ui->page_Sperker->currentPlan_s,
                         &NewRadioBtnText::SetITextSignal,
                         ui->widget_eq->currentPlan_e,
                         &NewRadioBtnText::setIndicatorText);
        // QObject::connect(ui->page_Sperker->eightPlan_s, &EightMyPlan::ShowBtn,
        //                  ui->widget_eq->eightPlan_e, &EightMyPlan::ShowFirstEight);
        QObject::connect(ui->page_Sperker->eightPlan_s,
                         &EightMyPlan::ShowBtn,
                         ui->widget_eq->eightPlan_e,
                         &EightMyPlan::ShowEightFavorite);
        QObject::connect(ui->page_Sperker->eightPlan_s,
                         &EightMyPlan::RenameBtn,
                         ui->widget_eq->eightPlan_e,
                         &EightMyPlan::Rename);

        setupEightMyPlanSync({ui->page_Sperker->eightPlan_s,
                              widget_listenSpeaker->eightPlan_l,
                              ui->widget_eq->eightPlan_e});

        //跳转到EQ界面
        connect(ui->page_Sperker, &SpeakerSet::EQpageChange, this, [this](bool ShowEqEn) {
            emit ApoManager::instance()->requestlogWithTime(
                        QString("EQpageChange ShowEqVal :%1").arg(ShowEqEn));
            ui->widget_eq->SwitchEqPage();
            ui->widget_eq->ShowEqVal(ShowEqEn);
            ui->widget_eq->ShowcurrentPlanVal();

            ui->stackedWidget->setCurrentWidget(ui->page_eq);
        });

        //点击收藏方案跳转到EQ界面
        connect(ui->page_Sperker, &SpeakerSet::FavEQpageChange, this, [this](bool ShowEqEn) {
            ui->widget_eq->SwitchEqPage();
            ui->widget_eq->ShowEqVal(ShowEqEn);
            ui->widget_eq->PageShowPlanVal();

            ui->stackedWidget->setCurrentWidget(ui->page_eq);
            //QString itemText = ui->treeWidget->currentItem()->text(0);
            qDebug("跳转到方案库界面\n");
        });

        //设置APO数值，不跳转到均衡器界面
        connect(ui->page_Sperker, &SpeakerSet::SetApoVal, this, [this]() {
            apo->logWithTime("ui->widget_eq->ShowcurrentPlanVal();前");
            ui->widget_eq->ShowcurrentPlanVal();
        });

        //重置
        connect(ui->page_Sperker, &SpeakerSet::PlanReset_S, ui->widget_eq, &SpeakerEq::resetVal);

        // 原始信号 → 启动对应的防抖定时器
        //动态保存, 我的预设的方案,onTriggerAsyncSaveModeVal
        connect(ui->page_Sperker,&SpeakerSet::RealTimeSaveModeVal_S,m_timerSaveModeVal,qOverload<>(&QTimer::start));
        //动态保存，系统预设的方案,onTriggerAsyncSaveSysPlan
        connect(ui->page_Sperker,&SpeakerSet::RealTimeSaveSysPlan_S,m_timerSaveSysPlan,qOverload<>(&QTimer::start));
        //动态保存，所有预设方案的初始值,onTriggerAsyncSaveSysPlanInit
        connect(ui->page_Sperker,&SpeakerSet::RealTimeSaveSysPlanValInit_S,m_timerSaveSysPlanInit,qOverload<>(&QTimer::start));

        // connect(ui->page_Sperker, &SpeakerSet::RealTimeSaveModeVal_S, this, [this] {
        //     {
        //         RealTimeSaveIni_ModeVal();
        //     }
        // });

        // connect(ui->page_Sperker, &SpeakerSet::RealTimeSaveSysPlan_S, this, [this] {
        //     {
        //         RealTimeSaveIni_SysPlan();
        //     }
        // });

        // connect(ui->page_Sperker, &SpeakerSet::RealTimeSaveSysPlanValInit_S, this, [this] {
        //     {
        //         RealTimeSaveIni_SysPlanValInit();
        //     }
        // });

        // connect(ui->page_Sperker, &SpeakerSet::PlanReset_S, this, [this] {
        //     ui->widget_eq->ShowcurrentPlanVal();
        // });
    }

    // 更多设置界面中，点击退出登录弹窗，并点击确定
    {
        connect(clp_user_setting_main_page_->clp_dialog_tips_,
                &CustomQDialogGeneralTips::confirmed,
                this,
                [this]() {
            // 调用退出登录 API
            if (!g_user_information.network.access_token.isEmpty()) {
                QNetworkReply *t_reply = HttpClient::instance().post(
                            "/user/logout",
                            RequestOptions{}.withTag("user"));
                connect(t_reply, &QNetworkReply::finished, this, [t_reply]() {
                    t_reply->deleteLater();
                });
            }

            // 先存盘再清内存 — 自定义壁纸数据需保留供下次登录恢复
            // 等待所有 pending 的 saveToDiskAsync 完成，避免旧快照覆盖最新数据
            QThreadPool::globalInstance()->waitForDone();
            g_user_information.saveToDisk();

            // 清除用户信息
            g_user_information.network = {};
            AuthStore::instance().clear(); ///< 清空新栈 token 缓存
            g_user_information.local.user_psw.clear();
            g_user_information.local.is_get_userInfo_first.store(true);

            // 退出登录只清认证态，不清本地壁纸数据。
            // 自定义壁纸 JSON 依赖 custom_wallpaper_map，清空内存后如果后续保存会覆盖成空列表。
            // 壁纸缓存重置暂不执行（产品暂定：退出登录保留当前背景）

            // 清除持久化登录状态
            globalSettings->setValue("Login/en", false);
            globalSettings->remove("Login/type");
            globalSettings->remove("Login/nickname");
            globalSettings->remove("Login/id");
            globalSettings->remove("Login/Account");

            // 清空列表缓存
            clp_user_setting_main_page_->clp_personal_center_settings_main_page_->resetData();

            // 跳转机型选择界面（devSelEn=true 时启动/切换流程进入机型选择页）
            devSelEn = true;
            ui->page_devSel->DevSelInitialization();
            ui->stackedWidget_SelMain->setCurrentWidget(ui->page_devSel);
        },
        Qt::UniqueConnection);
        // 更新翻译
        connect(clp_user_setting_main_page_->clp_interface_settings_main_page_, &InterfaceSettingsMainPage::languageChange, this, &MainWindow::LanguageSet);

        // 背景透明度
        connect(clp_user_setting_main_page_->clp_interface_settings_main_page_, &InterfaceSettingsMainPage::backgroundTransparencyChanged, this, &MainWindow::onBackgroundTransparencyChanged);
        // 面板模糊度
        connect(clp_user_setting_main_page_->clp_interface_settings_main_page_, &InterfaceSettingsMainPage::panelBlurChanged, this, &MainWindow::onPanelBlurChanged);
        // 壁纸变更
        connect(clp_user_setting_main_page_->clp_interface_settings_main_page_,
                &InterfaceSettingsMainPage::backgroundChanged,
                this, &MainWindow::onBackgroundChanged,
                Qt::UniqueConnection);

        // 恢复默认背景
        connect(clp_user_setting_main_page_->clp_interface_settings_main_page_,
                &InterfaceSettingsMainPage::defaultBackgroundRestored,
                this, &MainWindow::onDefaultBackgroundRestored,
                Qt::UniqueConnection);

    }

    {
        connect (this,&MainWindow::updateVideoHoverPosition,widget_listenSpeaker,[=](){
            widget_listenSpeaker->updateVideoHoverPosition ();// 更新小窗口位置
        });
    }
    // 头像双击事件
    ui->lab_user_Avatar->installEventFilter(this);
}

// //加载微信头像
// QPixmap MainWindow::loadAvatar()
// {
//     // 构建保存时的相同路径
//     // QString avatarPath = QApplication::applicationDirPath() + "/ProgramData/avatar.png";

//     QString avatarPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).first()
//             + "/ProgramData/avatar.png";

//     QPixmap pixmap;

//     if (QFile::exists(avatarPath)) {
//         if (pixmap.load(avatarPath)) {
//             qDebug() << "头像加载成功:" << avatarPath;
//         } else {
//             qDebug() << "头像加载失败: 文件可能损坏:" << avatarPath;
//         }
//     } else {
//         qDebug() << "头像文件不存在:" << avatarPath;
//     }

//     return pixmap; // 如果失败，返回空QPixmap
// }

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    setAttribute(Qt::WA_DeleteOnClose);

    QString currentDate = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    QString CeShiPath = QApplication::applicationDirPath()
            + QString("/Debug.%1.log").arg(currentDate);

    //删除两天前的日志文件
    QDate today = QDate::currentDate();
    QDir logDir(QApplication::applicationDirPath());

    for (const QFileInfo &info : logDir.entryInfoList({"Debug.*.log"}, QDir::Files)) {
        QDate fileDate = QDate::fromString(info.fileName().mid(6, 10), "yyyy-MM-dd");
        if (fileDate.isValid() && fileDate.daysTo(today) > 1) {
            QFile::remove(info.absoluteFilePath());
        }
    }
    QString twoDaysAgoLogPath = QApplication::applicationDirPath() + QString("/Debug.log");
    QFile::remove(twoDaysAgoLogPath); // 删除Debug.log（如果存在）

    CeShiSettings = new QFile(CeShiPath);
    CeShiSettings->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    stream.setDevice(CeShiSettings);
    stream.setCodec(QTextCodec::codecForName("UTF-8"));

    /*apo->LoadApoLibrary();

    //APO
    QFuture<void> m_future;
    m_future = QtConcurrent::run([]() {
        // 创建对象或获取单例
        if (!apo) {
            //qWarning() << "Failed to create LoadApoDLL instance.";
            emit ApoManager::instance()->requestlogWithTime("Failed to create LoadApoDLL instance.");
            return; // void，直接返回
        }else
        {
            emit ApoManager::instance()->requestlogWithTime("apo Existence");
        }
        // 调用初始化函数
        int ret = apo->InitialDependencyResource();
        if(ret == RESULT_SUCCEED)
        {
            emit ApoManager::instance()->requestlogWithTime("InitialDependencyResource success");
            //初始化是否成功
            retB = apo->IsInitialDependencyResource();
            if(!retB)
            {
                emit ApoManager::instance()->requestlogWithTime("IsInitialDependencyResource failed");
                return;
            }
        }else
        {
            emit ApoManager::instance()->requestlogWithTime("InitialDependencyResource failed");
            retB = false;
            return;
        }

    });
    m_future.waitForFinished();
    if(!retB)
    {
        msgBox.critical(NULL,tr("错误"),tr("加载驱动失败！"));
        // exit(0);
        // return;
    }else
    {
        // qDebug("MainPageChange加载驱动成功\n");
        emit ApoManager::instance()->requestlogWithTime("MainPageChange加载驱动成功");
    }*/
    apo->LoadApoLibrary();

    // 用一个成员变量标记是否已完成加载
    m_apoReady = false;

    // APO 初始化改为完全异步，不 wait
    QFuture<bool> future = QtConcurrent::run([]() -> bool {
        if (!apo) {
            emit ApoManager::instance()->requestlogWithTime(
                        "Failed to create LoadApoDLL instance.");
            return false;
        }
        emit ApoManager::instance()->requestlogWithTime("apo Existence");

        int ret = apo->InitialDependencyResource();
        if (ret == RESULT_SUCCEED) {
            emit ApoManager::instance()->requestlogWithTime("InitialDependencyResource success");
            if (!apo->IsInitialDependencyResource()) {
                emit ApoManager::instance()->requestlogWithTime(
                            "IsInitialDependencyResource failed");
                return false;
            }
        } else {
            emit ApoManager::instance()->requestlogWithTime("InitialDependencyResource failed");
            return false;
        }
        return true;
    });

    // 用 QFutureWatcher 监视任务完成
    auto *watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher]() {
        retB = watcher->result();
        if (!retB) {
            msgBox.critical(nullptr, tr("错误"), tr("加载驱动失败！"));
            // 可以决定是否退出
        } else {
            emit ApoManager::instance()->requestlogWithTime("MainPageChange加载驱动成功");
        }
        m_apoReady = true; // 标记 APO 就绪
        emit apoReady();   // 发出信号（如果需要在 LoginAndInit 里等待）
        watcher->deleteLater();
    });
    watcher->setFuture(future);

    ui->setupUi(this);

    InitUIInformation(); ///< 初始化UI的默认信息

    // 创建启动动画，设为无限循环
    movie2 = new QMovie(":/Skin/Images/Login/OpeningAnimation.gif");
    ui->label_splash->setMovie(movie2);
    ui->label_splash->setScaledContents(true); // 动画跟随 label_splash 大小拉伸
    movie2->setCacheMode(QMovie::CacheAll); // 可选：缓存所有帧
    // movie2 默认就循环播放，无需额外设置
    movie2->start();
    connect(movie2, &QMovie::finished, this, [this]() { movie2->setPaused(true); });

    {
        //// 动画启动后，异步执行广告列表获取（内部仍为同步下载），避免阻塞 QMovie 首帧渲染
        QTimer::singleShot(0, this, [this]() {
            ui->page_LoginActivationCode->cl_advertisement_aelection_main_page_
                    ->updateAdvertisementList();
        });
    }

    // 当 stackedWidget 离开启动页时自动停止动画
    connect(ui->stackedWidget_SelMain, &QStackedWidget::currentChanged, this, [this](int /*index*/) {
        if (ui->stackedWidget_SelMain->currentWidget() != ui->page_OpeningAnimation) {
            if (movie2 && movie2->state() == QMovie::Running) {
                qDebug("结束动画\n");
                movie2->stop();
            }
        }
    });

    // 监听广告列表下载完成信号
    connect(ui->page_LoginActivationCode->cl_advertisement_aelection_main_page_,
            &AdvertisementSelectionMainPage::advertisementListReady,
            this,
            [this]() {
        m_adsReady = true;
        tryProceedToLoginAndInit();
    });

    // 广告失败/超时兜底：10s 后广告仍未就绪也进入 LoginAndInit（广告失败不应阻塞核心功能初始化）
    QTimer::singleShot(10000, this, [this]() {
        if (!m_adsReady) {
            m_adsReady = true;
            tryProceedToLoginAndInit();
        }
    });

    // 动画启动后，读取配置文件
    ReadAllFile();
    restoreBackgroundFromModel();



    // 在动画显示的同时等待 APO 就绪
    if (m_apoReady) {
        // APO 已就绪，检查广告是否也完成（由 advertisementListReady 信号触发后续）
        tryProceedToLoginAndInit();
    } else {
        // APO 未就绪，连接信号等待
        connect(this, &MainWindow::apoReady, this, [this]() { tryProceedToLoginAndInit(); });
    }
}

void MainWindow::tryProceedToLoginAndInit()
{
    // 两个前置条件都满足时才延时 2s 进入 LoginAndInit：
    // 1. APO 加载完成 (m_apoReady)
    // 2. 广告列表及图片下载完成 (m_adsReady)
    // 防重入：m_loginInitScheduled 保证 LoginAndInit 只被调度一次——
    // 广告 10s 兜底置位 m_adsReady 后，迟到的 advertisementListReady 不得再次调度（否则 InitMember 二次执行，首页构造两遍）
    if (m_apoReady && m_adsReady && !m_loginInitScheduled) {
        m_loginInitScheduled = true;
        ui->page_LoginActivationCode->cl_advertisement_aelection_main_page_->cl_change_timer_->start(
                    2000); // 就绪后启动定时器 广告循环定时器
        QTimer::singleShot(2000, this, &MainWindow::LoginAndInit);
    }
}

void MainWindow::LoginAndInit()
{
    // 初始化 SSL 证书管理（自动探测系统证书、加载/更新本地证书）
    SslCertManager certManager;
    bool sslReady = certManager.initialize();//("cacert.pem", "private.pem");
    if (!sslReady) {
        qWarning() << "SSL initialization failed, some network features may not work.";
    }

    InitGlobalVars(); ///< 初始化全局可用变量
    // 从 INI 恢复 token 到缓存（readIni 要到设备选择后才调，太晚）
    {
        QVariantMap t_map = globalSettings->value("Login/Account").toMap();
        g_user_information.network.access_token = t_map["access_token"].toString();
        AuthStore::instance().setToken(g_user_information.network.access_token);
    }

    ui->page_LoginActivationCode->CheckServerMaintenanceSta("");

    Initialize();
    emit ApoManager::instance()->requestlogWithTime("Initialize");

    // CreateScrollArea();
    // emit ApoManager::instance()->requestlogWithTime("CreateScrollArea");


    InitMember();  ///< 初始化内部成员
    InitConnect(); ///< 连接默认的信号槽

    ui->page_devSel->DevSelInitialization();
    // ui->widget_more->DevGetVersion();
    // ui->widget_more->SoftGetVersion();///WBLIU: 修改更多设置中

    if (devSelEn) {
        //进入机型选择界面
        ui->stackedWidget_SelMain->setCurrentWidget(ui->page_devSel);

    } else {
        {
            // WBLIU：
            // 遍历首页产品的名称和 PID VID GUID,更新 homePage 产品展示页面的产品图片
            for (int i = 0; i < ui->page_devSel->clp_device_selection_mainPage_
                 ->clp_scrollArea_device_selection_->cl_all_device_list_.size();
                 ++i) {
                // 设备基础信息
                unsigned short t_selDev_VID = ui->page_devSel->clp_device_selection_mainPage_
                        ->clp_scrollArea_device_selection_
                        ->cl_all_device_list_.at(i)
                        ->cl_device_info_.SelDev_VID;
                unsigned short t_selDev_PID = ui->page_devSel->clp_device_selection_mainPage_
                        ->clp_scrollArea_device_selection_
                        ->cl_all_device_list_.at(i)
                        ->cl_device_info_.SelDev_PID;
                QString SelDev_DeviceName = ui->page_devSel->clp_device_selection_mainPage_
                        ->clp_scrollArea_device_selection_
                        ->cl_all_device_list_.at(i)
                        ->cl_device_info_.DeviceSysTypeName;
                QString t_selDev_Guid = ui->page_devSel->clp_device_selection_mainPage_
                        ->clp_scrollArea_device_selection_->cl_all_device_list_
                        .at(i)
                        ->cl_device_info_.DeviceGuid;

                QString t_targetImagePath
                        = ui->page_devSel->clp_device_selection_mainPage_
                        ->clp_scrollArea_device_selection_->cl_all_device_list_.at(i)
                        ->cl_device_info_.DeviceHomePagePixmapPath; //首页图片路径
                QString t_targetImagePath_leftTop_normal
                        = ui->page_devSel->clp_device_selection_mainPage_
                        ->clp_scrollArea_device_selection_->cl_all_device_list_.at(i)
                        ->cl_device_info_
                        .DeviceHomePageTopLeftPixmapPath_normal; //首页 左上角设备正常 图片路径
                QString t_targetImagePath_leftTop_abnormal
                        = ui->page_devSel->clp_device_selection_mainPage_
                        ->clp_scrollArea_device_selection_->cl_all_device_list_.at(i)
                        ->cl_device_info_
                        .DeviceHomePageTopLeftPixmapPath_abnormal; //首页 左上角设备异常 图片路径

                // 存在完全匹配项
                if ((t_selDev_VID == SelDev_VID) && (t_selDev_PID == SelDev_PID)
                        && (t_selDev_Guid == SelDev_DeviceGuid)) {
                    // 更新首页图片
                    cl_widget_home_main_page_->cl_product_display_->UpdateBackgroundImageImmediately(
                                t_targetImagePath);

                    break;
                }
            }
        }
        LoginInEn();
    }

    //是否重新登录
    connect(ui->page_LoginActivationCode, &LoginAndActivationCode::LoginAgain, this, [this] {
        emit ApoManager::instance()->requestlogWithTime("LoginAndActivationCode:LoginAgain");
        qDebug("LoginAndActivationCode::LoginAgain\n");
        showLogin();
        // ui->page_LoginActivationCode->UploadACode();del
    });

    //在线时长追踪器
    auto *tracker = new OnlineDurationTracker(this);


    //跳转到zhu界面
    connect(ui->page_LoginActivationCode, &LoginAndActivationCode::MainPageChange, this, [this,tracker] {
        qDebug("进入MainPageChange\n");

        // 启动在线时长计时
        tracker->start();

        // ui->page_LoginActivationCode->showWaitPage();
        if (!retB) //若已经初始化APO，说明已经执行下列步骤
        {
            qDebug("MainPageChange\n");
            // emit ApoManager::instance()->requestInitEngine();
            //APO
            QFuture<void> m_future;
            m_future = QtConcurrent::run([this]() {
                // 创建对象或获取单例
                if (!apo) {
                    //qWarning() << "Failed to create LoadApoDLL instance.";
                    return; // void，直接返回
                }
                // 调用初始化函数
                int ret = apo->InitialDependencyResource();
                if (ret == RESULT_SUCCEED) {
                    emit ApoManager::instance()->requestlogWithTime(
                                "InitialDependencyResource success");
                    //初始化是否成功
                    retB = apo->IsInitialDependencyResource();
                    if (!retB) {
                        emit ApoManager::instance()->requestlogWithTime(
                                    "IsInitialDependencyResource failed");
                        return;
                    }
                } else {
                    emit ApoManager::instance()->requestlogWithTime(
                                "InitialDependencyResource failed");
                    retB = false;
                    return;
                }
            });

            m_future.waitForFinished();
            if (!retB) {
                msgBox.critical(NULL, tr("错误"), tr("MainPageChange加载驱动失败！"));
                // exit(0);
                // return;
            } else {
                // qDebug("MainPageChange加载驱动成功\n");
                emit ApoManager::instance()->requestlogWithTime("MainPageChange加载驱动成功");
            }
        }

        // {
        //     /// WBLIU:旧版首页图片
        //     QString imagePath;
        //     /*if(SelDev_DeviceName.contains("K03S",Qt::CaseInsensitive))
        // {
        //     //imagePath = ":/Skin/Images/DevSel/K03S-bk.png";
        //     imagePath = ":/Skin/Images/home/K03S-big.png";
        // }else */
        //     if (SelDev_DeviceName.contains("K03S", Qt::CaseInsensitive) && SelDev_PID == 0xF016) {
        //         // imagePath = ":/Skin/Images/home/K03S-Super-big.png";
        //         imagePath = ui->page_devSel->clp_device_selection_mainPage_
        //                         ->cl_selected_device_information()
        //                         .DeviceHomePagePixmapPath;
        //     } else if (SelDev_DeviceName.contains("K06S", Qt::CaseInsensitive)) {
        //         imagePath = ":/Skin/Images/home/K06S-big.png";
        //     } else if (SelDev_DeviceName.contains("T10", Qt::CaseInsensitive)) {
        //         if (SelDev_DeviceName.contains("Wireless", Qt::CaseInsensitive)) {
        //             imagePath = ":/Skin/Images/home/T10Wireless-big.png";
        //         } else {
        //             //imagePath = ":/Skin/Images/DevSel/T10-bk.png";
        //             imagePath = ":/Skin/Images/home/T10-big.png";
        //         }

        //     } else if (SelDev_DeviceName.contains("T7", Qt::CaseInsensitive)) {
        //         imagePath = ":/Skin/Images/home/T7-big.png";
        //     } /*else
        // {
        //     imagePath = ":/Skin/Images/home/General-big.png";
        // }*/
        // }

        /// WBLIU: 新版首页图片 ：ui->page_devSel->clp_device_selection_mainPage_->cl_selected_device_information().DeviceHomePagePixmapPath
        apo->InitialUpApo();

        {
            // WBLIU：
            // 遍历首页产品的名称和 PID VID GUID,更新 homePage 产品展示页面的产品图片
            for (int i = 0; i < ui->page_devSel->clp_device_selection_mainPage_
                 ->clp_scrollArea_device_selection_->cl_all_device_list_.size();
                 ++i) {
                // 设备基础信息
                unsigned short t_selDev_VID = ui->page_devSel->clp_device_selection_mainPage_
                        ->clp_scrollArea_device_selection_
                        ->cl_all_device_list_.at(i)
                        ->cl_device_info_.SelDev_VID;
                unsigned short t_selDev_PID = ui->page_devSel->clp_device_selection_mainPage_
                        ->clp_scrollArea_device_selection_
                        ->cl_all_device_list_.at(i)
                        ->cl_device_info_.SelDev_PID;
                QString SelDev_DeviceName = ui->page_devSel->clp_device_selection_mainPage_
                        ->clp_scrollArea_device_selection_
                        ->cl_all_device_list_.at(i)
                        ->cl_device_info_.DeviceSysTypeName;
                QString t_selDev_Guid = ui->page_devSel->clp_device_selection_mainPage_
                        ->clp_scrollArea_device_selection_->cl_all_device_list_
                        .at(i)
                        ->cl_device_info_.DeviceGuid;

                QString t_targetImagePath
                        = ui->page_devSel->clp_device_selection_mainPage_
                        ->clp_scrollArea_device_selection_->cl_all_device_list_.at(i)
                        ->cl_device_info_.DeviceHomePagePixmapPath; //首页图片路径
                QString t_targetImagePath_leftTop_normal
                        = ui->page_devSel->clp_device_selection_mainPage_
                        ->clp_scrollArea_device_selection_->cl_all_device_list_.at(i)
                        ->cl_device_info_
                        .DeviceHomePageTopLeftPixmapPath_normal; //首页 左上角设备正常 图片路径
                QString t_targetImagePath_leftTop_abnormal
                        = ui->page_devSel->clp_device_selection_mainPage_
                        ->clp_scrollArea_device_selection_->cl_all_device_list_.at(i)
                        ->cl_device_info_
                        .DeviceHomePageTopLeftPixmapPath_abnormal; //首页 左上角设备异常 图片路径

                // 存在完全匹配项
                if ((t_selDev_VID == SelDev_VID) && (t_selDev_PID == SelDev_PID)
                        && (t_selDev_Guid == SelDev_DeviceGuid)) {
                    // 更新 备选择页面 设备信息
                    ui->page_devSel->clp_device_selection_mainPage_
                            ->setCl_selected_device_information(
                                ui->page_devSel->clp_device_selection_mainPage_
                                ->clp_scrollArea_device_selection_->cl_all_device_list_.at(i)
                                ->cl_device_info_);
                    break;
                }
            }

        }

        SelDevSuccess(ui->page_devSel->clp_device_selection_mainPage_
                      ->cl_selected_device_information(),
                      {});

        ui->lab_user_name->setText(g_user_information.network.username);
        // ui->lab_user_id->setText("ID:"+g_user_information.network.id);
        if (g_user_information.network.login_type == "account") {
            //邮箱登录
            // 头像路径如果为空，说明未更改过头像，默认设置为系统头像 01
            if(g_user_information.network.avatar.isEmpty ()){
                {
                    QString t_dir = g_user_information.userDirName(); QString t_local = g_user_information.avatarFilePath(); QPixmap t_avatar; if (QFile::exists(t_local)) { t_avatar = QPixmap(t_local); } else { QDir().mkpath(t_dir); t_avatar = QPixmap(":/Skin/Images/system/system_avatar/system_avatar_2x_01.png"); t_avatar.save(t_local, "PNG"); }
                    setCircularAvatar(ui->lab_user_Avatar, t_avatar, ui->lab_user_Avatar->width());
                }
                // 已将默认头像保存至本地用户目录
            }else{
                // 网络上下载对应头像
                QString t_dir = g_user_information.userDirName();
                QDir().mkpath(t_dir);
                QString t_file_path = g_user_information.avatarFilePath();

                auto *t_nam = new QNetworkAccessManager(this);
                t_nam->setTransferTimeout(60000);
                QNetworkRequest t_request(QUrl(g_user_information.network.avatar));
                QNetworkReply *t_reply = t_nam->get(t_request);
                connect(t_reply, &QNetworkReply::finished, this, [this, t_reply, t_file_path, t_nam]() {
                    t_nam->deleteLater();
                    if (t_reply->error() == QNetworkReply::NoError) {
                        QPixmap t_pixmap;
                        t_pixmap.loadFromData(t_reply->readAll());
                        if (!t_pixmap.isNull()) {
                            t_pixmap.save(t_file_path, "PNG");
                            setCircularAvatar(ui->lab_user_Avatar, t_pixmap, ui->lab_user_Avatar->width());
                        }
                    }
                    t_reply->deleteLater();
                });
            }

        } else if (g_user_information.network.login_type == "wechat") {
            //微信登录
            // 头像路径如果为空，说明未更改过头像，默认设置为系统头像 01
            if(g_user_information.network.avatar.isEmpty ()){
                {
                    QString t_dir = g_user_information.userDirName(); QString t_local = g_user_information.avatarFilePath(); QPixmap t_avatar; if (QFile::exists(t_local)) { t_avatar = QPixmap(t_local); } else { QDir().mkpath(t_dir); t_avatar = QPixmap(":/Skin/Images/system/system_avatar/system_avatar_2x_01.png"); t_avatar.save(t_local, "PNG"); }
                    setCircularAvatar(ui->lab_user_Avatar, t_avatar, ui->lab_user_Avatar->width());
                }
                // 已将默认头像保存至本地用户目录
            }else{
                // 网络上下载对应头像
                QString t_dir = g_user_information.userDirName();
                QDir().mkpath(t_dir);
                QString t_file_path = g_user_information.avatarFilePath();

                auto *t_nam = new QNetworkAccessManager(this);
                t_nam->setTransferTimeout(60000);
                QNetworkRequest t_request(QUrl(g_user_information.network.avatar));
                QNetworkReply *t_reply = t_nam->get(t_request);
                connect(t_reply, &QNetworkReply::finished, this, [this, t_reply, t_file_path, t_nam]() {
                    t_nam->deleteLater();
                    if (t_reply->error() == QNetworkReply::NoError) {
                        QPixmap t_pixmap;
                        t_pixmap.loadFromData(t_reply->readAll());
                        if (!t_pixmap.isNull()) {
                            t_pixmap.save(t_file_path, "PNG");
                            setCircularAvatar(ui->lab_user_Avatar, t_pixmap, ui->lab_user_Avatar->width());
                        }
                    }
                    t_reply->deleteLater();
                });
            }
        }

        //检测是否存在最新版本
        swUpdateBtnClicked = false;
        clp_user_setting_main_page_->clp_version_settings_main_page_->GetUpdateMsg();
        // qDebug("ui->widget_more->GetUpdateMsg(); 11111");///WBLIU: 修改更多设置中

        {
            //更多设置相关信息更新一次
            if(clp_user_setting_main_page_){
                clp_user_setting_main_page_->UpdateAllSubPageUIInformation(UserSettingMainPage::SubPage::All);
                // 壁纸列表：登录后 custom_wallpaper_map 已加载，立即刷新以应用用户保存的壁纸
                if (clp_user_setting_main_page_->clp_interface_settings_main_page_
                        && clp_user_setting_main_page_->clp_interface_settings_main_page_->clp_background_component_view_) {
                    clp_user_setting_main_page_->clp_interface_settings_main_page_->clp_background_component_view_->refreshList();
                }
            }
        }
        // OpenAPOEffects();

        movie->stop(); //停止登录中页面的动画
    });

    // WBLIU: 修改更多设置中
    // {
    //     //恢复出厂设置，跳转到登录界面
    //     connect(ui->widget_more, &MoreSet::LoginPage, this, [this]{
    //         if(!globalSettings)
    //         {
    //             globalSettings = new QSettings(filePath, QSettings::IniFormat);
    //         }

    //         ui->page_LoginActivationCode->on_pBt_backLogin_FAR_clicked();

    //         ui->stackedWidget_SelMain->setCurrentWidget(ui->page_LoginActivationCode);
    //         ui->page_LoginActivationCode->cl_advertisement_aelection_main_page_->update(); // 主动触发一次重绘
    //     });
    // }

    if (apo->GetInitEn()) {
        ui->widget_mic->setUpEn(true);
    } else {
        ui->widget_mic->setUpEn(false);
    }

    qDebug("LoginAndInit结束\n");

    // 启动时恢复已保存的语言翻译（此时所有页面已构建完毕，安全调用）
    LanguageSet();
}

MainWindow::~MainWindow()
{
    // if(apo)
    // {
    //     emit ApoManager::instance()->requestSetGlobalInputGainDb(0);
    //     emit ApoManager::instance()->requestSetRenderState(false);//APO功能全部关闭
    // }
    // // instance = nullptr;

    // 停止所有实时保存文件，防抖定时器
    m_timerSaveModeVal->stop();
    m_timerSaveSysPlan->stop();
    m_timerSaveSysPlanInit->stop();

    // 等待所有异步写入结束
    QThreadPool::globalInstance()->waitForDone();

    //保存modeVal
    writeIni();



    uGuide->deleteLater();

    // 释放资源
    if (pEndpointVolume[0])
        pEndpointVolume[0]->Release();
    if (pEndpointVolume[1])
        pEndpointVolume[1]->Release();
    if (pDevice)
        pDevice->Release();
    if (pEnumerator)
        pEnumerator->Release();

    CoUninitialize();

    if (isHidRun) {
        timer_connect->stop();
        timer_connect->deleteLater();

        TimerR->stop();
        TimerR->deleteLater();
        lolib->closeCard();
    }

    QFuture<void> m_future;
    m_future = QtConcurrent::run([this]() {
        // 创建对象或获取单例
        if (!apo) {
            qWarning() << "Failed to create LoadApoDLL instance.";
            return; // void，直接返回
        }
        //初始化是否成功
        //if(emit ApoManager::instance()->requestIsInitialDependencyResource())
        {
            // emit ApoManager::instance()->requestSetGlobalInputGainDb(0);
            // emit ApoManager::instance()->requestSetRenderState(false);//APO功能全部关闭
            emit apo->ReleaseDependencyResource();
            delete apo;
            apo = nullptr;
        }
    });
    // emit ApoManager::instance()->requestg_pDeinit();




    stream.flush();            // 刷新缓冲区
    stream.setDevice(nullptr); // 断开与设备的连接

    CeShiSettings->close();
    delete CeShiSettings;
    CeShiSettings = nullptr;

    delete[] idSaved;
    idSaved = nullptr;

    qDebug("离开~MainWindow()\n");
    delete ui;

    delete globalSettings;
    globalSettings = nullptr;
}
void MainWindow::closeEvent(QCloseEvent *e)
{
    emit ApoManager::instance()->requestlogWithTime("进入~closeEvent()\n");

    if (!retB) {
        flushHomePageEQValueSave(); // 退出兜底：停防抖 + 排空在途异步 + 同步落盘（定时器未创建时自动跳过）
        g_user_information.saveToDisk();  // 退出前同步落盘（异步写与进程退出存在竞态，会丢最后一次修改）
        e->accept();
        qApp->quit(); // 显式请求退出事件循环
    } else if (retB && g_user_system_settings_config_info.is_exit_directly.load()) {
        flushHomePageEQValueSave(); // 同上
        g_user_information.saveToDisk();  // 同上：同步落盘
        e->accept();
        qApp->quit(); // 显式请求退出事件循环
    } else {
        //在后台运行
        setVisible(false);
        e->ignore();
    }

}
void MainWindow::closeInit()
{
    // emit ApoManager::instance()->requestlogWithTime("进入~closeInit()\n");
    // if(apo)
    // {
    //     emit ApoManager::instance()->requestSetGlobalInputGainDb(0);
    //     emit ApoManager::instance()->requestSetRenderState(false);//APO功能全部关闭
    // }

    //保存modeVal
    writeIni();

    // 首页算法值退出兜底：托盘退出不经过 closeEvent，防抖未触发时在此同步落盘
    flushHomePageEQValueSave();

    // 同步保存用户本地数据（异步写与进程退出竞态，可能丢失最后一次修改）
    g_user_information.saveToDisk();

    delete globalSettings;
    globalSettings = nullptr;

    uGuide->deleteLater();

    // 释放资源
    if (pEndpointVolume[0])
        pEndpointVolume[0]->Release();
    if (pEndpointVolume[1])
        pEndpointVolume[1]->Release();
    if (pDevice)
        pDevice->Release();
    if (pEnumerator)
        pEnumerator->Release();

    CoUninitialize();

    if (isHidRun) {
        timer_connect->stop();
        timer_connect->deleteLater();

        TimerR->stop();
        TimerR->deleteLater();
        lolib->closeCard();
    }

    QFuture<void> m_future;
    m_future = QtConcurrent::run([this]() {
        // 创建对象或获取单例
        if (!apo) {
            qWarning() << "Failed to create LoadApoDLL instance.";
            return; // void，直接返回
        }
        //初始化是否成功
        //if(emit ApoManager::instance()->requestIsInitialDependencyResource())
        {
            // emit ApoManager::instance()->requestSetGlobalInputGainDb(0);
            // emit ApoManager::instance()->requestSetRenderState(false);//APO功能全部关闭
            emit apo->ReleaseDependencyResource();
            delete apo;
            apo = nullptr;
        }
    });

    // emit ApoManager::instance()->requestg_pDeinit();


    stream.flush();            // 刷新缓冲区
    stream.setDevice(nullptr); // 断开与设备的连接

    CeShiSettings->close();
    delete CeShiSettings;
    CeShiSettings = nullptr;

    delete ui;
}

void MainWindow::refreshUserDisplay()
{
    // 昵称
    ui->lab_user_name->setText(g_user_information.network.username);

    // 头像 — 优先本地文件
    QString t_file = g_user_information.avatarFilePath();
    if (QFile::exists(t_file)) {
        QPixmap t_pm(t_file);
        setCircularAvatar(ui->lab_user_Avatar, t_pm, ui->lab_user_Avatar->width());
    }
}

void MainWindow::showPanel()
{
    // 显示窗口
    if (this->isMinimized()) {
        this->showNormal(); //如果最小化，先恢复正常状态
    } else {
        this->show();
    }
    //on_pBt_max_clicked(ui->pBt_max->isChecked());

    // 将窗口置顶，确保其显示在最前面
    this->raise();

    // 激活窗口，获取焦点
    activateWindow();
}
void MainWindow::closeSoftWare()
{
    closeInit();
    qApp->quit(); // 显式请求退出事件循环
}

void MainWindow::syncSpeakerMuteUi(bool muted, bool animate)
{
    if (!cl_widget_home_main_page_ || !cl_widget_home_main_page_->cl_speaker_setting_) {
        return;
    }

    auto *speakerSetting = cl_widget_home_main_page_->cl_speaker_setting_;
    const bool audible = !muted;

    if (speakerSetting->pBt_spk_switch_) {
        QSignalBlocker blocker(speakerSetting->pBt_spk_switch_);
        speakerSetting->pBt_spk_switch_->setChecked(audible);
    }
    if (ui && ui->pBt_spk) {
        QSignalBlocker blocker(ui->pBt_spk);
        ui->pBt_spk->setChecked(muted);
    }
    if (speakerSetting->cl_HSlider_) {
        auto *speakerSlider = speakerSetting->cl_HSlider_;
        if (animate) {
            speakerSlider->animateHandleColor(
                        muted ? QColor("#FFFFFF") : QColor("#ACACAC"),
                        muted ? QColor("#ACACAC") : QColor("#FFFFFF"),
                        100);
            speakerSlider->animateFillColor(
                        muted ? QColor("#0091C6") : QColor("#0F6796"),
                        muted ? QColor("#0F6796") : QColor("#0091C6"),
                        100);
        } else {
            speakerSlider->setHandleColor(muted ? QColor("#ACACAC") : QColor("#FFFFFF"));
            speakerSlider->setFillColor(muted ? QColor("#0F6796") : QColor("#0091C6"));
        }

        if (muted && speakerSlider->isSliderDown()) {
            speakerSlider->setEnabled(true);
            if (!m_speakerDisableOnReleaseConnection_) {
                m_speakerDisableOnReleaseConnection_ =
                        connect(speakerSlider, &QSlider::sliderReleased, this, [this]() {
                    QObject::disconnect(m_speakerDisableOnReleaseConnection_);
                    m_speakerDisableOnReleaseConnection_ = QMetaObject::Connection();

                    if (!cl_widget_home_main_page_ || !cl_widget_home_main_page_->cl_speaker_setting_) {
                        return;
                    }

                    auto *slider = cl_widget_home_main_page_->cl_speaker_setting_->cl_HSlider_;
                    if (slider && slider->value() <= slider->minimum()) {
                        slider->setEnabled(false);
                    }
                });
            }
        } else {
            if (m_speakerDisableOnReleaseConnection_) {
                QObject::disconnect(m_speakerDisableOnReleaseConnection_);
                m_speakerDisableOnReleaseConnection_ = QMetaObject::Connection();
            }
            speakerSlider->setEnabled(audible);
        }
    }
    speakerSetting->changeTextColor(muted, animate);
}

void MainWindow::syncMicrophoneMuteUi(bool muted, bool animate)
{
    if (!cl_widget_home_main_page_ || !cl_widget_home_main_page_->cl_microphone_setting_) {
        return;
    }

    auto *microphoneSetting = cl_widget_home_main_page_->cl_microphone_setting_;
    const bool audible = !muted;
    MicOpenEn = audible;

    if (microphoneSetting->pBt_mic_switch_) {
        QSignalBlocker blocker(microphoneSetting->pBt_mic_switch_);
        microphoneSetting->pBt_mic_switch_->setChecked(audible);
    }
    if (ui && ui->pBt_mic) {
        QSignalBlocker blocker(ui->pBt_mic);
        ui->pBt_mic->setChecked(muted);
    }
    if (microphoneSetting->cl_mic_hSlider_) {
        if (animate) {
            microphoneSetting->cl_mic_hSlider_->animateHandleColor(
                        muted ? QColor("#FFFFFF") : QColor("#C7C7C7"),
                        muted ? QColor("#C7C7C7") : QColor("#FFFFFF"),
                        100);
            microphoneSetting->cl_mic_hSlider_->animateFillColor(
                        muted ? QColor("#0091C6") : QColor("#006184"),
                        muted ? QColor("#006184") : QColor("#0091C6"),
                        100);
        } else {
            microphoneSetting->cl_mic_hSlider_->setHandleColor(
                        muted ? QColor("#C7C7C7") : QColor("#FFFFFF"));
            microphoneSetting->cl_mic_hSlider_->setFillColor(
                        muted ? QColor("#006184") : QColor("#0091C6"));
        }
        microphoneSetting->cl_mic_hSlider_->setEnabled(audible);
    }
    microphoneSetting->changeTextColor(muted, animate);
}

//获得系统音量，当系统音量变化时，会触发
void MainWindow::Volume(EDataFlow dataFlow)
{
    float fVolume = 0.0f;
    int intVolume = 0;
    BOOL mute = FALSE;
    const int index = audioEndpointIndex(dataFlow);
    if (index < 0) {
        return;
    }

    if (pDevice) {
        pDevice->Release();
        pDevice = NULL;
    }
    if (pEnumerator) {
        pEnumerator->Release();
        pEnumerator = NULL;
    }

    if (!refreshDefaultEndpointVolume(dataFlow) || !pEndpointVolume[index]) {
        return;
    }
    if (FAILED(pEndpointVolume[index]->GetMasterVolumeLevelScalar(&fVolume))) {
        return;
    }

    intVolume = fVolume * 100; //+1;
    if (intVolume > 100) {
        intVolume = 100;
    }
    //静音
    if (FAILED(pEndpointVolume[index]->GetMute(&mute))) {
        return;
    }

    if (dataFlow == eRender) {
        {
            QSignalBlocker blocker(cl_widget_home_main_page_->cl_speaker_setting_
                                       ->cl_HSlider_);
            cl_widget_home_main_page_->cl_speaker_setting_->cl_HSlider_->setValue(
                        intVolume); ///WBLIU:新版
        }
        //ui->lEdit_spkVol->setText(QString::number(intVolume));
        ui->label_level->setText(QString::number(intVolume));

        syncSpeakerMuteUi(mute, false);

        VolumeMonitor *m_volumeMonitor = new VolumeMonitor(dataFlow, this);
        // 连接信号槽，扬声器系统音量变化
        connect(m_volumeMonitor, &VolumeMonitor::volumeChanged, this, [this](float volume, bool Muted) {
            int sliderValue = (int) round(volume
                                          * 100); //四舍五入，否则volume = 0.020000，sliderValue = 1
            emit ApoManager::instance()->requestlogWithTime(
                        QString("22222 SYS eRender volumeChanged SYS:%1 Value:%2")
                        .arg(volume)
                        .arg(sliderValue));
            //qDebug("系统扬声器：sliderValue:%d,volume:%f,hSlider_spk:%d\n",sliderValue,volume,ui->hSlider_spk->value());
            ui->label_level->setText(QString::number(sliderValue));

            // WBLIU: 新版
            if (cl_widget_home_main_page_->cl_speaker_setting_->cl_HSlider_->value()
                    != sliderValue) {
                emit ApoManager::instance()->requestlogWithTime("!= Not equal");
                QSignalBlocker blocker(cl_widget_home_main_page_->cl_speaker_setting_
                                           ->cl_HSlider_);
                cl_widget_home_main_page_->cl_speaker_setting_->cl_HSlider_->setValue(sliderValue);
            } else {
                emit ApoManager::instance()->requestlogWithTime("== equal");
            }

            syncSpeakerMuteUi(Muted, false);
        });
    } else if (dataFlow == eCapture) {
        // WBLIU:新版
        {
            QSignalBlocker blocker(cl_widget_home_main_page_->cl_microphone_setting_
                                       ->cl_mic_hSlider_);
            cl_widget_home_main_page_->cl_microphone_setting_->cl_mic_hSlider_->setValue(
                        intVolume);
        }
        micLevel = intVolume;
        //ui->lEdit_micVol->setText(QString::number(intVolume));
        syncMicrophoneMuteUi(mute, false);

        VolumeMonitor *m_MicvolumeMonitor = new VolumeMonitor(dataFlow, this);
        // 连接信号槽，麦克风系统音量变化
        connect(m_MicvolumeMonitor,
                &VolumeMonitor::MicvolumeChanged,
                this,
                [this](float volume, bool Muted) {
            int sliderValue = (int) round(volume * 100);
            {
                // WBLIU: 新版
                if (cl_widget_home_main_page_->cl_microphone_setting_->cl_mic_hSlider_
                        ->value()
                        != sliderValue) {
                    QSignalBlocker blocker(cl_widget_home_main_page_->cl_microphone_setting_
                                               ->cl_mic_hSlider_);
                    cl_widget_home_main_page_->cl_microphone_setting_->cl_mic_hSlider_
                            ->setValue(sliderValue);
                    micLevel = sliderValue;
                }

                syncMicrophoneMuteUi(Muted, false);
            }
        });
    }
}

void MainWindow::LoginInEn()
{
    if (isLogin) {
        if (!g_user_information.network.access_token.isEmpty()) {
            emit ApoManager::instance()->requestlogWithTime("access_token not empty");
            ui->page_LoginActivationCode->showWaitPage();
            //判断access_token是否失效，若失效则重新登录
            ui->page_LoginActivationCode->TokenEn();
        } else {
            emit ApoManager::instance()->requestlogWithTime("access_token empty");
            isLogin = false;
            showLogin();
        }

        emit ApoManager::instance()->requestlogWithTime("SelDevSuccess isLogin true");

    } else {
        emit ApoManager::instance()->requestlogWithTime("access_token empty");
        isLogin = false;
        showLogin();

        return;
    }
}

void MainWindow::saveHomePageExtraEQValue()
{
    if (!globalSettings) {
        return; // Initialize 未执行（广告未就绪等）：内存值已更新，Initialize 后下次修改即保存
    }
    QVariantList variantList;
    for (int val : HomePageExtraEQValue) {
        variantList.append(val);
    }
    QtConcurrent::run([this, variantList]() {
        QMutexLocker locker(&m_saveMutex);
        globalSettings->setValue("HomeMainPage/ExtraValue", variantList);
        globalSettings->sync();
    });
}

/// \brief 首页算法值同步落盘（退出兜底专用；异步版可能未及写完即退出）
void MainWindow::saveHomePageExtraEQValueSync()
{
    if (!globalSettings) {
        return; // Initialize 未执行：无处落盘，跳过
    }
    QVariantList variantList;
    for (int val : HomePageExtraEQValue) {
        variantList.append(val);
    }
    QMutexLocker locker(&m_saveMutex);
    globalSettings->setValue("HomeMainPage/ExtraValue", variantList);
    globalSettings->sync();
}

/// \brief 首页算法值退出兜底：停防抖 + 排空在途异步保存 + 同步落盘
/// closeEvent 与 closeInit（托盘退出）共用；定时器可能尚未创建（启动阶段即退出），需判空；
/// waitForDone 排空在途 QtConcurrent 快照任务，避免旧快照在同步落盘后覆盖新值
void MainWindow::flushHomePageEQValueSave()
{
    if (m_timerSaveHomePageEQ) {
        m_timerSaveHomePageEQ->stop();
    }
    QThreadPool::globalInstance()->waitForDone();
    saveHomePageExtraEQValueSync();
}

/// \brief 首页方案开关应用（方案开关 toggled handler 主体）
/// 开：互斥关闭首页算法开关，并按当前方案 En 标志应用 EQ/算法/空间/DRC 通道（SetEQSwitchShadow 附带 UI 同步）；
/// 关：仅对标志为 true 的通道发关闭请求（不改写方案数据、不切换当前方案）。
/// 说明：SetEQSwitchShadow 会写 currentPlanVal.xxxOpenEn（临时污染），但不写回 radio/不保存，
/// 下次方案切换（SpeakerSet.cpp 选中时 getAllPlanValue 覆盖）即恢复原标志。
void MainWindow::applyHomePagePlansSwitch(bool checked)
{
    auto *plansSelection = cl_widget_home_main_page_->cl_plans_selection_;
    if (!plansSelection) {
        return;
    }

    if (checked && !plansSelection->isCurrentPlanHomePlan()) {
        if (NewRadioBtn *firstPlan = plansSelection->firstAvailableHomePlan()) {
            closeHomePageAlgorithmSwitch();
            if (firstPlan->isChecked()) {
                currentPlanRadio = firstPlan;
                currentPlanVal = firstPlan->getAllPlanValue();
                ui->page_Sperker->ShowcurrentPlanVal();
                return;
            }
            firstPlan->setChecked(true);
            return;
        }
    }

    if (currentPlanRadio == nullptr) {
        // 设备未连接/方案未就绪：仅同步真源与 UI（调用方负责），不做 APO 操作
        return;
    }

    if (checked) {
        closeHomePageAlgorithmSwitch();
        ui->page_Sperker->ShowcurrentPlanVal();
    } else {
        // 关闭：仅对已开启通道发关闭请求（标志为 false 的通道不动，最小 UI/标志副作用）
        if (currentPlanVal.eqOpenEn) {
            ui->widget_eq->SetEQSwitchShadow(false, 0);
        }
        if (currentPlanVal.AlgoOpenEn) {
            ui->widget_eq->SetEQSwitchShadow(false, 1);
        }
        if (currentPlanVal.spaceOpenEn) {
            ui->widget_eq->SetEQSwitchShadow(false, 2);
        }
        if (currentPlanVal.drcOpenEn) {
            ui->widget_eq->SetEQSwitchShadow(false, 3);
        }
    }
}

void MainWindow::applyHomePresetPlan(const QPair<QString, QString> &planKey)
{
    auto *plansSelection = cl_widget_home_main_page_->cl_plans_selection_;
    if (!plansSelection) {
        return;
    }

    NewRadioBtn *targetPlan = plansSelection->findHomePlan(planKey);
    if (!targetPlan) {
        return;
    }

    setHomePagePlansSwitchOpenSilently();
    closeHomePageAlgorithmSwitch();
    if (targetPlan->isChecked()) {
        currentPlanRadio = targetPlan;
        currentPlanVal = targetPlan->getAllPlanValue();
        ui->page_Sperker->ShowcurrentPlanVal();
        return;
    }

    targetPlan->setChecked(true);
}

void MainWindow::closeHomePageAlgorithmSwitch()
{
    auto *algorithmSwitch = cl_widget_home_main_page_->cl_algorithm_adjustment_setting
                                ->cl_customPushButton_;
    if (algorithmSwitch && algorithmSwitch->isChecked()) {
        algorithmSwitch->setChecked(false);
    }
}

void MainWindow::setHomePagePlansSwitchOpenSilently()
{
    auto *planSwitch = cl_widget_home_main_page_->cl_plans_selection_->cl_customPushButton_;
    if (!planSwitch) {
        return;
    }

    QSignalBlocker blocker(planSwitch);
    planSwitch->setChecked(true);
    HomePagePlansOpen = true;
    globalSettings->setValue("HomeMainPage/PlansOpen", HomePagePlansOpen);
    QtConcurrent::run([this]() {
        QMutexLocker locker(&m_saveMutex);
        globalSettings->sync();
    });
}

void MainWindow::showLogin()
{
    emit ApoManager::instance()->requestlogWithTime("showLogin()");

    if (isLogin) {
        emit ApoManager::instance()->requestlogWithTime("showLogin() isLogin true");
        ui->page_LoginActivationCode->showWaitPage();
        // 加载头像
        // QPixmap pixmap_Avatar = loadAvatar();

        ui->lab_user_name->setText(g_user_information.network.username);
        // ui->lab_user_id->setText("ID:"+g_user_information.network.id);
        if (g_user_information.network.login_type == "account") {
            //邮箱登录
            // 头像路径如果为空，说明未更改过头像，默认设置为系统头像 01
            if(g_user_information.network.avatar.isEmpty ()){
                {
                    QString t_dir = g_user_information.userDirName(); QString t_local = g_user_information.avatarFilePath(); QPixmap t_avatar; if (QFile::exists(t_local)) { t_avatar = QPixmap(t_local); } else { QDir().mkpath(t_dir); t_avatar = QPixmap(":/Skin/Images/system/system_avatar/system_avatar_2x_01.png"); t_avatar.save(t_local, "PNG"); }
                    setCircularAvatar(ui->lab_user_Avatar, t_avatar, ui->lab_user_Avatar->width());
                }
                // 已将默认头像保存至本地用户目录
            }else{
                // 网络上下载对应头像
                QString t_dir = g_user_information.userDirName();
                QDir().mkpath(t_dir);
                QString t_file_path = g_user_information.avatarFilePath();

                auto *t_nam = new QNetworkAccessManager(this);
                t_nam->setTransferTimeout(60000);
                QNetworkRequest t_request(QUrl(g_user_information.network.avatar));
                QNetworkReply *t_reply = t_nam->get(t_request);
                connect(t_reply, &QNetworkReply::finished, this, [this, t_reply, t_file_path, t_nam]() {
                    t_nam->deleteLater();
                    if (t_reply->error() == QNetworkReply::NoError) {
                        QPixmap t_pixmap;
                        t_pixmap.loadFromData(t_reply->readAll());
                        if (!t_pixmap.isNull()) {
                            t_pixmap.save(t_file_path, "PNG");
                            setCircularAvatar(ui->lab_user_Avatar, t_pixmap, ui->lab_user_Avatar->width());
                        }
                    }
                    t_reply->deleteLater();
                });
            }
        } else if (g_user_information.network.login_type == "wechat") {
            //微信登录
            // 头像路径如果为空，说明未更改过头像，默认设置为系统头像 01
            if(g_user_information.network.avatar.isEmpty ()){
                {
                    QString t_dir = g_user_information.userDirName(); QString t_local = g_user_information.avatarFilePath(); QPixmap t_avatar; if (QFile::exists(t_local)) { t_avatar = QPixmap(t_local); } else { QDir().mkpath(t_dir); t_avatar = QPixmap(":/Skin/Images/system/system_avatar/system_avatar_2x_01.png"); t_avatar.save(t_local, "PNG"); }
                    setCircularAvatar(ui->lab_user_Avatar, t_avatar, ui->lab_user_Avatar->width());
                }
                // 已将默认头像保存至本地用户目录
            }else{
                // 网络上下载对应头像
                QString t_dir = g_user_information.userDirName();
                QDir().mkpath(t_dir);
                QString t_file_path = g_user_information.avatarFilePath();

                auto *t_nam = new QNetworkAccessManager(this);
                t_nam->setTransferTimeout(60000);
                QNetworkRequest t_request(QUrl(g_user_information.network.avatar));
                QNetworkReply *t_reply = t_nam->get(t_request);
                connect(t_reply, &QNetworkReply::finished, this, [this, t_reply, t_file_path, t_nam]() {
                    t_nam->deleteLater();
                    if (t_reply->error() == QNetworkReply::NoError) {
                        QPixmap t_pixmap;
                        t_pixmap.loadFromData(t_reply->readAll());
                        if (!t_pixmap.isNull()) {
                            t_pixmap.save(t_file_path, "PNG");
                            setCircularAvatar(ui->lab_user_Avatar, t_pixmap, ui->lab_user_Avatar->width());
                        }
                    }
                    t_reply->deleteLater();
                });
            }
        }
        // {
        // WBLIU: 旧版 已改为直接从 ui->page_devSel->clp_device_selection_mainPage_->cl_selected_device_information_ 中获取
        // QString imagePath;
        // /*if(SelDev_DeviceName.contains("K03S",Qt::CaseInsensitive))
        // {
        //     //imagePath = ":/Skin/Images/DevSel/K03S-bk.png";
        //     imagePath = ":/Skin/Images/home/K03S-big.png";
        // }else*/if(SelDev_DeviceName.contains("K03S",Qt::CaseInsensitive) && SelDev_PID == 0xF016)
        // {
        //     imagePath = ":/Skin/Images/home/K03S-Super-big.png";
        // }else if(SelDev_DeviceName.contains("K06S",Qt::CaseInsensitive))
        // {

        //     imagePath = ":/Skin/Images/home/K06S-big.png";
        // }else if(SelDev_DeviceName.contains("T10",Qt::CaseInsensitive))
        // {
        //     if(SelDev_DeviceName.contains("Wireless",Qt::CaseInsensitive))
        //     {
        //         imagePath = ":/Skin/Images/home/T10Wireless-big.png";
        //     }else
        //     {

        //         imagePath = ":/Skin/Images/home/T10-big.png";
        //     }

        // }else if(SelDev_DeviceName.contains("T7",Qt::CaseInsensitive))
        // {
        //     imagePath = ":/Skin/Images/home/T7-big.png";
        // }/*else
        // {
        //     imagePath = ":/Skin/Images/home/General-big.png";
        // }*/
        // }
        apo->InitialUpApo();

        {
            // WBLIU：
            // 遍历首页产品的名称和 PID VID GUID,更新 homePage 产品展示页面的产品图片
            for (int i = 0; i < ui->page_devSel->clp_device_selection_mainPage_
                 ->clp_scrollArea_device_selection_->cl_all_device_list_.size();
                 ++i) {
                // 设备基础信息
                unsigned short t_selDev_VID = ui->page_devSel->clp_device_selection_mainPage_
                        ->clp_scrollArea_device_selection_
                        ->cl_all_device_list_.at(i)
                        ->cl_device_info_.SelDev_VID;
                unsigned short t_selDev_PID = ui->page_devSel->clp_device_selection_mainPage_
                        ->clp_scrollArea_device_selection_
                        ->cl_all_device_list_.at(i)
                        ->cl_device_info_.SelDev_PID;
                QString SelDev_DeviceName = ui->page_devSel->clp_device_selection_mainPage_
                        ->clp_scrollArea_device_selection_
                        ->cl_all_device_list_.at(i)
                        ->cl_device_info_.DeviceSysTypeName;
                QString t_selDev_Guid = ui->page_devSel->clp_device_selection_mainPage_
                        ->clp_scrollArea_device_selection_->cl_all_device_list_
                        .at(i)
                        ->cl_device_info_.DeviceGuid;

                QString t_targetImagePath
                        = ui->page_devSel->clp_device_selection_mainPage_
                        ->clp_scrollArea_device_selection_->cl_all_device_list_.at(i)
                        ->cl_device_info_.DeviceHomePagePixmapPath; //首页图片路径
                QString t_targetImagePath_leftTop_normal
                        = ui->page_devSel->clp_device_selection_mainPage_
                        ->clp_scrollArea_device_selection_->cl_all_device_list_.at(i)
                        ->cl_device_info_
                        .DeviceHomePageTopLeftPixmapPath_normal; //首页 左上角设备正常 图片路径
                QString t_targetImagePath_leftTop_abnormal
                        = ui->page_devSel->clp_device_selection_mainPage_
                        ->clp_scrollArea_device_selection_->cl_all_device_list_.at(i)
                        ->cl_device_info_
                        .DeviceHomePageTopLeftPixmapPath_abnormal; //首页 左上角设备异常 图片路径

                // 存在完全匹配项
                if ((t_selDev_VID == SelDev_VID) && (t_selDev_PID == SelDev_PID)
                        && (t_selDev_Guid == SelDev_DeviceGuid)) {
                    // 更新 备选择页面 设备信息
                    ui->page_devSel->clp_device_selection_mainPage_
                            ->setCl_selected_device_information(
                                ui->page_devSel->clp_device_selection_mainPage_
                                ->clp_scrollArea_device_selection_->cl_all_device_list_.at(i)
                                ->cl_device_info_);
                    break;
                }
            }
        }
        SelDevSuccess(ui->page_devSel->clp_device_selection_mainPage_
                      ->cl_selected_device_information(),
                      {});

    } else {
        emit ApoManager::instance()->requestlogWithTime("showLogin() isLogin false");
        //若没有登录过，则登录,
        ui->stackedWidget_SelMain->setCurrentWidget(ui->page_LoginActivationCode);
        ui->page_LoginActivationCode->cl_advertisement_aelection_main_page_
                ->update(); // 主动触发一次重绘
        ui->page_LoginActivationCode->on_pBt_backLogin_FAR_clicked();
        // ui->page_LoginActivationCode->GetWechatCode();
    }
}

void MainWindow::Initialize()
{
    ui->lab_status->hide();

    ui->pBt_mini->setToolTip(tr("最小化"));
    ui->pBt_max->setToolTip(tr("最大化"));
    ui->pBt_close->setToolTip(tr("关闭"));

    group_Nav = new QButtonGroup(this);
    group_Nav->addButton(ui->pBt_mainItem, 0);
    group_Nav->addButton(ui->pBt_eqItem, 1);
    group_Nav->addButton(ui->pBt_MicItem, 2);
    group_Nav->addButton(ui->pBt_SelfItem, 3);
    group_Nav->addButton(ui->pBt_CommItem, 4);
    group_Nav->addButton(ui->pBt_MoreItem, 5);
    group_Nav->setExclusive(true);

    ui->pBt_SelfItem->hide();
    // ui->pBt_CommItem->hide();

    connect(group_Nav,
            QOverload<int, bool>::of(&QButtonGroup::buttonToggled),
            this,
            &MainWindow::Nav_toggled);

    // //获得系统音量
    // Volume(eRender);
    // Volume(eCapture);

    //机型选择
    connect(ui->page_devSel,
            &DeviceSel::imageSelected,
            [this](const DeSheng::DeviceInfo &deviceInfo, const QRect &sourceGeometry) {
        if (isHidRun) {
            timer_connect->stop();

            TimerR->stop();
            lolib->closeCard();
            isHidRun = false;
        }

        if (!DSevlBtnEn) {
            apo->InitialUpApo();
            LoginInEn();

        } else {
            DSevlBtnEn = false;
            SelDevSuccess(deviceInfo, sourceGeometry);
        }
    });
    //

    // 初始化动画
    scaleAnimation = new QPropertyAnimation(this);
    moveAnimation = new QPropertyAnimation(this);
    animationGroup = new QParallelAnimationGroup(this);

    //qDebug("界面显示devSelEn:%d\n",devSelEn);

    //ui->stackedWidget_SelMain->setCurrentWidget(ui->page_Selmain);
    {
        // WBLIU：旧版
        //     // 为widget添加阴影效果
        //     QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect;
        //     shadowEffect->setBlurRadius(10); // 阴影模糊半径，对应box-shadow的第三个值（10px）
        //     shadowEffect->setColor(QColor(0, 0, 0, 76)); // 颜色，0.3透明度，所以是0.3*255=76.5
        //     shadowEffect->setOffset(0, 4); // 阴影偏移，对应box-shadow的前两个值（0px, 4px）

        //     cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_->setGraphicsEffect(shadowEffect);
    }
    // ui->widget_Mic->setGraphicsEffect(shadowEffect); //WBLIU 旧版阴影

    TimerR = new QTimer();
    TimerR->setInterval(10000); // 设置定时器间隔为10秒
    connect(TimerR, SIGNAL(timeout()), this, SLOT(Timer_readData()));
    readWatcher = new QFutureWatcher<int>(this);
    connect(readWatcher, &QFutureWatcher<int>::finished, this, &MainWindow::onReadFinished);

    // 创建实时保存内容，防抖定时器，300ms 内重复触发只执行最后一次
    m_timerSaveModeVal = new QTimer(this);
    m_timerSaveModeVal->setSingleShot(true);
    m_timerSaveModeVal->setInterval(300);

    m_timerSaveSysPlan = new QTimer(this);
    m_timerSaveSysPlan->setSingleShot(true);
    m_timerSaveSysPlan->setInterval(300);

    m_timerSaveSysPlanInit = new QTimer(this);
    m_timerSaveSysPlanInit->setSingleShot(true);
    m_timerSaveSysPlanInit->setInterval(300);

    // 定时器超时 → 执行异步保存
    connect(m_timerSaveModeVal, &QTimer::timeout, this, &MainWindow::onTriggerAsyncSaveModeVal);
    connect(m_timerSaveSysPlan, &QTimer::timeout, this, &MainWindow::onTriggerAsyncSaveSysPlan);
    connect(m_timerSaveSysPlanInit,
            &QTimer::timeout,
            this,
            &MainWindow::onTriggerAsyncSaveSysPlanInit);

    // 首页算法值防抖定时器已在构造函数 InitMember 创建（不依赖 Initialize，见 InitMember）

    //2.4G重连动画
    m_baseStyle_connect = "background: transparent;"
                          "font-family: \"Noto Sans S Chinese\";"
                          "font-weight: 500;"
                          "font-size: 10px;"
                          "border: none;";

    // 创建颜色动画
    m_colorAnimation = new QVariantAnimation(this);
    m_colorAnimation->setDuration(800);                          // 一个完整周期 0.5 秒
    m_colorAnimation->setStartValue(QColor(116, 124, 131, 255)); // 完全不透明 #747C83
    m_colorAnimation->setEndValue(QColor(116, 124, 131, 127));   // 50% 透明度
    m_colorAnimation->setEasingCurve(QEasingCurve::InOutSine);   // 平滑过渡
    m_colorAnimation->setLoopCount(-1);                          // 无限循环

    connect(m_colorAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        QColor c = value.value<QColor>();
        QString colorStyle = QString("color: rgba(%1, %2, %3, %4);")
                .arg(c.red())
                .arg(c.green())
                .arg(c.blue())
                .arg(c.alpha());
        // 合并基础样式和动态颜色
        ui->lab_Reconnect->setStyleSheet(m_baseStyle_connect + colorStyle);
    });
    // 创建定时器，每 60 * 1000 毫秒（1分钟）触发一次
    timer_connect = new QTimer(this);
    connect(timer_connect, &QTimer::timeout, this, &MainWindow::onTimeout_connect);

    tip_connect = new NewCustomToolTip(this);
    tip_connect->AddToolTip(ui->pbt_explain_Reconnect,
                            tr("请确认耳机是否开机且连接2.4G。发射器常亮即2.4G连接成功。"),
                            Qt::AlignLeft);
    tip_connect->setLabelStyle(1);

    ui->pBt_Reconnect->hide();
}

// WBLIU: 更新更多设置中,已经迁移至 InitMember 和 InitConnect 中
// void MainWindow::CreateScrollArea()
// {
//     if (!ui->widget_listen) {
//         emit ApoManager::instance()->requestlogWithTime("CreateScrollArea widget_listen 不存在");
//     }

//     // 创建内容区域
//     content_listen = new QWidget();

//     content_listen->setObjectName("content_listen");
//     content_listen->setAutoFillBackground(true);
//     // content_listen->setStyleSheet(R"(
//     // QWidget#content_listen {
//     //     background-color: #12151d;

//     // }
//     // )");

//     //content_listen->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
//     content_listen->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

//     // content_listen->setStyleSheet("background-color: red;");
//     //栅格布局
//     QGridLayout *l_layout = new QGridLayout(content_listen);

//     //l_layout->setSpacing(0);                   // 清除控件间距
//     l_layout->setContentsMargins(0, 0, 0, 0); // 清除默认边距

//     widget_listenSpeaker = new SpeakerListen(content_listen);
//     widget_listenSpeaker->cl_sound_test_main_page_->cl_ffmpeg_main_page_->minView
//         = std::make_shared<VideoHover>(this);
//     widget_listenSpeaker->cl_sound_test_main_page_->cl_ffmpeg_main_page_->minView
//         ->hide(); ///画中画-小窗口, 默认隐藏
//     widget_listenSpeaker->cl_sound_test_main_page_->cl_ffmpeg_main_page_->minView
//         ->move(rect().center().x(), 124); /// 中偏右

//     //跳转到其他界面
//     connect(widget_listenSpeaker,
//             &SpeakerListen::ChangeToSpeakerPage,
//             this,
//             [this](int index, bool ShowVH, int VHIdx, float currentPos) {
//                 //0:跳转page_Sperker 1：跳转到page_Sperker的方案库界面 3:均衡器界面  4:试听选择界面
//                 if (index == 4) {
//                     /// 跳转 至试听 播放界面
//                     ui->stackedWidget->setCurrentWidget(ui->page_listen);
//                 }

//                 if (index != 4) {
//                     ui->stackedWidget->setCurrentWidget(ui->page_Sperker);
//                 }

//                 if (index == 1) {
//                     ui->stackedWidget->setCurrentWidget(ui->page_Sperker);
//                     // ui->page_Sperker->on_rBt_currentPlan_clicked();
//                 } else if (index == 3) {
//                     if (currentPlanRadio == NULL) {
//                         ui->stackedWidget->setCurrentWidget(ui->page_Sperker);
//                     } else {
//                         //跳转到均衡器界面
//                         // ui->page_Sperker->on_pBt_ClosePlanPage_clicked();
//                         ui->widget_eq->SwitchEqPage();
//                         ui->widget_eq->ShowEqVal(currentPlanVal.DataVisibleEn);
//                         ui->widget_eq->PageShowPlanVal();
//                         ui->stackedWidget->setCurrentWidget(ui->page_eq);
//                     }
//                 }
//             });

//     //一起改变IndicatorText
//     QObject::connect(ui->page_Sperker->currentPlan_s,
//                      &NewRadioBtnText::SetITextSignal,
//                      widget_listenSpeaker->currentPlan_l,
//                      &NewRadioBtnText::setIndicatorText);
//     // QObject::connect(ui->page_Sperker->eightPlan_s, &EightMyPlan::ShowBtn,
//     //                  widget_listenSpeaker->eightPlan_l, &EightMyPlan::ShowFirstEight);
//     QObject::connect(ui->page_Sperker->eightPlan_s,
//                      &EightMyPlan::ShowBtn,
//                      widget_listenSpeaker->eightPlan_l,
//                      &EightMyPlan::ShowEightFavorite);
//     QObject::connect(ui->page_Sperker->eightPlan_s,
//                      &EightMyPlan::RenameBtn,
//                      widget_listenSpeaker->eightPlan_l,
//                      &EightMyPlan::Rename);

//     widget_listenSpeaker->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//     l_layout->addWidget(widget_listenSpeaker, 0, 0); // 明确指定行列位置

//     // 可选：设置行间距和列间距
//     // l_layout->setHorizontalSpacing(5);
//     // l_layout->setVerticalSpacing(12);
//     content_listen->setLayout(l_layout);

//     content_listen->setMinimumSize(widget_listenSpeaker->minimumSizeHint());
//     scrollArea_listen->setWidget(content_listen);
//     //content_listen->setMinimumSize(1, 1);  // 防止内容区域尺寸为0
//     //垂直布局
//     QGridLayout *mainLayout_listen = new QGridLayout(ui->widget_listen);
//     mainLayout_listen->setContentsMargins(0, 0, 0, 0); // 可选：设置边距
//     //mainLayout_listen->setSpacing(0);
//     mainLayout_listen->addWidget(scrollArea_listen, 0, 0); // 添加到(0,0)
//     //mainLayout_listen->setRowStretch(0, 1);  // 设置行拉伸因子
//     //mainLayout_listen->setColumnStretch(0, 1); // 设置列拉伸因子
//     //qDebug("111ScrollArea height: %d,content_listen height:%d\n", scrollArea_listen->height(),content_listen->height());
//     // 关键修复4：确保父容器正确设置布局
//     ui->widget_listen->setLayout(mainLayout_listen); // 明确设置布局

//     //跳转到试听界面
//     connect(ui->page_Sperker, &SpeakerSet::pageChange, this, [this] {
//         ui->stackedWidget->setCurrentWidget(ui->page_listen);
//     });

//     //跳转到方案库界面
//     connect(ui->widget_eq, &SpeakerEq::CurrentpageChange, this, [this] {
//         ui->stackedWidget->setCurrentWidget(ui->page_Sperker);
//         // ui->page_Sperker->on_rBt_currentPlan_clicked();
//     });
//     //跳转到试听界面
//     connect(ui->widget_eq, &SpeakerEq::ListenpageChange, this, [this] {
//         ui->stackedWidget->setCurrentWidget(ui->page_listen);
//     });

//     // 截断信号 控制按键显示
//     connect(ui->widget_eq,
//             &SpeakerEq::UpdateHomePageUIInfo,
//             cl_widget_home_main_page_->cl_plans_selection_,
//             [=](bool targetStatus) {
//                 // 截断信号，只更新UI
//                 cl_widget_home_main_page_->cl_plans_selection_->cl_customPushButton_->blockSignals(
//                     true);
//                 cl_widget_home_main_page_->cl_plans_selection_->cl_customPushButton_->setChecked(
//                     HomePagePlansOpen);
//                 cl_widget_home_main_page_->cl_plans_selection_->cl_customPushButton_->blockSignals(
//                     false);

//                 HomePagePlansOpen = targetStatus;
//                 globalSettings->setValue("HomeMainPage/PlansOpen", HomePagePlansOpen);
//                 // 异步写入文件
//                 QtConcurrent::run([this]() {
//                     QMutexLocker locker(&m_saveMutex); // 加锁，保证 QSettings 线程安全
//                     globalSettings->sync();
//                 });
//             });

//     //保存
//     connect(ui->widget_eq, &SpeakerEq::PlanSave_E, ui->page_Sperker, &SpeakerSet::PlanSave);
//     //重置
//     connect(ui->widget_eq, &SpeakerEq::PlanReset_E, ui->page_Sperker, &SpeakerSet::ResetValue);
//     //创建二创方案
//     connect(ui->widget_eq, &SpeakerEq::CreateDerivPlan, ui->page_Sperker, [this]() {
//         ui->stackedWidget->setCurrentWidget(ui->page_Sperker);
//         ui->page_Sperker->CreateDerivPlanSlot();
//     });

//     //一起改变IndicatorText
//     QObject::connect(ui->page_Sperker->currentPlan_s,
//                      &NewRadioBtnText::SetITextSignal,
//                      ui->widget_eq->currentPlan_e,
//                      &NewRadioBtnText::setIndicatorText);
//     // QObject::connect(ui->page_Sperker->eightPlan_s, &EightMyPlan::ShowBtn,
//     //                  ui->widget_eq->eightPlan_e, &EightMyPlan::ShowFirstEight);
//     QObject::connect(ui->page_Sperker->eightPlan_s,
//                      &EightMyPlan::ShowBtn,
//                      ui->widget_eq->eightPlan_e,
//                      &EightMyPlan::ShowEightFavorite);
//     QObject::connect(ui->page_Sperker->eightPlan_s,
//                      &EightMyPlan::RenameBtn,
//                      ui->widget_eq->eightPlan_e,
//                      &EightMyPlan::Rename);

//     setupEightMyPlanSync({ui->page_Sperker->eightPlan_s,
//                           widget_listenSpeaker->eightPlan_l,
//                           ui->widget_eq->eightPlan_e});

//     //跳转到EQ界面
//     connect(ui->page_Sperker, &SpeakerSet::EQpageChange, this, [this](bool ShowEqEn) {
//         emit ApoManager::instance()->requestlogWithTime(
//             QString("EQpageChange ShowEqVal :%1").arg(ShowEqEn));
//         ui->widget_eq->SwitchEqPage();
//         ui->widget_eq->ShowEqVal(ShowEqEn);
//         ui->widget_eq->ShowcurrentPlanVal();

//         ui->stackedWidget->setCurrentWidget(ui->page_eq);
//     });

//     //点击收藏方案跳转到EQ界面
//     connect(ui->page_Sperker, &SpeakerSet::FavEQpageChange, this, [this](bool ShowEqEn) {
//         ui->widget_eq->SwitchEqPage();
//         ui->widget_eq->ShowEqVal(ShowEqEn);
//         ui->widget_eq->PageShowPlanVal();

//         ui->stackedWidget->setCurrentWidget(ui->page_eq);
//         //QString itemText = ui->treeWidget->currentItem()->text(0);
//         qDebug("跳转到方案库界面\n");
//     });

//     //设置APO数值，不跳转到均衡器界面
//     connect(ui->page_Sperker, &SpeakerSet::SetApoVal, this, [this]() {
//         apo->logWithTime("ui->widget_eq->ShowcurrentPlanVal();前");
//         ui->widget_eq->ShowcurrentPlanVal();
//     });

//     //重置
//     connect(ui->page_Sperker, &SpeakerSet::PlanReset_S, ui->widget_eq, &SpeakerEq::resetVal);

//     // 原始信号 → 启动对应的防抖定时器
//     //动态保存, 我的预设的方案
//     connect(ui->page_Sperker,
//             &SpeakerSet::RealTimeSaveModeVal_S,
//             m_timerSaveModeVal,
//             qOverload<>(&QTimer::start));
//     //动态保存，系统预设的方案
//     connect(ui->page_Sperker,
//             &SpeakerSet::RealTimeSaveSysPlan_S,
//             m_timerSaveSysPlan,
//             qOverload<>(&QTimer::start));
//     //动态保存，所有预设方案的初始值
//     connect(ui->page_Sperker,
//             &SpeakerSet::RealTimeSaveSysPlanValInit_S,
//             m_timerSaveSysPlanInit,
//             qOverload<>(&QTimer::start));

//     // connect(ui->page_Sperker, &SpeakerSet::RealTimeSaveModeVal_S, this, [this] {
//     //     {
//     //         RealTimeSaveIni_ModeVal();
//     //     }
//     // });

//     // connect(ui->page_Sperker, &SpeakerSet::RealTimeSaveSysPlan_S, this, [this] {
//     //     {
//     //         RealTimeSaveIni_SysPlan();
//     //     }
//     // });

//     // connect(ui->page_Sperker, &SpeakerSet::RealTimeSaveSysPlanValInit_S, this, [this] {
//     //     {
//     //         RealTimeSaveIni_SysPlanValInit();
//     //     }
//     // });

//     // connect(ui->page_Sperker, &SpeakerSet::PlanReset_S, this, [this] {
//     //     ui->widget_eq->ShowcurrentPlanVal();
//     // });
// }

void MainWindow::ReadAllFile()
{
    bool ShowSysPlanEn = true;

    //一个设备一个.ini文件，有个共同的.ini文件。若相同的设备插着多个，则使用相同的INI。使用相同的INI
    //msgBox.setStyleSheet("QMessageBox{border-image: url(:/imag e/DialogMessageBox/bg.png);}");

    /*//地址为exe所在文件夹下
    filePath = QApplication::applicationDirPath()+QString("/ProgramData/"+SelDev_DeviceName+"/setting.ini");
    filePath_Load = QApplication::applicationDirPath()+QString("/ProgramData/LoadPlan/setting.ini");*/

    //地址为C:\Users\<用户名>\AppData\Roaming\XIBERIA X HUB\ProgramData
    filePath = QFileInfo(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).absolutePath()
            + "/XIBERIA X HUB/ProgramData/setting.ini";
    //先保留只用一个文档，不修改为安装机型分别保存文档。以防用户反馈，后续又要改回一个
    // if (!QFile::exists(filePath)) {
    //     // 文件不存在
    //     filePath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).first() + QString("/ProgramData/%1/setting.ini").arg(SelDev_DeviceName);
    //     filePath_Read = filePath;
    // }else
    // {
    //     filePath_Read = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).first() + QString("/ProgramData/%1/setting.ini").arg(SelDev_DeviceName);
    // }

    if (!globalSettings) {
        globalSettings = new QSettings(filePath, QSettings::IniFormat);
    }

    //如果不存在该名称的文件夹（则默认值，然后保存该命名下的文件夹）
    // connect(ui->widget_more, &MoreSet::LanguageChange, this, &MainWindow::LanguageSet);///WBLIU: 修改更多设置中

    // 检查文件是否存在，先选择语言
    QFile configFile(filePath);
    if (!configFile.exists()) {
        //qDebug() << "配置文件不存在，使用默认语言设置、默认背景主题、显示用户使用指南";
        emit ApoManager::instance()->requestlogWithTime("配置文件不存在");
        // ui->widget_more->readIniValue(0,0);///WBLIU: 修改更多设置中
        exists = 0;

        ShowSysPlanEn = true;
    } else {
        emit ApoManager::instance()->requestlogWithTime("配置文件存在");
        //语言选择
        // QSettings settings(filePath,QSettings::IniFormat);
        int Language = globalSettings->value("Language").toInt();
        int Theme = globalSettings->value("Theme").toInt();

        // 应用已保存的语言设置（替代旧的 MoreSet::readIniValue）
        LanguageIdx = Language;
        qApp->removeTranslator(&tran);
        if (Language == 0) {
            tran.load(":/LanguageDemo_zh_CN.qm");
        } else if (Language == 1) {
            tran.load(":/LanguageDemo_zh_TC.qm");
        } else { // 默认英语
            tran.load(":/LanguageDemo_en_US.qm");
        }
        qApp->installTranslator(&tran);
        // 翻译器已切换，但构造函数未完成、子页面可能未就绪；
        // LanguageSet() 改到 LoginAndInit() 末尾统一调用。

        // 主题索引已保存（当前 InterfaceSettings::onThemeChanged 为预留
        // 占位，主题实际应用待该 slot 落地后由 MainWindow 统一接线）
        Q_UNUSED(Theme);

        exists = 1;

        UserGuideEn = globalSettings->value("UserGuideEn", true).toBool(); //true:显示，false:不显示

        //为了兼容之前的软件，所以通过文本中ShowSysPlanEn判断
        ShowSysPlanEn = globalSettings->value("ShowSysPlanEn", true).toBool(); //true:显示，false:不显示
        SysPlanAdd_New = globalSettings->value("SysPlanAdd_New", 0).toInt(); //为0或1则要新增。下一次则为0和1和2要新增。下次新增只新增新的，旧的保持不变

        //用户登录信息
        bool loginEn = globalSettings->value("Login/en").toBool();
        if (loginEn) {
            isLogin = true;
            //已登录，跳过登录页面
            {
                // 兼容旧版 int 型 login_type 迁移到 QString
                QVariant t_type = globalSettings->value("Login/type");
                QString t_typeStr = t_type.toString();
                if (t_typeStr == "0") t_typeStr = "email";
                else if (t_typeStr == "1") t_typeStr = "wechat";
                g_user_information.network.login_type = t_typeStr;
            }
            g_user_information.network.username = globalSettings->value("Login/nickname").toString(); //用户名
            g_user_information.network.id = globalSettings->value("Login/id").toString(); //用户ID(唯一且不可更改)

            QVariantMap map = globalSettings->value("Login/Account").toMap();
            g_user_information.network.email = map["user_email"].toString();
            g_user_information.local.user_psw = map["user_psw"].toString();
            g_user_information.network.access_token = map["access_token"].toString(); //访问令牌
        }

        // WBLIU：新增键 首页 算法 数据
        if (!globalSettings->contains("HomeMainPage/ExtraValue")) {
            // qDebug() << "首页 算法 额外EQ 键不存在";
            // 旧版本 ini 或者 新创建
            // 补充初始值（补齐 7 项 0，不依赖全局初始化器的元素数）
            HomePageExtraEQValue.clear();
            while (HomePageExtraEQValue.size() < 7)
                HomePageExtraEQValue.append(0);
            QVariantList variantList;
            for (int val : HomePageExtraEQValue) {
                variantList.append(val);
            }
            HomePageExtraEQOpen = false;
            HomePagePlansOpen = false;
            globalSettings->setValue("HomeMainPage/ExtraValue", variantList);
            globalSettings->setValue("HomeMainPage/ExtraOpen", HomePageExtraEQOpen);
            globalSettings->setValue("HomeMainPage/PlansOpen", HomePagePlansOpen);

        } else {
            HomePageExtraEQValue.clear();
            // qDebug() << "首页 算法 额外EQ 键存在";
            // HomePageExtraEQValue.reserve(7);
            QVariantList variantList = globalSettings->value("HomeMainPage/ExtraValue").toList();
            for (const QVariant &v : variantList) {
                HomePageExtraEQValue.append(v.toInt());
            }
            // 防御：补齐/截断到 7 项（旧 INI 条目 <4 时，updateValue(at(0..3)) 会抛 std::out_of_range 崩溃；
            // QList 无 resize()，不足 append(0)、超出 removeLast()）
            while (HomePageExtraEQValue.size() < 7)
                HomePageExtraEQValue.append(0);
            while (HomePageExtraEQValue.size() > 7)
                HomePageExtraEQValue.removeLast();
            HomePageExtraEQOpen = globalSettings->value("HomeMainPage/ExtraOpen").toBool();
            HomePagePlansOpen = globalSettings->value("HomeMainPage/PlansOpen").toBool();
        }
    }


    if (ShowSysPlanEn) {
        emit ApoManager::instance()->requestlogWithTime(QString("显示系统方案使能：%1").arg(ShowSysPlanEn));
        ui->page_Sperker->ShowSysPlan();
    } else {
        if (SysPlanAdd_New < 1 /*|| SysPlanVal.SysPlanRadioList_Mode.isEmpty()*/) {
            SysPlanVal = {};
            SysPlanVal_Init = {};
            SysPlanVal_Index = 0;
            ui->page_Sperker->ShowSysPlan();
        }
    }

    // if (QFile::exists(filePath))
    // {
    //     QFile::remove(filePath);
    // }

    // ui->page_Sperker->ShowPlansType();

    emit ApoManager::instance()->requestlogWithTime("ReadAllFile Over");
}

bool MainWindow::RefreshDeviceSupport(const QString &deviceGUID,
                                      unsigned short &vId,
                                      unsigned short &pId)
{
    //判断选择的设备是否被APO支持
    QString VidPid;
    VidPid = QString("VID_%1&PID_%2")
            .arg(vId, 4, 16, QChar('0'))
            .toUpper()
            .arg(pId, 4, 16, QChar('0'))
            .toUpper();
    //是否支持LHDC(即APO是否支持该设备)
    int isSupported = 0;
    int isSuccessed = apo->IsLhdcDeviceSupport(deviceGUID, VidPid,isSupported);
    return isSuccessed;
}

//机型选择后
void MainWindow::SelDevSuccess(const DeSheng::DeviceInfo &deviceInfo, const QRect &sourceGeometry)
{
    LOG_INFO("[Device] 设备连接: {} VID=0x{:04X} PID=0x{:04X}",
             deviceInfo.DeviceTypeName.toStdString(), deviceInfo.SelDev_VID,
             deviceInfo.SelDev_PID);
    RetrievePlan = false;
    if (exists && !AlreadyRead) {
        qDebug("SelDevSuccess readIni\n");
        readIni(); //读取所有。选择机型后，再读取数据。因为以前没有选择机型的方案。要默认添加为所选设备的机型
        if (SysPlanVal.SysPlanRadioList_Mode.isEmpty()) {
            SysPlanVal = {};
            SysPlanVal_Init = {};
            SysPlanVal_Index = 0;
            ui->page_Sperker->ShowSysPlan();
        }
    }
    if (!MovieVal.AllPlanRadioList_Dev.isEmpty()) {
        MovieVal.AllPlanRadioList_Dev.clear();
    }

    OpenAPOEffects();


    // WBLIU:更新首页 预设 UI
    DeviceInfo t_device_info;
    t_device_info.SelDev_device_name_ = SelDev_DeviceName;
    t_device_info.SelDev_pid_ = SelDev_PID;
    t_device_info.SelDev_vid_ = SelDev_VID;

    cl_widget_home_main_page_->cl_plans_selection_->updatePlansUIInfo(t_device_info);
    cl_widget_home_main_page_->updateUIInfo(); //同步更新 UI
    ui->page_Sperker->ShowPlansType();
    SetSpeakerMic(); //根据不同机型，首页显示不同效果的喇叭和麦克风设备,关闭该机型不需要的功能

    // ui->widget_more->DevGetVersion();
    // ui->widget_more->SoftGetVersion();///WBLIU: 修改更多设置中
    clp_user_setting_main_page_->clp_version_settings_main_page_->setDeviceIconPixmap (deviceInfo.DeviceMoreSetPixmapPath);
    clp_user_setting_main_page_->clp_contact_settings_main_page_->setQrCodePixmap (deviceInfo.DeviceMoreSetQrCodePixmapPath);

    ui->page_LoginActivationCode->UpdateDev();
    ui->page_LoginActivationCode->GetDevMsg();

    ui->pBt_mainItem->setChecked(true);
    ui->stackedWidget_SelMain->setCurrentWidget(ui->page_Selmain);

    if (!sourceGeometry.isEmpty()) {
        // WBLIU: 新版 首页产品动画
        QPoint globalStart = cl_widget_home_main_page_->mapToGlobal(sourceGeometry.topLeft());
        QPoint globalTarget = cl_widget_home_main_page_->mapToGlobal(
                    cl_widget_home_main_page_->geometry().topLeft());

        cl_widget_home_main_page_->cl_product_display_->UpdateBackgroundImage(
                    deviceInfo.DeviceHomePagePixmapPath,
                    globalStart, // 全局坐标
                    sourceGeometry.size(),
                    globalTarget, // 全局坐标
                    cl_widget_home_main_page_->cl_product_display_->geometry().size());

        /// WBLIU：旧版 首页产品动画
        // {
        //     //动画效果
        //     //GetBatteryLevel();//获取电量
        //     ui->label_img->setPixmap(QPixmap());
        //     // 将屏幕坐标转换为ui->label_img父部件的局部坐标
        //     QPoint localPos = ui->label_img->parentWidget()->mapFromGlobal(sourceGeometry.topLeft());

        //     // 创建临时动画标签
        //     QLabel *flyLabel = new QLabel(ui->label_img->parentWidget());
        //     flyLabel->setStyleSheet(QString("border-image: url(%1);").arg(imagePath));
        //     //flyLabel->setPixmap(pixmap.scaled(80, 80, Qt::KeepAspectRatio));
        //     //flyLabel->setGeometry(sourceGeometry);
        //     flyLabel->move(localPos);
        //     flyLabel->resize(sourceGeometry.size());
        //     flyLabel->raise();
        //     flyLabel->show();

        //     // 计算目标位置（第二页中心）
        //     //ui->stackedWidget->setCurrentIndex(1);
        //     QRect targetRect = ui->label_img->geometry();
        //     //targetRect.moveCenter(ui->label_img->parentWidget()->rect().center());

        //     // 计算目标位置的中央坐标
        //     QPoint targetCenter = targetRect.center();
        //     QPoint targetLeft = targetRect.topLeft();

        //     QPoint localStartPoint = mapFromGlobal(localPos);
        //     QPoint localTargetPoint = mapFromGlobal(
        //         cl_widget_home_main_page_->cl_product_display_->rect().topLeft());

        //     // 创建动画组
        //     QParallelAnimationGroup *animGroup = new QParallelAnimationGroup(this);

        //     // 位置动画
        //     QPropertyAnimation *posAnim = new QPropertyAnimation(flyLabel, "pos");
        //     posAnim->setDuration(300); //播放时间 800 ms
        //     posAnim->setStartValue(flyLabel->pos());
        //     //posAnim->setEndValue(targetRect.topRight());
        //     // 关键修改：计算控件中心点移动后的左上角坐标
        //     QSize finalSize = targetRect.size();
        //     QPoint targetTopLeft = targetCenter
        //                            - QPoint(finalSize.width() / 2, finalSize.height() / 2 - 50);
        //     //QPoint targetTopLeft = targetCenter - QPoint(0, finalSize.height()/2 - 50);
        //     posAnim->setEndValue(targetLeft);

        //     // 缩放动画 - 放大到目标尺寸
        //     QPropertyAnimation *sizeAnim = new QPropertyAnimation(flyLabel, "size");
        //     sizeAnim->setDuration(300); //播放时间 800 ms
        //     sizeAnim->setStartValue(flyLabel->size());
        //     //sizeAnim->setEndValue(targetRect.size()/3);
        //     sizeAnim->setEndValue(targetRect.size());

        //     // 添加缓动曲线使动画更平滑
        //     posAnim->setEasingCurve(QEasingCurve::OutQuad);
        //     sizeAnim->setEasingCurve(QEasingCurve::OutQuad);

        //     animGroup->addAnimation(posAnim);
        //     animGroup->addAnimation(sizeAnim);

        //     QTimer *timeoutTimer = new QTimer(this);
        //     timeoutTimer->setSingleShot(true);
        //     timeoutTimer->setInterval(500); // 比动画时长稍长

        //     // 动画完成后的处理
        //     connect(animGroup, &QParallelAnimationGroup::finished, [=]() {
        //         timeoutTimer->stop();
        //         flyLabel->hide();
        //         flyLabel->deleteLater();

        //         QImage Image;
        //         Image.load(imagePath);
        //         QPixmap pixmap = QPixmap::fromImage(Image);
        //         int with = 1920;
        //         int height = 1080;
        //         //QPixmap fitpixmap = pixmap.scaled(with, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);  // 饱满填充
        //         QPixmap fitpixmap = pixmap.scaled(with,
        //                                           height,
        //                                           Qt::KeepAspectRatio,
        //                                           Qt::SmoothTransformation); // 按比例缩放
        //         ui->label_img->setPixmap(fitpixmap);

        //         ui->label_img->show();
        //     });
        //     // 超时处理
        //     connect(timeoutTimer, &QTimer::timeout, this, [=]() {
        //         timeoutTimer->stop();
        //         if (animGroup->state() == QAbstractAnimation::Running) {
        //             animGroup->stop(); // 强制停止动画
        //         }
        //         flyLabel->hide();
        //         flyLabel->deleteLater();
        //         // 执行后备操作：直接显示图片
        //         QImage Image;
        //         Image.load(imagePath);
        //         QPixmap pixmap = QPixmap::fromImage(Image);
        //         int with = 1920;
        //         int height = 1080;
        //         //QPixmap fitpixmap = pixmap.scaled(with, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);  // 饱满填充
        //         QPixmap fitpixmap = pixmap.scaled(with,
        //                                           height,
        //                                           Qt::KeepAspectRatio,
        //                                           Qt::SmoothTransformation); // 按比例缩放
        //         ui->label_img->setPixmap(fitpixmap);

        //         ui->label_img->show();
        //     });

        //     animGroup->start(QAbstractAnimation::DeleteWhenStopped);
        //     timeoutTimer->start();
        // }

    } else {
        /// WBLIU：新版
        if (cl_widget_home_main_page_->cl_product_display_->animGroup->state()
                == QAbstractAnimation::Running) {
            cl_widget_home_main_page_->cl_product_display_->animGroup->stop(); // 强制停止动画
        }
        cl_widget_home_main_page_->cl_product_display_->flyLabel->hide();
        cl_widget_home_main_page_->cl_product_display_->UpdateBackgroundImageImmediately(
                    deviceInfo.DeviceHomePagePixmapPath);

        // /// WBLIU：旧版
        // {
        //     QImage Image;
        //     Image.load(imagePath);
        //     QPixmap pixmap = QPixmap::fromImage(Image);
        //     int with = 1920;
        //     int height = 1080;
        //     //QPixmap fitpixmap = pixmap.scaled(with, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);  // 饱满填充
        //     QPixmap fitpixmap = pixmap.scaled(with,
        //                                       height,
        //                                       Qt::KeepAspectRatio,
        //                                       Qt::SmoothTransformation); // 按比例缩放
        //     ui->label_img->setPixmap(fitpixmap);

        //     ui->label_img->show();
        // }
    }

    if (retB) {
        //机型选择后，才有效
        QVector<bool> enables(10, true);
        emit ApoManager::instance()->requestSetExtendEqualizerBandEnableEx(0, enables);
        emit ApoManager::instance()->requestSetExtendEqualizerBandEnableEx(1, enables);
        enables = {true, false, false, false, false, false, false, false, false, false};
        emit ApoManager::instance()->requestSetExtendEqualizerBandEnableEx(2, enables); //脚步增强

        enables = {true, true, false, false, false, false, false, false, false, false};
        emit ApoManager::instance()->requestSetExtendEqualizerBandEnableEx(3, enables); //枪声优化

        enables = {true, false, false, false, false, false, false, false, false, false};
        emit ApoManager::instance()->requestSetExtendEqualizerBandEnableEx(4, enables); //声场控制

        enables = {true, false, false, false, false, false, false, false, false, false};
        emit ApoManager::instance()->requestSetExtendEqualizerBandEnableEx(5, enables); //清晰度

        enables = {true, true, true, false, false, false, false, false, false, false};
        emit ApoManager::instance()->requestSetExtendEqualizerBandEnableEx(6, enables); //余音消除

        enables = {true, false, false, false, false, false, false, false, false, false};
        emit ApoManager::instance()->requestSetExtendEqualizerBandEnableEx(7, enables); //空间混响

        enables = {true, true, true, false, false, false, false, false, false, false};
        emit ApoManager::instance()->requestSetExtendEqualizerBandEnableEx(8, enables); //风声弱化
        QVector<bool> enables2(10, false);
        emit ApoManager::instance()->requestSetExtendEqualizerBandEnableEx(9, enables2); //不使用
        emit ApoManager::instance()->requestSetExtendEqState(9, false); //每一组的总开关

        emit ApoManager::instance()->requestSetArEffectState(true); //不开启，则无音效
    }

    // ui->widget_status->setStyleSheet("background-color: rgb(14, 17, 22);");

    //强制触发resizeEvent
    // this->resize(this->size() - QSize(1, 1));
    // this->resize(this->size() + QSize(1, 1));
    //resizeEvent(NULL);
    updateSize();

    ui->page_Sperker->IntegratePlansAndCompatible();
}
void SetCBoxShadow(NewComboBox *cBox)
{
    QWidget *containerObj = cBox->view()->parentWidget();
    if (containerObj) {
        containerObj->setWindowFlags(containerObj->windowFlags() | Qt::FramelessWindowHint
                                     | Qt::NoDropShadowWindowHint);
        //containerObj->setAttribute(Qt::WA_TranslucentBackground);
        containerObj->setStyleSheet("background-color: #313A48;border: 1px solid #1A1A1A;");
        // 创建阴影效果
        QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(containerObj);
        shadowEffect->setBlurRadius(8);               // 阴影模糊半径
        shadowEffect->setColor(QColor(0, 0, 0, 179)); // 颜色
        shadowEffect->setOffset(0, 0);                // 阴影偏移

        containerObj->setGraphicsEffect(shadowEffect);

        //设置QAbstractItemView样式
        cBox->view()->setStyleSheet(R"(
                                    QListView{border-radius: 2px;padding-left: 17px;padding-right: 17px;color: rgb(206, 207, 211);outline: 0;selection-background-color: transparent !important;;show-decoration-selected: 0 !important;;}
                                    QListView::item {height: 31px;padding-right: 20px;border-bottom: 1px solid rgba(216, 216, 216, 0.1);}

                                    QListView::item:selected{background-color: transparent;background-image: url(:/Skin/Images/cBox/item_se.png);background-repeat: no-repeat;background-position:right center;}
                                    QListView::item:hover {
                                        background-color: transparent;
                                    }
                                    QListView::item:focus {
                                        background-color: transparent;
                                        outline: 0;
                                    }
                                    QListView::item:selected:hover {
                                        background-color: transparent;
                                        outline: 0;
                                    }
                                    QListView::item:selected:focus {
                                        background-color: transparent;
                                        outline: 0;
                                    }
                )");
    }
}
//根据不同机型，首页显示不同效果的喇叭和麦克风设备,关闭该机型不需要的功能
void MainWindow::SetSpeakerMic()
{
    bool openHIDLib = false; //true;
    {
        // WBLIU ：旧版
        // cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_->setDisplayIcon(
        //     QIcon(":/Skin/Images/cBox/sel.png"));
        // // WBLIU:新版（cl_cBox_Mic_）
        // cl_widget_home_main_page_->cl_microphone_setting_->cl_cBox_Mic_->setDisplayIcon(
        //     QIcon(":/Skin/Images/cBox/sel.png"));
        ui->widget_eq->ShowDRC();
        cl_widget_home_main_page_->cl_speaker_setting_->changeTextColor(cl_widget_home_main_page_->cl_speaker_setting_->pBt_spk_switch_->isChecked (),true);
        cl_widget_home_main_page_->cl_microphone_setting_->changeTextColor(cl_widget_home_main_page_->cl_microphone_setting_->pBt_mic_switch_->isChecked (),true);

    }


    // //获得系统音量
    // Volume(eRender);
    // Volume(eCapture);

    // ui->cBox_Speaker->setView(new QListView());
    // ui->cBox_Mic->setView(new QListView());

    {

        QString styleSheet = (R"(QComboBox{color: #B2A1A8B3;border:null;}QComboBox::drop-down{height:0px;width:0px;})");
        // {
        //     // WBLIU: 旧版
        //     ui->cBox_Speaker->setStyleSheet(styleSheet);
        //     ui->cBox_Speaker->setAttribute(Qt::WA_TransparentForMouseEvents, true); // 忽略鼠标事件
        //     ui->cBox_Speaker->setFocusPolicy(Qt::NoFocus);                          // 移除焦点
        // }
        {
            {
                // WBLIU::2026.06.06
                cl_widget_home_main_page_->cl_speaker_setting_->changeTextColor(cl_widget_home_main_page_->cl_speaker_setting_->pBt_spk_switch_->isChecked (),true);
                cl_widget_home_main_page_->cl_microphone_setting_->changeTextColor(cl_widget_home_main_page_->cl_microphone_setting_->pBt_mic_switch_->isChecked (),true);
            }

            cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_
                    ->setAttribute(Qt::WA_TransparentForMouseEvents, true); // 忽略鼠标事件
            cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_->setFocusPolicy(
                        Qt::NoFocus); // 移除焦点
        }

        {
            cl_widget_home_main_page_->cl_microphone_setting_->changeTextColor(cl_widget_home_main_page_->cl_microphone_setting_->pBt_mic_switch_->isChecked (),true);

        }
        cl_widget_home_main_page_->cl_microphone_setting_->cl_cBox_Mic_
                ->setAttribute(Qt::WA_TransparentForMouseEvents, true); // 忽略鼠标事件
        cl_widget_home_main_page_->cl_microphone_setting_->cl_cBox_Mic_->setFocusPolicy(Qt::NoFocus); // 移除焦点


        // //T10有线不连接HIDAPI
        // if(SelDev_DeviceName.contains("T10",Qt::CaseInsensitive))
        // {
        //     if(!SelDev_DeviceName.contains("Wireless",Qt::CaseInsensitive))
        //     {
        //         openHIDLib = false;
        //     }
        // }

        /*//T10无线和K06S无线，连接HIDAPI
        if(SelDev_DeviceName.contains("T10",Qt::CaseInsensitive))
        {
            if(SelDev_DeviceName.contains("Wireless",Qt::CaseInsensitive))
            {
                openHIDLib = true;
                emit ApoManager::instance()->requestlogWithTime("openHIDLib");
            }
        }else if(SelDev_DeviceName.contains("K06S",Qt::CaseInsensitive))
        {
            openHIDLib = true;
            emit ApoManager::instance()->requestlogWithTime("openHIDLib");
        }*/

        if ((SelDev_DeviceName.contains("T10", Qt::CaseInsensitive)&& SelDev_DeviceName.contains("Wireless", Qt::CaseInsensitive))
                || SelDev_DeviceName.contains("K06S", Qt::CaseInsensitive)
                || (SelDev_DeviceName.contains("T7", Qt::CaseInsensitive)&& (SelDev_PID == 0xF014 || SelDev_PID == 0xF008))
                || (SelDev_DeviceName.contains("K03S", Qt::CaseInsensitive) && (SelDev_PID == 0xF016 || SelDev_PID == 0xF017))
                || (SelDev_DeviceName.contains("T7 GT",Qt::CaseInsensitive) && (SelDev_PID == 0xF009 || SelDev_PID == 0xF015))
                || (SelDev_DeviceName.contains("S21",Qt::CaseInsensitive) && (SelDev_PID == 0xC001))
                )
        {
            openHIDLib = true;
            emit ApoManager::instance()->requestlogWithTime("openHIDLib");
        }

        Mapping();
        if(openHIDLib)
        {
            //导入HIDAPI
            if(lolib->openCard() == 1)
                //if(lolib->openANDswitchCard() == 1)
            {
                isHidRun = true;

                // ui->widget_more->DevGetVersion();///WBLIU: 修改更多设置中

            }else
            {
                isHidRun = false;
                //msgBox.critical(NULL,tr("错误"),tr("2.4G 模式，耳机未连接，电量显示与固件升级不可使用"),tr("关闭"));//(第一版T10无线没有该协议)
                showDevSta(false,false);

                lolib->closeCard();

                //开启定时器，定时重连，重连三次没成功，则显示重连按钮
                if (!timer_connect->isActive())
                {
                    on_pBt_Reconnect_clicked();
                }

            }
            if(isHidRun)
            {
                int res = GetDevSta();
                if(res == 1)
                {
                    // showDevSta(true,true);
                    TimerR->start(); // 启动定时器
                }else if(res == -1)
                {
                    emit ApoManager::instance()->requestlogWithTime("获得设备状态返回-1");
                    // //此处这样写，第二个参数为true，是因为前期一些无线耳机没有hid协议，所有返回失败
                    showDevSta(false,true);
                }else if(res == 0)
                {
                    //开启定时器，定时重连，重连三次没成功，则显示重连按钮
                    if (!timer_connect->isActive())
                    {
                        on_pBt_Reconnect_clicked();
                    }
                }

            }
        }else
        {
            showDevSta(false,false);
            ui->pBt_Reconnect->hide();
        }

        General = false;
        //将当前耳机的音频输出设备加载到下拉框
        // ui->cBox_Speaker->clear(); // WBLIU :旧版
        cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_->clear();   //  WBLIU :新版

        //QHash<QString,  QHash<QString, QString>> devices = DefaultOutput::enumDevices2(eRender);
        int cnt = 0;
        for( QHash<QString,  QHash<QString, QString>>::iterator iter = ERdevs.begin(); iter != ERdevs.end(); iter++ )
        {
            QString name = iter.value().value("Name");
            if(name.contains(SelDev_DeviceName))
            {
                qDebug("ERdevs cnt:%d\n",cnt++);
                if(SelDev_DeviceName.contains("Wireless", Qt::CaseInsensitive))
                {
                    if(!name.contains("Wireless", Qt::CaseInsensitive))
                    {
                        continue;
                    }
                }else if(!SelDev_DeviceName.contains("Wireless", Qt::CaseInsensitive))
                {
                    if(name.contains("Wireless", Qt::CaseInsensitive))
                    {
                        continue;
                    }
                }

                // QHash<QString, QString> Msg = iter.value();
                // QString name = Msg.value("Name");
                // unsigned short vid = Msg.value("vid").toUShort();
                // unsigned short pid = Msg.value("pid").toUShort();


                SelDev_DeviceGuid = iter.key();

                // {
                //     // WBLIU ： 旧版
                //     ui->cBox_Speaker->blockSignals(true);
                //     ui->cBox_Speaker->addItem(QIcon(":/Skin/Images/cBox/sel.png"),
                //                               name,
                //                               iter.key());
                //     ui->cBox_Speaker->blockSignals(false);
                // }

                {
                    // WBLIU ： 新版
                    cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_->blockSignals(true);
                    cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_
                            ->addItem(QIcon(":/Skin/Images/cBox/sel.png"), name, iter.key());
                    cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_->blockSignals(false);
                }

                //ui->cBox_Speaker->addItem(iter.value(), iter.key() );
                // on_cBox_Speaker_currentIndexChanged(0);  /// WBLIU：旧版
                cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_->setCurrentIndex(0);
                emit cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_->currentIndexChanged(0);

            }
        }
        // WBLIU:新版（cl_cBox_Mic_）
        cl_widget_home_main_page_->cl_microphone_setting_->cl_cBox_Mic_->clear();
        QHash<QString,  QHash<QString, QString>> devices2 = DefaultOutput::enumDevices2(eCapture);
        for( QHash<QString,  QHash<QString, QString>>::iterator iter = devices2.begin(); iter != devices2.end(); iter++ )
        {
            QString name = iter.value().value("Name");
            if(name.contains(SelDev_DeviceName))
            {

                if(SelDev_DeviceName.contains("Wireless", Qt::CaseInsensitive))
                {
                    // if(!iter.value().value("Name").contains("Wireless", Qt::CaseInsensitive))
                    if(!name.contains("Wireless", Qt::CaseInsensitive))
                    {
                        continue;
                    }
                }else if(!SelDev_DeviceName.contains("Wireless", Qt::CaseInsensitive))
                {
                    if(name.contains("Wireless", Qt::CaseInsensitive))
                    {
                        continue;
                    }
                }
                // QHash<QString, QString> Msg = iter.value();
                // // QString name = Msg.value("Name");
                // unsigned short vid = Msg.value("vid").toUShort();
                // unsigned short pid = Msg.value("pid").toUShort();

                // qDebug()<<"cBox_Mic->addItem:"<<name;
                // WBLIU:新版（cl_cBox_Mic_）
                cl_widget_home_main_page_->cl_microphone_setting_->cl_cBox_Mic_->blockSignals(true);
                cl_widget_home_main_page_->cl_microphone_setting_->cl_cBox_Mic_->addItem(QIcon(":/Skin/Images/cBox/sel.png"),name, iter.key() );
                cl_widget_home_main_page_->cl_microphone_setting_->cl_cBox_Mic_->blockSignals(false);
                //ui->cBox_Mic->addItem(iter.value(), iter.key() );
                SetMicCurrentIndexChanged(0);
            }
        }
    }

    //获得系统音量
    Volume(eRender);
    Volume(eCapture);

    //emit ApoManager::instance()->requestSetLhdcDevice(SelDev_DeviceGuid);
}
//删除音频设备的槽函数
void MainWindow::onDeviceDel(QString deviceName)
{
    General = true;
    int idx = cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_->findText(deviceName);
    if(idx != -1)
    {
        //  WBLIU :新版
        cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_->blockSignals(true);
        cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_->removeItem(idx);
        cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_->blockSignals(false);
    }
    // WBLIU:新版（cl_cBox_Mic_）
    idx = cl_widget_home_main_page_->cl_microphone_setting_->cl_cBox_Mic_->findText(deviceName);
    if(idx != -1)
    {
        cl_widget_home_main_page_->cl_microphone_setting_->cl_cBox_Mic_->blockSignals(true);
        cl_widget_home_main_page_->cl_microphone_setting_->cl_cBox_Mic_->removeItem(idx);
        cl_widget_home_main_page_->cl_microphone_setting_->cl_cBox_Mic_->blockSignals(false);
    }

    ERdevs.clear();
    General = false; //非查找通用模式下设备


    if(deviceName.contains(SelDev_DeviceName,Qt::CaseInsensitive))
    {
        CloseTimer();
        // 视频播放中则暂停；存在悬浮小窗口（画中画）则关闭
        if (widget_listenSpeaker)
        {
            FFmpegMainPage *t_ffmpegPage = widget_listenSpeaker->cl_sound_test_main_page_->cl_ffmpeg_main_page_.get();
            if (t_ffmpegPage)
            {
                auto *t_ffmpegGlobal = t_ffmpegPage->cl_ffmpeg_global_.get();
                if (t_ffmpegGlobal && !t_ffmpegGlobal->current_media_filename().isEmpty()
                    && !t_ffmpegGlobal->cl_is_stop())
                {
                    t_ffmpegPage->pause(); // 视频播放中，暂停
                }
                if (t_ffmpegPage->minView)
                {
                    t_ffmpegPage->onMinWidgetSlots(false); // 关闭悬浮小窗口
                }
            }
        }
        ui->stackedWidget_SelMain->setCurrentWidget(ui->page_devSel);
    }
    QThread::msleep(1);  // 当前线程休眠 1秒，不阻塞其他线程
    ERdevs = DefaultOutput::enumDevices2(eRender);

    //选择设备界面显示
    ui->page_devSel->UpdateDeviceSelectionMainPage(); //更新机型选择界面

}
//添加音频设备的槽函数
void MainWindow::onDeviceAdd(QString deviceName)
{
    QThread::msleep(1);  // 当前线程休眠 1秒，不阻塞其他线程
    ERdevs.clear();
    General = false; //非查找通用模式下设备
    ERdevs = DefaultOutput::enumDevices2(eRender);
    //选择设备界面显示
    // ui->page_devSel->clearExistingLayout();//先清理旧按钮
    // ui->page_devSel->CreateButton();
    ui->page_devSel->UpdateDeviceSelectionMainPage(); //更新机型选择界面
}
//更改输出设备的默认音频
void MainWindow::onDefaultOutPutDeviceChanged(QString deviceName)
{
    //输出设备
    // int idx = ui->cBox_Speaker->findText(deviceName);   // WBLIU : 旧版
    int idx = cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_->findText(deviceName);   // WBLIU : 新版

    emit ApoManager::instance()->requestlogWithTime(QString("OutPutDeviceChanged idx:%1,deviceName:%2").arg(idx).arg(deviceName));
    if(idx != -1)
    {

        QString styleSheet = (R"(QComboBox{color: #B2A1A8B3;border:null;}QComboBox::drop-down{height:0px;width:0px;})");

        // group_S->stop();    /// WBLIU:旧版
        // {
        //     //  WBLIU : 旧版
        //     ui->cBox_Speaker->setStyleSheet(styleSheet);
        //     ui->cBox_Speaker->setDisplayIcon(QIcon(":/Skin/Images/cBox/sel.png"));

        //     ui->cBox_Speaker->blockSignals(true);
        //     ui->cBox_Speaker->setCurrentIndex(idx);
        //     ui->cBox_Speaker->blockSignals(false);
        // }

        cl_widget_home_main_page_->cl_speaker_setting_->cl_sequential_animation_group_->stop(); /// WBLIU: 新版
        {
            //  WBLIU : 新版
            // cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_->setStyleSheet(styleSheet);
            cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_->setDisplayIcon(QIcon(":/Skin/Images/cBox/sel.png"));

            cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_->blockSignals(true);
            cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_->setCurrentIndex(idx);
            cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_->blockSignals(false);
        }


    }else
    {
        QString styleSheet = (R"(QComboBox{color: #B2A1A8B3;border:null;}QComboBox::drop-down{height:0px;width:0px;})");
        {
            // WBLIU：新版
            // cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_->setStyleSheet(styleSheet);

            QString txt = "请将播放设备切换至" + SelDev_DeviceName;
            cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_->addItem(QIcon(":/Skin/Images/cBox/dis.png"), txt, NULL);
            cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_->setDisplayIcon(QIcon(":/Skin/Images/cBox/dis.png"));
            int index = cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_->count() - 1; // 新添加的项位于最后
            cl_widget_home_main_page_->cl_speaker_setting_->cl_spk_->setCurrentIndex(index);
            cl_widget_home_main_page_->cl_speaker_setting_->cl_sequential_animation_group_->start();
        }
    }
}
//更改输入设备的默认音频
void MainWindow::onDefaultInPutDeviceChanged(QString deviceName)
{
    //输入设备
    // WBLIU:新版（cl_cBox_Mic_）
    int idx = cl_widget_home_main_page_->cl_microphone_setting_->cl_cBox_Mic_->findText(deviceName);
    emit ApoManager::instance()->requestlogWithTime(QString("InPutDeviceChanged idx:%1,deviceName:%2").arg(idx).arg(deviceName));
    if(idx != -1)
    {
        cl_widget_home_main_page_->cl_microphone_setting_->cl_sequential_animation_group_->stop();
        QString styleSheet = (R"(QComboBox{color: #B2A1A8B3;border:null;}QComboBox::drop-down{height:0px;width:0px;})");
        // cl_widget_home_main_page_->cl_microphone_setting_->cl_cBox_Mic_->setStyleSheet(styleSheet);
        cl_widget_home_main_page_->cl_microphone_setting_->cl_cBox_Mic_->setDisplayIcon(QIcon(":/Skin/Images/cBox/sel.png"));

        cl_widget_home_main_page_->cl_microphone_setting_->cl_cBox_Mic_->blockSignals(true);
        cl_widget_home_main_page_->cl_microphone_setting_->cl_cBox_Mic_->setCurrentIndex(idx);
        cl_widget_home_main_page_->cl_microphone_setting_->cl_cBox_Mic_->blockSignals(false);
    }else
    {
        QString styleSheet = (R"(QComboBox{color: #B2A1A8B3;border:null;}QComboBox::drop-down{height:0px;width:0px;})");
        // cl_widget_home_main_page_->cl_microphone_setting_->cl_cBox_Mic_->setStyleSheet(styleSheet);

        QString txt = "请将录制设备切换至" + SelDev_DeviceName;
        cl_widget_home_main_page_->cl_microphone_setting_->cl_cBox_Mic_->addItem(QIcon(":/Skin/Images/cBox/dis.png"),txt, NULL );
        cl_widget_home_main_page_->cl_microphone_setting_->cl_cBox_Mic_->setDisplayIcon(QIcon(":/Skin/Images/cBox/dis.png"));
        int index = cl_widget_home_main_page_->cl_microphone_setting_->cl_cBox_Mic_->count() - 1;  // 新添加的项位于最后
        cl_widget_home_main_page_->cl_microphone_setting_->cl_cBox_Mic_->setCurrentIndex(index);

        cl_widget_home_main_page_->cl_microphone_setting_->cl_sequential_animation_group_->start();
    }
}

// 在 QWidget 子类中重写 mouseDoubleClickEvent 方法
void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // 检查是否点击在顶部0-29像素的标题栏区域
        if (event->pos().y() <= 29) // 鼠标按在窗口顶部0-29像素范围内
        {
            // 切换最大化/正常状态
            bool en = ui->pBt_max->isChecked();
            on_pBt_max_clicked(!en);
            ui->pBt_max->setChecked(!en);
            event->accept();
            return;
        }
    } else {
        // QWidget::mouseDoubleClickEvent(event);
        event->ignore();
    }
}
//窗体拖动
void MainWindow::mousePressEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton)//当鼠标左键按下时
    {
        // 无边框窗体边缘拉伸：命中边缘条时记录拉伸状态，
        // 在 mouseMoveEvent 中手动 setGeometry 完成拉伸
        // （QWindow::startSystemResize 在 Qt 5.15 Windows 平台实现不可靠，改用纯 Qt 方案）
        if (!isMaximized() && !isFullScreen()) {
            Qt::Edges t_edges = edgesAtPos(event->pos());
            if (t_edges) {
                m_resizeEdges = t_edges;
                m_resizeStartPos = event->globalPos();
                m_resizeStartGeometry = frameGeometry();
                event->accept();
                return;
            }
        }
        QRect innerRect = rect().adjusted(25, 80, -25, -50);
        if (!innerRect.contains(event->pos()))//鼠标按在在窗口四周边缘100像素范围内区域
        {
            dragPosition = event->globalPos() - this->frameGeometry().topLeft();//计算拖动起始位置(dragPosition)：鼠标全局坐标减去窗口左上角坐标
            isDragging = true;
            event->accept();//事件已被处理
            return;
        }
    }
    event->ignore();
}
// 按鼠标总位移计算并应用新窗口几何（含最小尺寸钳制），客户区/原生两条拉伸路径共用
void MainWindow::applyResizeDelta(const QPoint &t_delta)
{
    QRect t_newGeo = m_resizeStartGeometry;
    if (m_resizeEdges & Qt::LeftEdge) t_newGeo.setLeft(m_resizeStartGeometry.left() + t_delta.x());
    if (m_resizeEdges & Qt::RightEdge) t_newGeo.setRight(m_resizeStartGeometry.right() + t_delta.x());
    if (m_resizeEdges & Qt::TopEdge) t_newGeo.setTop(m_resizeStartGeometry.top() + t_delta.y());
    if (m_resizeEdges & Qt::BottomEdge) t_newGeo.setBottom(m_resizeStartGeometry.bottom() + t_delta.y());

    // 左/上边缘拉伸时需锚定右/下边缘，防止尺寸小于最小值
    const QSize t_min = minimumSize();
    if (t_newGeo.width() < t_min.width()) {
        if (m_resizeEdges & Qt::LeftEdge) t_newGeo.setLeft(t_newGeo.right() - t_min.width() + 1);
        else t_newGeo.setRight(t_newGeo.left() + t_min.width() - 1);
    }
    if (t_newGeo.height() < t_min.height()) {
        if (m_resizeEdges & Qt::TopEdge) t_newGeo.setTop(t_newGeo.bottom() - t_min.height() + 1);
        else t_newGeo.setBottom(t_newGeo.top() + t_min.height() - 1);
    }

    setGeometry(t_newGeo);
}

void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    // 边缘拉伸：根据鼠标位移计算新窗口几何，并夹紧最小尺寸
    // 注：客户区按下路径（m_resizeNative == false）在此处理；
    // NC 按下路径（m_resizeNative == true）由 nativeEvent 原生驱动，消息被吞不走到这里
    if (m_resizeEdges && !m_resizeNative) {
        // 拉伸过程中强制保持箭头光标（WM_SETCURSOR 判定区外的兜底）
        LPCTSTR t_cursorId = resizeCursorId(m_resizeEdges);
        if (t_cursorId) SetCursor(LoadCursor(nullptr, t_cursorId));
        applyResizeDelta(event->globalPos() - m_resizeStartPos);
        event->accept();
        return;
    }

    if( (event->buttons() & Qt::LeftButton) && isDragging)//当鼠标左键按下且移动 且鼠标按在在窗口四周边缘区域时拖动
    {
        if(ui->pBt_max->isChecked())
        {
            ui->pBt_max->blockSignals(true);
            ui->pBt_max->setChecked(false);
            ui->pBt_max->blockSignals(false);
            this->setWindowState(Qt::WindowNoState);
        }
        move(event->globalPos() - dragPosition);//移动窗口到新位置（当前鼠标全局坐标减去之前保存的dragPosition差值）
        event->accept();
    }
}
void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    isDragging = false;
    ReleaseCapture();// 释放 NC 按下拉伸时设置的鼠标捕获
    const bool t_wasResizing = bool(m_resizeEdges);
    m_resizeEdges = Qt::Edges();//结束边缘拉伸
    m_resizeNative = false;
    if (t_wasResizing) {
        updateBackgroundCache(); // 拉伸结束，补齐拖动期间跳过的背景/模糊重算
    }
}

QVariantMap MainWindow::SysValToVariantMap(const SysVal &sysVal)
{
    QVariantMap map;
    map["SysPlanCnt"] = sysVal.SysPlanRadioList_Mode.count();

    // 创建嵌套map存储按钮信息
    QVariantMap planMap;  // 存储所有按钮的map
    // 保存每个按钮的基本信息(名称、是否从所有预设添加(添加后从未修改)、从哪个所有预设的方案添加的、PlanVal结构体、)
    for (int i = 0; i < sysVal.SysPlanRadioList_Mode.size(); ++i) {
        if (sysVal.SysPlanRadioList_Mode[i]) {
            // 为每个按钮创建子map
            QVariantMap buttonMap;

            // 保存按钮属性
            // 保存按钮名称
            buttonMap["Name"] = sysVal.SysPlanRadioList_Mode[i]->lab_name->text();
            // 保存按钮描述
            buttonMap["Description"] = sysVal.SysPlanRadioList_Mode[i]->property("fullText");
            //标签1
            buttonMap["Lab1"] = sysVal.SysPlanRadioList_Mode[i]->getLabDevs();
            //标签2
            buttonMap["Lab2"] = sysVal.SysPlanRadioList_Mode[i]->lab2->text();
            //所属分类（0:没有，1：分类1，2：分类2，3：分类3,....8：分类8）
            buttonMap["PlanTypeIdx"] = sysVal.SysPlanRadioList_Mode[i]->PlanPageSel;
            //分享码（id）
            buttonMap["ShareCodeId"] = sysVal.SysPlanRadioList_Mode[i]->ShareCodeId;
            //分享码
            buttonMap["ShareCode"] = sysVal.SysPlanRadioList_Mode[i]->ShareCode;

            // 保存PlanVal结构体字段
            QVariantMap planValMap;
            planValMap["IsLoad"] = sysVal.SysPlanRadioList_Mode[i]->IsLoad;
            planValMap["DataVisibleEn"] = sysVal.SysPlanRadioList_Mode[i]->getAllPlanValue().DataVisibleEn;
            planValMap["ParentPlanName"] = sysVal.SysPlanRadioList_Mode[i]->getAllPlanValue().ParentPlanName;
            planValMap["AlgoOpenEn"] = sysVal.SysPlanRadioList_Mode[i]->getAllPlanValue().AlgoOpenEn;
            planValMap["spaceOpenEn"] = sysVal.SysPlanRadioList_Mode[i]->getAllPlanValue().spaceOpenEn;
            planValMap["eqOpenEn"] = sysVal.SysPlanRadioList_Mode[i]->getAllPlanValue().eqOpenEn;
            planValMap["drcOpenEn"] = sysVal.SysPlanRadioList_Mode[i]->getAllPlanValue().drcOpenEn;
            //额外eq
            QVariantList ExtraEqValList;
            for (int j = 0; j < 7; ++j) {
                ExtraEqValList.append(sysVal.SysPlanRadioList_Mode[i]->getAllPlanValue().ExtraEq[j]);
            }
            planValMap["ExtraEq"] = ExtraEqValList;

            planValMap["lowVal"] = sysVal.SysPlanRadioList_Mode[i]->getAllPlanValue().lowVal;
            planValMap["drcVal"] = sysVal.SysPlanRadioList_Mode[i]->getAllPlanValue().drcVal;
            planValMap["GainVal"] = sysVal.SysPlanRadioList_Mode[i]->getAllPlanValue().GainVal;
            planValMap["spaceVal"] = sysVal.SysPlanRadioList_Mode[i]->getAllPlanValue().spaceVal;
            planValMap["spaceReverb"] = sysVal.SysPlanRadioList_Mode[i]->getAllPlanValue().spaceReverb;
            planValMap["spaceSize"] = sysVal.SysPlanRadioList_Mode[i]->getAllPlanValue().spaceSize;

            //添加频点值
            QVariantList freqValList;
            for (int j = 0; j < 10; ++j) {
                freqValList.append(sysVal.SysPlanRadioList_Mode[i]->getAllPlanValue().freqVal[j]);
            }
            planValMap["freqVal"] = freqValList;
            // 添加eqVal数组 int eqVal[10]
            QVariantList eqValList;
            for (int j = 0; j < 10; ++j) {
                eqValList.append(sysVal.SysPlanRadioList_Mode[i]->getAllPlanValue().eqVal[j]);
            }
            planValMap["eqVal"] = eqValList;
            // 添加qVal数组 double qVal[10]
            QVariantList qValList;
            for (int j = 0; j < 10; ++j) {
                qValList.append(sysVal.SysPlanRadioList_Mode[i]->getAllPlanValue().qVal[j]);
            }
            planValMap["qVal"] = qValList;
            //添加滤波器
            QVariantList filterValList;
            for (int j = 0; j < 10; ++j) {
                filterValList.append(sysVal.SysPlanRadioList_Mode[i]->getAllPlanValue().filterVal[j]);
            }
            planValMap["filterVal"] = filterValList;


            //二创内容
            //添加频点值
            QVariantList freqValList_deriv;
            for (int j = 0; j < 10; ++j) {
                freqValList_deriv.append(sysVal.SysPlanRadioList_Mode[i]->getAllPlanValue().freqVal_deriv[j]);
            }
            planValMap["freqVal_deriv"] = freqValList_deriv;
            // 添加eqVal数组 int eqVal[10]
            QVariantList eqValList_deriv;
            for (int j = 0; j < 10; ++j) {
                eqValList_deriv.append(sysVal.SysPlanRadioList_Mode[i]->getAllPlanValue().eqVal_deriv[j]);
            }
            planValMap["eqVal_deriv"] = eqValList_deriv;
            // 添加qVal数组 double qVal[10]
            QVariantList qValList_deriv;
            for (int j = 0; j < 10; ++j) {
                qValList_deriv.append(sysVal.SysPlanRadioList_Mode[i]->getAllPlanValue().qVal_deriv[j]);
            }
            planValMap["qVal_deriv"] = qValList_deriv;
            //添加滤波器
            QVariantList filterValList_deriv;
            for (int j = 0; j < 10; ++j) {
                filterValList_deriv.append(sysVal.SysPlanRadioList_Mode[i]->getAllPlanValue().filterVal_deriv[j]);
            }
            planValMap["filterVal_deriv"] = filterValList_deriv;


            QVariantList IsAddList;

            IsAddList.append(sysVal.SysPlanRadioList_Mode[i]->IsAdded);
            // 不足 3 个时，用 false 补齐,防止用户重新安装之前的驱动，驱动不可用
            while (IsAddList.size() < 3) {
                IsAddList.append(false);
            }

            planValMap["AddEn"] = IsAddList;


            QVariantList favIdxList;

            favIdxList.append(sysVal.SysPlanRadioList_Mode[i]->favIdx);
            // 不足 3 个时，用 false 补齐,防止用户重新安装2.0ui的驱动，驱动不可用
            while (favIdxList.size() < 3) {
                favIdxList.append(false);
            }
            planValMap["FavIdx"] = favIdxList;

            // 嵌套PlanVal
            buttonMap["PlanVal"] = planValMap;

            // 以索引为键存储按钮数据
            planMap[QString::number(i)] = buttonMap;
        }
    }
    //添加存储按钮信息的map
    map["SysPlanRadioList"] = planMap;

    return map;
}
SysVal MainWindow::variantMapToSysVal(const QVariantMap& map)
{
    SysVal val;
    QVariantMap planMap = map["SysPlanRadioList"].toMap();

    MovieVal.AllPlanRadioList.clear();

    for(int i = 0; i < map["SysPlanCnt"].toInt();i++)
    {
        QString key = QString::number(i);
        if (planMap.contains(key))
        {
            QVariantMap buttonMap = planMap[key].toMap();
            QVariantMap planValMap = buttonMap["PlanVal"].toMap();

            BOOL IsLoad = planValMap["IsLoad"].toBool();
            PlanVal RVal;
            RVal.DataVisibleEn = planValMap["DataVisibleEn"].toBool();
            RVal.ParentPlanName = planValMap.value("ParentPlanName","").toString();
            RVal.AlgoOpenEn = planValMap["AlgoOpenEn"].toBool();
            RVal.spaceOpenEn = planValMap["spaceOpenEn"].toBool();
            RVal.eqOpenEn = planValMap["eqOpenEn"].toBool();
            RVal.drcOpenEn = planValMap["drcOpenEn"].toBool();
            //额外eq
            memset(RVal.ExtraEq,0,sizeof(RVal.ExtraEq));
            QVariantList ExtraEqValList = planValMap["ExtraEq"].toList();
            for (int j = 0; j < ExtraEqValList.size(); ++j) {
                RVal.ExtraEq[j] = ExtraEqValList[j].toInt();
            }
            RVal.lowVal = planValMap["lowVal"].toInt();
            RVal.drcVal = planValMap["drcVal"].toInt();
            RVal.GainVal = planValMap["GainVal"].toInt();
            RVal.spaceVal = planValMap["spaceVal"].toInt();
            RVal.spaceReverb = planValMap["spaceReverb"].toInt();
            RVal.spaceSize = planValMap["spaceSize"].toInt();

            QVariantList freqValList = planValMap["freqVal"].toList();
            for (int j = 0; j < 10; ++j) {
                RVal.freqVal[j] = freqValList.value(j).toDouble();
            }
            QVariantList eqValList = planValMap["eqVal"].toList();
            for (int j = 0; j < 10; ++j) {
                RVal.eqVal[j] = eqValList.value(j).toDouble();
            }
            QVariantList qValList = planValMap["qVal"].toList();
            for (int j = 0; j < 10; ++j) {
                RVal.qVal[j] = qValList.value(j).toDouble();
            }
            QVariantList filterValList = planValMap["filterVal"].toList();
            for (int j = 0; j < 10; ++j) {
                RVal.filterVal[j] = filterValList.value(j).toInt();  // 安全，越界返回0
            }

            //二创内容
            //频点值
            QVariantList freqValList_deriv = planValMap["freqVal_deriv"].toList();
            int count = freqValList_deriv.size();
            for (int j = 0; j < 10; ++j) {
                if (j < count) {
                    RVal.freqVal_deriv[j] = freqValList_deriv.value(j).toDouble();
                } else {
                    RVal.freqVal_deriv[j] = 20000.0;
                }
            }

            QVariantList eqValList_deriv = planValMap["eqVal_deriv"].toList();
            count = eqValList_deriv.size();
            for (int j = 0; j < 10; ++j) {
                if (j < count) {
                    RVal.eqVal_deriv[j] = eqValList_deriv.value(j).toDouble();
                } else {
                    RVal.eqVal_deriv[j] = 0;
                }
            }

            QVariantList qValList_deriv = planValMap["qVal_deriv"].toList();
            count = qValList_deriv.size();
            for (int j = 0; j < 10; ++j) {
                if (j < count) {
                    RVal.qVal_deriv[j] = qValList_deriv.value(j).toDouble();
                } else {
                    RVal.qVal_deriv[j] = 0.7;
                }
            }
            QVariantList filterValList_deriv = planValMap["filterVal_deriv"].toList();
            count = filterValList_deriv.size();
            for (int j = 0; j < 10; ++j) {
                if (j < count) {
                    RVal.filterVal_deriv[j] = filterValList_deriv.value(j).toInt();
                } else {
                    RVal.filterVal_deriv[j] = 0;
                }
            }


            bool IsAdded = false;
            QVariantList IsAddList = planValMap["AddEn"].toList();
            for(int j = 0; j < IsAddList.size(); ++j)
            {
                //IsAdded[j] = IsAddList[j].toBool();
                if(IsAddList[j].toBool())
                {
                    IsAdded = true;
                    break;
                }
            }
            //2.0ui版本分游戏，音乐，电影三个模式，所以兼容旧版本
            //int favIdx[3] = {-1,-1,-1};
            int favIdx = -1;
            QVariantList favIdxList = planValMap["FavIdx"].toList();
            for(int j = 0; j < favIdxList.size(); ++j)
            {
                //favIdx[j] = favIdxList[j].toInt();
                if(favIdxList[j].toInt() != -1)
                {
                    favIdx = favIdxList[j].toInt();
                    break;
                }
            }



            //若为空
            QStringList dev = buttonMap.value("Lab1").toStringList();
            if(dev.isEmpty())
            {
                if (SelDev_DeviceName.contains("T10", Qt::CaseInsensitive)) {
                    if (SelDev_DeviceName.contains("Wireless", Qt::CaseInsensitive))
                        dev << "T10无线";
                    else
                        dev << "T10有线";
                }else if(SelDev_DeviceName.contains("K03S",Qt::CaseInsensitive) && (SelDev_PID == 0xF016 || SelDev_PID == 0xF017))
                {
                    dev << "K03S超竞版";
                }else if(SelDev_DeviceName.contains("K06S",Qt::CaseInsensitive))
                {
                    dev << "K06S";
                }else if(SelDev_DeviceName.contains("T7",Qt::CaseInsensitive))
                {
                    dev << "T7";
                }else if(SelDev_DeviceName.contains("K03S",Qt::CaseInsensitive))
                {
                    dev << "K03S";
                }else if(SelDev_DeviceName.contains("S21",Qt::CaseInsensitive))
                {
                    dev << "S21无线智充版";
                }
            }
            ui->page_Sperker->addAllPlan(buttonMap["Name"].toString(),buttonMap.value("Description", "").toString(),IsLoad,RVal,IsAdded,favIdx,false,false,dev,buttonMap.value("Lab2", "").toString(),0,true,false,buttonMap.value("ShareCodeId", "").toString(),buttonMap.value("ShareCode", "").toString());
        }
    }
    val.SysPlanRadioList_Mode = MovieVal.AllPlanRadioList;
    val.SysPlanRadioHash_Mode = MovieVal.AllPlanRadioHash;
    return val;

}
QVariantMap MainWindow::SysValInitToVariantMap(const QHash<QString, PlanVal> &SysVal_Init)
{
    QVariantMap map;
    map["SysPlanCnt"] = SysVal_Init.count();

    // 存储所有方案详细信息的 map
    QVariantMap planMap;

    // 遍历哈希表（使用迭代器）
    QHashIterator<QString, PlanVal> it(SysVal_Init);
    while (it.hasNext()) {
        it.next();
        const QString &planName = it.key();   // 方案名称
        const PlanVal &val = it.value();     // 对应的 PlanVal

        // 为每个方案创建一个子 map
        QVariantMap buttonMap;
        buttonMap["Name"] = planName;        // 保存方案名称（如果需要）

        // 保存 PlanVal 结构体字段
        QVariantMap planValMap;
        planValMap["DataVisibleEn"] = val.DataVisibleEn;
        planValMap["ParentPlanName"] = val.ParentPlanName;
        planValMap["AlgoOpenEn"]    = val.AlgoOpenEn;
        planValMap["spaceOpenEn"]   = val.spaceOpenEn;
        planValMap["eqOpenEn"]      = val.eqOpenEn;
        planValMap["drcOpenEn"]      = val.drcOpenEn;
        //额外eq
        QVariantList ExtraEqValList;
        for (int j = 0; j < 7; ++j) {
            ExtraEqValList.append(val.ExtraEq[j]);
        }
        planValMap["ExtraEq"] = ExtraEqValList;
        planValMap["lowVal"]        = val.lowVal;
        planValMap["drcVal"] =val.drcVal;
        planValMap["GainVal"]       = val.GainVal;
        planValMap["spaceVal"]      = val.spaceVal;
        planValMap["spaceReverb"]      = val.spaceReverb;
        planValMap["spaceSize"]      = val.spaceSize;


        // 数组 freqVal
        QVariantList freqValList;
        for (int j = 0; j < 10; ++j)
            freqValList.append(val.freqVal[j]);
        planValMap["freqVal"] = freqValList;

        // 数组 eqVal
        QVariantList eqValList;
        for (int j = 0; j < 10; ++j)
            eqValList.append(val.eqVal[j]);
        planValMap["eqVal"] = eqValList;

        // 数组 qVal
        QVariantList qValList;
        for (int j = 0; j < 10; ++j)
            qValList.append(val.qVal[j]);
        planValMap["qVal"] = qValList;

        //添加滤波器
        QVariantList filterValList;
        for (int j = 0; j < 10; ++j) {
            filterValList.append(val.filterVal[j]);
        }
        planValMap["filterVal"] = filterValList;

        //二创内容
        //添加频点值
        QVariantList freqValList_deriv;
        for (int j = 0; j < 10; ++j) {
            freqValList_deriv.append(val.freqVal_deriv[j]);
        }
        planValMap["freqVal_deriv"] = freqValList_deriv;
        // 添加eqVal数组 int eqVal[10]
        QVariantList eqValList_deriv;
        for (int j = 0; j < 10; ++j) {
            eqValList_deriv.append(val.eqVal_deriv[j]);
        }
        planValMap["eqVal_deriv"] = eqValList_deriv;
        // 添加qVal数组 double qVal[10]
        QVariantList qValList_deriv;
        for (int j = 0; j < 10; ++j) {
            qValList_deriv.append(val.qVal_deriv[j]);
        }
        planValMap["qVal_deriv"] = qValList_deriv;
        //添加滤波器
        QVariantList filterValList_deriv;
        for (int j = 0; j < 10; ++j) {
            filterValList_deriv.append(val.filterVal_deriv[j]);
        }
        planValMap["filterVal_deriv"] = filterValList_deriv;


        buttonMap["PlanVal"] = planValMap;

        // 用方案名称作为键，而不是数字索引
        planMap[planName] = buttonMap;
    }

    map["SysPlanRadioList"] = planMap;
    return map;
}
QHash<QString, PlanVal> MainWindow::variantMapToSysValInit(const QVariantMap &map)
{
    QHash<QString, PlanVal> result;

    // 获取存储所有方案的 map
    QVariantMap planMap = map["SysPlanRadioList"].toMap();

    // 遍历每个方案（键是方案名称）
    for (auto it = planMap.constBegin(); it != planMap.constEnd(); ++it) {
        const QString &planName = it.key();          // 方案名称
        QVariantMap buttonMap = it.value().toMap();
        QVariantMap planValMap = buttonMap["PlanVal"].toMap();

        PlanVal val;
        val.DataVisibleEn = planValMap["DataVisibleEn"].toBool();
        val.ParentPlanName = planValMap.value("ParentPlanName","").toString();
        val.AlgoOpenEn    = planValMap["AlgoOpenEn"].toBool();
        val.spaceOpenEn   = planValMap["spaceOpenEn"].toBool();
        val.eqOpenEn      = planValMap["eqOpenEn"].toBool();
        val.drcOpenEn      = planValMap["drcOpenEn"].toBool();
        //额外eq
        memset(val.ExtraEq,0,sizeof(val.ExtraEq));
        QVariantList ExtraEqValList = planValMap["ExtraEq"].toList();
        for (int j = 0; j < ExtraEqValList.size(); ++j) {
            val.ExtraEq[j] = ExtraEqValList[j].toInt();
        }
        val.lowVal        = planValMap["lowVal"].toInt();
        val.drcVal = planValMap["drcVal"].toInt();
        val.GainVal       = planValMap["GainVal"].toInt();
        val.spaceVal      = planValMap["spaceVal"].toInt();
        val.spaceReverb      = planValMap["spaceReverb"].toInt();
        val.spaceSize = planValMap["spaceSize"].toInt();

        // 读取数组（带长度保护）
        auto readDoubleList = [](const QVariantMap &m, const QString &key, double *dst, int maxLen) {
            QVariantList list = m[key].toList();
            for (int i = 0; i < maxLen && i < list.size(); ++i) {
                dst[i] = list[i].toDouble();
            }
        };
        auto readIntList = [](const QVariantMap &m, const QString &key, int *dst, int maxLen) {
            QVariantList list = m[key].toList();
            for (int i = 0; i < maxLen && i < list.size(); ++i) {
                dst[i] = list[i].toInt();
            }
        };
        readDoubleList(planValMap, "freqVal", val.freqVal, 10);
        readDoubleList(planValMap, "eqVal",   val.eqVal,   10);
        readDoubleList(planValMap, "qVal",    val.qVal,    10);
        readIntList(planValMap, "filterVal",    val.filterVal,    10);
        readIntList(planValMap, "filterVal_deriv",    val.filterVal_deriv,    10);


        // 插入哈希表
        result.insert(planName, val);
    }

    return result;
}
/*QVariantMap MainWindow::SysValInitToVariantMap(const QHash<QString, PlanVal> &SysVal_Init)
{
    QVariantMap map;
    map["SysPlanCnt"] = SysVal_Init.count();

    // 创建嵌套map存储按钮信息
    QVariantMap planMap;  // 存储所有按钮的map
    // 保存每个按钮的基本信息(名称、PlanVal结构体)
    for (int i = 0; i < SysVal_Init.size(); ++i) {

            // 为每个按钮创建子map
            QVariantMap buttonMap;

            // 保存按钮属性
            // buttonMap["Name"] = SysVal_Init.SysPlanRadioList_Mode[i]->property("fullText");
            // buttonMap["Id"] = i;

            // 保存PlanVal结构体字段
            QVariantMap planValMap;
            planValMap["DataVisibleEn"] = SysVal_Init[i].DataVisibleEn;
            planValMap["AlgoOpenEn"] = SysVal_Init[i].AlgoOpenEn;
            planValMap["spaceOpenEn"] = SysVal_Init[i].spaceOpenEn;
            planValMap["eqOpenEn"] = SysVal_Init[i].eqOpenEn;
            planValMap["lowVal"] = SysVal_Init[i].lowVal;
            //qDebug("save lowVal:%d\n",planValMap["lowVal"].toInt());
            planValMap["spaceVal"] = SysVal_Init[i].spaceVal;
            planValMap["GainVal"] = SysVal_Init[i].GainVal;
            //添加频点值
            QVariantList freqValList;
            for (int j = 0; j < 10; ++j) {
                freqValList.append(SysVal_Init[i].freqVal[j]);
            }
            planValMap["freqVal"] = freqValList;
            // 添加eqVal数组 int eqVal[10]
            QVariantList eqValList;
            for (int j = 0; j < 10; ++j) {
                eqValList.append(SysVal_Init[i].eqVal[j]);
            }
            planValMap["eqVal"] = eqValList;
            // 添加qVal数组 double qVal[10]
            QVariantList qValList;
            for (int j = 0; j < 10; ++j) {
                qValList.append(SysVal_Init[i].qVal[j]);
            }
            planValMap["qVal"] = qValList;

            // 嵌套PlanVal
            buttonMap["PlanVal"] = planValMap;

            // 以索引为键存储按钮数据
            planMap[QString::number(i)] = buttonMap;

    }
    //添加存储按钮信息的map
    map["SysPlanRadioList"] = planMap;

    return map;
}

QHash<QString, PlanVal> MainWindow::variantMapToSysValInit(const QVariantMap& map)
{
    QHash<QString, PlanVal> val;
    QVariantMap planMap = map["SysPlanRadioList"].toMap();

    SysPlanVal_Init.clear();
    SysPlanVal_Index = 0;

    for(int i = 0; i < map["SysPlanCnt"].toInt();i++)
    {
        QString key = QString::number(i);
        if (planMap.contains(key))
        {
            QVariantMap buttonMap = planMap[key].toMap();
            QVariantMap planValMap = buttonMap["PlanVal"].toMap();

            PlanVal RVal;
            RVal.DataVisibleEn = planValMap["DataVisibleEn"].toBool();
            RVal.AlgoOpenEn = planValMap["AlgoOpenEn"].toBool();
            RVal.spaceOpenEn = planValMap["spaceOpenEn"].toBool();
            RVal.eqOpenEn = planValMap["eqOpenEn"].toBool();
            RVal.lowVal = planValMap["lowVal"].toInt();
            //qDebug("read lowVal:%d\n",planValMap["lowVal"].toInt());
            RVal.spaceVal = planValMap["spaceVal"].toInt();
            RVal.GainVal = planValMap["GainVal"].toInt();
            QVariantList freqValList = planValMap["freqVal"].toList();
            for (int j = 0; j < 10; ++j) {
                RVal.freqVal[j] = freqValList[j].toDouble();
            }
            QVariantList eqValList = planValMap["eqVal"].toList();
            for (int j = 0; j < 10; ++j) {
                RVal.eqVal[j] = eqValList[j].toDouble();
            }
            QVariantList qValList = planValMap["qVal"].toList();
            for (int j = 0; j < 10; ++j) {
                RVal.qVal[j] = qValList[j].toDouble();
            }
            val.append(RVal);
            SysPlanVal_Index++;
        }
    }
    return val;

}*/
//转换我的预设（自建+导入）为map
QVariantMap MainWindow::modeValToVariantMap(const ModeVal &modeVal)
{
    QVariantMap map;
    map["ModeName"] = modeVal.ModeName;
    map["MyPlanCnt"] = modeVal.MyPlanRadioList.count();

    // 创建嵌套map存储按钮信息
    QVariantMap planMap;  // 存储所有按钮的map
    // 保存每个按钮的基本信息(名称、是否从所有预设添加(添加后从未修改)、从哪个所有预设的方案添加的、PlanVal结构体、)
    for (int i = 0; i < modeVal.MyPlanRadioList.size(); ++i) {
        if (modeVal.MyPlanRadioList [i]) {
            // 为每个按钮创建子map
            QVariantMap buttonMap;

            // 保存按钮名称
            buttonMap["Name"] = modeVal.MyPlanRadioList[i]->lab_name->text();
            // 保存按钮描述
            buttonMap["Description"] = modeVal.MyPlanRadioList[i]->property("fullText");
            //标签1
            buttonMap["Lab1"] = modeVal.MyPlanRadioList[i]->getLabDevs();
            //标签2
            buttonMap["Lab2"] = modeVal.MyPlanRadioList[i]->lab2->text();
            //所属分类（0:没有，1：分类1，2：分类2，3：分类3,....8：分类8）
            buttonMap["PlanTypeIdx"] = modeVal.MyPlanRadioList[i]->PlanPageSel;
            //分享码（id）
            buttonMap["ShareCodeId"] = modeVal.MyPlanRadioList[i]->ShareCodeId;
            //分享码
            buttonMap["ShareCode"] = modeVal.MyPlanRadioList[i]->ShareCode;


            // 保存PlanVal结构体字段
            QVariantMap planValMap;
            planValMap["DataVisibleEn"] = modeVal.MyPlanRadioList [i]->getAllPlanValue().DataVisibleEn;
            planValMap["ParentPlanName"] = modeVal.MyPlanRadioList[i]->getAllPlanValue().ParentPlanName;
            planValMap["AlgoOpenEn"] = modeVal.MyPlanRadioList [i]->getAllPlanValue().AlgoOpenEn;
            planValMap["spaceOpenEn"] = modeVal.MyPlanRadioList [i]->getAllPlanValue().spaceOpenEn;
            planValMap["eqOpenEn"] = modeVal.MyPlanRadioList [i]->getAllPlanValue().eqOpenEn;
            planValMap["drcOpenEn"] = modeVal.MyPlanRadioList [i]->getAllPlanValue().drcOpenEn;
            //额外eq
            QVariantList ExtraEqValList;
            for (int j = 0; j < 7; ++j) {
                ExtraEqValList.append(modeVal.MyPlanRadioList [i]->getAllPlanValue().ExtraEq[j]);
            }
            planValMap["ExtraEq"] = ExtraEqValList;
            planValMap["lowVal"] = modeVal.MyPlanRadioList [i]->getAllPlanValue().lowVal;
            planValMap["drcVal"] = modeVal.MyPlanRadioList [i]->getAllPlanValue().drcVal;
            planValMap["GainVal"] = modeVal.MyPlanRadioList [i]->getAllPlanValue().GainVal;
            planValMap["spaceVal"] = modeVal.MyPlanRadioList [i]->getAllPlanValue().spaceVal;
            planValMap["spaceReverb"] = modeVal.MyPlanRadioList [i]->getAllPlanValue().spaceReverb;
            planValMap["spaceSize"] = modeVal.MyPlanRadioList [i]->getAllPlanValue().spaceSize;

            //添加频点值
            QVariantList freqValList;
            for (int j = 0; j < 10; ++j) {
                freqValList.append(modeVal.MyPlanRadioList[i]->getAllPlanValue().freqVal[j]);
            }
            planValMap["freqVal"] = freqValList;
            // 添加eqVal数组 int eqVal[10]
            QVariantList eqValList;
            for (int j = 0; j < 10; ++j) {
                eqValList.append(modeVal.MyPlanRadioList[i]->getAllPlanValue().eqVal[j]);
            }
            planValMap["eqVal"] = eqValList;
            // 添加qVal数组 double qVal[10]
            QVariantList qValList;
            for (int j = 0; j < 10; ++j) {
                qValList.append(modeVal.MyPlanRadioList[i]->getAllPlanValue().qVal[j]);
            }
            planValMap["qVal"] = qValList;
            //添加滤波器
            QVariantList filterValList;
            for (int j = 0; j < 10; ++j) {
                filterValList.append(modeVal.MyPlanRadioList[i]->getAllPlanValue().filterVal[j]);
            }
            planValMap["filterVal"] = filterValList;

            //二创内容
            //添加频点值
            QVariantList freqValList_deriv;
            for (int j = 0; j < 10; ++j) {
                freqValList_deriv.append(modeVal.MyPlanRadioList[i]->getAllPlanValue().freqVal_deriv[j]);
            }
            planValMap["freqVal_deriv"] = freqValList_deriv;
            // 添加eqVal数组 int eqVal[10]
            QVariantList eqValList_deriv;
            for (int j = 0; j < 10; ++j) {
                eqValList_deriv.append(modeVal.MyPlanRadioList[i]->getAllPlanValue().eqVal_deriv[j]);
            }
            planValMap["eqVal_deriv"] = eqValList_deriv;
            // 添加qVal数组 double qVal[10]
            QVariantList qValList_deriv;
            for (int j = 0; j < 10; ++j) {
                qValList_deriv.append(modeVal.MyPlanRadioList[i]->getAllPlanValue().qVal_deriv[j]);
            }
            planValMap["qVal_deriv"] = qValList_deriv;
            //添加滤波器
            QVariantList filterValList_deriv;
            for (int j = 0; j < 10; ++j) {
                filterValList_deriv.append(modeVal.MyPlanRadioList[i]->getAllPlanValue().filterVal_deriv[j]);
            }
            planValMap["filterVal_deriv"] = filterValList_deriv;

            QVariantList IsAddList;

            IsAddList.append(modeVal.MyPlanRadioList[i]->IsAdded);
            // 不足 3 个时，用 false 补齐,防止用户重新安装之前的驱动，驱动不可用
            while (IsAddList.size() < 3) {
                IsAddList.append(false);
            }

            planValMap["AddEn"] = IsAddList;

            QVariantList favIdxList;

            favIdxList.append(modeVal.MyPlanRadioList[i]->favIdx);
            // 不足 3 个时，用 false 补齐,防止用户重新安装之前的驱动，驱动不可用
            while (favIdxList.size() < 3) {
                favIdxList.append(false);
            }

            planValMap["FavIdx"] = favIdxList;

            // 嵌套PlanVal
            buttonMap["PlanVal"] = planValMap;

            // 以索引为键存储按钮数据
            planMap[QString::number(i)] = buttonMap;
        }
    }
    //添加存储按钮信息的map,不修改名，为了兼容1.9以及之前版本的配置
    map["MyPlanRadioList"] = planMap;
    map["C_PlanPageSel"] = modeVal.C_PlanPageSel;
    //qDebug("save C_PlanPageSel:%d\n",map["C_PlanPageSel"].toInt());
    map["C_PlanName"] = modeVal.C_PlanName;
    map["C_PlanDev"] = modeVal.C_PlanDev;

    return map;
}
//把map转换为我的预设（自建+导入）
ModeVal MainWindow::variantMapToModeVal(const QVariantMap &map, bool ifMerge)
{
    ModeVal val;
    QStringList dev;
    QString modeName = map["ModeName"].toString();
    modeName.remove("模式");          // 删除所有出现的“模式”
    val.ModeName = modeName;
    QVariantMap planMap = map["MyPlanRadioList"].toMap();

    for(int i = 0; i < map["MyPlanCnt"].toInt();i++)
    {
        QString key = QString::number(i);
        if (planMap.contains(key))
        {
            QVariantMap buttonMap = planMap[key].toMap();
            QVariantMap planValMap = buttonMap["PlanVal"].toMap();

            PlanVal RVal;
            RVal.DataVisibleEn = planValMap["DataVisibleEn"].toBool();
            RVal.ParentPlanName = planValMap.value("ParentPlanName","").toString();
            RVal.AlgoOpenEn = planValMap["AlgoOpenEn"].toBool();
            RVal.spaceOpenEn = planValMap["spaceOpenEn"].toBool();
            RVal.eqOpenEn = planValMap["eqOpenEn"].toBool();
            RVal.drcOpenEn = planValMap["drcOpenEn"].toBool();
            //额外eq
            memset(RVal.ExtraEq,0,sizeof(RVal.ExtraEq));
            QVariantList ExtraEqValList = planValMap["ExtraEq"].toList();
            for (int j = 0; j < ExtraEqValList.size(); ++j) {
                RVal.ExtraEq[j] = ExtraEqValList[j].toInt();
            }
            RVal.lowVal = planValMap["lowVal"].toInt();
            RVal.drcVal = planValMap["drcVal"].toInt();
            RVal.GainVal = planValMap["GainVal"].toInt();
            RVal.spaceVal = planValMap["spaceVal"].toInt();
            RVal.spaceReverb = planValMap["spaceReverb"].toInt();
            RVal.spaceSize = planValMap["spaceSize"].toInt();

            QVariantList freqValList = planValMap["freqVal"].toList();
            for (int j = 0; j < 10; ++j) {
                RVal.freqVal[j] = freqValList.value(j).toDouble();
            }
            QVariantList eqValList = planValMap["eqVal"].toList();
            for (int j = 0; j < 10; ++j) {
                RVal.eqVal[j] = eqValList.value(j).toDouble();
            }
            QVariantList qValList = planValMap["qVal"].toList();
            for (int j = 0; j < 10; ++j) {
                RVal.qVal[j] = qValList.value(j).toDouble();
            }
            QVariantList filterValList = planValMap["filterVal"].toList();
            for (int j = 0; j < 10; ++j) {
                RVal.filterVal[j] = filterValList.value(j).toInt();
            }

            //二创内容
            //频点值
            QVariantList freqValList_deriv = planValMap["freqVal_deriv"].toList();
            int count = freqValList_deriv.size();
            for (int j = 0; j < 10; ++j) {
                if (j < count) {
                    RVal.freqVal_deriv[j] = freqValList_deriv.value(j).toDouble();
                } else {
                    RVal.freqVal_deriv[j] = 20000.0;
                }
            }

            QVariantList eqValList_deriv = planValMap["eqVal_deriv"].toList();
            count = eqValList_deriv.size();
            for (int j = 0; j < 10; ++j) {
                if (j < count) {
                    RVal.eqVal_deriv[j] = eqValList_deriv.value(j).toDouble();
                } else {
                    RVal.eqVal_deriv[j] = 0;
                }
            }

            QVariantList qValList_deriv = planValMap["qVal_deriv"].toList();
            count = qValList_deriv.size();
            for (int j = 0; j < 10; ++j) {
                if (j < count) {
                    RVal.qVal_deriv[j] = qValList_deriv.value(j).toDouble();
                } else {
                    RVal.qVal_deriv[j] = 0.7;
                }
            }

            QVariantList filterValList_deriv = planValMap["filterVal_deriv"].toList();
            count = filterValList_deriv.size();
            for (int j = 0; j < 10; ++j) {
                if (j < count) {
                    RVal.filterVal_deriv[j] = filterValList_deriv[j].toInt();
                } else {
                    RVal.filterVal_deriv[j] = 0;
                }
            }

            //bool IsAdded[3] = {false,false,false};
            bool IsAdded = false;
            QVariantList IsAddList = planValMap["AddEn"].toList();
            for(int j = 0; j < IsAddList.size(); ++j)
            {
                //IsAdded[j] = IsAddList[j].toBool();
                if(IsAddList[j].toBool())
                {
                    IsAdded = true;
                    break;
                }
            }
            //int favIdx[3] = {-1,-1,-1};
            int favIdx = -1;
            QVariantList favIdxList = planValMap["FavIdx"].toList();
            for(int j = 0; j < favIdxList.size(); ++j)
            {
                //favIdx[j] = favIdxList[j].toInt();
                if(favIdxList[j].toInt() != -1)
                {
                    favIdx = favIdxList[j].toInt();
                    break;
                }
            }

            dev = buttonMap.value("Lab1").toStringList();
            if(dev.isEmpty())
            {
                if (SelDev_DeviceName.contains("T10", Qt::CaseInsensitive)) {
                    if (SelDev_DeviceName.contains("Wireless", Qt::CaseInsensitive))
                        dev << "T10无线";
                    else
                        dev << "T10有线";
                }else if(SelDev_DeviceName.contains("K03S",Qt::CaseInsensitive) && (SelDev_PID == 0xF016 || SelDev_PID == 0xF017))
                {
                    dev << "K03S超竞版";
                }else if(SelDev_DeviceName.contains("K06S",Qt::CaseInsensitive))
                {
                    dev << "K06S";
                }else if(SelDev_DeviceName.contains("T7",Qt::CaseInsensitive) && (SelDev_PID == 0xF014 || SelDev_PID == 0xF008))
                {
                    dev << "T7";
                }else if(SelDev_DeviceName.contains("T7 GT",Qt::CaseInsensitive) && (SelDev_PID == 0xF015 || SelDev_PID == 0xF009))
                {
                    dev << "T7 GT";
                }else if(SelDev_DeviceName.contains("K03S",Qt::CaseInsensitive))
                {
                    dev << "K03S";
                }else if(SelDev_DeviceName.contains("S21",Qt::CaseInsensitive))
                {
                    dev << "S21无线智充版";
                }
            }
            //ui->page_Sperker->addMyPlan(buttonMap["Name"].toString(),buttonMap.value("Description", "").toString(),false,RVal,IsAdded,favIdx,false,buttonMap.value("Lab1", "").toString(),buttonMap.value("Lab2", "").toString());
            qDebug() << "[variantMapToModeVal] addAllPlan:" << buttonMap["Name"].toString()
                     << "dev=" << dev << "sysEn=false";
            ui->page_Sperker->addAllPlan(buttonMap["Name"].toString(),buttonMap.value("Description", "").toString(),false,RVal,IsAdded,favIdx,false,false,dev,buttonMap.value("Lab2", val.ModeName).toString(),buttonMap.value("PlanTypeIdx", 1).toInt(),false,ifMerge,buttonMap.value("ShareCodeId", "").toString(),buttonMap.value("ShareCode", "").toString());
        }
    }
    val.AllPlanRadioList  = MovieVal.AllPlanRadioList;
    val.AllPlanRadioHash = MovieVal.AllPlanRadioHash;
    val.MyPlanRadioList = MovieVal.MyPlanRadioList;
    val.MyPlanRadioHash = MovieVal.MyPlanRadioHash;

    val.C_PlanPageSel = map["C_PlanPageSel"].toInt();//预设分类
    //qDebug("read C_PlanPageSel:%d\n",val.C_PlanPageSel);
    val.C_PlanName = map["C_PlanName"].toString();
    QStringList dev2 = map["C_PlanDev"].toStringList();
    if(dev2.isEmpty())
    {
        val.C_PlanDev = dev;
    }else
    {
        val.C_PlanDev = dev2;
    }

    return val;
}

int MainWindow::writeIni()
{
    //QSettings settings(filePath,QSettings::IniFormat);
    bool Eqen = 0, ClearVocalsEn = 0, RichVocalsEn = 0, listenEn = 0, NoiseEn = 0;
    int EqPlanIdx = 0, NoiseVal = 0;

    //系统方案
    globalSettings->setValue("SysVal", SysValToVariantMap(SysPlanVal));
    //系统方案初始值
    globalSettings->setValue("SysValInit", SysValInitToVariantMap(SysPlanVal_Init));

    //电影、音乐、游戏的ModeVal
    globalSettings->setValue("MovieVal", modeValToVariantMap(MovieVal));
    // globalSettings->setValue("MusicVal", modeValToVariantMap(MusicVal));
    // globalSettings->setValue("GameVal", modeValToVariantMap(GameVal));

    //方案分类
    QVariantMap plansMap;
    QVariantList Name;
    QVariantList Enable;
    for (auto &plan : PlansTypes) {
        Name.append(plan.Name);
        Enable.append(plan.en);
    }
    plansMap["TName"] = Name;
    plansMap["TEn"] = Enable;
    globalSettings->setValue("PlansTypes", plansMap);

    //麦克风和提示音后续相关协议修改后需删除。
    //麦克风侦听开关、降噪开关、降噪值
    ui->widget_mic->saveIniValue(ClearVocalsEn, RichVocalsEn, listenEn, NoiseEn, NoiseVal);

    globalSettings->setValue(QString("MIC/ClearVocalsEn"), ClearVocalsEn);
    globalSettings->setValue(QString("MIC/RichVocalsEn"), RichVocalsEn);
    globalSettings->setValue(QString("MIC/listeningEn"), listenEn);
    globalSettings->setValue(QString("MIC/Noise Reduction En"), NoiseEn);
    globalSettings->setValue(QString("MIC/Noise Reduction Val"), NoiseVal);

    //是否记住关闭选择、关闭选择、语言选择
    // Language/Theme 已在 onLanguageChanged/onThemeChanged 中即时保存，此处不再覆盖

    globalSettings->setValue("bRemember",
                             g_user_system_settings_config_info.is_remember_choice.load());
    globalSettings->setValue("bExitDirectly",
                             g_user_system_settings_config_info.is_exit_directly.load());

    //用户登录信息 — 仅在已登录时持久化，避免未登录时误写 Login/en=true
    globalSettings->setValue("Login/en", isLogin);
    if (isLogin) {
        globalSettings->setValue("Login/type", g_user_information.network.login_type);
        globalSettings->setValue("Login/nickname", g_user_information.network.username);
        globalSettings->setValue("Login/id", g_user_information.network.id);

        QVariantMap map;
        map["user_email"] = g_user_information.network.email;
        map["user_psw"] = g_user_information.local.user_psw;
        map["access_token"] = g_user_information.network.access_token;
        globalSettings->setValue("Login/Account", map);
    }

    globalSettings->sync(); //立即写入

    return 1;
}
int MainWindow::readIni()
{
    QSettings settings(filePath, QSettings::IniFormat);
    QVariantList list;
    QString key;

    bool EQen = 1, ClearVocalsEn = 0, RichVocalsEn = 0, listenEn = 0, NoiseEn = 0, loginEn = 0;
    int EqPlanIdx = 0, NoiseVal = 0;

    if (SysPlanAdd_New >= 1) {
        SysPlanVal = variantMapToSysVal(settings.value("SysVal").toMap());
        //系统方案初始值
        SysPlanVal_Init = variantMapToSysValInit(settings.value("SysValInit").toMap());
    }

    //电影、音乐、游戏的ModeVal
    MovieVal.MyPlanRadioList.clear();
    MovieVal = variantMapToModeVal(settings.value("MovieVal").toMap(), false);
    qDebug() << "[readIni] MovieVal loaded: MyPlanRadioList=" << MovieVal.MyPlanRadioList.size()
             << "AllPlan=" << MovieVal.AllPlanRadioList.size()
             << "hasMovieValKey=" << settings.contains("MovieVal");
    // variantMapToModeVal(settings.value("MovieVal").toMap(),false);

    //若文档中存在
    bool hasMusic = settings.contains("MusicVal");
    bool hasGame = settings.contains("GameVal");
    if (hasMusic || hasGame) {
        // ModeVal MusicVal = variantMapToModeVal(settings.value("MusicVal").toMap(),true);
        // ModeVal GameVal = variantMapToModeVal(settings.value("GameVal").toMap(),true);
        variantMapToModeVal(settings.value("MusicVal").toMap(), true);
        variantMapToModeVal(settings.value("GameVal").toMap(), true);

        // 读取完成后，立即删除该键
        settings.remove("MusicVal");
        settings.remove("GameVal");
    }

    //方案分类
    if (!settings.contains("PlansTypes")) {
        // 键不存在(旧版本驱动，存在我的预设，所以要加上)
        PlansTypes[0].Name = "我的预设";
        PlansTypes[0].en = true;
    } else {
        QVariantMap plansMap = settings.value("PlansTypes").toMap();
        QVariantList Name = plansMap["TName"].toList();
        QVariantList Enable = plansMap["TEn"].toList();

        //压缩，不可存在数据与数据之间有空的情况
        QVariantList newName;
        QVariantList newEnable;
        // 同步遍历，保证索引一致
        int count = qMin(Name.size(), Enable.size()); // 取较小长度，防止越界
        for (int i = 0; i < count; ++i) {
            QString name = Name[i].toString();
            if (!name.isEmpty()) {
                newName.append(name);
                newEnable.append(Enable.value(i, false)); // 若 Enable 长度不足则默认 false
            }
        }


        const int MAX_PLANS = sizeof(PlansTypes) / sizeof(PlansTypes[0]);
        for (int j = 0; j < newName.size() && j < MAX_PLANS; j++) {
            PlansTypes[j].Name = newName[j].toString();
        }
        for (int j = 0; j < newEnable.size() && j < MAX_PLANS; j++) {
            PlansTypes[j].en = newEnable[j].toBool();
        }
    }

    //麦克风 侦听、降噪、增益开关，降噪值，增益值

    ClearVocalsEn = settings.value(QString("MIC/ClearVocalsEn")).toBool();
    RichVocalsEn = settings.value(QString("MIC/RichVocalsEn")).toBool();
    listenEn = settings.value(QString("MIC/listeningEn")).toBool();
    NoiseEn = settings.value(QString("MIC/Noise Reduction En")).toBool();
    NoiseVal = settings.value(QString("MIC/Noise Reduction Val")).toInt();

    ui->widget_mic->readIniValue(ClearVocalsEn, RichVocalsEn, listenEn, NoiseEn, NoiseVal);

    //语言选择、背景主题
    // Language = settings.value("Language").toInt();
    // Theme = settings.value("Theme").toInt();
    // ui->widget_more->readIniValue(Language,Theme);

    g_user_system_settings_config_info.is_remember_choice.store(
                settings.value("bRemember").toBool());
    g_user_system_settings_config_info.is_exit_directly.store(
                settings.value("bExitDirectly").toBool());

    // //用户登录信息
    // loginEn = settings.value("Login/en").toBool();
    // if(loginEn)
    // {
    //     isLogin = true;
    //     //已登录，跳过登录页面
    //     user_login_type = settings.value("Login/type").toInt();//登录类型
    //     g_user_information.network.nickname = settings.value("Login/nickname").toString();//用户名
    //     g_user_information.network.id = settings.value("Login/id").toString();//用户ID(唯一且不可更改)

    //     QVariantMap map = settings.value("Login/Account").toMap();
    //     user_email = map["user_email"].toString();
    //     user_psw = map["user_psw"].toString();
    //     access_token = map["access_token"].toString();//访问令牌
    // }

    AlreadyRead = true;

    return 1;
}

void MainWindow::RealTimeSaveIni_ModeVal()
{
    //电影、音乐、游戏的ModeVal

    globalSettings->setValue("MovieVal", modeValToVariantMap(MovieVal));

    globalSettings->sync(); //立即写入
}
void MainWindow::RealTimeSaveIni_SysPlan()
{
    //系统方案
    globalSettings->setValue("SysVal", SysValToVariantMap(SysPlanVal));
}

void MainWindow::RealTimeSaveIni_SysPlanValInit()
{
    //系统方案初始值
    globalSettings->setValue("SysValInit", SysValInitToVariantMap(SysPlanVal_Init));
}

void MainWindow::onTriggerAsyncSaveModeVal()
{
    // 1. 在主线程复制数据（MovieVal 是你已更新的成员变量）
    QVariantMap movieValMap = modeValToVariantMap(MovieVal);

    // 2. 异步写入文件（值捕获，安全）
    QtConcurrent::run([this, movieValMap]() {
        QMutexLocker locker(&m_saveMutex); // 加锁，保证 QSettings 线程安全
        globalSettings->setValue("MovieVal", movieValMap);
        globalSettings->sync();
    });
}

void MainWindow::onTriggerAsyncSaveSysPlan()
{
    QVariantMap sysPlanMap = SysValToVariantMap(SysPlanVal);

    QtConcurrent::run([this, sysPlanMap]() {
        QMutexLocker locker(&m_saveMutex);
        globalSettings->setValue("SysVal", sysPlanMap);
        globalSettings->sync();
    });
}

void MainWindow::onTriggerAsyncSaveSysPlanInit()
{
    QVariantMap sysPlanInitMap = SysValInitToVariantMap(SysPlanVal_Init);

    QtConcurrent::run([this, sysPlanInitMap]() {
        QMutexLocker locker(&m_saveMutex);
        globalSettings->setValue("SysValInit", sysPlanInitMap);
        globalSettings->sync();
    });
}

// Qt::Edges → 拉伸光标 ID（无匹配返回 nullptr）
static LPCTSTR resizeCursorId(Qt::Edges t_edges)
{
    if (t_edges & Qt::LeftEdge && t_edges & Qt::TopEdge) return IDC_SIZENWSE;
    if (t_edges & Qt::RightEdge && t_edges & Qt::BottomEdge) return IDC_SIZENWSE;
    if (t_edges & Qt::RightEdge && t_edges & Qt::TopEdge) return IDC_SIZENESW;
    if (t_edges & Qt::LeftEdge && t_edges & Qt::BottomEdge) return IDC_SIZENESW;
    if (t_edges & (Qt::LeftEdge | Qt::RightEdge)) return IDC_SIZEWE;
    if (t_edges & (Qt::TopEdge | Qt::BottomEdge)) return IDC_SIZENS;
    return nullptr;
}

// Windows 非客户区命中码 → Qt::Edges（WM_NCLBUTTONDOWN 启动手动拉伸用）
static Qt::Edges hitCodeToEdges(long t_hit)
{
    switch (t_hit) {
    case HTLEFT: return Qt::LeftEdge;
    case HTRIGHT: return Qt::RightEdge;
    case HTTOP: return Qt::TopEdge;
    case HTBOTTOM: return Qt::BottomEdge;
    case HTTOPLEFT: return Qt::LeftEdge | Qt::TopEdge;
    case HTTOPRIGHT: return Qt::RightEdge | Qt::TopEdge;
    case HTBOTTOMLEFT: return Qt::LeftEdge | Qt::BottomEdge;
    case HTBOTTOMRIGHT: return Qt::RightEdge | Qt::BottomEdge;
    default: return Qt::Edges();
    }
}

// Qt::Edges → Windows 命中码（非边缘返回 0）
static long hitTestEdges(Qt::Edges t_edges)
{
    if (t_edges & Qt::LeftEdge && t_edges & Qt::TopEdge) return HTTOPLEFT;
    if (t_edges & Qt::RightEdge && t_edges & Qt::BottomEdge) return HTBOTTOMRIGHT;
    if (t_edges & Qt::RightEdge && t_edges & Qt::TopEdge) return HTTOPRIGHT;
    if (t_edges & Qt::LeftEdge && t_edges & Qt::BottomEdge) return HTBOTTOMLEFT;
    if (t_edges & Qt::LeftEdge) return HTLEFT;
    if (t_edges & Qt::RightEdge) return HTRIGHT;
    if (t_edges & Qt::TopEdge) return HTTOP;
    if (t_edges & Qt::BottomEdge) return HTBOTTOM;
    return 0;
}

// 无边框窗体边缘拉伸（光标与拉伸动作分离）：
//  1) WM_NCHITTEST：命中边缘条时返回 HT* 命中码（供系统识别边缘区域，并驱动
//     后续按下走 WM_NCLBUTTONDOWN 路径）；
//  2) WM_SETCURSOR：Qt 不拦截此消息，必然送达——悬停最外层 6px 时手动切换拉伸箭头光标；
//  3) WM_NCLBUTTONDOWN：边缘按下会以非客户区消息到达（HT* 命中码所致），Qt 不将其
//     转为控件事件 → 在此记录拉伸状态并 SetCapture，使后续移动转为客户区
//     WM_MOUSEMOVE 进入 mouseMoveEvent 手动 setGeometry 完成拉伸；
//  4) 客户区边缘按下（判定区外其余条带）走 mousePressEvent 同一套手动拉伸。
bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
#ifdef Q_OS_WIN
    if (eventType == "windows_generic_MSG") {
        MSG *pMsg = static_cast<MSG *>(message);
        const bool t_resizable = !isMaximized() && !isFullScreen() && !isMinimized();

        if (pMsg->message == WM_NCHITTEST && t_resizable) {
            // lParam 为屏幕坐标，先转窗口坐标再判边缘
            QPoint t_globalPt((short)LOWORD(pMsg->lParam), (short)HIWORD(pMsg->lParam));
            long t_hit = hitTestEdges(edgesAtPos(mapFromGlobal(t_globalPt)));
            if (t_hit) {
                *result = t_hit;
                return true; // 已处理，按下时将由 WM_NCLBUTTONDOWN 分支接管
            }
        }

        if (pMsg->message == WM_SETCURSOR && t_resizable) {
            // WM_SETCURSOR 的 lParam 是命中码/消息 ID 而非坐标，光标位置用 QCursor::pos()
            // 拉伸进行中：鼠标可能已离开 6px 判定区甚至窗口边界，强制保持箭头光标
            Qt::Edges t_edges = m_resizeEdges;
            if (!t_edges) {
                // 悬停：箭头光标与按下拉伸判定区一致，均为窗口最外层 6px（见 edgesAtPos）
                QPoint t_cursorPos = mapFromGlobal(QCursor::pos());
                const int t_border = 6;
                if (t_cursorPos.x() < t_border) t_edges |= Qt::LeftEdge;
                if (t_cursorPos.x() >= width() - t_border) t_edges |= Qt::RightEdge;
                if (t_cursorPos.y() < t_border) t_edges |= Qt::TopEdge;
                if (t_cursorPos.y() >= height() - t_border) t_edges |= Qt::BottomEdge;
            }
            LPCTSTR t_cursorId = resizeCursorId(t_edges);
            if (t_cursorId) {
                SetCursor(LoadCursor(nullptr, t_cursorId));
                *result = TRUE;
                return true;
            }
        }

        // 边缘按下会以非客户区消息到达（WM_NCHITTEST 返回 HT* 命中码所致），
        // Qt 不将其转为控件事件 → 在此直接启动原生驱动的拉伸：
        // SetCapture 保证光标移出窗口后移动/释放消息仍到达本窗口，
        // 由下方 WM_MOUSEMOVE / WM_LBUTTONUP 分支接管（Qt 层收不到窗外移动）
        if (pMsg->message == WM_NCLBUTTONDOWN && t_resizable) {
            Qt::Edges t_edges = hitCodeToEdges(long(pMsg->wParam & 0xffff));
            if (t_edges) {
                m_resizeNative = true;
                m_resizeEdges = t_edges;
                m_resizeStartPos = QCursor::pos();
                m_resizeStartGeometry = frameGeometry();
                SetCapture((HWND)winId());
                *result = 0;
                return true; // 吞掉，避免走系统原生边框拉伸
            }
        }

        // 原生路径拉伸中：移动（光标可能已在窗口外，Qt 层收不到）
        if (pMsg->message == WM_MOUSEMOVE && m_resizeEdges && m_resizeNative) {
            // WM_MOUSEMOVE 的 lParam 为窗口客户区坐标（光标在窗外时为越界值，mapToGlobal 仍线性有效）
            QPoint t_clientPt((short)LOWORD(pMsg->lParam), (short)HIWORD(pMsg->lParam));
            applyResizeDelta(mapToGlobal(t_clientPt) - m_resizeStartPos);
            *result = 0;
            return true; // 吞掉，避免 Qt 层 mouseMoveEvent 重复计算
        }

        // 原生路径拉伸结束：清状态并补齐拖动期间跳过的背景/模糊重算
        if (pMsg->message == WM_LBUTTONUP && m_resizeEdges && m_resizeNative) {
            m_resizeNative = false;
            m_resizeEdges = Qt::Edges();
            ReleaseCapture();
            updateBackgroundCache(); // 拉伸结束，补齐拖动期间跳过的背景/模糊重算
            // 不 return true：让 Qt 也处理一次释放，保持按钮状态同步
        }
    }
#endif
    return QMainWindow::nativeEvent(eventType, message, result);
}

// 返回鼠标所在的窗口边缘（6px 内），非边缘返回空
Qt::Edges MainWindow::edgesAtPos(const QPoint &pos) const
{
    const int t_border = 6; // 边缘判定宽度（像素）
    Qt::Edges t_edges;
    if (pos.x() < t_border) t_edges |= Qt::LeftEdge;
    if (pos.x() >= width() - t_border) t_edges |= Qt::RightEdge;
    if (pos.y() < t_border) t_edges |= Qt::TopEdge;
    if (pos.y() >= height() - t_border) t_edges |= Qt::BottomEdge;
    return t_edges;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    if (this->isMinimized()) {
        return; // 最小化时不更新
    }

    if (g_shareCodeCopyHint) {
        g_shareCodeCopyHint->move(rect().center().x() - g_shareCodeCopyHint->width() / 2, 69);
        g_shareCodeCopyHint->hide();
    }

    QMainWindow::resizeEvent(event); // 调用基类的方法

    if (m_resizeEdges) {
        // 边缘拉伸拖动中：跳过昂贵的背景缓存/模糊重算（每帧重载资源+整窗缩放会卡住拖动），
        // 保证实时跟手；松手后在 mouseReleaseEvent 一次性补齐
        emit updateVideoHoverPosition();
        updateSize();
        return;
    }

    updateBackgroundCache();         // 窗口大小变化时重新预缩放背景（this->size() 此时已更新）
    emit updateVideoHoverPosition();
    updateSize();
}

void MainWindow::updateSize()
{
    // 使用单次定时器延迟执行，确保布局已完成
    QTimer::singleShot(0, this, [this]() {
        UpdateShadowLabelSize(lab_Speakershadow_Top);
        UpdateShadowLabelSize(lab_Speakershadow_Buttom);
        UpdateShadowLabelSize(lab_Micshadow_Top);
        UpdateShadowLabelSize(lab_Micshadow_Buttom);
    });
    // UpdateShadowLabelSize(lab_Speakershadow_Top);
    // UpdateShadowLabelSize(lab_Speakershadow_Buttom);
    // UpdateShadowLabelSize(lab_Micshadow_Top);
    // UpdateShadowLabelSize(lab_Micshadow_Buttom);
}

//窗体缩小
void MainWindow::on_pBt_mini_clicked()
{
    // 设置窗口最小化
    showMinimized();
}

// 事件过滤器实现(ui->pBt_max忽略双击事件)
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->pBt_max) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            // 完全禁用按钮的双击事件
            return true;
        }
    }
    if (obj == ui->lab_user_Avatar) {
        if (event->type() == QEvent::MouseButtonPress) {
            onAvatarDoubleClicked();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}
//窗体扩大
void MainWindow::on_pBt_max_clicked(bool checked)
{
    // 如果有正在执行的窗口状态变更，直接返回
    if (m_windowStateFuture.isRunning()) {
        return;
    }

    ui->pBt_max->setEnabled(false);

    // 使用QtConcurrent异步执行窗口状态变更，防止用户多次点击放大放小时，跟不上
    m_windowStateFuture = QtConcurrent::run([this, checked]() {
        // 窗口状态变更操作放在主线程执行
        QMetaObject::invokeMethod(this, [this, checked]() {
            if (checked) {
                ui->pBt_max->setToolTip(tr("还原"));
                qDebug("窗口最大化\n");
                ui->page_Sperker->FullScreenEn = 1;
                // showMaximized();
                // 先确保清除所有旧状态（最大化/最小化）
                this->setWindowState(Qt::WindowNoState);
                // 再强制设置最大化状态（替代 showMaximized()）
                this->setWindowState(Qt::WindowMaximized);
                // showFullScreen();
                // 设置窗口最大化
                showMaximized();

                // ui->page_Sperker->FullScreenEn = 1;
                // ui->page_Sperker->setupRadioButtons(true);
                // // 全屏状态
                // QTimer::singleShot(50, this, &MainWindow::updateVideoHoverPosition);

                QTimer::singleShot(50, this, [this]() {
                    // ui->page_Sperker->FullScreenEn = 1;
                    // qDebug("CCCC m_windowStateFuture\n");
                    ui->page_Sperker->setupRadioButtons(true);
                    emit updateVideoHoverPosition();
                });
            } else {
                ui->pBt_max->setToolTip(tr("最大化"));
                qDebug("窗口还原\n");
                ui->page_Sperker->FullScreenEn = 0;
                //showNormal();
                // 先确保清除所有旧状态（最大化/最小化）
                this->setWindowState(Qt::WindowNoState);
                //showNormal();

                // ui->page_Sperker->FullScreenEn = 0;
                // ui->page_Sperker->setupRadioButtons(false);
                // // 从全屏恢复
                // QTimer::singleShot(50, this, &MainWindow::updateVideoHoverPosition);
                QTimer::singleShot(50, this, [this]() {
                    // ui->page_Sperker->FullScreenEn = 0;
                    // qDebug("CCCC m_windowStateFuture2\n");
                    ui->page_Sperker->setupRadioButtons(false);
                    emit updateVideoHoverPosition();
                });
            }
        });

        // 等待窗口状态变更完成
        QThread::msleep(300);

        // 恢复按钮状态
        QMetaObject::invokeMethod(this, [this]() { ui->pBt_max->setEnabled(true); });
    });
}
//语言更新
void MainWindow::LanguageSet()
{
    //qDebug("MainWindow::LanguageSet(int index)\n");

    //刷新文本
    ui->retranslateUi(this);
    //ui->widget_eq->LanguageSet();
    if (ui->page_Sperker) ui->page_Sperker->LanguageSet();
    if (ui->widget_mic) ui->widget_mic->LanguageSet();

    if (ui->widget_eq) ui->widget_eq->LanguageSet();
    if (widget_listenSpeaker) widget_listenSpeaker->LanguageSet();

    if (ui->page_devSel) ui->page_devSel->LanguageSet();
    if (cl_widget_home_main_page_) cl_widget_home_main_page_->LanguageSet();
    if (ui->page_LoginActivationCode) ui->page_LoginActivationCode->LanguageSet();
    if (clp_user_setting_main_page_) clp_user_setting_main_page_->LanguageSet();
    if (clp_community_) clp_community_->LanguageSet();
    clp_user_setting_main_page_->clp_topButtons_->LanguageSet();

    //更新失败，则再次书写，额外更新
    ui->pBt_mini->setToolTip(tr("最小化"));
    ui->pBt_max->setToolTip(tr("最大化"));
    ui->pBt_close->setToolTip(tr("关闭"));

    ui->lab_user_name->setText(g_user_information.network.username);
    // ui->lab_user_id->setText("ID:"+g_user_information.network.id);
}

void MainWindow::on_pBt_spk_toggled(bool checked)
{
    // WBLIU:新版（pBt_spk checked=静音，pBt_spk_switch_ checked=有声，需反相）
    {
        if (checked) {
            cl_widget_home_main_page_->cl_speaker_setting_->pBt_spk_switch_->setChecked(false);
        } else {
            cl_widget_home_main_page_->cl_speaker_setting_->pBt_spk_switch_->setChecked(true);
        }
    }
}

void MainWindow::on_pBt_mic_toggled(bool checked)
{
    // WBLIU:新版（pBt_mic checked=静音，pBt_mic_switch_ checked=有声，需反相）
    if (checked) {
        cl_widget_home_main_page_->cl_microphone_setting_->pBt_mic_switch_->setChecked(false);
    } else {
        cl_widget_home_main_page_->cl_microphone_setting_->pBt_mic_switch_->setChecked(true);
    }
}

// 旧版
// //修改麦克风系统音量
// void MainWindow::on_hSlider_mic_valueChanged(int value)
// {
//     float volume = value / 100.0f;
//     pEndpointVolume[1]->SetMasterVolumeLevelScalar(volume, NULL);
//     micLevel = value;
// }
//旧版本OTA不带电量协议，所以只要这两个使能一个为true，就代表2.4G链接上
void MainWindow::showDevSta(bool en, bool OTAEn)
{
    ui->lab_status->show();

    bool forceDisableOTA = false;    // 声卡模式需强制禁用固件升级
    const bool is2_4GConnected = (OTAEn || en);   // 只要任一为 true 即表示 2.4G 已连接

    QString t_targetImagePath;                  //首页图片路径
    QString t_targetImagePath_leftTop_normal;   //首页 左上角设备正常 图片路径
    QString t_targetImagePath_leftTop_abnormal; //首页 左上角设备异常 图片路径

    {
        // WBLIU：
        // 遍历首页产品的名称和 PID VID GUID,更新 homePage 产品展示页面的产品图片
        for (int i = 0; i < ui->page_devSel->clp_device_selection_mainPage_
             ->clp_scrollArea_device_selection_->cl_all_device_list_.size();
             ++i) {
            // 获得设备基础信息
            unsigned short t_selDev_VID = ui->page_devSel->clp_device_selection_mainPage_
                    ->clp_scrollArea_device_selection_->cl_all_device_list_
                    .at(i)
                    ->cl_device_info_.SelDev_VID;
            unsigned short t_selDev_PID = ui->page_devSel->clp_device_selection_mainPage_
                    ->clp_scrollArea_device_selection_->cl_all_device_list_
                    .at(i)
                    ->cl_device_info_.SelDev_PID;
            QString SelDev_DeviceName = ui->page_devSel->clp_device_selection_mainPage_
                    ->clp_scrollArea_device_selection_->cl_all_device_list_
                    .at(i)
                    ->cl_device_info_.DeviceSysTypeName;
            QString t_selDev_Guid = ui->page_devSel->clp_device_selection_mainPage_
                    ->clp_scrollArea_device_selection_->cl_all_device_list_
                    .at(i)
                    ->cl_device_info_.DeviceGuid;

            // 存在完全匹配项
            if ((t_selDev_VID == SelDev_VID) && (t_selDev_PID == SelDev_PID)
                    && (t_selDev_Guid == SelDev_DeviceGuid)) {
                // 更新首页左上角 状态图片

                t_targetImagePath = ui->page_devSel->clp_device_selection_mainPage_
                        ->clp_scrollArea_device_selection_->cl_all_device_list_
                        .at(i)
                        ->cl_device_info_.DeviceHomePagePixmapPath; //首页图片路径
                t_targetImagePath_leftTop_normal
                        = ui->page_devSel->clp_device_selection_mainPage_
                        ->clp_scrollArea_device_selection_->cl_all_device_list_.at(i)
                        ->cl_device_info_
                        .DeviceHomePageTopLeftPixmapPath_normal; //首页 左上角设备正常 图片路径
                t_targetImagePath_leftTop_abnormal
                        = ui->page_devSel->clp_device_selection_mainPage_
                        ->clp_scrollArea_device_selection_->cl_all_device_list_.at(i)
                        ->cl_device_info_
                        .DeviceHomePageTopLeftPixmapPath_abnormal; //首页 左上角设备异常 图片路径
                break;
            }
        }
    }

    // 设置lab_status图片的通用函数
    auto setStatusImage = [&](bool connected) {
        ui->lab_status->clear();
        const QString &path = connected ? t_targetImagePath_leftTop_normal : t_targetImagePath_leftTop_abnormal;
        ui->lab_status->setPixmap(QPixmap(path).scaled(ui->lab_status->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    };


    // 不同机型的特殊处理
    if (SelDev_DeviceName.contains("T10", Qt::CaseInsensitive))
    {
        if (SelDev_DeviceName.contains("Wireless", Qt::CaseInsensitive))
        {
            cl_widget_home_main_page_->cl_microphone_setting_->cl_mic_hSlider_->show();
            setStatusImage(is2_4GConnected);
        } else {
            ui->proBar_BatteryLevel->hide(); //T10有线不显示电量
            ui->lab_battery->hide();

            ui->lab_Reconnect->hide();
            ui->pbt_explain_Reconnect->hide();
            ui->pBt_Reconnect->hide();

            cl_widget_home_main_page_->cl_microphone_setting_->cl_mic_hSlider_->hide();

            // ui->lab_status->setStyleSheet("border-image: url(:/Skin/Images/home/T10-no.png);");
            ui->lab_status->clear();
            ui->lab_status->setPixmap(QPixmap(t_targetImagePath_leftTop_normal)
                                      .scaled(ui->lab_status->size(),
                                              Qt::IgnoreAspectRatio,
                                              Qt::SmoothTransformation));
            //设置OTA升级是否可用
            clp_user_setting_main_page_->clp_version_settings_main_page_->SetOTAEn(false);

            return;
        }

    } /*else if(SelDev_DeviceName.contains("K03S",Qt::CaseInsensitive))
    {
        // ui->hSlider_mic->show();
        // ui->lEdit_micVol->show();
        ui->stackedWidget_Mic->setCurrentWidget(ui->page_MicShow);

        ui->lab_status->setStyleSheet("border-image: url(:/Skin/Images/home/K03S-no.png);");


    }*/
    else if (SelDev_DeviceName.contains("K03S", Qt::CaseInsensitive) && (SelDev_PID == 0xF016 || SelDev_PID == 0xF017))
    {
        cl_widget_home_main_page_->cl_microphone_setting_->cl_mic_hSlider_->show();
        setStatusImage(is2_4GConnected);

        //声卡模式不可使用固件升级回退
        if(SelDev_PID == 0xF017)
        {
            forceDisableOTA = true;
        }

    }else if((SelDev_DeviceName.contains("K03",Qt::CaseInsensitive)) && (!SelDev_DeviceName.contains("Wireless",Qt::CaseInsensitive)) && SelDev_PID == 0xE003)
    {
        ui->proBar_BatteryLevel->hide();//K03有线二代不显示电量，也不需要显示2.4G未连接
        ui->lab_battery->hide();

        ui->lab_Reconnect->hide();
        ui->pbt_explain_Reconnect->hide();
        ui->pBt_Reconnect->hide();


        cl_widget_home_main_page_->cl_microphone_setting_->cl_mic_hSlider_->show();
        // ui->lab_status->setStyleSheet("border-image: url(:/Skin/Images/home/K03-Wired-no.png);");
        ui->lab_status->clear();
        ui->lab_status->setPixmap(QPixmap(t_targetImagePath_leftTop_normal)
                                  .scaled(ui->lab_status->size(),
                                          Qt::IgnoreAspectRatio,
                                          Qt::SmoothTransformation));

        //设置OTA升级不可用
        clp_user_setting_main_page_->clp_version_settings_main_page_->SetOTAEn(false);

        return;
    } else if (SelDev_DeviceName.contains("K06S", Qt::CaseInsensitive))
    {
        cl_widget_home_main_page_->cl_microphone_setting_->cl_mic_hSlider_->show();
        setStatusImage(is2_4GConnected);
    } else if (SelDev_DeviceName.contains("T7 GT", Qt::CaseInsensitive) && (SelDev_PID == 0xF015 || SelDev_PID == 0xF009))
    {
        cl_widget_home_main_page_->cl_microphone_setting_->cl_mic_hSlider_->show();
        setStatusImage(is2_4GConnected);

        //声卡模式不可使用固件升级回退
        if(SelDev_PID == 0xF015)
        {
            forceDisableOTA = true;
        }

    } else if (SelDev_DeviceName.contains("T7", Qt::CaseInsensitive)&& (SelDev_PID == 0xF014 || SelDev_PID == 0xF008))
    {
        cl_widget_home_main_page_->cl_microphone_setting_->cl_mic_hSlider_->show();
        setStatusImage(is2_4GConnected);

        //声卡模式不可使用固件升级回退
        if(SelDev_PID == 0xF014)
        {
            forceDisableOTA = true;
        }

    } else if (SelDev_DeviceName.contains("S21", Qt::CaseInsensitive))
    {
        cl_widget_home_main_page_->cl_microphone_setting_->cl_mic_hSlider_->show();
        setStatusImage(is2_4GConnected);
    } else {
        // 其他机型没有状态图片设置
        cl_widget_home_main_page_->cl_microphone_setting_->cl_mic_hSlider_->show();
    }
    //2.4G模式是否连接，是否显示电量
    if (en)
    {
        apo->logWithTime("打开电量显示");
        ui->proBar_BatteryLevel->show();
        ui->lab_battery->show();

        ui->lab_Reconnect->hide();
        ui->pbt_explain_Reconnect->hide();
        ui->pBt_Reconnect->hide();

    } else {
        if (!timer_connect->isActive())
        {
            on_pBt_Reconnect_clicked();
        }

        emit ApoManager::instance()->requestlogWithTime("隐藏电量显示\n");
        ui->proBar_BatteryLevel->hide();
        ui->lab_battery->hide();

        if (OTAEn || en) {
            ui->lab_Reconnect->hide();
            ui->pbt_explain_Reconnect->hide();
        } else {
            ui->lab_Reconnect->show();
            ui->pbt_explain_Reconnect->show();
        }
    }
    //设置OTA升级是否可用
    if (forceDisableOTA)
    {
        OTAEn = false;
    }
    clp_user_setting_main_page_->clp_version_settings_main_page_->SetOTAEn(OTAEn);
}

//关闭定时器
void MainWindow::CloseTimer()
{
    timer_connect->stop();
    TimerR->stop();
}

//定时器（异步执行操作，放置阻塞ui）
void MainWindow::Timer_readData()
{
    qDebug("进入定时\n");
    if (readWatcher->isRunning()) {
        qDebug("上次读取尚未完成，跳过");
        return;
    }

    // 启动异步读取
    QFuture<int> future = QtConcurrent::run([this]() -> int {
        // 注意：确保 lolib->read 是线程安全的
        memset(readBuffer, 0, sizeof(readBuffer));
        return lolib->read(readBuffer, 64, 100, 3);
    });
    readWatcher->setFuture(future);
}
void MainWindow::onReadFinished()
{
    int ret = readWatcher->result();
    if (ret < 0) {
        isHidRun = false;
        lolib->closeCard();
        TimerR->stop();
        showDevSta(false, false);

        if (!timer_connect->isActive())
        {
            on_pBt_Reconnect_clicked();
        }

    } else if (ret > 0) {
        if (readBuffer[1] == 0x10 && readBuffer[2] == 0x01) {
            int batteryLevel = readBuffer[7] & 0x7F;
            ui->proBar_BatteryLevel->setValue(batteryLevel);
            if(batteryLevel<=20)
            {
                ui->proBar_BatteryLevel->setStyleSheet("QProgressBar{border-image: url(:/Skin/Images/home/border-less.png);padding: 2px;padding-right:4px;}QProgressBar::chunk{border-image: url(:/Skin/Images/home/full-less.png);}");
            }else
            {
                ui->proBar_BatteryLevel->setStyleSheet("QProgressBar{border-image: url(:/Skin/Images/home/border.png);padding: 2px;padding-right:4px;}QProgressBar::chunk{border-image: url(:/Skin/Images/home/full.png);}");
            }
            // if(readBuffer[7] & 0x80)
            // {
            //     ui->proBar_BatteryLevel->setStyleSheet("...");
            // }
        }else if(readBuffer[1] == 0x10 && readBuffer[2] == 0x09)
        {
            int batteryLevel = readBuffer[4] & 0x7F;
            ui->proBar_BatteryLevel->setValue(batteryLevel);
            if(batteryLevel<=20)
            {
                ui->proBar_BatteryLevel->setStyleSheet("QProgressBar{border-image: url(:/Skin/Images/home/border-less.png);padding: 2px;padding-right:4px;}QProgressBar::chunk{border-image: url(:/Skin/Images/home/full-less.png);}");
            }else
            {
                ui->proBar_BatteryLevel->setStyleSheet("QProgressBar{border-image: url(:/Skin/Images/home/border.png);padding: 2px;padding-right:4px;}QProgressBar::chunk{border-image: url(:/Skin/Images/home/full.png);}");
            }
        }
    }
}

//握手上报耳机状态信息
int MainWindow::GetDevSta()
{
    int res = -1;
    DevStatus sta;
    res = lolib->GetDevStatus(sta);
    if (res < 0 || res == 0) {
        //qDebug("Unable to read()\n");
        // msgBox.critical(NULL,tr("错误"),tr("获得耳机状态信息失败"));//不要显示，因为以前版本没有协议，肯定出错
        return -1;
    } else if (res > 0) {
        //耳机连接状态
        if (sta.ConnectSta == 0) {
            //msgBox.critical(NULL,tr("错误"),tr("2.4G 模式，耳机状态未连接，电量显示与固件升级不可使用"),tr("关闭"));//(第一版T10无线没有该协议)
            lolib->closeCard();
            isHidRun = false;
            //显示电量
            ui->stackedWidget_battery->setCurrentWidget(ui->page_battery);
            //换耳机状态图标为未连接
            showDevSta(false,false);
            return 0;

        } else if((sta.ConnectSta == 1) || (sta.ConnectSta == 2))
        {
            //1：2.4G模式，2：有线模式
            if(sta.ConnectSta == 1)
            {
                //显示电量
                ui->stackedWidget_battery->setCurrentWidget(ui->page_battery);
                //换耳机状态图标为连接
                showDevSta(true,true);
            }else
            {
                //显示有线连接
                ui->stackedWidget_battery->setCurrentWidget(ui->page_Wired);
                //换耳机状态图标为连接
                showDevSta(true,false);
            }

            isHidRun = true;
        }else
        {
            lolib->closeCard();
            isHidRun = false;
            //换耳机状态图标为未连接
            showDevSta(false,false);
            return 0;
        }
        //电池电量
        ui->proBar_BatteryLevel->setValue(sta.electricity);
        if(sta.electricity<=20)
        {
            ui->proBar_BatteryLevel->setStyleSheet("QProgressBar{border-image: url(:/Skin/Images/home/border-less.png);padding: 2px;padding-right:4px;}QProgressBar::chunk{border-image: url(:/Skin/Images/home/full-less.png);}");
        }else
        {
            ui->proBar_BatteryLevel->setStyleSheet("QProgressBar{border-image: url(:/Skin/Images/home/border.png);padding: 2px;padding-right:4px;}QProgressBar::chunk{border-image: url(:/Skin/Images/home/full.png);}");
        }
        // //麦克风状态
        // if(sta.MicEn == 1)
        // {
        //     ui->pBt_mic->setChecked(true);
        // }else if(sta.MicEn == 2)
        // {
        //     ui->pBt_mic->setChecked(false);
        // }
        // //麦克风侦听状态
        // ui->widget_mic->GetDevListen(sta.MicListening);
        // //麦克风增强状态
        // ui->widget_mic->GetDevListen(sta.MicSta);
    }
    return 1;
}

//导航切换
void MainWindow::Nav_toggled(int id, bool checked)
{
    if (checked) {
        switch (id) {
        case 0: //首页
            // ui->widget_status->setStyleSheet("background-color: rgb(14, 17, 22);");
            ui->stackedWidget->setCurrentWidget(ui->page_main);
            // cl_widget_home_main_page_->updateUIInfo();//同步更新 UI
            if (currentPlanRadio == nullptr) {
                return;
            }
        {
            {
                // 主页预设方案页面
                QPair<QString, QString> t_target_plan_Key
                        = qMakePair(currentPlanRadio->lab_name->text(),
                                    currentPlanRadio->lab1->text());
                cl_widget_home_main_page_->cl_plans_selection_->updatePlansUIInfo(
                            t_target_plan_Key);
            }

            {
                // 算法页面
                // 如果当前预设 有开启 下面任意一项
                if (currentPlanVal.AlgoOpenEn || currentPlanVal.spaceOpenEn
                        || currentPlanVal.eqOpenEn || currentPlanVal.drcOpenEn) {
                    HomePageExtraEQOpen = false;
                    // 避免重复
                    cl_widget_home_main_page_->cl_algorithm_adjustment_setting
                            ->cl_customPushButton_->blockSignals(true);
                    cl_widget_home_main_page_->cl_algorithm_adjustment_setting
                            ->cl_customPushButton_->setChecked(HomePageExtraEQOpen);
                    cl_widget_home_main_page_->cl_algorithm_adjustment_setting
                            ->cl_customPushButton_->blockSignals(false);

                    cl_widget_home_main_page_->cl_algorithm_adjustment_setting->setEditStatus(
                                false);
                } else {
                    // HomePageExtraEQOpen = false;
                    cl_widget_home_main_page_->cl_algorithm_adjustment_setting
                            ->cl_single_algorithm_setting_1_->updateValue(
                                HomePageExtraEQValue.at(0));
                    cl_widget_home_main_page_->cl_algorithm_adjustment_setting
                            ->cl_single_algorithm_setting_2_->updateValue(
                                HomePageExtraEQValue.at(1));
                    cl_widget_home_main_page_->cl_algorithm_adjustment_setting
                            ->cl_single_algorithm_setting_3_->updateValue(
                                HomePageExtraEQValue.at(2));
                    cl_widget_home_main_page_->cl_algorithm_adjustment_setting
                            ->cl_single_algorithm_setting_4_->updateValue(
                                HomePageExtraEQValue.at(3));

                    // 避免重复
                    cl_widget_home_main_page_->cl_algorithm_adjustment_setting
                            ->cl_customPushButton_->blockSignals(true);
                    cl_widget_home_main_page_->cl_algorithm_adjustment_setting
                            ->cl_customPushButton_->setChecked(HomePageExtraEQOpen);
                    cl_widget_home_main_page_->cl_algorithm_adjustment_setting
                            ->cl_customPushButton_->blockSignals(false);

                    // 手动发射信号
                    emit cl_widget_home_main_page_->cl_algorithm_adjustment_setting
                            ->cl_customPushButton_->toggled(HomePageExtraEQOpen);
                }
            }
        }
            break;
        case 1: //耳机
            //只有第一次点击耳机时，显示预设库界面，否则显示被选中方案的
            if(EqPage_FirstClicked || (currentPlanRadio == NULL))
            {
                EqPage_FirstClicked = false;
                ui->stackedWidget->setCurrentWidget(ui->page_Sperker);
                ui->page_Sperker->SwitchMode();
            }else
            {
                ui->widget_eq->SwitchEqPage();
                ui->widget_eq->ShowEqVal(currentPlanVal.DataVisibleEn);
                ui->widget_eq->PageShowPlanVal();
                ui->stackedWidget->setCurrentWidget(ui->page_eq);
            }

            //模式界面（电影、音乐、游戏）
            globalSettings->setValue("ModePage", 0);
            globalSettings->sync(); // 立即写入所有未保存的更改


            //上位机使用指南--先隐藏
            // if (UserGuideEn) {
            //     on_pBt_UserGuide_clicked();
            // }
            break;
        case 2: //麦克风

            ui->stackedWidget->setCurrentWidget(ui->page_mic);

            // ui->widget_mic->Test();
            break;
        case 3: //自定义
            break;
        case 4: //社区
            ui->stackedWidget->setCurrentWidget(ui->page_community);
            break;
        case 5: //设置、更多
        {

            clp_user_setting_main_page_->UpdateAllSubPageUIInformation ();
            ui->stackedWidget->setCurrentWidget(ui->page_more);
        }
            break;
        default:
            break;
        }
    }
}

//电量改变
void MainWindow::on_proBar_BatteryLevel_valueChanged(int value)
{
    ui->lab_battery->setText(QString::number(value) + "%");
}

void MainWindow::setupEightMyPlanSync(const QList<EightMyPlan *> &plans)
{
    for (EightMyPlan *sender : plans) {
        for (EightMyPlan *receiver : plans) {
            if (sender == receiver)
                continue;

            QObject::connect(sender,
                             &EightMyPlan::layoutChanged,
                             receiver,
                             &EightMyPlan::updateLayoutFromOrder);

            QObject::connect(sender,
                             &EightMyPlan::btnCheckedChanged,
                             receiver,
                             &EightMyPlan::updateChecked);

            QObject::connect(sender,
                             &EightMyPlan::btnAllDisChecked,
                             receiver,
                             &EightMyPlan::AllBtnDisChecked);

            //跳转到均衡器页面(系统方案不显示EQ),试听界面不跳转
            //connect(sender, &EightMyPlan::FavToEq, ui->page_Sperker, &SpeakerSet::on_pBt_ClosePlanPage_clicked);
            connect(sender, &EightMyPlan::FavToEq, this, [this, sender]() {
                if (sender != widget_listenSpeaker->eightPlan_l) {
                    emit ApoManager::instance()->requestlogWithTime("int EightMyPlan::FavToEq");

                    ui->stackedWidget->setCurrentWidget(ui->page_Sperker);
                    ui->page_Sperker->FavShowEQpageChange();
                }
            });

            //保存
            connect(sender, &EightMyPlan::PlanSave_F, ui->page_Sperker, &SpeakerSet::PlanSave);
        }
    }
    /* //效果如下
     * connect(ui->page_Sperker->eightPlan_s, &EightMyPlan::layoutChanged,
            widget_listenSpeaker->eightPlan_l, &EightMyPlan::updateLayoutFromOrder);

    connect(ui->page_Sperker->eightPlan_s, &EightMyPlan::layoutChanged,
            ui->widget_eq->eightPlan_e, &EightMyPlan::updateLayoutFromOrder);

    connect(widget_listenSpeaker->eightPlan_l, &EightMyPlan::layoutChanged,
            ui->page_Sperker->eightPlan_s, &EightMyPlan::updateLayoutFromOrder);
    connect(widget_listenSpeaker->eightPlan_l, &EightMyPlan::layoutChanged,
            ui->widget_eq->eightPlan_e, &EightMyPlan::updateLayoutFromOrder);

    connect(ui->widget_eq->eightPlan_e, &EightMyPlan::layoutChanged,
            ui->page_Sperker->eightPlan_s, &EightMyPlan::updateLayoutFromOrder);

    connect(ui->widget_eq->eightPlan_e, &EightMyPlan::layoutChanged,
            widget_listenSpeaker->eightPlan_l, &EightMyPlan::updateLayoutFromOrder);


    connect(ui->page_Sperker->eightPlan_s, &EightMyPlan::btnCheckedChanged,
            widget_listenSpeaker->eightPlan_l, &EightMyPlan::updateChecked);
    connect(ui->page_Sperker->eightPlan_s, &EightMyPlan::btnCheckedChanged,
            ui->widget_eq->eightPlan_e, &EightMyPlan::updateChecked);

    connect(widget_listenSpeaker->eightPlan_l, &EightMyPlan::btnCheckedChanged,
            ui->page_Sperker->eightPlan_s, &EightMyPlan::updateChecked);
    connect(widget_listenSpeaker->eightPlan_l, &EightMyPlan::btnCheckedChanged,
            ui->widget_eq->eightPlan_e, &EightMyPlan::updateChecked);

    connect(ui->widget_eq->eightPlan_e, &EightMyPlan::btnCheckedChanged,
            ui->page_Sperker->eightPlan_s, &EightMyPlan::updateChecked);

    connect(ui->widget_eq->eightPlan_e, &EightMyPlan::btnCheckedChanged,
            widget_listenSpeaker->eightPlan_l, &EightMyPlan::updateChecked);*/
}

// // WBLIU:旧版
// //设置系统默认扬声器
// void MainWindow::on_cBox_Speaker_currentIndexChanged(int index)
// {
//     if(index != -1)
//     {
//         QString deviceId = ui->cBox_Speaker->itemData(index).toString();
//         SelDev_DeviceGuid = deviceId;
//         DefaultOutput::changeDevice( deviceId );
//         emit ApoManager::instance()->requestSetLhdcDevice(SelDev_DeviceGuid);
//     }
// }

//设置系统默认麦克风
void MainWindow::SetMicCurrentIndexChanged(int index)
{
    if (index != -1) {
        QString deviceId = cl_widget_home_main_page_->cl_microphone_setting_->cl_cBox_Mic_
                ->itemData(index)
                .toString();
        if (DefaultOutput::changeDevice(deviceId)) {
            refreshDefaultEndpointVolume(eCapture);
        }

        std::wstring wstr = deviceId.toStdWString();
        // idSaved = const_cast<wchar_t*>(wstr.c_str());
        size_t len = wstr.length() + 1;
        idSaved = new wchar_t[len];
        wcscpy_s(idSaved, len, wstr.c_str());
        if (apo->IsSupportEP()) {
            qDebug("设备支持上行操作\n");
            ui->widget_mic->setUpEn(true);
        } else {
            qDebug("设备不支持上行操作\n");
            //麦克风AI降噪不可用
            ui->widget_mic->setUpEn(false);
        }
    }
}

//更新阴影大小
void MainWindow::UpdateShadowLabelSize(QLabel *&labelOut)
{
    if (labelOut) {
        {
            // 设置几何区域为父控件大小
            labelOut->setGeometry(0,
                                  0,
                                  labelOut->parentWidget()->width(),
                                  labelOut->parentWidget()->height());
        }
    }
}
//创建阴影
void MainWindow::createShadowLabel(QWidget *parent, QLabel *&labelOut)
{
    // 创建 QLabel 并设置父控件
    labelOut = new QLabel(parent);

    // 设置透明度效果
    // QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect;
    // opacityEffect->setOpacity(0.2); // 50% 透明度
    // labelOut->setGraphicsEffect(opacityEffect);

    // 提升到顶层并显示
    labelOut->raise();
    labelOut->show();
}
//点击关闭按钮
void MainWindow::onAvatarDoubleClicked()
{
    // WBLIU:头像双击
    clp_user_setting_main_page_->clp_topButtons_->cl_all_settings_type_.at(0)->click();
    ui->pBt_MoreItem->setChecked(true); // 显示更多设置界面
    // group_Nav->button (5)->click ();
}

//点击关闭按钮
void MainWindow::on_pBt_close_clicked()
{
    if (!g_user_system_settings_config_info.is_remember_choice.load()) {


        bExitDirectly *bED = new bExitDirectly(this);
        bED->setModal(true);
        // 获取父窗口的中心点
        int x = this->x() + (this->width() - bED->width()) / 2;
        int y = this->y() + (this->height() - bED->height()) / 2;
        bED->move(x, y);
        bED->ShowExitMode();
        int result = bED->exec();
        if (result == QDialog::Accepted) {
            bED->SetExitMode();
            //if(!boolExitDirectly)
            {
                clp_user_setting_main_page_->clp_system_settings_mainPage_->UpdateSystemSettingsUIInformation ();
                clp_user_setting_main_page_->clp_system_settings_mainPage_->repaint ();
            }
            close();

        }
        bED->deleteLater();

    }else
    {
        close();
    }

}


void MainWindow::OpenAPOEffects()
{
    // qDebug("进入OpenAPOEffects\n");
    int idx = globalSettings->value("ModePage").toUInt();
    ModeVal TempVal;

    bool en = false;
    switch(idx)
    {
    case 0:
        //电影
        TempVal = MovieVal;

        break;
    default:
        en = false;
        break;
    }
    // qDebug("Test Mode:%d,en:%d\n",idx,en);

    auto selectFirstPresetForDevice = [TempVal](const QString& dev) {
        for (auto* btn : TempVal.AllPlanRadioList) {
            // 假设按钮有 device() 方法或我们可以通过某种方式获取其设备
            if (btn->lab1->text() == dev) { // 需要确认接口
                btn->setChecked(true);
                // currentPlanVal = btn->getAllPlanValue();
                return;
            }
        }
    };

    QString name = TempVal.C_PlanName;
    QStringList devs = TempVal.C_PlanDev;
    QString currentDev = ui->page_Sperker->GetCurrentDeviceIdentifier();
    if(devs.isEmpty())
    {
        return;
    }
    // 方案开关门控：关闭时不应用方案效果（原逻辑无条件 setChecked/ShowcurrentPlanVal，
    // 会绕过开关使上次选定方案直接生效——视觉与实效不匹配）
    const bool t_plans_open = HomePagePlansOpen;
    {
        //分机型显示时
        // for (const QString& dev : devs)
    }

    //不分机型显示时
    QString dev = QString();
    {
        if(!TempVal.C_PlanName.isEmpty() && ui->page_Sperker->isDeviceMatchingPlanDev(dev))
            // if(!TempVal.C_PlanName.isEmpty())
        {
            // 使用哈希表查找目标按钮
            NewRadioBtn* target = MovieVal.AllPlanRadioHash.value(qMakePair(name, dev), nullptr);

            if (target) {
                if (t_plans_open) {
                    target->setChecked(true); // 选中 + 触发 toggled → 应用方案
                } else {
                    // 仅恢复选中显示（高亮）与数据链（currentPlanRadio/currentPlanVal），不触发应用链路；
                    // 否则开关关闭态重启后二者恒空，打开开关也无法应用方案效果
                    QSignalBlocker blocker(target);
                    target->setChecked(true);
                    currentPlanRadio = target;
                    currentPlanVal = target->getAllPlanValue();
                }
            }else
            {
                return;
                // selectFirstPresetForDevice(currentDev);
            }
        }else
        {
            return;
            // selectFirstPresetForDevice(currentDev);
        }
    }


    if (t_plans_open) {
        ui->page_Sperker->ShowcurrentPlanVal(); // 应用当前方案到 APO
    }
    // emit ApoManager::instance()->requestlogWithTime("OpenAPOEffects()");


}
//点击-用户手册
void MainWindow::on_pBt_UserGuide_clicked()
{


    if(!uGuide)
    {
        uGuide = new UserGuide(m);
        uGuide->setModal(m);
    }
    // 获取父窗口的中心点
    int x = m->x() + (m->width() - uGuide->width()) / 2;
    int y = m->y() + (m->height() - uGuide->height()) / 2;
    uGuide->move(x, y);
    int result = uGuide->exec();
    //if(result == QDialog::Accepted)
    {
        UserGuideEn = false;
        globalSettings->setValue("UserGuideEn",false);//true:下次显示，false:下次不显示
    }



}

//点击重试，定时三次链接，成功则停止。三次不成功也停止
void MainWindow::on_pBt_Reconnect_clicked()
{
    timer_connect->stop();
    m_colorAnimation->start();// 启动文本颜色闪烁动画

    ui->pBt_Reconnect->hide();
    m_remainCount = 0;
    ui->lab_Reconnect->setText(tr("发射器与耳机连接中（0/3）"));
    // 启动定时器1分钟
    timer_connect->start(30000);
}
void MainWindow::onTimeout_connect()
{
    // 每次定时器触发
    qDebug() << "定时器触发，剩余次数：" << m_remainCount;


    //导入HIDAPI
    if(lolib->openCard() == 1)
    {
        isHidRun = true;
    }else
    {
        isHidRun = false;
    }
    if(isHidRun)
    {
        int res = GetDevSta();
        if(res == 1)
        {
            showDevSta(true,true);

            timer_connect->stop();

            // 停止动画，恢复颜色为完全不透明
            m_colorAnimation->stop();
            ui->lab_Reconnect->setStyleSheet(m_baseStyle_connect + "color: rgba(116, 124, 131, 255);");

            qDebug() << "2.4G已连接";

            TimerR->start(); // 启动定时器
            return;
        }
    }

    m_remainCount++;
    ui->lab_Reconnect->setText(tr("发射器与耳机连接中（%1/3）").arg(m_remainCount));
    // 触发三次，停止定时器
    if (m_remainCount >= 3) {
        timer_connect->stop();

        // 停止动画，恢复颜色为完全不透明
        m_colorAnimation->stop();
        ui->lab_Reconnect->setStyleSheet(m_baseStyle_connect + "color: rgba(116, 124, 131, 255);");

        ui->lab_Reconnect->setText(tr("发射器与耳机连接失败"));
        ui->pBt_Reconnect->show();
        qDebug() << "三次定时完成，定时器已停止";
    }
}

//设置主题（用不到切换的控件，这里没写）
void MainWindow::setThemeAndPanelTransparency(int idx,int PValue)
{

    //后续把图片都添加后缀
    {
        QString suffix;
        switch (idx)
        {
        case 0: suffix = ""/*"_darkBlue"*/; break;//深蓝色（还未修改主题图片）
        case 1: suffix = "_white";  break;//白色
        case 2: suffix = "_black";  break;//黑色
        default: suffix = "";      break;
        }
        QString imagePath = QString(":/Skin/Images/home/background%1.png").arg(suffix);
        ui->centralwidget->setStyleSheet(QString("border-image: url(%1);").arg(imagePath));
        ui->stackedWidget_SelMain->setStyleSheet("background:transparent;border:none;border-image: none;");
        ui->stackedWidget->setStyleSheet("background:transparent;border-image: none;border:none;");
        ui->widget_status->setStyleSheet("background:transparent;border-image: none;border:none;");
        //教程和用户名之间的分隔符（不确定主题是否会改，先保持写死）
        ui->label->setStyleSheet("border-image:none;image: url(:/Skin/Images/home/VerticalBar.png);");
        //用户教程
        ui->pBt_UserGuide->setStyleSheet(
                    QString("QPushButton{"
                            "border-image: url(:/Skin/Images/home/UserGuide-no%1.png); }"
                            "QPushButton:hover{"
                            " border-image: url(:/Skin/Images/home/UserGuide-ho%1.png);}")
                    .arg(suffix)
                    );
        //喇叭
        ui->pBt_spk->setStyleSheet(
                    QString("QPushButton{"
                            "border-image: url(:/Skin/Images/home/speaker-open%1.png);}"
                            "QPushButton:checked{"
                            "border-image: url(:/Skin/Images/home/speaker-close%1.png);}")
                    .arg(suffix)
                    );
        //麦克风
        ui->pBt_mic->setStyleSheet(
                    QString("QPushButton{"
                            "border-image: url(:/Skin/Images/home/mic-open%1.png);}"
                            "QPushButton:checked{"
                            "border-image: url(:/Skin/Images/home/mic-close%1.png);}")
                    .arg(suffix)
                    );
        //重连问号提示按钮
        ui->pbt_explain_Reconnect->setStyleSheet(
                    QString("QPushButton{"
                            "border-image: url(:/Skin/Images/Headphones/explain-no2%1.png);}")
                    .arg(suffix)
                    );
        //电量显示进度条
        ui->proBar_BatteryLevel->setStyleSheet(
                    QString("QPushButton{"
                            "border-image: url(:/Skin/Images/home/border%1.png);"
                            "padding: 2px; /* 设置内边距，使chunk与边框产生间隔 */"
                            "padding-right:4px;"
                            "}"
                            "QPushButton::chunk{"
                            "border-image: url(:/Skin/Images/home/full%1.png);}")
                    .arg(suffix)
                    );
        //导航栏logo
        ui->label_logo->setStyleSheet(
                    QString("QPushButton{"
                            "image: url(:/Skin/Images/nav/logo%1.png);"
                            "background:transparent;border-image: none;border:none;}")
                    .arg(suffix)
                    );
        //导航栏图标
        ui->pBt_mainItem->setStyleSheet(
                    QString("QPushButton{"
                            "image: url(:/Skin/Images/nav/home-no%1.png);"
                            "background:transparent;border-image: none;border:none;"
                            "}"
                            "QPushButton::checked{"
                            "image: url(:/Skin/Images/nav/home-se%1.png);"
                            "}"
                            "QPushButton:hover:!checked{"
                            "image: url(:/Skin/Images/nav/home-se%1.png);}")
                    .arg(suffix)
                    );
        ui->pBt_eqItem->setStyleSheet(
                    QString("QPushButton{"
                            "image: url(:/Skin/Images/nav/horn-no%1.png);"
                            "background:transparent;border-image: none;border:none;"
                            "}"
                            "QPushButton::checked{"
                            "image: url(:/Skin/Images/nav/horn-se%1.png);"
                            "}"
                            "QPushButton:hover:!checked{"
                            "image: url(:/Skin/Images/nav/horn-se%1.png);}")
                    .arg(suffix)
                    );
        ui->pBt_MicItem->setStyleSheet(
                    QString("QPushButton{"
                            "image: url(:/Skin/Images/nav/mic-no%1.png);"
                            "background:transparent;border-image: none;border:none;"
                            "}"
                            "QPushButton::checked{"
                            "image: url(:/Skin/Images/nav/mic-se%1.png);"
                            "}"
                            "QPushButton:hover:!checked{"
                            "image: url(:/Skin/Images/nav/mic-se%1.png);}")
                    .arg(suffix)
                    );
        ui->pBt_SelfItem->setStyleSheet(
                    QString("QPushButton{"
                            "image: url(:/Skin/Images/nav/self-no%1.png);"
                            "background:transparent;border-image: none;border:none;"
                            "}"
                            "QPushButton::checked{"
                            "image: url(:/Skin/Images/nav/self-se%1.png);"
                            "}"
                            "QPushButton:hover:!checked{"
                            "image: url(:/Skin/Images/nav/self-se%1.png);}")
                    .arg(suffix)
                    );
        ui->pBt_CommItem->setStyleSheet(
                    QString("QPushButton{"
                            "image: url(:/Skin/Images/nav/comm-no%1.png);"
                            "background:transparent;border-image: none;border:none;"
                            "}"
                            "QPushButton::checked{"
                            "image: url(:/Skin/Images/nav/comm-se%1.png);"
                            "}"
                            "QPushButton:hover:!checked{"
                            "image: url(:/Skin/Images/nav/comm-se%1.png);}")
                    .arg(suffix)
                    );
        ui->pBt_MoreItem->setStyleSheet(
                    QString("QPushButton{"
                            "image: url(:/Skin/Images/nav/more-no%1.png);"
                            "background:transparent;border-image: none;border:none;"
                            "}"
                            "QPushButton::checked{"
                            "image: url(:/Skin/Images/nav/more-se%1.png);"
                            "}"
                            "QPushButton:hover:!checked{"
                            "image: url(:/Skin/Images/nav/more-se%1.png);}")
                    .arg(suffix)
                    );
        //关闭按钮
        ui->pBt_close->setStyleSheet(
                    QString("QPushButton{"
                            "border: 0px;color: rgb(255, 255, 255);background:transparent;"
                            "border-image: url(:/Skin/Images/home/exit%1.png);"
                            "}"
                            "QPushButton:hover{"
                            "border-image: url(:/Skin/Images/home/exit-ho%1.png);}")
                    .arg(suffix)
                    );
        //扩大按钮
        ui->pBt_max->setStyleSheet(
                    QString("QPushButton{"
                            "border: 0px;color: rgb(255, 255, 255);background:transparent;"
                            "border-image: url(:/Skin/Images/home/max%1.png);"
                            "}"
                            "QPushButton:checked{"
                            "border: 0px;"
                            "border-image: url(:/Skin/Images/home/Reduce%1.png);"
                            "}"
                            "QPushButton::hover{"
                            "border-image: url(:/Skin/Images/home/max-ho%1.png);"
                            "}"
                            "QPushButton::hover:checked{"
                            "border-image: url(:/Skin/Images/home/Reduce-ho%1.png);}")
                    .arg(suffix)
                    );
        //缩小按钮
        ui->pBt_max->setStyleSheet(
                    QString("QPushButton{"
                            "border: 0px;color: rgb(255, 255, 255);background:transparent;"
                            "border-image: url(:/Skin/Images/home/mini%1.png);"
                            "}"
                            "QPushButton:hover{"
                            "border-image: url(:/Skin/Images/home/mini-ho%1.png);}")
                    .arg(suffix)
                    );
    }


    // 根据主题索引选择文字颜色
    {
        QString textColor,textColor_ho;
        //重连提示
        switch (idx) {
        case 0: textColor = "#747C83"; break;   // 深蓝色
        case 1: textColor = "#747C83"; break;   // 白色
        case 2: textColor = "#747C83"; break;   // 黑色
        default: textColor = "#747C83"; break;
        }
        ui->lab_Reconnect->setStyleSheet(
                    QString("border:none;"
                            "background: transparent;"
                            "color: %1;"
                            "font-family: \"Noto Sans S Chinese\";"
                            "font-weight:500;"
                            "font-size: 10px;")
                    .arg(textColor)
                    );


        switch (idx) {
        case 0: textColor = "#A0A5AC"; break;   // 深蓝色
        case 1: textColor = "#A0A5AC"; break;   // 白色
        case 2: textColor = "#A0A5AC"; break;   // 黑色
        default: textColor = "#A0A5AC"; break;
        }
        //用户名
        ui->lab_user_name->setStyleSheet(
                    QString("border:none;"
                            "background:transparent;"
                            "color:%1;"
                            "font-family:\"Noto Sans S Chinese\";"
                            "font-weight:500;"
                            "font-size:14px;")
                    .arg(textColor)
                    );
        //电量
        ui->lab_battery->setStyleSheet(
                    QString("font-family: \"Noto Sans S Chinese\";"
                            "font-width:500;"
                            "font-size: 10px;"
                            "color: %1;"
                            "border-image:none;"
                            "background: transparent;")
                    .arg(textColor)
                    );
        //扬声器音量
        ui->label_level->setStyleSheet(
                    QString("font-family: \"Noto Sans S Chinese\";"
                            "font-width:500;"
                            "font-size: 10px;"
                            "color: %1;"
                            "border-image:none;"
                            "background: transparent;")
                    .arg(textColor)
                    );

        //重连按钮
        switch (idx) {
        case 0: {textColor = "#4793D1";textColor_ho = "#31658F"; break;}   // 深蓝色
        case 1: {textColor = "#4793D1";textColor_ho = "#31658F"; break;}   // 白色
        case 2: {textColor = "#4793D1";textColor_ho = "#31658F"; break;}   // 黑色
        default: {textColor = "#4793D1";textColor_ho = "#31658F"; break;}
        }
        ui->pBt_Reconnect->setStyleSheet(
                    QString("QPushButton{font-family: \"Noto Sans S Chinese\";"
                            "font-width:500;"
                            "font-size: 10px;"
                            "color: %1;"
                            "border-image:none;"
                            "background: transparent;}"
                            "QPushButton:hover{color: %2;}")
                    .arg(textColor).arg(textColor_ho)
                    );

    }

    //面板（透明度+模糊度）
    {
        setPanelTransparency(idx,PValue);
        setPanelBlur(PValue);
    }

}
//设置面板透明度
void MainWindow::setPanelTransparency(int idx,int PValue)
{
    double PanelTransparency = PValue / 100.0;   //面板透明度(默认值是0.2)
    int r, g, b;
    switch (idx) {
    case 0:
        r = 52; g = 65; b = 91; break;// 深蓝色
    case 1:
        r = 52; g = 65; b = 91; break;// 白色
    case 2:
        r = 52; g = 65; b = 91; break; // 黑色
    default:
        r = 52; g = 65; b = 91; break;
    }

    QString colorStr = QString("rgba(%1, %2, %3, %4)")
            .arg(r).arg(g).arg(b).arg(PanelTransparency);
    //标题栏
    ui->widget_title->setStyleSheet(QString("background-color: %1; border: none; border-image: none;").arg(colorStr));
    switch (idx) {
    case 0:
        r = 59; g = 74; b = 105; break;// 深蓝色
    case 1:
        r = 59; g = 74; b = 105; break;// 白色
    case 2:
        r = 59; g = 74; b = 105; break; // 黑色
    default:
        r = 59; g = 74; b = 105; break;
    }
    colorStr = QString("rgba(%1, %2, %3, %4)")
            .arg(r).arg(g).arg(b).arg(PanelTransparency);
    //导航栏
    ui->widget_tree->setStyleSheet(QString("background-color: %1;border:none;border-image: none;").arg(colorStr));
}
//设置面板模糊度
void MainWindow::setPanelBlur(int PValue)
{
    Q_UNUSED(PValue); // 模糊半径以 g_user_information.local.panel_blur_radius_ 为准
    scheduleBlurRebuild();
}

/// \brief 触发面板模糊重算（降采样后成本很低，直接实时重算，跟手无延时）
void MainWindow::scheduleBlurRebuild()
{
    rebuildPanelBlur();
}

/// \brief 重算面板毛玻璃模糊缓存（防抖超时执行）
void MainWindow::rebuildPanelBlur()
{
    // 归一化 0~1 → 0~25px（配合 XIBERIA_X_HUB_Utils::blurPixmap 的三遍高斯区间）
    // 最低模糊半径 1（模糊值不允许为 0）；存储值 0..1 与滑块往返一致
    const int t_radius = qMax(1, qRound(g_user_information.local.panel_blur_radius_ * 25.0));
    AppImageCache::instance().updateBlurredBackdrop(size(),
                                                    t_radius,
                                                    g_user_information.local.panel_opacity_);
    update();
}

/// \brief 界面设置 — 背景透明度变化
void MainWindow::onBackgroundTransparencyChanged(qreal opacity)
{
    setBackgroundTransparency(qRound(opacity * 100));
}

/// \brief 界面设置 — 面板模糊度变化
void MainWindow::onPanelBlurChanged(qreal radius)
{
    setPanelBlur(qRound(radius * 100));
}

/// \brief 界面设置 — 壁纸变更
void MainWindow::onBackgroundChanged(const QString &path)
{
    if (!QFile::exists(path))
        return;
    QPixmap t_pm(path);
    if (t_pm.isNull())
        return;
    AppImageCache &t_cache = AppImageCache::instance();
    t_cache.cl_background_pixmap_ = t_pm;
    t_cache.updateBackgroundCache(size());
    scheduleBlurRebuild(); // 壁纸变了，模糊快照需重算
    update();
}

/// \brief 界面设置 — 恢复默认背景
void MainWindow::onDefaultBackgroundRestored()
{
    AppImageCache &t_cache = AppImageCache::instance();
    t_cache.cl_background_pixmap_ = QPixmap();
    t_cache.cl_background_scaled_cache_ = QPixmap();
    scheduleBlurRebuild(); // 壁纸清空，模糊快照需重算
    update();
}

void MainWindow::restoreBackgroundFromModel()
{
    const auto t_selected = g_user_information.local.selectedWallpaper();
    if (!t_selected.first || !t_selected.first->contains(t_selected.second)) {
        onDefaultBackgroundRestored();
        return;
    }

    const auto t_entry = t_selected.first->value(t_selected.second);
    if (!t_entry || t_entry->path.isEmpty()) {
        onDefaultBackgroundRestored();
        return;
    }

    onBackgroundChanged(t_entry->path);
}

/// \brief 导入下载的方案文件到方案库，完成清理临时文件
void MainWindow::importDownloadedPlan(const QString &filePath)
{
    if (!ui->page_Sperker) return;

    int t_ret = ui->page_Sperker->readExportPlanIni(filePath,false);
    if (t_ret == 1 && g_shareCodeCopyHint) {
        g_shareCodeCopyHint->setText(tr("已下载到预设库"));
        g_shareCodeCopyHint->show();
        QTimer::singleShot(2000, g_shareCodeCopyHint, &QLabel::hide);
    }

    QFile::remove(filePath); ///< 清理临时文件
}

/// \brief 预缩放背景图缓存（仅在原图或窗口大小变更时调用）
void MainWindow::updateBackgroundCache()
{
    AppImageCache::instance().updateBackgroundCache(size());
    scheduleBlurRebuild(); // 尺寸/背景变化 → 模糊快照同步重算
}

//设置背景透明度
void MainWindow::setBackgroundTransparency(int PValue)
{
    //ui->centralwidget
    scheduleBlurRebuild(); // 透明度会烘进模糊合成图，需重算
    update();
}


void MainWindow::paintEvent(QPaintEvent *event)
{
    AppImageCache &t_cache = AppImageCache::instance();

    // 底图 + 壁纸覆盖层（在子控件之下）
    if (!t_cache.cl_default_background_cache_.isNull() || !t_cache.cl_background_scaled_cache_.isNull()) {
        QPainter t_painter(this);
        t_painter.setRenderHint(QPainter::Antialiasing, true);

        const qreal t_dpr = devicePixelRatioF();

        // 拉伸拖动中：缓存为旧尺寸，按 cover 语义（等比放大居中裁剪）铺满当前窗口，
        // 避免新暴露区域露白边（松手后 updateBackgroundCache 重建缓存恢复正常绘制）
        const bool t_resizing = bool(m_resizeEdges);
        const auto t_drawCover = [&](const QPixmap &t_pm) {
            const QSizeF t_cacheSz = t_pm.size() / t_dpr;
            const qreal t_scale = qMax(width() / t_cacheSz.width(), height() / t_cacheSz.height());
            const QSizeF t_drawSz = t_cacheSz * t_scale;
            t_painter.drawPixmap(QRectF((width() - t_drawSz.width()) / 2.0,
                                        (height() - t_drawSz.height()) / 2.0,
                                        t_drawSz.width(), t_drawSz.height()).toRect(),
                                 t_pm);
        };

        // 第一层：默认底部背景（始终绘制）
        if (!t_cache.cl_default_background_cache_.isNull()) {
            if (t_resizing) {
                t_drawCover(t_cache.cl_default_background_cache_);
            } else {
                const QSizeF t_logicalSz = t_cache.cl_default_background_cache_.size() / t_dpr;
                int t_x = qRound((width() - t_logicalSz.width()) / 2.0);
                int t_y = qRound((height() - t_logicalSz.height()) / 2.0);
                t_painter.drawPixmap(t_x, t_y, t_cache.cl_default_background_cache_);
            }
        }

        // 第二层：用户壁纸（覆盖层，支持透明度）
        if (!t_cache.cl_background_scaled_cache_.isNull()) {
            t_painter.setOpacity(g_user_information.local.panel_opacity_);
            if (t_resizing) {
                t_drawCover(t_cache.cl_background_scaled_cache_);
            } else {
                const QSizeF t_logicalSz = t_cache.cl_background_scaled_cache_.size() / t_dpr;
                int t_x = qRound((width() - t_logicalSz.width()) / 2.0);
                int t_y = qRound((height() - t_logicalSz.height()) / 2.0);
                t_painter.drawPixmap(t_x, t_y, t_cache.cl_background_scaled_cache_);
            }
        }
    }

    // 面板毛玻璃：在标题栏/导航栏区域铺模糊背景，面板自身半透明底色随后叠加成磨砂效果
    // 最低模糊半径恒 ≥1（qMax 钳制），滑块 0 也绘制最小模糊，故不再按 radius_>0 门禁
    if (!t_cache.cl_background_blurred_cache_.isNull()) {
        QPainter t_blurPainter(this);
        QRegion t_clip;
        for (QWidget *t_panel : {ui->widget_title, ui->widget_tree}) {
            if (t_panel)
                t_clip += QRect(t_panel->mapTo(this, QPoint(0, 0)), t_panel->size());
        }
        t_blurPainter.setClipRegion(t_clip);
        // 模糊缓存为降采样小图，拉伸铺满窗口后由裁剪区限定到面板区
        t_blurPainter.setRenderHint(QPainter::SmoothPixmapTransform);  // 平滑放大，避免 4× 降采样颗粒
        t_blurPainter.drawPixmap(QRect(0, 0, width(), height()), t_cache.cl_background_blurred_cache_);
    }

    // 再绘制子控件（在上层）
    QMainWindow::paintEvent(event);
}
