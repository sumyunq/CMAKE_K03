#include "modules/UserSetting/user_setting_main_page.h"
#include "ui_user_setting_main_page.h"

#include "APOThread/ApoManager.h" ///< ApoManager::instance()
#include "Popup/FactoryReset.h"         ///< FactoryReset
#include "LoadApoDLL.h"           ///< m_sharedMemory
#include "LoadLib.h"              ///< access_token

QString SoftWareVer = "";
char DongleVer[31] = {};  // 30字节 + \0
char EarVer[31] = {};

UserSettingMainPage::UserSettingMainPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UserSetMainPage)
{
    ui->setupUi(this);
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽

    // {
    //     ui->label_6->adjustSize();

    //     // DevGetVersion();

    //     taskName = L"XIBERIA X HUB StartUp"; //任务名称
    //     appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath()).toStdWString();
    //     //当前登录用户名称
    //     userName = QProcessEnvironment::systemEnvironment().value("USERNAME").toStdWString();

    //     hr = InitializeTaskService(&pService);
    //     if (FAILED(hr)) {
    //         return;
    //     }
    //     if (this->IsTaskExists(pService, taskName)) {
    //         if (this->IsTaskPathMatch(pService, taskName, appPath)) {
    //             ui->pBt_SelfStart->setChecked(true);
    //         } else {
    //             // 任务存在但路径错误,重新创建任务
    //             if (UpdateTaskPath(pService, taskName, appPath)) {
    //                 ui->pBt_SelfStart->setChecked(true);
    //             } else {
    //                 ui->pBt_SelfStart->setChecked(false);
    //                 emit ApoManager::instance()->requestlogWithTime(
    //                     "自启动任务路径自动修复失败"); // 可选
    //             }
    //         }

    //     } else {
    //         ui->pBt_SelfStart->setChecked(false);
    //     }

    //     QString styleSheet
    //         = (R"(QComboBox{border-radius: 2px;combobox-popup: 0;border-image: url(:/Skin/Images/cBox/dropdownCollapse_bk.png);padding-left: 10px; color: rgb(255, 255, 255);icon: url(:/image/Headphones/AllEdit/add.png);}
    //                                     QComboBox::drop-down{border-image: url(:/Skin/Images/cBox/droptriangle_no.png);margin-top:0px;subcontrol-origin: padding;subcontrol-position: center right; margin-right:10px;height:8px;width:5px;}
    //                                     QComboBox::drop-down:checked{border-image: url(:/Skin/Images/cBox/droptriangle_se.png);margin-top:0px;margin-right:10px;subcontrol-origin: padding;subcontrol-position: center right;height:5px;width:8px;}
    //                                     )");

    //     QListView *listView = new QListView();

    //     listView->setStyleSheet(R"(
    //                                 QListView{font-family: "Noto Sans S Chinese";"
    //           "font-weight: 500;font-size: 14px;margin-top:6px;border-radius: 2px;padding-left: 17px;padding-right: 17px;color: #FFFFFF;outline: 0;selection-background-color: transparent !important;;show-decoration-selected: 0 !important;;}
    //                                 QListView::item {height: 31px;padding-right: 20px;border-bottom: 1px solid rgba(216, 216, 216, 0.1);}
    //                                 /*下方下拉列表项选中项的样式*/
    //                                 QListView::item:selected{background-color: transparent;background-image: url(:/Skin/Images/cBox/item_se.png);background-repeat: no-repeat;background-position:right center;}
    //                                /*下方下拉列表项鼠标悬停的样式*/
    //                                 QListView::item:hover
    //                                 {
    //                                     background-color: transparent;
    //                                 }
    //                                 QListView::item:focus {
    //                                     background-color: transparent;
    //                                     outline: 0;
    //                                 }
    //                             )");
    //     //一个 QListView 不能同时被两个 QComboBox 使用,为 cBox_Mic 创建另一个独立 ListView
    //     QListView *listView2 = new QListView();
    //     listView2->setStyleSheet(listView->styleSheet()); // 复用样式

    //     ui->cBox_Theme->setStyleSheet(styleSheet);
    //     ui->cBox_Theme->setView(listView);
    //     ui->cBox_language->setStyleSheet(styleSheet);
    //     ui->cBox_language->setView(listView2);

    //     M_SetCBoxShadow(ui->cBox_Theme);
    //     M_SetCBoxShadow(ui->cBox_language);
    //     // 在设置样式表后应用委托
    //     ui->cBox_Theme->setItemDelegate(new NoIconEffectDelegate(ui->cBox_Theme));

    //     // 获取文件名
    //     QString fileName = "XIBERIA X HUB.exe"; //"Xiberia_GP_Setup.exe";
    //     QString fileName_RX = "T10WirelessRx.bin";
    //     QString fileName_TX = "T10WirelessTx.bin";
    //     // 保存到临时目录
    //     QString tempDir = QDir::tempPath();
    //     DfilePath = QDir(tempDir).filePath(fileName);
    //     DfilePath_RX = QDir(tempDir).filePath(fileName_RX);
    //     DfilePath_TX = QDir(tempDir).filePath(fileName_TX);
    //     QDateTime now = QDateTime::currentDateTime();
    //     QString timestamp = now.toString("yyyyMMdd_hhmmss");
    //     QString baseName = QFileInfo(DfilePath).completeBaseName();
    //     QString suffix = QFileInfo(DfilePath).suffix();
    //     DfilePath = QDir(tempDir).filePath(baseName + "_" + timestamp + "." + suffix);
    //     file = new QFile(DfilePath, this); // 以 this 为父对象，自动清理

    //     baseName = QFileInfo(DfilePath_RX).completeBaseName();
    //     suffix = QFileInfo(DfilePath_RX).suffix();
    //     DfilePath_RX = QDir(tempDir).filePath(baseName + "_" + timestamp + "." + suffix);
    //     file_RX = new QFile(DfilePath_RX, this); // 以 this 为父对象，自动清理

    //     baseName = QFileInfo(DfilePath_TX).completeBaseName();
    //     suffix = QFileInfo(DfilePath_TX).suffix();
    //     DfilePath_TX = QDir(tempDir).filePath(baseName + "_" + timestamp + "." + suffix);
    //     file_TX = new QFile(DfilePath_TX, this); // 以 this 为父对象，自动清理

    //     if (!uota) {
    //         uota = new UpdateOTA(this);
    //         connect(uota, &UpdateOTA::GetOTAFile_U, this, [this](int type) {
    //             qDebug("产生 GetOTAFile_U信号");
    //             GetOTAFile(type, false);
    //         });
    //         connect(uota, &UpdateOTA::SetFailed_U, this, [this]() {
    //             qDebug("产生 SetFailed_U");
    //             showDownloadError(1);
    //         });
    //         uota->setModal(true);
    //     }

    //     ue = new UpdateError(this);
    //     ue->setModal(true);

    //     ie = new InstallError(this);
    //     ie->setModal(true);

    //     UV = new UpdateVersion(this);
    //     UV->setModal(true);

    //     connect(ui->pBt_Feedback, &QPushButton::clicked, this, [this]() {
    //         qDebug("用户反馈界面");



    //         if (clp_feedBackPage_ == nullptr) {
    //             clp_feedBackPage_ = new FeedbackMainPage(m);
    //             clp_feedBackPage_->setModal(true);
    //         }

    //         // 获取父窗口的中心点
    //         int x = m->x() + (m->width() - clp_feedBackPage_->width()) / 2;
    //         int y = m->y() + (m->height() - clp_feedBackPage_->height()) / 2;

    //         clp_feedBackPage_->move(x, y);

    //         clp_feedBackPage_->showOutInfo();                    //准备一下相关信息
    //         clp_feedBackPage_->setCl_access_token(access_token); //设置用户token

    //         int result = clp_feedBackPage_->exec();
    //         if (result == QDialog::Accepted) {
    //             globalSettings->setValue("UserGuideEn", false); //true:下次显示，false:下次不显示
    //         }
    //     });
    // }
}

UserSettingMainPage::~UserSettingMainPage()
{
    delete ui;
}

void UserSettingMainPage::DevGetVersion()
{
    if (isHidRun) {
        // 第4~33字节:Dongle 版本信息  提取第34~63字节:耳机版本信息
        // char DongleVer[31];  // 30字节 + \0
        // char EarVer[31];

        int res = lolib->GetVersion(DongleVer, EarVer);
        if (res < 0) {
            qDebug("Unable to write()\n");
            msgBox.critical(NULL, tr("错误"), tr("获取版本信息失败"));
        } else if (res > 0) {
            for (int i = 0; i < 30; i++) {
                if (DongleVer[i] != 0) {
                    // ui->tEdit_dongle->setText(DongleVer);
                    break;
                }
            }
            for (int i = 0; i < 30; i++) {
                if (EarVer[i] != 0) {
                    //ui->lab_EarPhonVer->setText(EarVer);
                    break;
                }
            }
        }
    }
}

void UserSettingMainPage::SoftGetVersion()
{
    SoftWareVer = clp_version_settings_main_page_->getVersionUIText();
    if (SoftWareVer.startsWith('v', Qt::CaseInsensitive)) {
        SoftWareVer.remove(0, 1); // 移除第一个字符
    }
}

//开机是否自启动不需要保存，查找自启动任务是否存在。
void UserSettingMainPage::saveIniValue(int &Language, int &Theme)
{
    Language = clp_interface_settings_main_page_->getLanguageIndex();
    Theme = clp_interface_settings_main_page_->getThemeIndex();
}
void UserSettingMainPage::readIniValue(int Language, int Theme)
{
    clp_interface_settings_main_page_->setLanguageIndex(Language);
    clp_interface_settings_main_page_->setThemeIndex(Theme);
}

void UserSettingMainPage::InitUIInformation()
{
    {
        // 默认显示第一个页面
        ui->stackedWidget->setCurrentIndex(0);
    }
    {
        ui->widget_settings_type->setObjectName("UserSettingMainPage_widget_settings_type");
        ui->widget_settings_type->setCornerRadius(8);
        ui->widget_settings_type->setStyleSheet(R"(
            #UserSettingMainPage_widget_settings_type {
                background-color: rgba(0, 0, 0, 77);
                border-radius: 8px;
            }
        )");
    }
    {
        // 顶部 设置类型 按键组
        clp_topButtons_ = new CustomQScrollAreaTopButtons(ui->widget_settings_type);
        ui->widget_settings_type->layout()->addWidget(clp_topButtons_);
    }
    {
        // 个人中心设置
        clp_personal_center_settings_main_page_
            = new PersonalCenterSettingsMainPage(ui->page_00_PersonalCenterSettings, this);
        ui->page_00_PersonalCenterSettings->layout()->addWidget(
            clp_personal_center_settings_main_page_);

        // 用户信息更新 → 通知 MainWindow 刷新
        connect(clp_personal_center_settings_main_page_,
                &PersonalCenterSettingsMainPage::userInfoUpdated,
                m,
                [this]() {
                    if (m) {
                        m->refreshUserDisplay();
                    }
                });
    }
    {
        // 系统设置
        clp_system_settings_mainPage_ = new SystemSettingsMainPage(ui->page_01_SystemSettings);
        ui->page_01_SystemSettings->layout()->addWidget(clp_system_settings_mainPage_);
    }
    {
        // 界面设置
        clp_interface_settings_main_page_ = new InterfaceSettingsMainPage(
            ui->page_02_InterfaceSettings);
        ui->page_02_InterfaceSettings->layout()->addWidget(clp_interface_settings_main_page_);
    }
    {
        // 版本升级
        clp_version_settings_main_page_ = new VersionSettingsMainPage(ui->page_03_VersionSettings,
                                                                      this);
        ui->page_03_VersionSettings->layout()->addWidget(clp_version_settings_main_page_);
    }
    {
        // 联系我们
        clp_contact_settings_main_page_ = new ContactSettingsMainPage(ui->page_04_ContactSettings,
                                                                      this);
        ui->page_04_ContactSettings->layout()->addWidget(clp_contact_settings_main_page_);
    }
}

void UserSettingMainPage::InitMember() {
    clp_dialog_tips_ = new CustomQDialogGeneralTips(this);
}

void UserSettingMainPage::InitConnect()
{
    // 连接 clp_topButtons_ 的 changeSettingsType 信号，使 ui->stackedWidget 跳转到指定页面
    connect(clp_topButtons_,
            &CustomQScrollAreaTopButtons::changeSettingsType,
            this,[this](int index){
                switch (index) {
                case 0:
                {
                }
                break;
                case 1:
                {}
                break;
                case 2:
                {
                    // 进入界面设置：完整刷新（壁纸列表 + 滑块同步，登录加载值在此生效）
                    UpdateAllSubPageUIInformation(SubPage::InterfaceSettings);
                }
                break;
                case 3:
                {}
                break;
                case 4:
                {}
                break;
                default:
                    break;
                }
                ui->stackedWidget->setCurrentIndex(index);
            },Qt::UniqueConnection);

    // 退出登录按钮 → 弹出通用提示弹窗
    connect(clp_personal_center_settings_main_page_->clp_user_info_->clp_logout_button_,
            &QPushButton::clicked,
            this,
            [this]() {
                if (!clp_dialog_tips_) {
                    clp_dialog_tips_ = new CustomQDialogGeneralTips(this);
                }
                // WBLIU: 提示文字和结果处理由用户填写
                clp_dialog_tips_->setCl_texts(tr("确认退出当前账号吗？"),
                                              tr("取消"),
                                              tr("退出登录"));
                clp_dialog_tips_->exec();

            });
}

void UserSettingMainPage::LanguageSet()
{
    ui->retranslateUi(this);

    if (clp_personal_center_settings_main_page_) clp_personal_center_settings_main_page_->LanguageSet();
    if (clp_system_settings_mainPage_) clp_system_settings_mainPage_->LanguageSet();
    if (clp_interface_settings_main_page_) clp_interface_settings_main_page_->LanguageSet();
    if (clp_version_settings_main_page_) clp_version_settings_main_page_->LanguageSet();
    if (clp_contact_settings_main_page_) clp_contact_settings_main_page_->LanguageSet();
}

void UserSettingMainPage::UpdateAllSubPageUIInformation(SubPage page)
{
    switch (page) {
    case All:
        clp_personal_center_settings_main_page_->UpdatePersonalCenterSettingsUIInformation();
        clp_system_settings_mainPage_->UpdateSystemSettingsUIInformation();
        // 轻量同步滑块（避免壁纸刷新卡顿；登录后 loadFromDisk 的值在此生效）
        clp_interface_settings_main_page_->syncSlidersFromModel();
        clp_version_settings_main_page_->UpdateVersionSettingsUIInformation();
        clp_contact_settings_main_page_->UpdateContactSettingsUIInformation();
        break;
    case PersonalCenter:
        clp_personal_center_settings_main_page_->UpdatePersonalCenterSettingsUIInformation();
        break;
    case SystemSettings:
        clp_system_settings_mainPage_->UpdateSystemSettingsUIInformation();
        break;
    case InterfaceSettings:
        clp_interface_settings_main_page_->UpdateInterfaceSettingsUIInformation();
        break;
    case VersionSettings:
        clp_version_settings_main_page_->UpdateVersionSettingsUIInformation();
        break;
    case ContactSettings:
        clp_contact_settings_main_page_->UpdateContactSettingsUIInformation();
        break;
    }
}

//恢复出厂设置-保存文件清除，回退到登录界面，激活码不需要清除
void UserSettingMainPage::ResetDefaultSetting()
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

//关闭与设备之间进行信息传输的定时器
void UserSettingMainPage::CloseReceiveTimer()
{
    emit CloseReceiveTimer_S();
}
