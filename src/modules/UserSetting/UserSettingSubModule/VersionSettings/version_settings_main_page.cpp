#include "modules/UserSetting/UserSettingSubModule/VersionSettings/version_settings_main_page.h"
#include "./Popup/Update/UpdateError.h"
#include "./Popup/Update/UpdateOTA.h"
#include "./Popup/Update/UpdateOTAFind.h"
#include "./Popup/Update/UpdateVersion.h"
#include "Popup/Update/UpdateOTASuccess.h"
#include "ui_version_settings_main_page.h"

#include <QDebug>

#include "LoadLib.h" ///< SoftWareVer
#include "data/api_global.h"
#include "network/http_client.h"
#include "network/request_options.h"
#include "network/server_router.h"
#include "APOThread/ApoManager.h"

#include "./Popup/Update/UpdateSoftWareFind.h"

bool UpdateTP1 = false;//先升级为2.0TP1
QString TP1TxVer = "";//2.0TP1发射器版本
QString TP1RxVer = "";//2.0TP1耳机版本
bool UpdateTP1En = false;//是否升级TP1，已弹窗告诉用户最新版信息
bool UpdateRxDone = false;//是否已升级完rx
bool UpdateTxDone = false;//是否已升级完tx
bool SkipUpdateRx = false;//rx已是最新版，跳过升级
bool SkipUpdateTx = false;//tx已是最新版，跳过升级

int iDevIndex = 1;
QString readlastVer_RX = "";//"260302";
QString readlastVer_TX = "";//"260305";
QString lastVerStr_RX = "";
QString lastVerStr_TX = "";


/// \brief 构造函数
VersionSettingsMainPage::VersionSettingsMainPage(QWidget *parent, UserSettingMainPage *targetObject)
    : QWidget(parent)
    , ui(new Ui::VersionSettingsMainPage)
    , clp_target_user_setting_main_page_(targetObject)
{
    ui->setupUi(this);
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽



    OTARollbackEn = false;

    // 获取文件名
    QString fileName = "XIBERIA HUB.exe";//"Xiberia_GP_Setup.exe";
    QString fileName_RX = "Rx.bin";
    QString fileName_TX = "Tx.bin";
    // 保存到临时目录
    QString tempDir = QDir::tempPath();
    DfilePath = QDir(tempDir).filePath(fileName);
    DfilePath_RX = QDir(tempDir).filePath(fileName_RX);
    DfilePath_TX = QDir(tempDir).filePath(fileName_TX);
    QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString("yyyyMMdd_hhmmss");
    QString baseName = QFileInfo(DfilePath).completeBaseName();
    QString suffix = QFileInfo(DfilePath).suffix();
    DfilePath = QDir(tempDir).filePath(baseName + "_" + timestamp + "." + suffix);
    file = new QFile(DfilePath, this); // 以 this 为父对象，自动清理

    baseName = QFileInfo(DfilePath_RX).completeBaseName();
    suffix = QFileInfo(DfilePath_RX).suffix();
    DfilePath_RX = QDir(tempDir).filePath(baseName + "_" + timestamp + "." + suffix);
    file_RX = new QFile(DfilePath_RX);

    baseName = QFileInfo(DfilePath_TX).completeBaseName();
    suffix = QFileInfo(DfilePath_TX).suffix();
    DfilePath_TX = QDir(tempDir).filePath(baseName + "_" + timestamp + "." + suffix);
    file_TX = new QFile(DfilePath_TX);


    uOTASuccess = new UpdateOTASuccess(m);
    uOTASuccess->setModal(true);

    uota = new UpdateOTA(m);
    connect(uota,&UpdateOTA::GetOTAFile_U,this,[this](int type){
        qDebug("产生 GetOTAFile_U 信号");
        GetOTAFile(type,false);
    });
    connect(uota,&UpdateOTA::UpdateNewOTAFile_U,this,[this](){
        qDebug("产生 UpdateNewOTAFile_U 信号");
        UpdateNewOTA();
    });
    connect(uota,&UpdateOTA::SetFailed_U,this,[this](){
        qDebug("产生 SetFailed_U");
        showDownloadError(1);
    });

    connect(uota,&UpdateOTA::SetSuccess_U,this,[this](){
        qDebug("产生 SetSuccess_U");
        if(OTARollbackEn)
        {
            uOTASuccess->UpdateTitle(1);
        }else
        {
            uOTASuccess->UpdateTitle(0);
        }
        uOTASuccess->exec();



        globalSettings->setValue("OTA/TX", OldOTAVerStr_Tx);
        globalSettings->setValue("OTA/RX", OldOTAVerStr_Rx);
        globalSettings->sync();


    });
    uota->setModal(true);



    uError = new UpdateError(m);
    uError->setModal(true);

    uSoftWareFind = new UpdateSoftWareFind(m);
    uSoftWareFind->setModal(true);

    uVersion = new UpdateVersion(m);
    uVersion->setModal(true);


    uOTAFind = new UpdateOTAFind(m);
    uOTAFind->setModal(true);

}

VersionSettingsMainPage::~VersionSettingsMainPage()
{
    uota->deleteLater();
    uError->deleteLater();
    uVersion->deleteLater();
    uOTAFind->deleteLater();
    uSoftWareFind->deleteLater();

    delete ui;
}

/// \brief 初始化UI的默认信息
void VersionSettingsMainPage::InitUIInformation()
{
    {
        // 左侧面板
        ui->widget->setObjectName("VersionSettings_widget");
        ui->widget->setCornerRadius(12);
        ui->widget->setStyleSheet(R"(
            #VersionSettings_widget {
                border-radius: 12px;
                background-color: rgba(81, 96, 122, 0.2);
            }
        )");
    }
    {
        // 右侧面板
        ui->widget_2->setObjectName("VersionSettings_widget_2");
        ui->widget_2->setCornerRadius(12);
        ui->widget_2->setStyleSheet(R"(
            #VersionSettings_widget_2 {
                border-radius: 12px;
                background-color: rgba(81, 96, 122, 0.2);
            }
        )");
    }
    {
        // 应用版本图标
        ui->label->setObjectName("VersionSettings_label");
        ui->label->setStyleSheet(R"(
            QLabel#VersionSettings_label{
                border-image: url(:/Skin/Images/more/version_settings/app_icon.png);
            }
        )");
    }
    {
        // 关于 XIBERIA HUB
        ui->label_2->setObjectName("VersionSettings_label_2");
        ui->label_2->setStyleSheet(R"(
            QLabel#VersionSettings_label_2 {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 14px;
                color: #A1A8B3;
                background-color: transparent;
            }
        )");
    }
    {
        // 当前驱动版本标签
        ui->label_3->setObjectName("VersionSettings_label_3");
        ui->label_3->setStyleSheet(R"(
            QLabel#VersionSettings_label_3 {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #A1A8B3;
                background-color: transparent;
            }
        )");
    }
    {
        // 驱动版本号
        ui->lab_SoftwareVer->setObjectName("VersionSettings_lab_SoftwareVer");
        ui->lab_SoftwareVer->setStyleSheet(R"(
            QLabel#VersionSettings_lab_SoftwareVer {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #A1A8B3;
                background-color: transparent;
            }
        )");
    }
    {
        // 检查更新按钮
        ui->pBt_UpdateSoftware->setObjectName("VersionSettings_pushButton");
        ui->pBt_UpdateSoftware->setStyleSheet(R"(
            QPushButton#VersionSettings_pushButton {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 14px;
                color: #FFFFFF;
                border-radius: 15px;
                border-image: url(:/Skin/Images/Popup/confirm-no.png);
            }
            QPushButton#VersionSettings_pushButton:hover {
                border-image: url(:/Skin/Images/Popup/confirm-ho.png);
            }
        )");
    }
    {
        // 耳机图片
        ui->label_deivce_icon->setObjectName("VersionSettings_label_4");
        ui->label_deivce_icon->setStyleSheet(R"()");
    }
    {
        // 固件信息标签
        ui->label_12->setObjectName("VersionSettings_label_12");
        ui->label_12->setStyleSheet(R"(
            QLabel#VersionSettings_label_12 {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 14px;
                color: #A1A8B3;
                background-color: transparent;
            }
        )");
    }

    {
        // 可升级固件版本标签
        ui->label_7->setObjectName("VersionSettings_label_7");
        ui->label_7->setStyleSheet(R"(
            QLabel#VersionSettings_label_7 {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #616871;
                background-color: transparent;
            }
        )");
    }
    {
        // 可回退固件版本标签
        ui->label_8->setObjectName("VersionSettings_label_8");
        ui->label_8->setStyleSheet(R"(
            QLabel#VersionSettings_label_8 {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #616871;
                background-color: transparent;
            }
        )");
    }
    {
        // 固件版本号 label_9/10/11
        ui->label_9->setObjectName("VersionSettings_label_9");
        ui->label_9->setStyleSheet(R"(
            QLabel#VersionSettings_label_9 {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #616871;
                background-color: transparent;
            }
        )");
        ui->label_10->setObjectName("VersionSettings_label_10");
        ui->label_10->setStyleSheet(R"(
            QLabel#VersionSettings_label_10 {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #616871;
                background-color: transparent;
            }
)");
        ui->label_11->setObjectName("VersionSettings_label_11");
        ui->label_11->setStyleSheet(R"(
            QLabel#VersionSettings_label_11 {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #616871;
                background-color: transparent;
            }
)");
        ui->label_7->hide();
        ui->label_8->hide();
        ui->label_10->hide();
        ui->label_11->hide();
    }
    {
        // 固件回退按钮
        ui->pBt_Rollback->setObjectName("VersionSettings_pushButton_2");
        ui->pBt_Rollback->setStyleSheet(R"(
            QPushButton#VersionSettings_pushButton_2 {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 14px;
                color: #009FEF;
                border-image: url(:/Skin/Images/Popup/cancel-no.png);
                border-radius: 15px;
            }
            QPushButton#VersionSettings_pushButton_2:hover {
                border-image: url(:/Skin/Images/Popup/cancel-ho.png);
            }
            QPushButton#VersionSettings_pushButton_2:disabled  {
                color: #CCCCCC;
                border-image: url(:/Skin/Images/Popup/confirm-ho.png);
            }
        )");
        ui->pBt_Rollback->hide();
    }
    {
        // 升级固件按钮
        ui->pBt_OTA->setObjectName("VersionSettings_pushButton_3");
        ui->pBt_OTA->setStyleSheet(R"(
            QPushButton#VersionSettings_pushButton_3 {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 14px;
                color: #FFFFFF;
                border-image: url(:/Skin/Images/Popup/confirm-no.png);
                border-radius: 15px;
            }
            QPushButton#VersionSettings_pushButton_3:hover {
                border-image: url(:/Skin/Images/Popup/confirm-ho.png);
            }
            QPushButton#VersionSettings_pushButton_3:disabled  {
                color: #CCCCCC;
                border-image: url(:/Skin/Images/Popup/confirm-ho.png);
            }
        )");
    }

    {
        // 当前固件版本标签
        ui->label_6->setObjectName("VersionSettings_label_6");
        ui->label_6->setStyleSheet(QString("QLabel#VersionSettings_label_6 {"
                                           " font-family: \"Noto Sans S Chinese\";"
                                           "font-weight: 500;"
                                           " font-size: 12px;"
                                           " color: #616871;"
                                           " text-align: right;"
                                           "}"));

        // // 计算两个文本的宽度
        // QFontMetrics fm(ui->label_7->font());
        // int width1 = fm.horizontalAdvance(ui->label_6->text());
        // int width2 = fm.horizontalAdvance(ui->label_7->text());

        // if (width1 < width2) {
        //     // 短文本需要拉伸，填补差值
        //     int diff = width2 - width1;
        //     int charCount = ui->label_6->text().length();
        //     if (charCount > 1) {
        //         // 在字符之间平均分配额外的间距
        //         int extraSpacing = diff / (charCount - 2);
        //         ui->label_6->setStyleSheet(QString("QLabel#VersionSettings_label_6 {"
        //                                            " font-family: \"Noto Sans S Chinese\";"
        //                                             "font-weight: 500;"
        //                                            " font-size: 12px;"
        //                                            " color: #616871;"
        //                                            " background-color: transparent;"
        //                                            " letter-spacing: %1px;"
        //                                            " text-align: right;"
        //                                            "}")
        //                                        .arg(extraSpacing));
        //     }
        // }
    }
}

/// \brief 初始化内部成员
void VersionSettingsMainPage::InitMember()
{
    // WBLIU: 预留内部成员初始化
}

/// \brief 连接默认的信号槽
void VersionSettingsMainPage::InitConnect()
{
    // 检查更新按钮 — 预留
    // 固件回退按钮 — 预留
    // 升级固件按钮 — 预留
}

QString VersionSettingsMainPage::getVersionUIText()
{
    return ui->lab_SoftwareVer->text();
}

/// \brief 刷新翻译文本
void VersionSettingsMainPage::LanguageSet()
{
    ui->retranslateUi(this);
}

void VersionSettingsMainPage::UpdateVersionSettingsUIInformation()
{
    // WBLIU: 更新版本设置 UI 信息预留
}

//下载错误（OTA+软件）
void VersionSettingsMainPage::showDownloadError(int type)
{
    if(type == 0)
    {
        //上位机安装包下载失败
        uError->setTitle(0);
        int res = uError->exec();
        if(res == QDialog::Accepted)
        {
            //重试
            on_pBt_UpdateSoftware_clicked();
        }else
        {
            uError->close();
        }
        // uError->deleteLater();
    }else if(type == 1)
    {
        if(OTARollbackEn)
        {
            uError->setTitle(2);
        }else
        {
            uError->setTitle(1);
        }

        //升级失败
        int res = uError->exec();
        if(res == QDialog::Accepted)
        {
            //重试
            on_pBt_OTA_clicked();
        }else
        {
            uError->close();
            if(uota->isVisible())
            {
                uota->close();
            }
        }
        //uError->deleteLater();
    }else if(type == 2)
    {
        //固件升级获得当前设备版本信息失败
        uError->setTitle(3);
        int res = uError->exec();
        if(res == QDialog::Accepted)
        {
            //重试
            on_pBt_OTA_clicked();
        }else
        {
            uError->close();
            if(uota->isVisible())
            {
                uota->close();
            }
        }

    }else
    {
        uError->setTitle(0);
        //上位机安装包安装失败
        int res = uError->exec();
        if(res == QDialog::Accepted)
        {
            //重试
            runDownloadedFile(DfilePath);
        }else
        {
            uError->close();
        }
        //uError->deleteLater();
    }

}


//更新软件（exe）
void VersionSettingsMainPage::on_pBt_UpdateSoftware_clicked()
{
    qDebug()<< QSslSocket::sslLibraryBuildVersionString();

    qDebug() << "OpenSSL支持情况:" << QSslSocket::supportsSsl();

    swUpdateBtnClicked = true;
    GetUpdateMsg();
}
//获取服务器新上位机安装包信息
void VersionSettingsMainPage::GetUpdateMsg()
{
    // 统一走新栈 ApiClient（URL 路由由 ServerRouter 按 tag "drive" 解析）
    const QString driveName = "X HUB";
    QUrlQuery query;
    query.addQueryItem("device_name", driveName); // 使用正确的参数名

    QNetworkReply *t_reply = HttpClient::instance().get("/drive/info",
        RequestOptions{}.withQuery(query).withTag("drive"));
    connect(t_reply, &QNetworkReply::finished, this,
            [this, t_reply]() { onReplyFinished(t_reply); });
}

//服务器响应上位机版本信息处理
void VersionSettingsMainPage::onReplyFinished(QNetworkReply *reply)
{
    // 确保 reply 最终被释放
    reply->deleteLater();

    // 错误处理
    if (reply->error() != QNetworkReply::NoError) {
        //QString errorMsg = "网络错误: " + reply->errorString();

        QString errorMsg = "网络请求失败，错误码: " + QString::number(reply->error());
        // 确保 reply 被正确释放
        reply->deleteLater();
        // msgBox.critical(NULL,tr("错误"),tr("网络错误，请检查连接后重试。"));
        emit ApoManager::instance()->requestlogWithTime(QString("获得上位机安装包信息: %1").arg(reply->errorString()));
        msgBox.critical(NULL,tr("错误"),errorMsg);

        return;
    }

    // 读取响应数据
    QByteArray responseData = reply->readAll();

    // 解析 JSON
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        QString errorMsg = "JSON解析错误: " + parseError.errorString();
        qWarning() << errorMsg;
        // emit driveInfoReceived(QString(), QString(), errorMsg);
        return;
    }

    QJsonObject rootObj = jsonDoc.object();

    // 获取当前显示的版本字符串
    QString currentVerStr = ui->lab_SoftwareVer->text();
    //先判断是否以 'v' 开头,是则去掉
    if (currentVerStr.startsWith('v', Qt::CaseInsensitive)) {
        currentVerStr = currentVerStr.mid(1);
    }
    // currentVerStr = "0.2.1";  //测试
    qDebug() << "纯版本号:" << currentVerStr; // 输出 "0.1.0"

    // 检查业务状态码
    QString code = rootObj["code"].toString();
    if (code != "success") {
        QString message = rootObj["message"].toString();
        QString errorMsg = QString("API error: code=%1, message=%2").arg(code, message);
        qWarning() << errorMsg;
        // emit driveInfoReceived(QString(), QString(), errorMsg);
        if(code.contains("db_not_found",Qt::CaseInsensitive) & swUpdateBtnClicked)
        {
            qDebug() << "已是最新版本";
            uVersion->UpdateVer(currentVerStr);
            uVersion->UpdateTitle(0);
            uVersion->exec();

        }
        return;
    }

    // 提取 data 字段
    QJsonObject dataObj = rootObj["data"].toObject();
    QString downloadUrl = dataObj["download_url"].toString();
    QString serverVerStr = dataObj["version"].toString();
    QString VerExplanation = dataObj["release_notes"].toString();


    // 检查关键字段是否为空（可选）
    if (downloadUrl.isEmpty() || serverVerStr.isEmpty()) {
        QString errorMsg = "API返回数据缺少 download_url 或 version";
        qWarning() << errorMsg;
        return;
    }

    qDebug() << "获得服务器中版本号:" << serverVerStr;

    //与ui->lab_SoftwareVer比较版本号
    // 解析为 QVersionNumber
    QVersionNumber currentVer = QVersionNumber::fromString(currentVerStr);
    QVersionNumber serverVer = QVersionNumber::fromString(serverVerStr);

    // 比较
    if (serverVer > currentVer) {
        qDebug() << "发现新版本:" << serverVer.toString() << "当前版本:" << currentVer.toString();
        qDebug() << "下载地址:" << downloadUrl;

        QVersionNumber igVer =  QVersionNumber::fromString(globalSettings->value("UpdateVer").toString());//已更新或忽略的版本
        // swUpdateBtnClicked:点击上位机更新按钮，serverVer > igVer：自动检测弹窗，最新版本大于用户之前忽略的版本
        if (swUpdateBtnClicked || serverVer > igVer)
        {

            uSoftWareFind->setShowPage(0);
            uSoftWareFind->UpdateExplanation(VerExplanation);
            uSoftWareFind->setVersion(serverVer.toString());

            // 解除旧连接解除旧连接
            disconnect(uSoftWareFind, &UpdateSoftWareFind::startUpdate, nullptr, nullptr);
            disconnect(uSoftWareFind, &QDialog::rejected, nullptr, nullptr);

            connect(uSoftWareFind, &UpdateSoftWareFind::startUpdate, this, [this,downloadUrl]{startDownload(downloadUrl);});
            connect(uSoftWareFind, &QDialog::rejected, this, &VersionSettingsMainPage::cancelCurrentDownload);

            SoftWareFindDlgRes = uSoftWareFind->exec();

            // if (uSoftWareFind->exec() == QDialog::Accepted)
            // {
            //     startDownload(downloadUrl);
            // }

            // 只有自动检测（非手动点击）时，才保存已提醒的版本并销毁对话框
            if (!swUpdateBtnClicked) {
                globalSettings->setValue("UpdateVer", serverVer.toString());
                uSoftWareFind->deleteLater();
            }
        }




    } else
    {
        //点击获取最新版本按钮，才显示已是最新版
        if(swUpdateBtnClicked)
        {
            qDebug() << "已是最新版本";
            uVersion->UpdateVer(currentVerStr);
            uVersion->UpdateTitle(0);
            uVersion->exec();
        }

    }

}
void VersionSettingsMainPage::cancelCurrentDownload()
{
    // 右侧关闭按钮，可在下载过程中点击。若不断开旧 reply 连接到finished 信号，会出现报错弹窗
    disconnect(SoftWareLoad_Reply, &QNetworkReply::finished, this, nullptr);
    if (SoftWareLoad_Reply) {
        if (SoftWareLoad_Reply->isRunning())
            SoftWareLoad_Reply->abort();
        SoftWareLoad_Reply->deleteLater();
        SoftWareLoad_Reply = nullptr;
    }
    if (SoftWareLoad_File) {
        SoftWareLoad_File->cancelWriting();
        delete SoftWareLoad_File;
        SoftWareLoad_File = nullptr;
    }
}
//上位机驱动安装包下载
void VersionSettingsMainPage::startDownload(const QString &urlString)
{
    // 先终止可能正在进行的旧下载
    if (SoftWareLoad_Reply && SoftWareLoad_Reply->isRunning()) {
        cancelCurrentDownload();
    }
    // 每次下载前清理旧文件，使用 QSaveFile 确保原子写入
    if (QFile::exists(DfilePath))
        QFile::remove(DfilePath);

    SoftWareLoad_File  = new QSaveFile(DfilePath); // QSaveFile 在 success 时才会真正覆盖
    if (!SoftWareLoad_File ->open(QIODevice::WriteOnly)) {
        qWarning() << "无法创建文件:" << SoftWareLoad_File ->errorString();
        showDownloadError(0);
        delete SoftWareLoad_File ;
        return;
    }


    if (!m_manager_exe) {
        m_manager_exe = new QNetworkAccessManager(this);
        m_manager_exe->setTransferTimeout(60000); // 超时时间为60秒
    }

    QUrl url(urlString);
    if (!url.isValid()) {
        showDownloadError(0);
        delete SoftWareLoad_File ;
        return;
    }

    QNetworkRequest request(url);

    // 设置请求头
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = m_manager_exe->get(request);
    SoftWareLoad_Reply = reply;


    // 连接进度信号
    connect(reply, &QNetworkReply::downloadProgress,uSoftWareFind,[this](qint64 bytesReceived, qint64 bytesTotal)
            {
                if (bytesTotal > 0) {
                    int percent = static_cast<int>((bytesReceived * 100) / bytesTotal);
                    uSoftWareFind->setShowPage(1);
                    uSoftWareFind->ShowProgress(percent);
                }
            });

    // 连接 readyRead 信号：将数据写入文件
    connect(reply, &QNetworkReply::readyRead, this, [this,reply]() {
        if (SoftWareLoad_File && SoftWareLoad_File->isOpen()) {
            SoftWareLoad_File->write(reply->readAll());
        }

    });

    // 统一处理完成信号（包括成功、错误、取消）
    connect(reply, &QNetworkReply::finished, this,[=]() mutable {
        // 无论成功失败，都要关闭文件
        if (SoftWareLoad_File) {
            SoftWareLoad_File->commit(); // 成功后提交，失败则回滚
            delete SoftWareLoad_File;
            SoftWareLoad_File = nullptr;
        }
        if (!SoftWareLoad_Reply) return;

        bool success = (SoftWareLoad_Reply->error() == QNetworkReply::NoError);
        if (success) {
            // 文件已完整落盘，直接启动安装
            if (uSoftWareFind && uSoftWareFind->isVisible())
            {
                uSoftWareFind->accept(); // 关闭进度框
            }

            onDownloadFinished(SoftWareLoad_Reply);
        } else {
            // 下载出错
            if (uSoftWareFind && uSoftWareFind->isVisible())
            {
                uSoftWareFind->blockSignals(true);
                uSoftWareFind->reject();
                uSoftWareFind->blockSignals(false);
            }
            showDownloadError(0);
            SoftWareLoad_Reply->deleteLater();
            SoftWareLoad_Reply = nullptr;
        }


    });

    // 显示进度对话框（模态阻塞）
    if (!uSoftWareFind->isVisible())
    {
        int res = uSoftWareFind->exec();
        if(res == QDialog::Rejected)
        {
            // 用户取消下载
            // if (safeReply && safeReply->isRunning())
            //     safeReply->abort();// 终止请求，会触发 finished 信号（此时 file 已空，安全）
            // if (file) {
            //     file->cancelWriting();
            //     delete file;
            //     file = nullptr;
            // }
            cancelCurrentDownload();  // 使用封装函数

        }
    }

}
// 上位机安装包下载完成后的安装启动
void VersionSettingsMainPage::onDownloadFinished(QNetworkReply *reply)
{
    if (!reply) return;

    // 检查错误
    if (reply->error() != QNetworkReply::NoError) {
        if (reply->error() != QNetworkReply::OperationCanceledError) {
            qDebug() << "下载失败:" << reply->errorString();
            showDownloadError(0);
        }

        reply->deleteLater();
        return;
    }

    // 检查HTTP状态码
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode >= 400) {
        qDebug() << "HTTP错误:" << statusCode;
        showDownloadError(0);
        reply->deleteLater();
        return;
    }



        qDebug() << "文件下载成功:" << DfilePath;
        // 保存文件路径
        m_downloadedFilePath = DfilePath;

        //立即安装
        runDownloadedFile(DfilePath);

        /*QLabel *lab = new QLabel(m);
        lab->setGeometry(0, 0, m->width(), m->height());
        lab->setStyleSheet("border-image: NULL;background-color: rgba(0, 0, 0, 60%);");
        // 最后显示阴影标签（保持底层）
        lab->raise();
        lab->show();
        // 询问是否运行
        IsInstall *install = new IsInstall(m);
        install->setModal(true);
        int res = install->exec();
        if(res == QDialog::Accepted)
        {
            //立即安装
            runDownloadedFile(DfilePath);
        }else
        {
            cleanupDownloadedFile(); // 删除文件
        }
        install->deleteLater();
        delete lab;
        lab = nullptr;  // 必须置空防止野指针*/


        // QMessageBox msgBox(this);
        // msgBox.setWindowTitle("下载完成");
        // msgBox.setText(QString("文件已下载到:\n%1\n\n是否立即运行安装程序？").arg(DfilePath));
        // msgBox.setIcon(QMessageBox::Question);
        // msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        // msgBox.setDefaultButton(QMessageBox::Yes);

        // if (msgBox.exec() == QMessageBox::Yes) {
        //     runDownloadedFile(DfilePath);
        // }


    reply->deleteLater();
}


//运行下载下来的exe
void VersionSettingsMainPage::runDownloadedFile(const QString &filePath)
{
    // 检查文件是否存在
    if (!QFile::exists(filePath)) {
        showDownloadError(1);
        return;
    }



    QString nativePath = QDir::toNativeSeparators(filePath);

    // 创建进程对象
    QProcess *process = new QProcess(this);

    // 启动进程
    bool success = false;

    // Windows: 使用ShellExecuteW
    SHELLEXECUTEINFOW sei = { sizeof(sei) };
    sei.lpVerb = L"open";
    sei.lpFile = reinterpret_cast<LPCWSTR>(nativePath.utf16());
    sei.nShow = SW_SHOWNORMAL;

    success = ShellExecuteExW(&sei);
    if (!success) {
        showDownloadError(1);
        cleanupDownloadedFile();
        return;
    }

    // 安装程序已启动，退出当前应用
    QApplication::quit();
}

void VersionSettingsMainPage::cleanupDownloadedFile()
{
    if (!m_downloadedFilePath.isEmpty() && QFile::exists(m_downloadedFilePath)) {
        qDebug() << "删除下载的文件:" << m_downloadedFilePath;

        // 尝试删除文件
        if (QFile::remove(m_downloadedFilePath)) {
            qDebug() << "文件删除成功";
        } else {
            qDebug() << "文件删除失败，可能文件正在使用中";

            // 如果删除失败，可以尝试延迟删除
            QTimer::singleShot(1000, this, [this]() {
                if (QFile::exists(m_downloadedFilePath)) {
                    if (QFile::remove(m_downloadedFilePath)) {
                        qDebug() << "延迟删除成功";
                    } else {
                        qDebug() << "延迟删除仍然失败";
                    }
                }
            });
        }

        m_downloadedFilePath.clear();
    }
}


//升级固件
void VersionSettingsMainPage::on_pBt_OTA_clicked()
{
    int kMaxRetries = 10;          // 最多尝试10次获取版本信息
    int kRetryIntervalMs = 50;     // 每次间隔50ms，避免空耗CPU
    UpdateTP1 = false;
    UpdateTP1En = false;//false:没有升级TP1
    UpdateRxDone = false;//false:RX没有升级完成
    UpdateTxDone = false;//false:TX没有升级完成
    OTARollbackEn = false;//false:点击的固件升级，true:点击的固件回退
    SkipUpdateRx = false;//false:没有跳过rx升级流程
    SkipUpdateTx = false;//false:没有跳过tx升级流程

    readlastVer_TX = "";
    readlastVer_RX = "";
    // if (DongleVer[0] == '\0' || EarVer[0] == '\0')
    // {
    //     clp_target_user_setting_main_page_->DevGetVersion();
    // }

   int retry = 0;
   while ((DongleVer[0] == '\0' || EarVer[0] == '\0') && retry < kMaxRetries)
   {
       clp_target_user_setting_main_page_->DevGetVersion();
       // 如果 DevGetVersion 是同步的，并且内部不会返回事件，可以稍微 sleep 或处理事件
       QThread::msleep(kRetryIntervalMs);          // 跨平台等待
       ++retry;
   }
   if (retry >= kMaxRetries) {
       // 超时处理：弹窗提示、使用默认值、降级处理等
       showDownloadError(2);
       return;
   }

    clp_target_user_setting_main_page_->CloseReceiveTimer();//升级固件时，要关闭和硬件沟通的定时器

    // 获取当前显示的版本字符串
    QString currentVerstrTx = QString(DongleVer).split('_').last();//只要结尾的日期 T10_TX_V2.0P2_260306
    QString currentVerstrRx = QString(EarVer).split('_').last();//只要结尾的日期 T10_TX_V2.0P2_260306
    OldOTAVerStr_Tx = currentVerstrTx;
    OldOTAVerStr_Rx = currentVerstrRx;
    if(SelDev_DeviceName.contains("T10",Qt::CaseInsensitive) && SelDev_DeviceName.contains("Wireless",Qt::CaseInsensitive))
    {
        TP1TxVer = "260611";
        TP1RxVer = "260623";//"260507";
    }else if(SelDev_DeviceName.contains("K06S",Qt::CaseInsensitive))
    {
        TP1TxVer = "260611";
        TP1RxVer = "260626";//"260614";
    }else if(SelDev_DeviceName.contains("T7",Qt::CaseInsensitive) && (SelDev_PID == 0xF014 || SelDev_PID == 0xF008))
    {
        TP1TxVer = "260608";
        TP1RxVer = "260608";
    }
    // 判断是否需要升级到 TP1 过渡版本
    bool needTP1 = ((currentVerstrTx.toInt() < TP1TxVer.toInt()) || (currentVerstrRx.toInt() < TP1RxVer.toInt()));

            // 特殊设备跳过 TP1 的情况
    bool skipTP1 = false;//true:跳过。false：不跳过
    if (needTP1 &&
        SelDev_DeviceName.contains("T7", Qt::CaseInsensitive) &&
        (SelDev_PID == 0xF014 || SelDev_PID == 0xF008)) {
        // 特定旧版本（因为这两个固件版本，为3.0的）直接走最终 OTA，不需 TP1
        if (currentVerstrTx.toInt() == 260604 || currentVerstrRx.toInt() == 260604 ||
            currentVerstrTx.toInt() == 260602 || currentVerstrRx.toInt() == 260601) {
            skipTP1 = true;
        }
    }

    if(needTP1 && !skipTP1)
    {
        //先升级为2.0TP1(先升dongle(Tx 0)，再升耳机(Rx1))，再升级为3.0最新版
        UpdateTP1 = true;

        iDevIndex = 0;
        GetOTAFile(0,true);//先更新TX，在更新RX

    }else
    {
        UpdateTP1 = false;

        iDevIndex = 1;
        GetOTAFile(1,true);//先更新RX，在更新TX
    }
}
//已升级为TP1，升级更高版本
void VersionSettingsMainPage::UpdateNewOTA()
{
    OTARollbackEn = false;
    readlastVer_TX = "";
    readlastVer_RX = "";
    if (DongleVer[0] == '\0' || EarVer[0] == '\0')
    {
        clp_target_user_setting_main_page_->DevGetVersion();
    }
    clp_target_user_setting_main_page_->CloseReceiveTimer();

    UpdateTP1 = false;
    iDevIndex = 1;
    GetOTAFile(1,false);//先更新RX，在更新TX

}
//参数:T7,T10,K06S,若版本比TP1低，则先升级为TP1(先升dongle(TX 0)再升耳机(RX 1)，再升级为3.0(先升耳机，再升升dongle),3.0不能回退成旧版本,en:是否显示提醒弹窗
void VersionSettingsMainPage::GetOTAFile(int type, bool en)
{
    QString Name = "";
    QString readlastVer = "";
    if(type == 0)
    {
        readlastVer = readlastVer_TX;
    }else if(type == 1)
    {
        readlastVer = readlastVer_RX;
    }
    qDebug()<< QSslSocket::sslLibraryBuildVersionString();

    qDebug() << "OpenSSL支持情况:" << QSslSocket::supportsSsl();

    if(SelDev_DeviceName.contains("T10",Qt::CaseInsensitive) && SelDev_DeviceName.contains("Wireless",Qt::CaseInsensitive)&& (SelDev_PID == 0xF001))
    {
        Name = "XIBERIA T10 Wireless ";
        emit ApoManager::instance()->requestlogWithTime("OTA NAME XIBERIA T10 Wireless ");
    }else if(SelDev_DeviceName.contains("K06S",Qt::CaseInsensitive)&& (SelDev_PID == 0xF002))
    {
        Name = "XIBERIA K06S Wireless ";
        emit ApoManager::instance()->requestlogWithTime("OTA NAME XIBERIA K06S Wireless ");
    }else if(SelDev_DeviceName.contains("T7 GT",Qt::CaseInsensitive) && (SelDev_PID == 0xF009))
    {
        Name = "CROCIRIS T7 GT ";
        emit ApoManager::instance()->requestlogWithTime("OTA NAME CROCIRIS T7 GT");
    }else if(SelDev_DeviceName.contains("T7",Qt::CaseInsensitive) && (SelDev_PID == 0xF008))
    {
        Name = "CROCIRIS T7 ";
        emit ApoManager::instance()->requestlogWithTime("OTA NAME CROCIRIS T7");
    }else{
        qDebug() << "不存在";

        if (OTARollbackEn) {
            uVersion->UpdateVer(readlastVer);
            uVersion->UpdateTitle(4);
        } else {
            QString currentVerStr;
            if (type == 0) {
                currentVerStr = QString(DongleVer);
            } else {
                currentVerStr = QString(EarVer);
            }
            currentVerStr = currentVerStr.split('_')
                                .last(); //只要结尾的日期  T10_TX_V2.0P2_260306
            uVersion->UpdateVer(currentVerStr);
            uVersion->UpdateTitle(1);
        }

        uVersion->exec();
        return;
    }

    // 发送网络请求（内部处理响应）
    sendOtaRequest(Name,type, en, readlastVer);
}
//构建 OTA 版本查询的 HTTP 请求并发送
void VersionSettingsMainPage::sendOtaRequest(QString Name,int type, bool en, const QString &readlastVer)
{
    QUrlQuery query;

    query.addQueryItem("device_name", Name + (type == 0 ? "Tx" : "Rx"));

    // 版本参数
    if (UpdateTP1 && UpdateTP1En) {
        // 强制使用 TP1 版本
        query.addQueryItem("version", (type == 0) ? TP1TxVer : TP1RxVer);
    } else if (!readlastVer.isEmpty()) {
        query.addQueryItem("version", readlastVer);
    }

    query.addQueryItem("status", "enabled");

    QNetworkReply *reply = HttpClient::instance().get(
        "/firmware/info",
        RequestOptions{}.withQuery(query).withTag("firmware"));

    // 输出完整的请求 URL
    qDebug() << "OTA Request URL:" << reply->request().url().toString();

    // 如果需要查看原始请求头（如 User-Agent, Accept 等），也可以输出
    qDebug() << "Request Headers:" << reply->request().rawHeaderList();

    // 响应处理（lambda 仍保留，但内部逻辑提取）
    connect(reply, &QNetworkReply::finished, this, [this, reply, type, en, readlastVer]() {
        if (reply->error() != QNetworkReply::NoError) {
            QString errorMsg = "网络错误: " + reply->errorString();
            qWarning() << errorMsg;
            reply->deleteLater();
            return;
        }

        QByteArray responseData = reply->readAll();
        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "JSON解析错误:" << parseError.errorString();
            reply->deleteLater();
            return;
        }

        QJsonObject rootObj = jsonDoc.object();
        QString code = rootObj["code"].toString();
        QString message = rootObj["message"].toString();

        if (code != "success") {
            handleApiError(code, message, type, en, readlastVer);
        } else {
            handleApiSuccess(rootObj, type, en, readlastVer);
        }
        reply->deleteLater();
    });
}
//处理服务器返回的“失败”响应
void VersionSettingsMainPage::handleApiError(const QString &code, const QString &message, int type, bool en, const QString &readlastVer)
{
    QString errorMsg = QString("API error: code=%1, message=%2").arg(code, message);
    qWarning() << errorMsg;

    if (code.contains("db_not_found", Qt::CaseInsensitive)) {
        qDebug() << "不存在";
        if (OTARollbackEn) {
            uVersion->UpdateVer(readlastVer);
            uVersion->UpdateTitle(4);
        } else {
            QString currentVerStr = (type == 0) ? QString(DongleVer) : QString(EarVer);
            currentVerStr = currentVerStr.split('_').last();
            uVersion->UpdateVer(currentVerStr);
            uVersion->UpdateTitle(1);
        }
        uVersion->exec();
    }
}
//处理服务器返回的“成功”响应
void VersionSettingsMainPage::handleApiSuccess(const QJsonObject &rootObj, int type, bool en, const QString &readlastVer)
{
    QJsonObject dataObj = rootObj["data"].toObject();
    QString downloadUrl = dataObj["download_url"].toString();
    serverOTAVerStr = dataObj["version"].toString();

    // QString VerExplanation = dataObj["release_notes"].toString(); // 未使用，注释掉

    if (downloadUrl.isEmpty() || serverOTAVerStr.isEmpty()) {
        qWarning() << "API返回数据缺少 download_url 或 version";
        return;
    }

    qDebug() << "获得服务器中版本号:" << serverOTAVerStr;

    // 处理不同的升级场景
    if (!readlastVer_TX.isEmpty() && en) {
        //用户手动点击检查更新（en=true）且已有旧版本号
        int res = uOTAFind->exec();

        if (res == QDialog::Accepted) {
            startDownloadByType(type, downloadUrl);
        }
        return;
    }

    if (UpdateTP1 && UpdateTP1En) {
        // TP1 升级流程
        // if (type == 1) { // RX 升级前保存版本
        //     if (!QString(DongleVer).split('_').last().isEmpty())
        //         globalSettings->setValue("OTA/TX", QString(DongleVer).split('_').last());
        //     if (!QString(EarVer).split('_').last().isEmpty())
        //         globalSettings->setValue("OTA/RX", QString(EarVer).split('_').last());
        //     globalSettings->sync();
        // }
        startDownloadByType(type, downloadUrl);
        return;
    }

    // 正常版本比较
    QString currentVerStr = (type == 0) ? QString(DongleVer) : QString(EarVer);
    currentVerStr = currentVerStr.split('_').last(); // 取日期部分


    // 保存当前版本字符串（用于后续）
    if (type == 0)
        lastVerStr_TX = currentVerStr;
    else if (type == 1)
        lastVerStr_RX = currentVerStr;

    qDebug() << "纯版本号:" << currentVerStr;

    // currentVerStr = "";//测试

    if (serverOTAVerStr.toInt() > currentVerStr.toInt()) {
        // 有新版本
        if (en) {
            // 弹出确认对话框
            if(OTARollbackEn)
            {
                uOTAFind->updateTitle(0,1,serverOTAVerStr);//后续第一个参数要改为主题全局变量
            }else
            {
                uOTAFind->updateTitle(0,0,serverOTAVerStr);//后续第一个参数要改为主题全局变量
            }

            uOTAFind->startTimer();
            int result = uOTAFind->exec();


            if (result == QDialog::Accepted) {
                // 用户确认升级
                if (UpdateTP1 && !UpdateTP1En) {
                    UpdateTP1En = true;
                    iDevIndex = 0;
                    GetOTAFile(0, true); // 递归调用，但会走 TP1 分支
                    return;
                }
                startDownloadByType(type, downloadUrl);
            }
        } else {
            // en=false 自动升级
            startDownloadByType(type, downloadUrl);
        }
    } else {
        // 当前版本已是新版本或等于服务器版本
        qDebug() << "已是最新版本";
        handleVersionComparison(type, currentVerStr, en);
    }
}
//判断服务器版本是否高于当前版本，并驱动更新状态
void VersionSettingsMainPage::handleVersionComparison(int type, const QString &currentVerStr, bool en)
{
    if(OTARollbackEn)
    {
       uota->setProgressTxt(tr("回退中"));
    }else
    {
        uota->setProgressTxt(tr("升级中"));
    }

    uota->SetTitleAndVer(OTARollbackEn,serverOTAVerStr);
    if (type == 0) {
        SkipUpdateTx = true;
        UpdateTxDone = true;
        if (!UpdateRxDone) {
            iDevIndex = 1;
            GetOTAFile(1, true);
            return;
        }else if(SkipUpdateTx && SkipUpdateRx)
        {
            if (en) {
                uVersion->UpdateVer(currentVerStr);
                uVersion->UpdateTitle(1);
                uVersion->exec();
                if (uota->isVisible())
                    uota->close();
            } else {
                uota->updateSuccess();
            }
        } else if (UpdateRxDone && UpdateTxDone) {
            uota->updateSuccess();
            return;
        }
    } else { // type == 1
        SkipUpdateRx = true;
        UpdateRxDone = true;
        if (!UpdateTxDone) {
            iDevIndex = 0;
            GetOTAFile(0, true);
            return;
        } else if (UpdateRxDone && UpdateTxDone) {
            uota->updateSuccess();
            return;
        }

        // 只有 type==1 且 en=true 时弹出“已是最新版本”对话框
        if (en) {


            uVersion->UpdateVer(currentVerStr);
            uVersion->UpdateTitle(1);
            uVersion->exec();


            if (uota->isVisible())
                uota->close();
        } else {
            uota->updateSuccess();
        }
    }
}

//根据地址下载
void VersionSettingsMainPage::startDownloadByType(int type, const QString &downloadUrl)
{
    qDebug() << "下载地址:" << downloadUrl;
    if (type == 0)
        startOTADownloadTX(downloadUrl);
    else if (type == 1)
        startOTADownloadRX(downloadUrl);
}


//开始Bin文件下载
void VersionSettingsMainPage::startBinDownload(const QString &urlString,
                               QFile* &filePtr,
                               const QString &filePath,
                               std::function<void(QNetworkReply*)> finishCallback)
{
    // 1. 确保文件对象存在并打开
    if (!filePtr) {
        filePtr = new QFile(filePath, this);
    }
    if (!filePtr->open(QIODevice::WriteOnly)) {
        // showDownloadError(0);
        filePtr->remove();
        delete filePtr;
        filePtr = nullptr;
        return;
    }

    // 2. 准备网络管理器
    if (!m_manager_exe) {
        m_manager_exe = new QNetworkAccessManager(this);
    }

    QUrl url(urlString);
    if (!url.isValid()) {
        showDownloadError(0);
        filePtr->close();
        filePtr->remove();
        delete filePtr;
        filePtr = nullptr;
        return;
    }

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = m_manager_exe->get(request);




    // 4. 连接下载进度
    connect(reply, &QNetworkReply::downloadProgress, uota,
            [this](qint64 bytesReceived, qint64 bytesTotal) {
                if (uota && bytesTotal > 0) {
                    uota->ShowProgress(static_cast<int>((bytesReceived * 100) / bytesTotal));
                }
            });

    // 5. 连接数据写入
    connect(reply, &QNetworkReply::readyRead, this, [reply, &filePtr]() {
        if (filePtr && filePtr->isOpen()) {
            filePtr->write(reply->readAll());
        }
    });

    // 6. 处理下载完成
    connect(reply, &QNetworkReply::finished, this, [this, reply, &filePtr, finishCallback]() {


        // 关闭进度对话框
        if (uota && uota->isVisible()) {
            uota->accept();
        }

        bool success = (reply->error() == QNetworkReply::NoError);
        if (success) {
            // 成功：关闭文件，执行回调
            if (filePtr) {
                filePtr->close();
                delete filePtr;
                filePtr = nullptr;
            }
            if (finishCallback) {
                finishCallback(reply);
            }
        } else {
            // 失败（非取消）
            if (reply->error() != QNetworkReply::OperationCanceledError) {
                if (filePtr) {
                    filePtr->close();
                    filePtr->remove();
                    delete filePtr;
                    filePtr = nullptr;
                }
                showDownloadError(0);
            } else {
                // 用户取消：清理文件
                if (filePtr) {
                    filePtr->close();
                    filePtr->remove();
                    delete filePtr;
                    filePtr = nullptr;
                }
            }
        }

        reply->deleteLater();
    });

    // 7. 模态执行（等待用户操作或下载完成）
    uota->exec();

}

// 下载耳机bin文件
void VersionSettingsMainPage::startOTADownloadRX(const QString &urlString)
{
    // startBinDownload(urlString, file_RX, DfilePath_RX,
    //                  [this](QNetworkReply* reply) {
    //                      onDownloadFinishedRX(reply);
    //                  });
    startGenericDownload(urlString, file_RX, DfilePath_RX,
                         [this](QNetworkReply* reply) {
                             onDownloadFinishedRX(reply);
                         }, true);
}
//下载dongle bin文件
void VersionSettingsMainPage::startOTADownloadTX(const QString &urlString)
{
    // startBinDownload(urlString, file_TX, DfilePath_TX,
    //                  [this](QNetworkReply* reply) {
    //                      onDownloadFinishedTX(reply);
    //                  });
    startGenericDownload(urlString, file_TX, DfilePath_TX,
                         [this](QNetworkReply* reply) {
                             onDownloadFinishedTX(reply);
                         }, true);
}
// RX 完成回调
void VersionSettingsMainPage::onDownloadFinishedRX(QNetworkReply *reply)
{
    handleDownloadFinished(reply, DfilePath_RX, 1); // RX 对应参数 1
}
// TX 完成回调
void VersionSettingsMainPage::onDownloadFinishedTX(QNetworkReply *reply)
{
    handleDownloadFinished(reply, DfilePath_TX, 0); // TX 对应参数 0
}
// 下载后bin文件后，进行固件更新
void VersionSettingsMainPage::handleDownloadFinished(QNetworkReply *reply,
                                     const QString &filePath,
                                     int otaParam)
{
    if (!reply) return;

    // 错误处理
    if (reply->error() != QNetworkReply::NoError) {
        if (reply->error() != QNetworkReply::OperationCanceledError) {
            qDebug() << "下载失败:" << reply->errorString();
            // showDownloadError(0);
        }
        reply->deleteLater();
        return;
    }

    // HTTP 状态码检查
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode >= 400) {
        qDebug() << "HTTP错误:" << statusCode;
        // showDownloadError(0);
        reply->deleteLater();
        return;
    }

    qDebug() << "文件下载成功:" << filePath;

    // 显示 OTA 更新界面
    if(!uota->isVisible())
    {
        uota->setVisible(true);
    }
    if(OTARollbackEn)
    {
        uota->setProgressTxt(tr("回退中"));
    }else
    {
        uota->setProgressTxt(tr("升级中"));
    }


    uota->runOTAFile(filePath, otaParam, "");
    uota->exec();

    reply->deleteLater();
}


//下载固件的bin文件
void VersionSettingsMainPage::startGenericDownload(
    const QString &urlString,
    QFile *&filePtr,
    const QString &filePath,
    std::function<void(QNetworkReply*)> onSuccess,
    bool isOta)   // 区分软件/固件，用于错误提示
{
    if (filePtr) {
        filePtr = nullptr;          // 立即置空，防止后续误用
        filePtr = new QFile(filePath);
    } else {
        filePtr = new QFile(filePath);
    }
    // if (!filePtr) {
    //     filePtr = new QFile(filePath);
    // }
    if (!filePtr->open(QIODevice::WriteOnly)) {
        qWarning() << "无法创建文件:" << filePtr->errorString();
        showDownloadError(isOta ? 1 : 0);
        delete filePtr;
        filePtr = nullptr;
        return;
    }

    if (!m_manager_exe) {
        m_manager_exe = new QNetworkAccessManager(this);
        m_manager_exe->setTransferTimeout(60000);
    }

    QUrl url(urlString);
    if (!url.isValid()) {
        showDownloadError(isOta ? 1 : 0);
        filePtr->close();
        filePtr->remove();
        delete filePtr;
        filePtr = nullptr;
        return;
    }

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = m_manager_exe->get(request);

    //进度对话框
    uota->setProgressTxt(tr("下载中"));
    uota->SetTitleAndVer(OTARollbackEn,serverOTAVerStr);
    connect(reply, &QNetworkReply::downloadProgress, uota,
            [this](qint64 received, qint64 total) {
                if (uota && total > 0)
                    uota->ShowProgress(static_cast<int>((received * 100) / total));
            });

    connect(reply, &QNetworkReply::readyRead, this, [reply, filePtr]() {
        if (filePtr && filePtr->isOpen())
            filePtr->write(reply->readAll());
    });

    connect(reply, &QNetworkReply::finished, this,
            [this, reply, filePtr, onSuccess, isOta]() mutable {

                // if (uota && uota->isVisible())
                //     uota->accept();

                bool ok = (reply->error() == QNetworkReply::NoError);
                if (ok) {
                    if (filePtr) {
                        filePtr->close();
                        delete filePtr;
                        filePtr = nullptr;
                    }
                    if (onSuccess)
                        onSuccess(reply);
                } else {
                    if (reply->error() != QNetworkReply::OperationCanceledError) {
                        if (filePtr) {
                            filePtr->close();
                            filePtr->remove();
                            delete filePtr;
                            filePtr = nullptr;
                        }
                        if (uota && uota->isVisible())
                            uota->accept();
                        showDownloadError(isOta ? 1 : 0);
                    } else {
                        // 用户取消，清理文件
                        if (filePtr) {
                            filePtr->close();
                            filePtr->remove();
                            delete filePtr;
                            filePtr = nullptr;
                        }
                    }
                }
                reply->deleteLater();
            });

    uota->exec();
    // 模态执行
    // int res = uota->exec();
    // if (res == QDialog::Rejected) {
    //     if (reply && reply->isRunning())
    //         reply->abort();
    //     if (filePtr) {
    //         filePtr->close();
    //         filePtr->remove();
    //         delete filePtr;
    //         filePtr = nullptr;
    //     }

    // }
}
//固件回退
void VersionSettingsMainPage::on_pBt_Rollback_clicked()
{
    OTARollbackEn = true;
    //保存升级前版本
    // globalSettings->setValue("OTA/RX",lastVerStr_RX);
    // globalSettings->setValue("OTA/TX",lastVerStr_TX);

    readlastVer_RX = globalSettings->value("OTA/RX","").toString();
    readlastVer_TX = globalSettings->value("OTA/TX","").toString();
    //这个3.0版本后续要修改，这里不正确
    if(SelDev_DeviceName.contains("T10",Qt::CaseInsensitive) && SelDev_DeviceName.contains("Wireless",Qt::CaseInsensitive))
    {
        TP1TxVer = "260622";
        TP1RxVer = "260622";
    }else if(SelDev_DeviceName.contains("K06S",Qt::CaseInsensitive))
    {
        TP1TxVer = "260622";
        TP1RxVer = "260622";
    }else if(SelDev_DeviceName.contains("T7",Qt::CaseInsensitive) && (SelDev_PID == 0xF014 || SelDev_PID == 0xF008))
    {
        TP1TxVer = "260622";
        TP1RxVer = "260622";
    }

    // readlastVer_RX = "260302";
    // readlastVer_TX = "260305";

    if(readlastVer_RX.isEmpty() || readlastVer_TX.isEmpty()
        || readlastVer_TX.toInt() < TP1TxVer.toInt()
        || readlastVer_RX.toInt() < TP1RxVer.toInt())
    {
        uVersion->UpdateVer(readlastVer_RX);
        uVersion->UpdateTitle(4);
        uVersion->exec();

        return;
    }

    if(readlastVer_TX.toInt() != QString(DongleVer).split('_').last().toInt())
    {
        iDevIndex = 0;
        GetOTAFile(0,true);
    }else if(readlastVer_RX.toInt() != QString(EarVer).split('_').last().toInt())
    {
        iDevIndex = 1;
        GetOTAFile(1,true);
    }else
    {
        uVersion->UpdateVer(readlastVer_RX);
        uVersion->UpdateTitle(3);
        uVersion->exec();
    }

}

//设置固件升级是否可用（只有T10和K06S、T7，三款无线耳机可用）
void VersionSettingsMainPage::SetOTAEn(bool en)
{
    ui->pBt_OTA->setEnabled(en);
    ui->pBt_Rollback->setEnabled(en);
}

void VersionSettingsMainPage::setDeviceIconPixmap(const QPixmap &t_pixmap)
{
    if (t_pixmap.isNull())
        return;

    ui->label_deivce_icon->clear();
    QPixmap t_scaled = t_pixmap.scaled(ui->label_deivce_icon->size(),
                                       Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation);
    ui->label_deivce_icon->setPixmap(t_scaled);
}
