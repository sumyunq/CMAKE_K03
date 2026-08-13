#include "modules/UserSetting/UserSettingSubModule/SystemSettings/system_settings_main_page.h"
#include "ui_system_settings_main_page.h"

#include <QDebug>
#include <QDir>
#include <QStandardPaths>

#include "APOThread/ApoManager.h" ///< ApoManager::instance()
#include "Popup/FactoryReset.h"         ///< FactoryReset
#include "LoadApoDLL.h"           ///<  m_sharedMemory
#include "LoadLib.h"              ///< globalSettings, g_user_system_settings_config_info, msgBox, m

/// \brief 构造函数
SystemSettingsMainPage::SystemSettingsMainPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SystemSettingsMainPage)
{
    ui->setupUi(this);
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽
}

SystemSettingsMainPage::~SystemSettingsMainPage()
{
    saveToDisk();
    delete ui;
}

void SystemSettingsMainPage::saveToDisk()
{
    if (globalSettings) {
        globalSettings->setValue("AutoStart", g_user_system_settings_config_info.is_auto_start.load());
        globalSettings->setValue("AutoStartShowWidget",
                                 g_user_system_settings_config_info.is_auto_start_show_widget.load());
        globalSettings->setValue("bExitDirectly",
                                 g_user_system_settings_config_info.is_exit_directly.load());
        globalSettings->setValue("bRemember",
                                 g_user_system_settings_config_info.is_remember_choice.load());
        globalSettings->sync();
    }
}

/// \brief 刷新翻译文本
void SystemSettingsMainPage::LanguageSet()
{
    ui->retranslateUi(this);
}

void SystemSettingsMainPage::UpdateSystemSettingsUIInformation()
{
    syncAutoStartState();

    // 关闭主界面 card
    {
        bool t_exit = g_user_system_settings_config_info.is_exit_directly.load();
        ui->radioButton_exit->setChecked(t_exit);
        ui->radioButton_minimize->setChecked(!t_exit);

        // 关闭时询问 = 不记住选择
        ui->checkBox_close_ask->setChecked(
            !g_user_system_settings_config_info.is_remember_choice.load());
    }
}

/// \brief 初始化UI的默认信息
void SystemSettingsMainPage::InitUIInformation()
{
    {
        // 开机自启动 card
        ui->pBt_SelfStart->setCursor(Qt::PointingHandCursor);
        ui->pBt_SelfStart->setCheckable(true);
        g_user_system_settings_config_info.is_auto_start.store(
            globalSettings->value("AutoStart", false).toBool());
        ui->pBt_SelfStart->setChecked(g_user_system_settings_config_info.is_auto_start.load());
        ui->pBt_SelfStart->setObjectName("SystemSettings_pBt_SelfStart");
        ui->pBt_SelfStart->setStyleSheet(R"(
            #SystemSettings_pBt_SelfStart {
                border: none;
                border-radius: 2px;
                background-color: transparent;
            }
        )");
        ui->widget_power_on_setting->setObjectName("SystemSettings_widget_power_on_setting");
        ui->widget_power_on_setting->setCornerRadius(12);
        ui->widget_power_on_setting->setStyleSheet(R"(
            #SystemSettings_widget_power_on_setting {
                border-radius: 12px;
                background-color: rgba(81, 96, 122, 0.2);
            }
        )");
        ui->label_title_1->setObjectName("SystemSettings_label_title_1");
        ui->label_title_1->setStyleSheet(R"(
            QLabel#SystemSettings_label_title_1 {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 14px;
                color: #A1A8B3;
                background-color: transparent;
            }
)");
        ui->label_auto_start_in_the_background->setObjectName("SystemSettings_label_auto_start_in_the_background");
        ui->label_auto_start_in_the_background->setEnabled(false);
        ui->label_auto_start_in_the_background->setStyleSheet(R"(
            QLabel#SystemSettings_label_auto_start_in_the_background {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 14px;
                color: #A1A8B3;
                background-color: transparent;
            }
            QLabel#SystemSettings_label_auto_start_in_the_background:disabled {
                color: #616871;
            }
)");
    }
    {
        // 自启动 功能开关
        ui->pBt_SelfStart_showWidget->setCursor(Qt::PointingHandCursor);
        ui->pBt_SelfStart_showWidget->setCheckable(true);
        g_user_system_settings_config_info.is_auto_start_show_widget.store(
            globalSettings->value("AutoStartShowWidget", false).toBool());
        ui->pBt_SelfStart_showWidget->setChecked(
            g_user_system_settings_config_info.is_auto_start_show_widget.load());
        // 初始状态：pBt_SelfStart 默认关闭，子控件不可用
        ui->pBt_SelfStart_showWidget->setEnabled(false);
        ui->label_auto_start_in_the_background->setEnabled(false);
        ui->pBt_SelfStart_showWidget->setObjectName("SystemSettings_pBt_SelfStart_showWidget");
    }
    {
        // 关闭主界面 card
        bool t_exit_directly = globalSettings->value("bExitDirectly", true).toBool();
        ui->radioButton_minimize->setChecked(!t_exit_directly);
        ui->radioButton_exit->setChecked(t_exit_directly);
        ui->radioButton_minimize->setObjectName("SystemSettings_radioButton_minimize");
        ui->radioButton_exit->setObjectName("SystemSettings_radioButton_exit");
        ui->radioButton_minimize->setStyleSheet(R"(
            QRadioButton#SystemSettings_radioButton_minimize {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #A1A8B3;
                background-color: transparent;
                spacing: 10px;
            }
            QRadioButton#SystemSettings_radioButton_minimize::indicator {
                width: 13px;
                height: 13px;
                image: url(:/Skin/Images/more/system_settings/round_radio_unchecked_13_13_2x_normal_darkBlue.svg);
            }
            QRadioButton#SystemSettings_radioButton_minimize::indicator:hover {
                image: url(:/Skin/Images/more/system_settings/round_radio_unchecked_13_13_2x_hover_darkBlue.svg);
            }
            QRadioButton#SystemSettings_radioButton_minimize::indicator:checked {
                image: url(:/Skin/Images/more/system_settings/round_radio_checked_13_13_2x_normal_darkBlue.svg);
            }
            QRadioButton#SystemSettings_radioButton_minimize::indicator:checked:hover {
                image: url(:/Skin/Images/more/system_settings/round_radio_checked_13_13_2x_hover_darkBlue.svg);
            }
        )");
        ui->radioButton_exit->setStyleSheet(R"(
            QRadioButton#SystemSettings_radioButton_exit {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #A1A8B3;
                background-color: transparent;
                spacing: 10px;
            }
            QRadioButton#SystemSettings_radioButton_exit::indicator {
                width: 13px;
                height: 13px;
                image: url(:/Skin/Images/more/system_settings/round_radio_unchecked_13_13_2x_normal_darkBlue.svg);
            }
            QRadioButton#SystemSettings_radioButton_exit::indicator:hover {
                image: url(:/Skin/Images/more/system_settings/round_radio_unchecked_13_13_2x_hover_darkBlue.svg);
            }
            QRadioButton#SystemSettings_radioButton_exit::indicator:checked {
                image: url(:/Skin/Images/more/system_settings/round_radio_checked_13_13_2x_normal_darkBlue.svg);
            }
            QRadioButton#SystemSettings_radioButton_exit::indicator:checked:hover {
                image: url(:/Skin/Images/more/system_settings/round_radio_checked_13_13_2x_hover_darkBlue.svg);
            }
        )");
        ui->checkBox_close_ask->setObjectName("SystemSettings_checkBox_close_ask");
        g_user_system_settings_config_info.is_remember_choice.store(
            globalSettings->value("bRemember", false).toBool());
        ui->checkBox_close_ask->setChecked(
            !g_user_system_settings_config_info.is_remember_choice.load());
        ui->checkBox_close_ask->setCursor(Qt::PointingHandCursor);
        ui->checkBox_close_ask->setStyleSheet(R"(
            QCheckBox#SystemSettings_checkBox_close_ask {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #616975;
                background-color: transparent;
                spacing: 10px;
            }
            QCheckBox#SystemSettings_checkBox_close_ask:checked {
                color: #A1A8B3;
            }
            QCheckBox#SystemSettings_checkBox_close_ask::indicator {
                width: 13px;
                height: 13px;
                color: #616975;
                image: url(:/Skin/Images/more/system_settings/square_radio_unchecked_12_12_2x_normal_darkBlue.png);
            }
            QCheckBox#SystemSettings_checkBox_close_ask::indicator:hover {
                color: #616975;
                image: url(:/Skin/Images/more/system_settings/square_radio_unchecked_12_12_2x_hover_darkBlue.png);
            }
            QCheckBox#SystemSettings_checkBox_close_ask::indicator:checked {
                color: #A1A8B3;
                image: url(:/Skin/Images/more/system_settings/square_radio_checked_12_12_2x_normal_darkBlue.png);
            }
            QCheckBox#SystemSettings_checkBox_close_ask::indicator:checked:hover {
                color: #A1A8B3;
                image: url(:/Skin/Images/more/system_settings/square_radio_checked_12_12_2x_hover_darkBlue.png);
            }
        )");

        ui->widget_close_main_interface_setting->setObjectName(
            "SystemSettings_widget_close_main_interface_setting");
        ui->widget_close_main_interface_setting->setCornerRadius(12);
        ui->widget_close_main_interface_setting->setStyleSheet(R"(
            #SystemSettings_widget_close_main_interface_setting {
                border-radius: 12px;
                background-color: rgba(81, 96, 122, 0.2);
            }
        )");
        ui->label_title_2->setObjectName("SystemSettings_label_title_2");
        ui->label_title_2->setStyleSheet(R"(
            QLabel#SystemSettings_label_title_2 {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 14px;
                color: #A1A8B3;
                background-color: transparent;
            }
)");
    }
    {
        // 恢复出厂 card
        ui->pBt_reset->setObjectName("SystemSettings_pBt_reset");
        ui->pBt_reset->setStyleSheet(R"(
            #SystemSettings_pBt_reset {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 14px;
                color: #FFFFFF;
                border-image: url(:/Skin/Images/Popup/confirm-no.png);
            }
            #SystemSettings_pBt_reset:hover {
               border-image: url(:/Skin/Images/Popup/confirm-ho.png);
            }
)");
        ui->widget_restore_factory_setting->setObjectName(
            "SystemSettings_widget_restore_factory_setting");
        ui->widget_restore_factory_setting->setCornerRadius(12);
        ui->widget_restore_factory_setting->setStyleSheet(R"(
            #SystemSettings_widget_restore_factory_setting {
                border-radius: 12px;
                background-color: rgba(81, 96, 122, 0.2);
            }
        )");
        ui->label_title_3->setObjectName("SystemSettings_label_title_3");
        ui->label_title_3->setStyleSheet(R"(
            QLabel#SystemSettings_label_title_3 {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 14px;
                color: #A1A8B3;
                background-color: transparent;
            }
)");

        ui->label_text->setObjectName("SystemSettings_label_text");
        ui->label_text->setStyleSheet(R"(
            QLabel#SystemSettings_label_text {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #616871;
                background-color: transparent;
            }
)");
    }
}

/// \brief 初始化内部成员
void SystemSettingsMainPage::InitMember()
{
    // 任务名称
    cl_task_name_ = L"XIBERIA X HUB StartUp";
    cl_app_path_ = QDir::toNativeSeparators(QCoreApplication::applicationFilePath()).toStdWString();
    // 当前登录用户名称
    cl_user_name_ = QProcessEnvironment::systemEnvironment().value("USERNAME").toStdWString();

    HRESULT t_hr = InitializeTaskService(&clp_task_service_);
    if (FAILED(t_hr)) {
        ApoManager::instance()->requestlogWithTime(
            QString("SystemSettings: 任务计划服务初始化失败 0x%1")
                .arg(QString::number(t_hr, 16).toUpper()));
        return;
    }
    syncAutoStartState();
}

/// \brief 连接默认的信号槽
void SystemSettingsMainPage::InitConnect()
{
    // 开机自启动 功能开关 — 控制 pBt_SelfStart 和 label 的可用状态
    connect(ui->pBt_SelfStart_showWidget,
            &QPushButton::toggled,
            this,
            &SystemSettingsMainPage::onAutoStartShowToggled,
            Qt::UniqueConnection);

    // 开机自启动 开关切换
    connect(ui->pBt_SelfStart,
            &QPushButton::toggled,
            this,
            &SystemSettingsMainPage::onAutoStartToggled,
            Qt::UniqueConnection);

    // 关闭主界面 → 最小化到托盘
    connect(ui->radioButton_minimize,
            &QRadioButton::toggled,
            this,
            &SystemSettingsMainPage::onExitModeMinimizeToggled,
            Qt::UniqueConnection);

    // 关闭主界面 → 退出应用
    connect(ui->radioButton_exit,
            &QRadioButton::toggled,
            this,
            &SystemSettingsMainPage::onExitModeExitToggled,
            Qt::UniqueConnection);

    // 关闭时询问
    connect(ui->checkBox_close_ask,
            &QCheckBox::toggled,
            this,
            [this](bool t_checked) {
                // checked = "关闭时询问" = 不记住选择
                g_user_system_settings_config_info.is_remember_choice.store(!t_checked);
                globalSettings->setValue("bRemember", !t_checked);
                globalSettings->sync();
            },
            Qt::UniqueConnection);

    // 恢复出厂设置 按钮点击
    connect(ui->pBt_reset,
            &QPushButton::clicked,
            this,
            &SystemSettingsMainPage::onFactoryResetClicked,
            Qt::UniqueConnection);
}

/// \brief 开机自启动开关
void SystemSettingsMainPage::onAutoStartToggled(bool checked)
{
    if (!clp_task_service_) {
        // 服务未初始化：回弹按钮并提示
        revertButtonCheck(checked);
        return;
    }

    if (checked) {
        // 开启自启动
        ApoManager::instance()->requestlogWithTime("SystemSettings: 设置开机自启动");
        try {
            if (!IsTaskExists(clp_task_service_, cl_task_name_)) {
                ApoManager::instance()->requestlogWithTime("SystemSettings: 开始创建计划任务");
                HRESULT t_hr = CreateScheduledTask(clp_task_service_,
                                                   cl_task_name_,
                                                   cl_app_path_,
                                                   cl_user_name_);
                if (FAILED(t_hr)) {
                    QString t_err_msg;
                    switch (t_hr) {
                    case E_ACCESSDENIED:
                        t_err_msg = tr("权限不足。请以管理员身份运行本程序。");
                        break;
                    case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
                        t_err_msg = tr("程序路径不存在：\n%1")
                                        .arg(QString::fromStdWString(cl_app_path_));
                        break;
                    case HRESULT_FROM_WIN32(ERROR_BAD_USERNAME):
                        t_err_msg = tr("用户名无效：\n%1")
                                        .arg(QString::fromStdWString(cl_user_name_));
                        break;
                    case HRESULT_FROM_WIN32(ERROR_SERVICE_NOT_ACTIVE):
                        t_err_msg = tr("任务计划服务未运行，请启动该服务后重试。");
                        break;
                    default:
                        t_err_msg = tr("创建计划任务失败。\n错误代码：0x%1")
                                        .arg(QString::number(t_hr, 16).toUpper());
                    }
                    ApoManager::instance()->requestlogWithTime(
                        QString("SystemSettings: 创建失败：%1").arg(t_err_msg));
                    msgBox.critical(this, tr("开机自启动失败"), t_err_msg);
                    revertButtonCheck(checked);
                    return;
                }
                ApoManager::instance()->requestlogWithTime(
                    "SystemSettings: 计划任务创建成功");
            } else {
                ApoManager::instance()->requestlogWithTime("SystemSettings: 任务已存在");
            }
            g_user_system_settings_config_info.is_auto_start.store(true);
            saveToDisk();
        } catch (const std::exception &e) {
            msgBox.critical(this, tr("异常"), tr("发生异常：%1").arg(e.what()));
            revertButtonCheck(checked);
            return;
        } catch (...) {
            msgBox.critical(this, tr("异常"), tr("发生未知异常。"));
            revertButtonCheck(checked);
            return;
        }
    } else {
        // 关闭自启动
        ApoManager::instance()->requestlogWithTime("SystemSettings: 取消开机自启动");

        try {
            HRESULT t_hr = DeleteScheduledTask(clp_task_service_, cl_task_name_);
            if (FAILED(t_hr)) {
                QString t_err_msg = tr("删除计划任务失败。\n错误代码：0x%1")
                                        .arg(QString::number(t_hr, 16).toUpper());
                ApoManager::instance()->requestlogWithTime(
                    QString("SystemSettings: 删除失败：%1").arg(t_err_msg));
                msgBox.critical(this, tr("取消自启动失败"), t_err_msg);
                revertButtonCheck(checked);
                return;
            }
            ApoManager::instance()->requestlogWithTime(
                "SystemSettings: 计划任务删除成功");
            g_user_system_settings_config_info.is_auto_start.store(false);
            saveToDisk();
        } catch (...) {
            msgBox.critical(this, tr("异常"), tr("删除任务时发生异常。"));
            revertButtonCheck(checked);
            return;
        }
    }


    //  是否需要关闭 pBt_SelfStart_hideWidget
    if (!checked) {
        ui->pBt_SelfStart_showWidget->setChecked(false);
        g_user_system_settings_config_info.is_auto_start_show_widget.store(false);
    }
    ui->pBt_SelfStart_showWidget->setEnabled(checked);
    ui->label_auto_start_in_the_background->setEnabled(checked);

}

/// \brief 同步自启动任务状态到全局和 UI
void SystemSettingsMainPage::syncAutoStartState()
{
    if (!clp_task_service_)
        return;

    //判断开机自启任务是否存在
    bool t_auto_start = IsTaskExists(clp_task_service_, cl_task_name_);
    //判断任务中的exe路径是否正确
    if (t_auto_start && !IsTaskPathMatch(clp_task_service_, cl_task_name_, cl_app_path_)) {
        // 任务存在但路径错误,重新创建任务
        t_auto_start = UpdateTaskPath(clp_task_service_, cl_task_name_, cl_app_path_);
        if (!t_auto_start) {
            ApoManager::instance()->requestlogWithTime(
                "SystemSettings: 自启动任务路径自动修复失败");
        }
    }
    g_user_system_settings_config_info.is_auto_start.store(t_auto_start);

    // 同步 UI
    ui->pBt_SelfStart->blockSignals(true);
    ui->pBt_SelfStart->setChecked(t_auto_start);
    ui->pBt_SelfStart->blockSignals(false);
    ui->label_auto_start_in_the_background->setEnabled(t_auto_start);
    ui->pBt_SelfStart_showWidget->setEnabled(t_auto_start);
    if (t_auto_start) {
        ui->pBt_SelfStart_showWidget->blockSignals(true);
        ui->pBt_SelfStart_showWidget->setChecked(
            g_user_system_settings_config_info.is_auto_start_show_widget.load());
        ui->pBt_SelfStart_showWidget->blockSignals(false);
    } else {
        ui->pBt_SelfStart_showWidget->blockSignals(true);
        ui->pBt_SelfStart_showWidget->setChecked(false);
        ui->pBt_SelfStart_showWidget->blockSignals(false);
    }
}

/// \brief 开机自启动 后台运行开关
void SystemSettingsMainPage::onAutoStartShowToggled(bool checked)
{
    g_user_system_settings_config_info.is_auto_start_show_widget.store(checked);
    saveToDisk();
}

/// \brief 回弹按钮状态（阻止信号重入）
void SystemSettingsMainPage::revertButtonCheck(bool currentState)
{
    ui->pBt_SelfStart->blockSignals(true);
    ui->pBt_SelfStart->setChecked(!currentState);
    ui->pBt_SelfStart->blockSignals(false);
    // 恢复全局状态
    g_user_system_settings_config_info.is_auto_start.store(!currentState);
    // 同步子控件：回弹到关时禁用 hideWidget
    if (currentState) {
        ui->label_auto_start_in_the_background->setEnabled(false);
        ui->pBt_SelfStart_showWidget->setEnabled(false);
        ui->pBt_SelfStart_showWidget->blockSignals(true);
        ui->pBt_SelfStart_showWidget->setChecked(false);
        ui->pBt_SelfStart_showWidget->blockSignals(false);
    }
}

/// \brief 关闭主界面 → 最小化到系统托盘
void SystemSettingsMainPage::onExitModeMinimizeToggled(bool checked)
{
    if (checked) {
        g_user_system_settings_config_info.is_exit_directly.store(false);
        saveToDisk();
    }
}

/// \brief 关闭主界面 → 退出应用
void SystemSettingsMainPage::onExitModeExitToggled(bool checked)
{
    if (checked) {
        g_user_system_settings_config_info.is_exit_directly.store(true);
        saveToDisk();
    }
}

/// \brief 恢复出厂设置按钮
void SystemSettingsMainPage::onFactoryResetClicked()
{


    FactoryReset *fr = new FactoryReset(this);
    fr->setModal(true);
    int result = fr->exec();
    if (result == QDialog::Accepted) {
        delete globalSettings;
        globalSettings = nullptr;

        // QString filePath_del = QApplication::applicationDirPath()+QString("/ProgramData");

        QString filePath_del = QFileInfo(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
                                   .absolutePath()
                               + "/XIBERIA X HUB/ProgramData";

        QDir dir(filePath_del);
        if (dir.exists()) {
            if (dir.removeRecursively()) {
                qDebug() << "文件夹删除成功：" << filePath_del;
            } else {
                qDebug() << "文件夹删除失败（可能权限不足或文件被占用）：" << filePath_del;
            }
        }
        //信号-跳转到登录界面（重新读取设置文件）
        // emit LoginPage();

        // 启动新实例
        if (m_sharedMemory) {
            if (m_sharedMemory->isAttached()) {
                m_sharedMemory->detach(); // 断开当前进程的连接
            }
            delete m_sharedMemory; // 删除对象，内部也会再次 detach（但已无影响）
            m_sharedMemory = nullptr;
            emit ApoManager::instance()->requestlogWithTime("delete m_sharedMemory");
        }

        emit ApoManager::instance()->requestlogWithTime("active new exe");
        //QProcess::startDetached(QApplication::applicationFilePath(), QStringList());
        bool success = QProcess::startDetached(QApplication::applicationFilePath(), QStringList());
        if (success) {
            emit ApoManager::instance()->requestlogWithTime("新进程启动请求已发送");
        } else {
            emit ApoManager::instance()->requestlogWithTime("新进程启动失败");
            ; // 只能得到启动时的错误，如找不到文件
        }
        // 退出当前实例
        QApplication::quit();
        emit ApoManager::instance()->requestlogWithTime("quit new exe");
    }
    fr->deleteLater();

}

/// @brief 初始化任务计划服务 COM 接口
HRESULT SystemSettingsMainPage::InitializeTaskService(ITaskService **ppService)
{
    HRESULT t_hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(t_hr) && t_hr != RPC_E_CHANGED_MODE) {
        msgBox.critical(nullptr,
                        tr("初始化失败"),
                        tr("COM库初始化失败。错误码：0x%1").arg(t_hr, 0, 16));
        return t_hr;
    }

    t_hr = CoInitializeSecurity(NULL,
                                -1,
                                NULL,
                                NULL,
                                RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                RPC_C_IMP_LEVEL_IMPERSONATE,
                                NULL,
                                0,
                                NULL);
    if (FAILED(t_hr) && t_hr != RPC_E_TOO_LATE) {
        msgBox.critical(nullptr,
                        tr("初始化失败"),
                        tr("安全设置失败。错误码：0x%1").arg(t_hr, 0, 16));
        CoUninitialize();
        return t_hr;
    }

    t_hr = CoCreateInstance(CLSID_TaskScheduler,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_ITaskService,
                            (void **) ppService);
    if (FAILED(t_hr)) {
        msgBox.critical(nullptr,
                        tr("服务错误"),
                        tr("无法创建任务计划服务实例。\n请确认Task Scheduler服务已启动。"));
        CoUninitialize();
        return t_hr;
    }

    t_hr = (*ppService)->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    if (FAILED(t_hr)) {
        msgBox.critical(nullptr,
                        tr("连接失败"),
                        tr("无法连接到任务计划服务。错误码：0x%1").arg(t_hr, 0, 16));
        (*ppService)->Release();
        *ppService = nullptr;
        CoUninitialize();
        return t_hr;
    }

    return S_OK;
}

/// @brief 创建开机自启动计划任务
HRESULT SystemSettingsMainPage::CreateScheduledTask(ITaskService *pService,
                                                    const std::wstring &taskName,
                                                    const std::wstring &appPath,
                                                    const std::wstring &userName)
{
    if (!pService)
        return E_POINTER;
    if (taskName.empty() || appPath.empty())
        return E_INVALIDARG;

    QString t_domain = QProcessEnvironment::systemEnvironment().value("USERDOMAIN");
    std::wstring t_full_author;
    if (!t_domain.isEmpty() && t_domain != ".") {
        t_full_author = t_domain.toStdWString() + L"\\" + userName;
    } else {
        t_full_author = userName;
    }

    HRESULT t_hr = S_OK;
    ITaskFolder *t_task_folder = nullptr;
    ITaskDefinition *t_task_def = nullptr;
    IRegisteredTask *t_registered_task = nullptr;

    try {
        t_hr = pService->GetFolder(_bstr_t(L"\\"), &t_task_folder);
        if (FAILED(t_hr))
            _com_issue_error(t_hr);

        // 删除已存在的同名任务（避免冲突）
        t_hr = t_task_folder->DeleteTask(_bstr_t(taskName.c_str()), 0);

        // 创建新任务定义
        t_hr = pService->NewTask(0, &t_task_def);
        if (FAILED(t_hr))
            _com_issue_error(t_hr);

        // 设置任务作者信息
        IRegistrationInfo *t_reg_info = nullptr;
        t_hr = t_task_def->get_RegistrationInfo(&t_reg_info);
        if (SUCCEEDED(t_hr)) {
            t_reg_info->put_Author(_bstr_t(t_full_author.c_str()));
            t_reg_info->Release();
        }

        // 设置最高权限运行
        IPrincipal *t_principal = nullptr;
        t_hr = t_task_def->get_Principal(&t_principal);
        if (SUCCEEDED(t_hr)) {
            t_hr = t_principal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
            t_principal->put_UserId(_bstr_t(t_full_author.c_str()));
            t_principal->put_LogonType(TASK_LOGON_INTERACTIVE_TOKEN);
            t_principal->Release();
        }

        // 电源选项：允许在使用电池时运行
        ITaskSettings *t_settings = nullptr;
        t_hr = t_task_def->get_Settings(&t_settings);
        if (SUCCEEDED(t_hr)) {
            t_settings->put_DisallowStartIfOnBatteries(VARIANT_FALSE);
            t_settings->put_StopIfGoingOnBatteries(VARIANT_FALSE);
            t_settings->Release();
        }

        // 触发器：当前用户登录时
        ITriggerCollection *t_trigger_collection = nullptr;
        t_hr = t_task_def->get_Triggers(&t_trigger_collection);
        if (SUCCEEDED(t_hr)) {
            ITrigger *t_trigger = nullptr;
            t_hr = t_trigger_collection->Create(TASK_TRIGGER_LOGON, &t_trigger);
            if (SUCCEEDED(t_hr)) {
                ILogonTrigger *t_logon_trigger = nullptr;
                t_hr = t_trigger->QueryInterface(IID_ILogonTrigger, (void **) &t_logon_trigger);
                if (SUCCEEDED(t_hr)) {
                    BSTR t_bstr_user = SysAllocString(t_full_author.c_str());
                    if (t_bstr_user) {
                        t_logon_trigger->put_UserId(t_bstr_user);
                        SysFreeString(t_bstr_user);
                    }
                    t_logon_trigger->Release();
                }
                t_trigger->Release();
            }
            t_trigger_collection->Release();
        }

        // 操作：启动应用程序
        IActionCollection *t_action_collection = nullptr;
        t_hr = t_task_def->get_Actions(&t_action_collection);
        if (SUCCEEDED(t_hr)) {
            IAction *t_action = nullptr;
            t_hr = t_action_collection->Create(TASK_ACTION_EXEC, &t_action);
            if (SUCCEEDED(t_hr)) {
                IExecAction *t_exec_action = nullptr;
                t_hr = t_action->QueryInterface(IID_IExecAction, (void **) &t_exec_action);
                if (SUCCEEDED(t_hr)) {
                    t_exec_action->put_Path(_bstr_t(appPath.c_str()));
                    t_exec_action->put_Arguments(_bstr_t(L"/autostart"));
                    t_exec_action->Release();
                }
                t_action->Release();
            }
            t_action_collection->Release();
        }

        // 注册任务
        t_hr = t_task_folder->RegisterTaskDefinition(_bstr_t(taskName.c_str()),
                                                     t_task_def,
                                                     TASK_CREATE_OR_UPDATE,
                                                     _variant_t(t_full_author.c_str()),
                                                     _variant_t(),
                                                     TASK_LOGON_INTERACTIVE_TOKEN,
                                                     _variant_t(),
                                                     &t_registered_task);

        if (FAILED(t_hr))
            _com_issue_error(t_hr);

    } catch (const _com_error &e) {
        t_hr = e.Error();
        qWarning() << "COM error in CreateScheduledTask:" << t_hr << e.ErrorMessage();
    } catch (...) {
        t_hr = E_FAIL;
        qWarning() << "Unknown exception in CreateScheduledTask";
    }

    if (t_registered_task)
        t_registered_task->Release();
    if (t_task_def)
        t_task_def->Release();
    if (t_task_folder)
        t_task_folder->Release();

    return t_hr;
}

/// @brief 删除开机自启动计划任务
HRESULT SystemSettingsMainPage::DeleteScheduledTask(ITaskService *pService,
                                                    const std::wstring &taskName)
{
    if (!pService)
        return E_POINTER;
    if (taskName.empty())
        return E_INVALIDARG;

    HRESULT t_hr = S_OK;
    ITaskFolder *t_task_folder = nullptr;

    try {
        t_hr = pService->GetFolder(_bstr_t(L"\\"), &t_task_folder);
        if (FAILED(t_hr))
            _com_issue_error(t_hr);

        t_hr = t_task_folder->DeleteTask(_bstr_t(taskName.c_str()), 0);
        if (t_hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
            qDebug() << "Task not found, treat as successfully deleted.";
            t_hr = S_OK;
        } else if (FAILED(t_hr)) {
            _com_issue_error(t_hr);
        }
    } catch (const _com_error &e) {
        t_hr = e.Error();
        qWarning() << "COM error in DeleteScheduledTask:" << t_hr << e.ErrorMessage();
    } catch (...) {
        t_hr = E_FAIL;
        qWarning() << "Unknown exception in DeleteScheduledTask";
    }

    if (t_task_folder)
        t_task_folder->Release();
    return t_hr;
}

/// @brief 立即执行计划任务
HRESULT SystemSettingsMainPage::RunScheduledTask(ITaskService *pService,
                                                 const std::wstring &taskName)
{
    if (!pService)
        return E_POINTER;
    if (taskName.empty())
        return E_INVALIDARG;

    HRESULT t_hr = S_OK;
    ITaskFolder *t_task_folder = nullptr;
    IRegisteredTask *t_registered_task = nullptr;
    IRunningTask *t_running_task = nullptr;

    try {
        t_hr = pService->GetFolder(_bstr_t(L"\\"), &t_task_folder);
        if (FAILED(t_hr))
            _com_issue_error(t_hr);

        t_hr = t_task_folder->GetTask(_bstr_t(taskName.c_str()), &t_registered_task);
        if (FAILED(t_hr))
            _com_issue_error(t_hr);

        t_hr = t_registered_task->Run(_variant_t(), &t_running_task);
        if (FAILED(t_hr))
            _com_issue_error(t_hr);
    } catch (const _com_error &e) {
        t_hr = e.Error();
        qWarning() << "COM error in RunScheduledTask:" << t_hr << e.ErrorMessage();
    } catch (...) {
        t_hr = E_FAIL;
        qWarning() << "Unknown exception in RunScheduledTask";
    }

    if (t_running_task)
        t_running_task->Release();
    if (t_registered_task)
        t_registered_task->Release();
    if (t_task_folder)
        t_task_folder->Release();
    return t_hr;
}

/// @brief 检测任务是否存在
bool SystemSettingsMainPage::IsTaskExists(ITaskService *pService, const std::wstring &taskName)
{
    if (!pService || taskName.empty())
        return false;

    HRESULT t_hr = S_OK;
    ITaskFolder *t_task_folder = nullptr;
    IRegisteredTask *t_registered_task = nullptr;
    bool t_exists = false;

    try {
        t_hr = pService->GetFolder(_bstr_t(L"\\"), &t_task_folder);
        if (FAILED(t_hr))
            _com_issue_error(t_hr);

        t_hr = t_task_folder->GetTask(_bstr_t(taskName.c_str()), &t_registered_task);
        if (SUCCEEDED(t_hr)) {
            t_exists = true;
        }
    } catch (const _com_error &e) {
        qWarning() << "COM error in IsTaskExists:" << e.Error() << e.ErrorMessage();
    } catch (...) {
        qWarning() << "Unknown exception in IsTaskExists";
    }

    if (t_registered_task)
        t_registered_task->Release();
    if (t_task_folder)
        t_task_folder->Release();
    return t_exists;
}

/// @brief 规范化路径：去除首尾空格/引号，统一反斜杠
std::wstring SystemSettingsMainPage::NormalizePath(const std::wstring &path)
{
    std::wstring t_path = path;
    size_t t_start = t_path.find_first_not_of(L" \t\"");
    if (t_start == std::wstring::npos)
        return L"";
    size_t t_end = t_path.find_last_not_of(L" \t\"");
    t_path = t_path.substr(t_start, t_end - t_start + 1);

    for (auto &ch : t_path) {
        if (ch == L'/')
            ch = L'\\';
    }
    return t_path;
}

/// @brief 判断任务存在且启动路径与期望路径匹配
bool SystemSettingsMainPage::IsTaskPathMatch(ITaskService *pService,
                                             const std::wstring &taskName,
                                             const std::wstring &expectedPath)
{
    if (!pService || taskName.empty() || expectedPath.empty())
        return false;

    ITaskFolder *t_task_folder = nullptr;
    IRegisteredTask *t_registered_task = nullptr;
    ITaskDefinition *t_task_def = nullptr;
    IActionCollection *t_actions = nullptr;
    bool t_match = false;

    try {
        HRESULT t_hr = pService->GetFolder(_bstr_t(L"\\"), &t_task_folder);
        if (FAILED(t_hr))
            _com_issue_error(t_hr);

        t_hr = t_task_folder->GetTask(_bstr_t(taskName.c_str()), &t_registered_task);
        if (FAILED(t_hr))
            return false;

        t_hr = t_registered_task->get_Definition(&t_task_def);
        if (FAILED(t_hr))
            _com_issue_error(t_hr);

        t_hr = t_task_def->get_Actions(&t_actions);
        if (FAILED(t_hr))
            _com_issue_error(t_hr);

        long t_count = 0;
        t_actions->get_Count(&t_count);
        std::wstring t_norm_expected = NormalizePath(expectedPath);

        for (long i = 1; i <= t_count; ++i) {
            IAction *t_action = nullptr;
            t_hr = t_actions->get_Item(i, &t_action);
            if (FAILED(t_hr))
                continue;

            _TASK_ACTION_TYPE t_type;
            t_action->get_Type(&t_type);
            if (t_type == TASK_ACTION_EXEC) {
                IExecAction *t_exec = nullptr;
                t_hr = t_action->QueryInterface(IID_IExecAction, (void **) &t_exec);
                if (SUCCEEDED(t_hr)) {
                    BSTR t_bstr_path = nullptr;
                    t_exec->get_Path(&t_bstr_path);
                    if (t_bstr_path) {
                        std::wstring t_task_path(t_bstr_path, SysStringLen(t_bstr_path));
                        SysFreeString(t_bstr_path);
                        if (_wcsicmp(NormalizePath(t_task_path).c_str(), t_norm_expected.c_str())
                            == 0) {
                            t_match = true;
                        }
                    }
                    t_exec->Release();
                }
            }
            t_action->Release();
            if (t_match)
                break;
        }
    } catch (const _com_error &e) {
        qWarning() << "COM error in IsTaskPathMatch:" << e.Error() << e.ErrorMessage();
    } catch (...) {
        qWarning() << "Unknown exception in IsTaskPathMatch";
    }

    if (t_actions)
        t_actions->Release();
    if (t_task_def)
        t_task_def->Release();
    if (t_registered_task)
        t_registered_task->Release();
    if (t_task_folder)
        t_task_folder->Release();
    return t_match;
}

/// @brief 更新任务中的启动路径
bool SystemSettingsMainPage::UpdateTaskPath(ITaskService *pService,
                                            const std::wstring &taskName,
                                            const std::wstring &newPath)
{
    if (!pService || taskName.empty() || newPath.empty())
        return false;

    QString t_user_name_str = QProcessEnvironment::systemEnvironment().value("USERNAME");
    std::wstring t_user_name = t_user_name_str.toStdWString();

    HRESULT t_hr = CreateScheduledTask(pService, taskName, newPath, t_user_name);
    return SUCCEEDED(t_hr);
}
