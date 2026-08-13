#include "LoginAndActivationCode.h"
#include <QAction>
#include <QGraphicsDropShadowEffect>
#include <QToolButton>
#include "APOThread/ApoManager.h"

#include "data/api_global.h"
#include "network/http_client.h"
#include "network/auth_store.h"  ///< 社区模块 token 同步
#include "network/request_options.h"
#include "modules/CommunityModule/infrastructure/logger/logger.h"  ///< LOG_INFO/LOG_WARN

/// \brief 网络详情日志的请求体脱敏：Release 打码 / Debug 全量（登录/注册等含密码/手机号，禁止裸打）
static QString logSafeBody(const QByteArray &t_body)
{
#ifdef NDEBUG
    Q_UNUSED(t_body);
    return QStringLiteral("<redacted>");
#else
    return QString::fromUtf8(t_body);
#endif
}

static void initUserLocalDataAfterLogin(); ///< 登录后初始化/恢复用户本地数据
#include "LoadApoDLL.h"
#include "LoadLib.h"
#include "Popup/WeChatQRCode/WeChatCode.h"
#include "ui_LoginAndActivationCode.h"

#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkProxy>
#include <QDir>
#include <QStandardPaths>
#include <QUrlQuery>
#include <QVBoxLayout>
//#include <QThread>

WeChatCode *VXCode;
int SceneType = 0;            //0:注册用户   1：找回密码   2：修改密码

UserInformation g_user_information; ///< 用户信息
ApiServerSwitch g_api_server_switch; ///< API 服务器开关

QMovie *movie;

QString DevId;   //用户反馈-设备ID
QString DriId;   //用户反馈-驱动ID
QString FWId;    //用户反馈-固件ID
QString DevType; //用户反馈-设备类型（耳机）

LoginAndActivationCode::LoginAndActivationCode(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginAndActivationCode)
{
    ui->setupUi(this);
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽
}

LoginAndActivationCode::~LoginAndActivationCode()
{
    delete ui;
}

/// \brief 重新登录 — 复位所有登录状态和数据
void LoginAndActivationCode::reLogin()
{
    hideLoginLoading();

    // 复位登录标记
    isLogin = false;

    // 清除用户网络信息（存盘已由 MainWindow 退出登录完成，此处仅清内存）
    g_user_information.network.access_token.clear();
    AuthStore::instance().clear(); ///< 清空新栈 token 缓存
    g_user_information.network.refresh_token.clear();
    g_user_information.network.id.clear();
    g_user_information.network.username.clear();
    g_user_information.network.email.clear();
    g_user_information.network.nickname.clear();
    g_user_information.network.avatar.clear();
    g_user_information.network.status.clear();
    g_user_information.network.login_type.clear();
    g_user_information.network.created_at.clear();
    g_user_information.network.last_login_at.clear();
    g_user_information.network.favorite_games.clear();
    g_user_information.network.activation_code.clear();
    g_user_information.network.city.clear();
    g_user_information.network.login_ip.clear();
    g_user_information.network.os_info.clear();
    g_user_information.network.bio.clear();

    // 复位本地状态
    g_user_information.local.is_get_userInfo_first.store(true);
    g_user_information.local.user_psw.clear();

    // 清除界面文本
    clearText();
    showError(false);
    showErrorFAR(false);
    showAError(false, QString());

    // 停止倒计时、复位
    if (m_timer) {
        m_timer->stop();
        Countdown = 60;
    }
    showTimer(false);
    ui->lab_timer->setText(QStringLiteral("60S"));
    ui->pBt_SendVerifyCode_FAR->setEnabled(true);
    ui->lEdit_password->setEchoMode(QLineEdit::Password);
    ui->lEdit_oldpsw_FAR->setEchoMode(QLineEdit::Password);
    ui->lEdit_newpsw_FAR->setEchoMode(QLineEdit::Password);
    if (lEdit_password_Action)
        lEdit_password_Action->setIcon(QIcon(":/Skin/Images/Login/v_2_0/invisible_normal.png"));
    if (lEdit_oldpsw_FAR_Action)
        lEdit_oldpsw_FAR_Action->setIcon(QIcon(":/Skin/Images/Login/v_2_0/invisible_normal.png"));
    if (lEdit_newpsw_FAR_Action)
        lEdit_newpsw_FAR_Action->setIcon(QIcon(":/Skin/Images/Login/v_2_0/invisible_normal.png"));
    ui->stackedWidget->setCurrentWidget(ui->page_Login);

    // 通知 MainWindow 切换到登录页
    emit LoginAgain();
}

// 辅助函数：将 QPixmap 转换为指定直径的圆形图片
QPixmap getCircularPixmap(const QPixmap &src, int diameter)
{
    if (src.isNull()) {
        return QPixmap();
    }

    // 1. 获取图片中心的正方形区域（取 min(宽,高)）
    int srcSize = qMin(src.width(), src.height());
    QRect squareRect((src.width() - srcSize) / 2, (src.height() - srcSize) / 2, srcSize, srcSize);
    QPixmap squarePix = src.copy(squareRect);

    // 2. 缩放至目标直径大小
    QPixmap scaledPix = squarePix.scaled(diameter,
                                         diameter,
                                         Qt::IgnoreAspectRatio,
                                         Qt::SmoothTransformation);

    // 3. 创建圆形图片
    QPixmap circularPix(diameter, diameter);
    circularPix.fill(Qt::transparent); // 透明背景
    QPainter painter(&circularPix);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(scaledPix));
    painter.setPen(Qt::NoPen); // 无边框
    painter.drawEllipse(0, 0, diameter, diameter);
    painter.end();

    return circularPix;
}

void LoginAndActivationCode::hideLoginLoading()
{
    if (clp_login_loading_) {
        clp_login_loading_->stop();
        clp_login_loading_->hide();
    }
}

void LoginAndActivationCode::clearText()
{
    ui->lEdit_email->clear();
    ui->lEdit_password->clear();
    ui->lEdit_ACode->clear();
    ui->lEdit_email_FAR->clear();
    ui->lEdit_oldpsw_FAR->clear();
    ui->lEdit_newpsw_FAR->clear();
    ui->lEdit_verifyCode_FAR->clear();

    user_psw_old.clear();
    user_psw_new.clear();
    user_verifyCode.clear();
}

//倒计时，发送按钮显示
void LoginAndActivationCode::showTimer(bool en)
{
    if (en) {
        ui->widget_timer->show();
        ui->pBt_SendVerifyCode_FAR->hide();
    } else {
        ui->widget_timer->hide();
        ui->pBt_SendVerifyCode_FAR->show();
    }
}
//显示忘记密码、注册错误
void LoginAndActivationCode::showErrorFAR(bool en)
{
    if (en) {
        ui->lab_error_FAR_E->show();
        ui->lab_error_FAR_O->show();
        ui->lab_error_FAR_N->show();
        ui->lab_error_FAR_C->show();
    } else {
        ui->lab_error_FAR_E->hide();
        ui->lab_error_FAR_O->hide();
        ui->lab_error_FAR_N->hide();
        ui->lab_error_FAR_C->hide();
    }
}

//显示邮箱登录错误
void LoginAndActivationCode::showError(bool en)
{
    if (en) {
        ui->lab_error_p->show();
        ui->lab_error->show();
    } else {
        ui->lab_error_p->hide();
        ui->lab_error->hide();
    }
}
//显示激活码错误
void LoginAndActivationCode::showAError(bool en, QString text)
{
    if (en) {
        ui->lab_AError->setText(text);
        ui->lab_AError->show();
    } else {
        ui->lab_AError->hide();
    }
}
void LoginAndActivationCode::En_pBt_Activate()
{
    ui->pBt_Activate->setEnabled(true);
}
//激活
void LoginAndActivationCode::on_pBt_Activate_clicked()
{
    // ui->pBt_Activate->setEnabled(false);
    // user_ActivationCode = ui->lEdit_ACode->text();
    // emit ApoManager::instance()->requestActivateAsync(user_ActivationCode);
}
//获得微信登录二维码(POST)
void LoginAndActivationCode::GetWechatCode()
{
    // 构建 JSON 请求体
    QJsonObject json;
    json["redirect_url"] = "https://hub.xiberia.net";
    QByteArray requestBody = QJsonDocument(json).toJson();

    //***********后期删-测试用***********
    QString requestId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    // 记录开始时间
    QElapsedTimer timer;
    timer.start();

    // 记录请求时间字符串（精确到毫秒）
    QString requestTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

    // 发送 POST 请求
    QNetworkReply *reply = HttpClient::instance().post("/wechat/oauth/preauth",
        RequestOptions{}.withBody(QJsonDocument(json).toJson()).withTag("wechatOauth").noAuth());

    connect(reply,
            &QNetworkReply::finished,
            this,
            [this, reply, requestTime, timer, requestBody, requestId]() {
                //***********后期删-测试用***********
                // 计算耗时（毫秒）
                qint64 elapsedMs = timer.elapsed();
                onReplyFinished(reply, requestTime, timer, requestBody, elapsedMs, requestId);

                // onReplyFinished(reply);
            });
}
//申请微信二维码POST回应
// void LoginAndActivationCode::onReplyFinished(QNetworkReply *reply)
void LoginAndActivationCode::onReplyFinished(QNetworkReply *reply,
                                             QString requestTime,
                                             QElapsedTimer timer,
                                             QByteArray requestBody,
                                             qint64 elapsedMs,
                                             QString requestId)
{
    // 错误处理
    if (reply->error() != QNetworkReply::NoError) {
        // QString errorMsg = "网络错误: " + reply->errorString();
        QString errorMsg = "网络请求失败，错误码: " + QString::number(reply->error());

        //***********后期删-测试用***********
        // 获取请求接口（URL）
        QString api = reply->request().url().toString();

        // 获取请求头（从原始请求中提取）
        QList<QByteArray> reqHeaders = reply->request().rawHeaderList();
        QString requestHeadersStr;
        for (const QByteArray &header : reqHeaders) {
            requestHeadersStr += QString("%1: %2\n")
                                     .arg(header.constData(),
                                          reply->request().rawHeader(header).constData());
        }

        // 获取响应状态码
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        // 获取响应头
        QList<QByteArray> respHeaders = reply->rawHeaderList();
        QString responseHeadersStr;
        for (const QByteArray &header : respHeaders) {
            responseHeadersStr += QString("%1: %2\n")
                                      .arg(header.constData(), reply->rawHeader(header).constData());
        }

        // 获取响应体（必须在任何可能导致数据被消耗的操作之前读取）
        QByteArray responseBody = reply->readAll();
        QString responseBodyStr = QString::fromUtf8(responseBody); // 假设响应为 UTF-8 文本

        // //获得代理信息
        // QNetworkProxy proxy = QNetworkProxy::applicationProxy();
        // QString msg = QString("Current proxy type: %1, host: %2, port: %3")
        //                   .arg(proxy.type())
        //                   .arg(proxy.hostName())
        //                   .arg(proxy.port());
        // apo->logWithTime(msg);

        // 构建详细日志
        QString logMsg = QString("\n========== 申请微信登录网络请求详情 ==========\n"
                                 "请求时间: %1\n"
                                 "Request-ID: %2\n"
                                 /*"接口: %3\n"*/
                                 "耗时: %3 ms\n"
                                 "HTTP状态码: %4\n"
                                 "-------- 请求头 --------\n%5"
                                 "-------- 请求体 --------\n%6\n"
                                 "-------- 响应头 --------\n%7"
                                 "-------- 响应体 --------\n%8\n"
                                 "错误描述：%9\n"
                                 "==================================\n")
                             .arg(requestTime,
                                  requestId /*, api*/,
                                  QString::number(elapsedMs),
                                  QString::number(statusCode),
                                  requestHeadersStr,
                                  logSafeBody(requestBody),
                                  responseHeadersStr,
                                  responseBodyStr,
                                  reply->errorString());
        emit ApoManager::instance()->requestlogWithTime(QString("申请微信二维码: %1").arg(logMsg));

        // 确保 reply 被正确释放
        reply->deleteLater();

        CheckServerMaintenanceSta(errorMsg);
        // msgBox.critical(NULL,tr("错误"),errorMsg);

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

    // 检查业务状态码
    QString code = rootObj["code"].toString();
    if (code != "success") {
        QString message = rootObj["message"].toString();
        QString errorMsg = QString("API error: code=%1, message=%2").arg(code, message);
        qWarning() << errorMsg;
        // emit driveInfoReceived(QString(), QString(), errorMsg);
        return;
    }

    // 提取 data 字段
    QJsonObject dataObj = rootObj["data"].toObject();

    QString app_id = dataObj["app_id"].toString();
    QString redirect_uri = dataObj["redirect_uri"].toString();
    QString scope = dataObj["scope"].toString();

    QString downloadUrl = dataObj["auth_url"].toString();
    evidence = dataObj["state"].toString();

    qDebug() << "获得微信应用ID:" << app_id;
    qDebug() << "获得微信回调地址:" << redirect_uri;
    qDebug() << "获得微信授权作用域:" << scope;

    qDebug() << "获得微信二维码:" << downloadUrl;
    qDebug() << "获得evidence:" << evidence;

    // 确保 reply 被正确释放
    reply->deleteLater();

    emit ApoManager::instance()->requestlogWithTime("new WeChatCode()");

    // VXCode = new WeChatCode(m);
    // connect(VXCode,&WeChatCode::ReadLoginRequest,this,[this]{
    //     emit ApoManager::instance()->requestlogWithTime("new WeChatCode() ReadLoginRequest");
    //     LoginRequest();
    // });
    // VXCode->ShowCode(downloadUrl);

    // VXCode->exec();
    // emit ApoManager::instance()->requestlogWithTime("new WeChatCode() VXCode exec");
    VXCode = new WeChatCode(m);
    VXCode->setAttribute(Qt::WA_DeleteOnClose);
    VXCode->setWindowModality(Qt::ApplicationModal); // 如果需要模态行为

    connect(VXCode, &WeChatCode::ReadLoginRequest, this, [this] {
        emit ApoManager::instance()->requestlogWithTime("new WeChatCode() ReadLoginRequest");
        LoginRequest();
    });

    VXCode->ShowCode(downloadUrl);
    VXCode->show(); // 不阻塞主线程
}
//判断微信登录状态
void LoginAndActivationCode::LoginRequest()
{
    QUrlQuery query;
    query.addQueryItem("evidence", evidence); // 使用正确的参数名

    //发送GET请求
    QNetworkReply *reply3 = HttpClient::instance().get("/wechat/login-status",
        RequestOptions{}.withQuery(query).withTag("wechatOauth").noAuth());
    emit ApoManager::instance()->requestlogWithTime("new WeChatCode() ReadLoginRequest get");
    // 连接 finished 信号
    connect(reply3, &QNetworkReply::finished, this, [this, reply3]() {
        // 检查错误
        if (reply3->error() != QNetworkReply::NoError) {
            // qWarning() << "获得微信用户信息失败:" << reply3->errorString();

            emit ApoManager::instance()->requestlogWithTime(
                QString("new WeChatCode() ReadLoginRequest get error:%1").arg(reply3->errorString()));
            CheckServerMaintenanceSta(QString::number(reply3->error()));
            // msgBox.critical(NULL,tr("获得微信用户信息失败:"),QString::number(reply3->error()));
            reply3->deleteLater();
            return;
        }

        // 读取响应数据
        QByteArray responseData = reply3->readAll();

        // 解析 JSON
        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            QString errorMsg = "JSON解析错误: " + parseError.errorString();
            qWarning() << errorMsg;
            emit ApoManager::instance()->requestlogWithTime(
                QString("new WeChatCode() ReadLoginRequest get JSON error:%1").arg(errorMsg));
            reply3->deleteLater();
            return;
        }

        QJsonObject rootObj = jsonDoc.object();

        // 检查业务状态码
        QString code = rootObj["code"].toString();
        if (code != "success") {
            QString message = rootObj["message"].toString();
            QString errorMsg = QString("API error: code=%1, message=%2").arg(code, message);
            qWarning() << errorMsg;
            emit ApoManager::instance()->requestlogWithTime(
                QString("new WeChatCode() ReadLoginRequest get code error:%1").arg(errorMsg));
            reply3->deleteLater();
            return;
        }

        // 提取 data 字段
        QJsonObject dataObj = rootObj["data"].toObject();
        QString status = dataObj["status"].toString();
        if (status == "pending") {
            qDebug("微信1等待中\n");
        } else if (status == "expired") {
            qDebug("微信1二维码过期\n");
        } else if (status == "success") {
            emit ApoManager::instance()->requestlogWithTime(
                "new WeChatCode() LoginRequest get login success");
            // qDebug("微信1成功\n");
            if (VXCode) {
                VXCode->close();
                VXCode->deleteLater();
                VXCode = nullptr; // 避免野指针
            }

            handleThirdPartyLoginSuccess(dataObj); ///< 公共成功处理（与 Google 登录共用）

        } else {
            qDebug("微信1什么也不是\n");
        }

        reply3->deleteLater();
    });
}

// 第三方登录成功公共处理（微信/Google 共用）：token 保存 → 用户信息 → 持久化 → 头像 → 进入主页
void LoginAndActivationCode::handleThirdPartyLoginSuccess(const QJsonObject &dataObj)
{
    g_user_information.network.access_token = dataObj["access_token"].toString();
    AuthStore::instance().setToken(g_user_information.network.access_token);
    g_user_information.network.refresh_token = dataObj["refresh_token"].toString();

    isLogin = true; ///< 登录成功后立即标记已登录，避免 LoginInEn 误判

    // 用 login 响应中的 user 对象填充字段（微信 14 字段 / Google 简化 5 字段，缺失留空）
    {
        QJsonObject userObj = dataObj["user"].toObject();
        DeSheng::UserLoginResponse::ReturnData::UserInfo t_user;
        t_user.id = userObj["id"].toString();
        t_user.username = userObj["username"].toString();
        t_user.email = userObj["email"].toString();
        t_user.nickname = userObj["nickname"].toString();
        t_user.avatar = userObj["avatar"].toString();
        t_user.status = userObj["status"].toString();
        t_user.login_type = userObj["login_type"].toString();
        t_user.created_at = userObj["created_at"].toString();
        t_user.last_login_at = userObj["last_login_at"].toString();
        t_user.favorite_games = userObj["favorite_games"].toString();
        t_user.activation_code = userObj["activation_code"].toString();
        t_user.city = userObj["city"].toString();
        t_user.login_ip = userObj["login_ip"].toString();
        t_user.os_info = userObj["os_info"].toString();
        g_user_information.updateFromServer(t_user);
    }

    QString avatar = g_user_information.network.avatar;
    emit ApoManager::instance()->requestlogWithTime(
        QString("ThirdPartyLogin avatar: %1").arg(avatar));

    // 持久化登录状态
    globalSettings->setValue("Login/en", true);                                 //已登录
    globalSettings->setValue("Login/type", g_user_information.network.login_type); //登录类型
    globalSettings->setValue("Login/nickname", g_user_information.network.username); //用户名
    globalSettings->setValue("Login/id", g_user_information.network.id);           //用户ID(唯一且不可更改)

    QVariantMap map;
    map["user_email"] = g_user_information.network.email;
    map["user_psw"] = g_user_information.local.user_psw;
    map["access_token"] = g_user_information.network.access_token;             //访问令牌
    globalSettings->setValue("Login/Account", map); //用户ID(唯一且不可更改)

    // 下载头像 → 显示 → 拉取完整用户信息 → 进入主页
    auto *t_nam = new QNetworkAccessManager(this);
    t_nam->setTransferTimeout(60000);
    QNetworkReply *reply2 = t_nam->get(QNetworkRequest(QUrl(avatar)));

    connect(reply2, &QNetworkReply::finished, this, [this, reply2, t_nam]() {
        t_nam->deleteLater();
        if (reply2->error() != QNetworkReply::NoError) {
            qWarning() << "下载图片失败:" << reply2->errorString();
            emit ApoManager::instance()->requestlogWithTime(
                QString("save png error: %1").arg(reply2->errorString()));
            reply2->deleteLater();
            return;
        }

        QByteArray imageData = reply2->readAll();
        QPixmap pixmap_Avatar;
        if (!pixmap_Avatar.loadFromData(imageData)) {
            qWarning() << "无法从数据加载图片";
            emit ApoManager::instance()->requestlogWithTime("save png loadFromData error");
            reply2->deleteLater();
            return;
        }

        // 缩放并保存头像至用户目录（160×160）
        QPixmap t_saved = pixmap_Avatar.scaled(160, 160,
                                               Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation);
        {
            QString t_user_dir = g_user_information.userDirName();
            QDir().mkpath(t_user_dir);
            QString t_file_path = g_user_information.avatarFilePath();
            t_saved.save(t_file_path, "PNG");
            emit ApoManager::instance()->requestlogWithTime("save avatar ok: " + t_file_path);
        }

        // 缩放至标签尺寸显示
        QPixmap t_display = pixmap_Avatar.scaled(ui->lab_Avatar->size(),
                                                 Qt::KeepAspectRatio,
                                                 Qt::SmoothTransformation);
        ui->lab_Avatar->setStyleSheet("");
        ui->lab_Avatar->setPixmap(t_display);

        t_display = pixmap_Avatar.scaled(ui->lab_Avatar_w->size(),
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);
        ui->lab_Avatar_w->setStyleSheet("");
        ui->lab_Avatar_w->setPixmap(t_display);

        // 设置用户名
        ui->lab_username->setText(g_user_information.network.username);
        ui->lab_username_w->setText(g_user_information.network.username);

        showWaitPage();
        reply2->deleteLater();

        // 获取当前登录用户完整信息
        if (g_user_information.local.is_get_userInfo_first) {
            QNetworkReply *t_reply = HttpClient::instance().get("/user",
                RequestOptions{}.withTag("user"));
            connect(t_reply, &QNetworkReply::finished, this, [this, t_reply]() {
                if (t_reply->error() == QNetworkReply::NoError) {
                    QJsonDocument t_doc = QJsonDocument::fromJson(t_reply->readAll());
                    DeSheng::GetCurrentUserResponse t_resp;
                    if (DeSheng::ProcessGetCurrentUserResult(t_resp, t_doc)) {
                        if (t_resp.code == "success") {
                            g_user_information.updateFromServer(t_resp.data);
                        }
                    }
                }

                initUserLocalDataAfterLogin();
                g_user_information.local.is_get_userInfo_first.store(false);
                // 更新 UI
                ui->lab_username->setText(g_user_information.network.username);
                ui->lab_username_w->setText(g_user_information.network.username);
                emit MainPageChange(); ///< GET /user 失败时也进入主页（login 响应已有基本信息）

                t_reply->deleteLater();
            });
        }
    });
}

//显示激活码页面头像
void LoginAndActivationCode::showAvatar(QPixmap pixmap_Avatar)
{
    ui->lab_username->setText(g_user_information.network.username);
    ui->lab_username_w->setText(g_user_information.network.username);

    if (g_user_information.network.login_type == "account") {
        //邮箱登录
        ui->lab_Avatar->setStyleSheet("border-radius:0px;border-image: "
                                      "url(:/Skin/Images/Login/Default Avatar.png);");
        ui->lab_Avatar_w->setStyleSheet("border-radius:0px;border-image: "
                                        "url(:/Skin/Images/Login/Default Avatar.png);");

    } else if (g_user_information.network.login_type == "wechat") {
        //微信登录
        ui->lab_Avatar->setStyleSheet("");
        // 缩放图片以适应标签，保持宽高比
        QSize labelSize = ui->lab_Avatar->size();
        if (labelSize.isValid() && !labelSize.isEmpty()) {
            pixmap_Avatar = pixmap_Avatar.scaled(labelSize,
                                                 Qt::KeepAspectRatio,
                                                 Qt::SmoothTransformation);
        }
        ui->lab_Avatar->setPixmap(pixmap_Avatar);

        ui->lab_Avatar_w->setStyleSheet("");
        ui->lab_Avatar_w->setPixmap(pixmap_Avatar);
    }
}

/*//是否显示激活码页面
void LoginAndActivationCode::ShowActivationCode()
{
    qDebug("int ShowActivationCode\n");
    isLogin = true;
    if(!IsActivated)
    {
        if(user_ActivationCode.isEmpty())
        {
            GetActivationCode();
        }else
        {
            emit ApoManager::instance()->requestActivateAsync(user_ActivationCode);
        }


    }else
    {
        emit MainPageChange();
    }
}*/
/*//上传激活码
void LoginAndActivationCode::UploadACode()
{
    qDebug("上传激活码\n");

    if (g_user_information.network.access_token.isEmpty()) {
        return;
    }
    if (!m_manager_login) {
        m_manager_login = new QNetworkAccessManager(this);

    }
    QString m_baseUrl = "https://hubsystest.xiberia.net//api/v1/user";

    // 1. 构建 URL（如果 baseUrl 不包含协议，可在此处补充，这里假设完整）
    QUrl url(m_baseUrl);

    // qDebug() << "获得微信二维码:" << url;

    // 创建请求（带token）
    QNetworkRequest request = createRequest(url);

    // user_ActivationCode = "SAVITEST66SAVITEST88";
    // 构建JSON数据
    QJsonObject json;
    if (!user_ActivationCode.isEmpty()) {
        json["activation_code"] = user_ActivationCode;
    }

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();



    // 输出 URL
    qDebug() << "Request URL:" << url.toString();

    // 输出请求头
    qDebug() << "Request headers:";
    const auto &headers = request.rawHeaderList();
    for (const QByteArray &header : headers) {
        qDebug() << header << ":" << request.rawHeader(header);
    }

    // 输出请求体（JSON 数据）
    qDebug() << "Request body:" << data;

    // 发送 PUT 请求
    QNetworkReply *reply = m_manager_login->put(request, data);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        // 错误处理
        if (reply->error() != QNetworkReply::NoError) {
            QString errorMsg = "上传激活码 网络错误: " + reply->errorString();
            qWarning() << errorMsg;
            // emit driveInfoReceived(QString(), QString(), errorMsg);
            // 确保 reply 被正确释放
            reply->deleteLater();
            return;
        }

        // 读取响应数据
        QByteArray responseData = reply->readAll();

        qDebug() << "Raw response:" << responseData;  // 添加这一行
        qDebug() << "Raw response (hex):" << responseData.toHex(); // 如果包含不可见字符

        // 解析 JSON
        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            QString errorMsg = "上传激活码 JSON解析错误: " + parseError.errorString();
            qWarning() << errorMsg;
            reply->deleteLater();
            return;
        }

        QJsonObject rootObj = jsonDoc.object();

        // 检查业务状态码
        QString code = rootObj["code"].toString();
        if (code != "success") {
            QString message = rootObj["message"].toString();
            QString errorMsg = QString("API error: code=%1, message=%2").arg(code, message);
            qWarning() << errorMsg;
            if(message.contains("激活码已被使用",Qt::CaseInsensitive))
            {
                msgBox.critical(NULL,tr("警告"),tr("该激活码已被别的账号绑定"));
            }
            reply->deleteLater();
            return;
        }
        qDebug("上传激活码成功\n");

        reply->deleteLater();
    });
}*/

//录入用户绑定设备
void LoginAndActivationCode::UpdateDev()
{
    emit ApoManager::instance()->requestlogWithTime("UpdateDev1");

    // 构建JSON数据
    QJsonObject json;
    json["user_id"] = g_user_information.network.id;
    json["device_name"] = SelDev_DeviceName;
    json["drive_version"] = SoftWareVer;

    // 发送 POST 请求
    QNetworkReply *reply_b = HttpClient::instance().post("/user-devices/bind",
        RequestOptions{}.withBody(QJsonDocument(json).toJson()).withTag("userDeviceLog"));
    emit ApoManager::instance()->requestlogWithTime("UpdateDev2");
    connect(reply_b, &QNetworkReply::finished, this, [this, reply_b]() {
        // 错误处理
        if (reply_b->error() != QNetworkReply::NoError) {
            QString errorMsg = "网络错误: "
                               + QString::number(reply_b->error()); //reply_b->errorString();

            qWarning() << errorMsg;
            // 确保 reply_b 被正确释放
            reply_b->deleteLater();
            emit ApoManager::instance()->requestlogWithTime(
                QString("UpdateDev3： %1").arg(reply_b->errorString()));
            CheckServerMaintenanceSta(errorMsg);
            // msgBox.critical(NULL,tr("错误"),errorMsg);
            return;
        }

        // 读取响应数据
        QByteArray responseData = reply_b->readAll();

        // 解析 JSON
        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            QString errorMsg = "JSON解析错误: " + parseError.errorString();
            qWarning() << errorMsg;
            emit ApoManager::instance()->requestlogWithTime(
                QString("UpdateDev4： %1").arg(errorMsg));
            reply_b->deleteLater();
            return;
        }

        QJsonObject rootObj = jsonDoc.object();

        // 检查业务状态码
        QString code = rootObj["code"].toString();
        if (code != "success") {
            QString message = rootObj["message"].toString();
            QString errorMsg = QString("绑定设备失败API error: code=%1, message=%2")
                                   .arg(code, message);
            qWarning() << errorMsg;
            emit ApoManager::instance()->requestlogWithTime(
                QString("UpdateDev5： %1").arg(errorMsg));
            reply_b->deleteLater();
            return;
        }

        // 提取 data 字段
        QJsonObject dataObj = rootObj["data"].toObject();
        DevId = dataObj["device_id"].toString();     //用户反馈-设备ID
        DriId = dataObj["drive_id"].toString();      //用户反馈-驱动ID(因服务器联动问题，该信息为空，需从上位机版本信息界面获取)
        FWId = dataObj["firmware_id"].toString();    //用户反馈-固件ID
        DevType = dataObj["device_type"].toString(); //用户反馈-设备类型（耳机）

        qDebug("绑定设备成功\n");
        emit ApoManager::instance()->requestlogWithTime("UpdateDev6：bind success");

        reply_b->deleteLater();
    });
}
//上报设备日志信息
void LoginAndActivationCode::GetDevMsg()
{
    emit ApoManager::instance()->requestlogWithTime("GetDevMsg0");

    emit ApoManager::instance()->requestlogWithTime("GetDevMsg1");

    // 构建JSON数据
    QJsonObject json;
    json["user_id"] = g_user_information.network.id;
    json["device_name"] = SelDev_DeviceName;
    json["device_type"] = "headset";

    // 发送 POST 请求
    QNetworkReply *reply_g = HttpClient::instance().post("/user-device-logs",
        RequestOptions{}.withBody(QJsonDocument(json).toJson()).withTag("userDeviceLog"));

    emit ApoManager::instance()->requestlogWithTime("GetDevMsg2");
    connect(reply_g, &QNetworkReply::finished, this, [this, reply_g]() {
        emit ApoManager::instance()->requestlogWithTime("GetDevMsg3 int");
        // 错误处理
        if (reply_g->error() != QNetworkReply::NoError) {
            QString errorMsg = "网络错误: "
                               + QString::number(reply_g->error()); //reply_g->errorString();
            qWarning() << errorMsg;
            // 确保 reply_g 被正确释放
            reply_g->deleteLater();
            CheckServerMaintenanceSta(errorMsg);
            //msgBox.critical(NULL,tr("错误"),errorMsg);
            emit ApoManager::instance()->requestlogWithTime(
                QString("GetDevMsg4: %1").arg(reply_g->errorString()));
            return;
        }

        // 读取响应数据
        QByteArray responseData = reply_g->readAll();

        // 解析 JSON
        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            QString errorMsg = "JSON解析错误: " + parseError.errorString();
            qWarning() << errorMsg;
            emit ApoManager::instance()->requestlogWithTime(QString("GetDevMsg5: %1").arg(errorMsg));
            reply_g->deleteLater();
            return;
        }

        QJsonObject rootObj = jsonDoc.object();

        // 检查业务状态码
        QString code = rootObj["code"].toString();
        if (code != "success") {
            QString message = rootObj["message"].toString();
            QString errorMsg = QString("API error: code=%1, message=%2").arg(code, message);
            qWarning() << errorMsg;
            emit ApoManager::instance()->requestlogWithTime(QString("GetDevMsg6: %1").arg(errorMsg));
            if (message.contains("未登录", Qt::CaseInsensitive)) {
                emit ApoManager::instance()->requestlogWithTime("AAA token error");
                qDebug("AAA登录状态不存在\n");
                //跳转到登录界面
                isLogin = false;
                emit LoginAgain();
            }
            reply_g->deleteLater();
            return;
        }

        reply_g->deleteLater();
    });
}

/*//获得与用户绑定的激活码（有则直接激活，无则蹦出激活码页面）
void LoginAndActivationCode::GetActivationCode()
{
    //获得用户个人信息

    //获得激活码
    // 1. 构建 URL（如果 baseUrl 不包含协议，可在此处补充，这里假设完整）
    // QString base = "https://hubsystest.xiberia.net//api/v1/users/"+g_user_information.network.id;
    //发送GET请求
    // 注：旧网络栈（BaseClient/ApiServerSwitch）已于 2026-08-04 退役；如需恢复参考新栈写法：
    // QNetworkReply *reply3 = HttpClient::instance().get("/user", RequestOptions{}.withTag("user"));

    // 连接 finished 信号
    connect(reply3, &QNetworkReply::finished, this, [this, reply3]() {
        // 检查错误
        if (reply3->error() != QNetworkReply::NoError) {
            qWarning() << "GetActivationCode 获得用户信息失败:" << reply3->errorString();
            reply3->deleteLater();
            ui->stackedWidget->setCurrentWidget(ui->page_ActivationCode);
            return;
        }

        // 读取响应数据
        QByteArray responseData = reply3->readAll();

        // 解析 JSON
        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            QString errorMsg = "GetActivationCode JSON解析错误: " + parseError.errorString();
            qWarning() << errorMsg;
            reply3->deleteLater();
            ui->stackedWidget->setCurrentWidget(ui->page_ActivationCode);

            return;
        }

        QJsonObject rootObj = jsonDoc.object();

        // 检查业务状态码
        QString code = rootObj["code"].toString();
        if (code != "success") {
            QString message = rootObj["message"].toString();
            QString errorMsg = QString("GetActivationCode API error: code=%1, message=%2").arg(code, message);
            qWarning() << errorMsg;
            ui->stackedWidget->setCurrentWidget(ui->page_ActivationCode);
            reply3->deleteLater();
            return;
        }

        // 提取 data 字段
        QJsonObject dataObj = rootObj["data"].toObject();

        QString activation_code = dataObj["activation_code"].toString();
        qDebug() << "获得activation_code:" << activation_code;

        QString nickname = dataObj["nickname"].toString();
        qDebug() << "获得nickname:" << nickname;
        if (activation_code.isEmpty()) {
            qDebug() << "activation_code 是空的";
            saveACode = true;//需要把激活码和用户绑定
            ui->stackedWidget->setCurrentWidget(ui->page_ActivationCode);

        } else {
            saveACode = false;//激活码和用户已绑定
            qDebug() << "activation_code 不为空，内容：" << activation_code;
            user_ActivationCode = activation_code;
            emit ApoManager::instance()->requestActivateAsync(user_ActivationCode);

        }
        reply3->deleteLater();
    });
}

//跳到激活码页面
void LoginAndActivationCode::showActivation()
{
    ui->stackedWidget->setCurrentWidget(ui->page_ActivationCode);
}*/
//跳到登录中界面
void LoginAndActivationCode::showWaitPage()
{
    ui->stackedWidget->setCurrentWidget(ui->page_Waiting);
    movie->start(); // 2. 启动动画
}

void LoginAndActivationCode::LanguageSet()
{
    ui->retranslateUi(this);
}

/// 设置默认 UI 样式
void LoginAndActivationCode::InitUIInformation()
{
    {
        // 设置无边框
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
        ui->widget->setAttribute(Qt::WA_TranslucentBackground);
        ui->stackedWidget->setAttribute(Qt::WA_TranslucentBackground);
        ui->page_Login->setAttribute(Qt::WA_StyledBackground, true);
        ui->page_ActivationCode->setAttribute(Qt::WA_StyledBackground, true);
        ui->page_FAR->setAttribute(Qt::WA_StyledBackground, true);
        ui->page_Waiting->setAttribute(Qt::WA_StyledBackground, true);
        ui->widget->setGraphicsEffect(nullptr);
        ui->widget->setStyleSheet(R"(
            QWidget#widget{
                border-image: none;
                border-radius: 10px;
                background: rgba(81, 96, 122, 0.2);
            }
        )");
        ui->stackedWidget->setStyleSheet(R"(
            QStackedWidget#stackedWidget{
                border-image: none;
                background: transparent;
            }
        )");
        ui->page_FAR->setStyleSheet(R"(
            QWidget#page_FAR{
                border-image: none;
                border-radius: 10px;
                background: rgba(81, 96, 122, 0.2);
            }
        )");
        ui->page_ActivationCode->setStyleSheet(R"(
            QWidget#page_ActivationCode{
                border-image: none;
                border-radius: 10px;
                background: rgba(81, 96, 122, 0.2);
            }
        )");
        ui->page_Waiting->setStyleSheet(R"(
            QWidget#page_Waiting{
                border-image: none;
                border-radius: 10px;
                background: rgba(81, 96, 122, 0.2);
            }
        )");
        ui->page_Login->setStyleSheet(R"(
            QWidget#page_Login{
                border-image: none;
                border-radius: 10px;
                background: rgba(81, 96, 122, 0.2);
            }
        )");

        showError(false);
        showAError(false, "");
        showTimer(false);
        showErrorFAR(false);

        this->setObjectName("LoginAndActivationCode");
        this->setStyleSheet(R"(
            QDialog#LoginAndActivationCode{
                border-image: url(:/Skin/Images/home/background.png);
            }
        )");
    }
    {
        // 广告页面
        cl_advertisement_aelection_main_page_ = new AdvertisementSelectionMainPage(
            ui->widget_advertisement_page);
        ui->widget_advertisement_page->layout()->addWidget(cl_advertisement_aelection_main_page_);

        auto *shadow = new QGraphicsDropShadowEffect(ui->widget_advertisement_page);
        shadow->setOffset(0, 0);
        shadow->setColor(QColor(0, 0, 0, 128));
        shadow->setBlurRadius(20);
        ui->widget_advertisement_page->setGraphicsEffect(shadow);
    }
    {
        // 安装事件过滤器 ui->pBt_wechat 鼠标悬停时上移
        // ui->pBt_wechat->installEventFilter(this);
        // cl_pBt_wechat_old_rect_ = QRect(185, 470, 50, 54); /// 微信图片指定位置
    }
    {
        // 行编辑器
        {
            // 登录页面
            {
                // 邮箱 账号
                this->ui->lEdit_email->setPlaceholderText(tr("请输入邮箱"));
                this->ui->lEdit_email->setStyleSheet(R"(
                    QLineEdit{
                        color: rgba(161, 168, 179, 0.6);
                        font-family: "Noto Sans S Chinese";
                font-weight: 500;
                        font-size: 14px;
                        padding-left:48px;
                        background-image: url(:/Skin/Images/Login/v_2_0/emaill_lineEdit.png);
                        border-radius: 6px;
                    }
                    QLineEdit::left-action {
                        margin-left: 10px;
                    }
                    QLineEdit::right-action {
                        margin-right: 10px;
                    }
                )");

                // 密码
                this->ui->lEdit_password->setPlaceholderText(tr("请输入密码"));
                this->ui->lEdit_password->setStyleSheet(R"(
    QLineEdit {
        color: rgba(161, 168, 179, 0.6);
        font-family: "Noto Sans S Chinese";
                font-weight: 500;
        font-size: 14px;
        padding-left: 48px;
        padding-right: 35px;
        background-image: url(:/Skin/Images/Login/v_2_0/password_lineEdit.png);
        border-radius: 6px;
        border: none;
    }
    QLineEdit::right-action {
        margin-right: 18px;
    }
)");

                // 创建 Action
                lEdit_password_Action = new QAction(this);
                lEdit_password_Action->setIcon(QIcon(":/Skin/Images/Login/v_2_0/invisible_normal.png"));
                this->ui->lEdit_password->addAction(lEdit_password_Action, QLineEdit::TrailingPosition);

                // 连接点击信号
                connect(lEdit_password_Action, &QAction::triggered, this, [this]() {
                    // 切换密码可见性
                    if (this->ui->lEdit_password->echoMode() == QLineEdit::Password) {
                        this->ui->lEdit_password->setEchoMode(QLineEdit::Normal);
                        lEdit_password_Action->setIcon(QIcon(":/Skin/Images/Login/v_2_0/visible_normal.png"));
                    } else {
                        this->ui->lEdit_password->setEchoMode(QLineEdit::Password);
                        lEdit_password_Action->setIcon(QIcon(":/Skin/Images/Login/v_2_0/invisible_normal.png"));
                    }
                });

                // 为密码切换按钮安装 hover 事件过滤器（QAction 非 QWidget，CSS hover 无效，用代码控制图标切换）
                setupActionHover(this->ui->lEdit_password, lEdit_password_Action);

            }
            // 注册页面
            {
                // 邮箱
                this->ui->lEdit_email_FAR->setPlaceholderText(tr("请输入邮箱"));
                this->ui->lEdit_email_FAR->setStyleSheet(R"(
                    QLineEdit{
                        color: rgba(161, 168, 179, 0.6);
                        font-family: "Noto Sans S Chinese";
                font-weight: 500;
                        font-size: 14px;
                        padding-left:48px;
                        background-image: url(:/Skin/Images/Login/v_2_0/emaill_lineEdit.png);
                        border-radius: 6px;
                    }
                )");

                // 密码
                this->ui->lEdit_oldpsw_FAR->setPlaceholderText(tr("请输入密码"));
                this->ui->lEdit_oldpsw_FAR->setStyleSheet(R"(
                    QLineEdit{
                        color: rgba(161, 168, 179, 0.6);
                        font-family: "Noto Sans S Chinese";
                font-weight: 500;
                        font-size: 14px;
                        padding-left:48px;
                        background-image: url(:/Skin/Images/Login/v_2_0/password_lineEdit.png);
                        border-radius:6px;
                    }
                    QLineEdit::right-action {
                        margin-right: 18px;
                    }
                )");

                lEdit_oldpsw_FAR_Action =  new QAction(QIcon(":/Skin/Images/Login/v_2_0/invisible_normal.png"), "", this); ///< 密码 右侧小图标
                this->ui->lEdit_oldpsw_FAR->addAction(lEdit_oldpsw_FAR_Action,
                                                      QLineEdit::TrailingPosition);

                // 连接点击信号：切换密码可见性
                connect(lEdit_oldpsw_FAR_Action, &QAction::triggered, this, [this]() {
                    if (this->ui->lEdit_oldpsw_FAR->echoMode() == QLineEdit::Password) {
                        this->ui->lEdit_oldpsw_FAR->setEchoMode(QLineEdit::Normal);
                        lEdit_oldpsw_FAR_Action->setIcon(QIcon(":/Skin/Images/Login/v_2_0/visible_normal.png"));
                    } else {
                        this->ui->lEdit_oldpsw_FAR->setEchoMode(QLineEdit::Password);
                        lEdit_oldpsw_FAR_Action->setIcon(QIcon(":/Skin/Images/Login/v_2_0/invisible_normal.png"));
                    }
                });

                // 为密码切换按钮安装 hover 事件过滤器
                setupActionHover(this->ui->lEdit_oldpsw_FAR, lEdit_oldpsw_FAR_Action);


                // 再次确认 密码
                this->ui->lEdit_newpsw_FAR->setPlaceholderText(tr("请再次输入密码"));
                this->ui->lEdit_newpsw_FAR->setStyleSheet(R"(
                    QLineEdit{
                        color: rgba(161, 168, 179, 0.6);
                        font-family: "Noto Sans S Chinese";
                font-weight: 500;
                        font-size: 14px;
                        padding-left:48px;
                        background-image: url(:/Skin/Images/Login/v_2_0/password_lineEdit.png);
                        border-radius:6px;
                    }
                    QLineEdit::right-action {
                        margin-right: 18px;
                    }
                )");

                lEdit_newpsw_FAR_Action =  new QAction(QIcon(":/Skin/Images/Login/v_2_0/invisible_normal.png"), tr("搜索"), this); ///< 确认密码 右侧小图标
                this->ui->lEdit_newpsw_FAR->addAction(lEdit_newpsw_FAR_Action,
                                                      QLineEdit::TrailingPosition);

                // 连接点击信号：切换密码可见性
                connect(lEdit_newpsw_FAR_Action, &QAction::triggered, this, [this]() {
                    if (this->ui->lEdit_newpsw_FAR->echoMode() == QLineEdit::Password) {
                        this->ui->lEdit_newpsw_FAR->setEchoMode(QLineEdit::Normal);
                        lEdit_newpsw_FAR_Action->setIcon(QIcon(":/Skin/Images/Login/v_2_0/visible_normal.png"));
                    } else {
                        this->ui->lEdit_newpsw_FAR->setEchoMode(QLineEdit::Password);
                        lEdit_newpsw_FAR_Action->setIcon(QIcon(":/Skin/Images/Login/v_2_0/invisible_normal.png"));
                    }
                });

                // 为确认密码切换按钮安装 hover 事件过滤器
                setupActionHover(this->ui->lEdit_newpsw_FAR, lEdit_newpsw_FAR_Action);

                // 验证码 密码
                this->ui->lEdit_verifyCode_FAR->setPlaceholderText(tr("请输入验证码"));
                this->ui->lEdit_verifyCode_FAR->setStyleSheet(R"(
                    QLineEdit{
                        color: rgba(161, 168, 179, 0.6);
                        font-family: "Noto Sans S Chinese";
                font-weight: 500;
                        font-size: 14px;
                        padding-left:48px;
                        background-color: transparent;
                    }
                )");

                this->ui->widget_3->setObjectName(tr("LoginAndActivationCode_widget_3"));
                this->ui->widget_3->setStyleSheet(R"(
                    QWidget#LoginAndActivationCode_widget_3
                    {
                        color: rgba(161, 168, 179, 0.6);
                        font-family: "Noto Sans S Chinese";
                font-weight: 500;
                        font-size: 14px;
                        padding-left:48px;
                        background-image: url(:/Skin/Images/Login/v_2_0/verificationCode_lineEdit.png);
                        border-radius:6px;
                    }
                )");
                this->ui->lab_timer->setStyleSheet(R"(
                    QLabel
                    {
                        background:transparent;
                        font-family: "Noto Sans S Chinese";
                font-weight: 500;
                        font-size: 14px;
                        color: rgba(161, 168, 179, 0.6);
                    }
                )");

            }

            // 忘记密码页面
            {
                // 同注册页
            }
        }

        // 按键
        {
            // 登录页面
            {
                this->ui->pBt_ELogin->setText(tr("登录"));
                this->ui->pBt_ELogin->setStyleSheet(R"(
                    QPushButton
                    {
                        outline: none;
                        font-family: "Noto Sans S Chinese";
                font-weight: 500;
                        font-size: 16px;
                        color:#FFFFFF;
                        border-image: url(:/Skin/Images/Login/v_2_0/big_pBt_normal.png);
                    }
                    QPushButton::hover
                    {
                        border-image: url(:/Skin/Images/Login/v_2_0/big_pBt_hover.png);
                    }
                )");
            }

            // 注册界面
            {
                this->ui->pBt_ok_FAR->setText(tr("注册"));
                this->ui->pBt_ok_FAR->setStyleSheet(R"(
                    QPushButton
                    {
                        outline: none;
                        font-family: "Noto Sans S Chinese";
                font-weight: 500;
                        font-size: 16px;
                        color:#FFFFFF;
                        border-image: url(:/Skin/Images/Login/v_2_0/big_pBt_normal.png);
                    }
                    QPushButton::hover
                    {
                        border-image: url(:/Skin/Images/Login/v_2_0/big_pBt_hover.png);
                    }
                )");
            }

            // 忘记密码页面
            {
            }

            // 返回登录
            this->ui->pBt_backLogin_FAR->setText(tr(""));
            this->ui->pBt_backLogin_FAR->setStyleSheet(R"(
                    QPushButton
                    {
                        outline: none;
                        font-family: "Noto Sans S Chinese";
                font-weight: 500;
                        font-size: 16px;
                        color:#FFFFFF;
                        border-image: url(:/Skin/Images/Login/v_2_0/reruen_normal.png);
                        border-radius:4px;
                    }
                    QPushButton::hover
                    {
                        border-image: url(:/Skin/Images/Login/v_2_0/reruen_hover.png);
                        border-radius:4px;
                    }

                )");
        }

        // 其他
        {
            this->ui->pBt_register_2->setText(tr("其他登录方式"));
            this->ui->pBt_register_2->setStyleSheet(R"(
                    QPushButton
                    {
                        background:transparent;
                        font-family: "Noto Sans S Chinese";
                        font-weight: 400;
                        font-size: 12px;
                        color: rgba(161, 168, 179, 0.6);
                    }
                )");

            this->ui->pBt_forgot->setText(tr("忘记密码"));
            this->ui->pBt_forgot->setStyleSheet(R"(
                    QPushButton
                    {
                        background:transparent;
                        font-family: "Noto Sans S Chinese";
                font-weight: 500;
                        font-size: 12px;
                        color: #4E8FB5;
                    }
                    QPushButton::hover
                    {
                        background:transparent;
                        font-family: "Noto Sans S Chinese";
                font-weight: 500;
                        font-size: 12px;
                        color: #3F6A88;
                    }
                )");

            this->ui->pBt_register->setText(tr("注册新账号"));
            this->ui->pBt_register->setStyleSheet(R"(
                    QPushButton
                    {
                        background:transparent;
                        font-family: "Noto Sans S Chinese";
                font-weight: 500;
                        font-size: 12px;
                        color: #4E8FB5;
                    }
                    QPushButton::hover
                    {
                        background:transparent;
                        font-family: "Noto Sans S Chinese";
                font-weight: 500;
                        font-size: 12px;
                        color: #3F6A88;
                    }
                )");

            this->ui->label->setText(tr("欢迎登录"));
            this->ui->label->setStyleSheet(R"(
                    QLabel
                    {
                        background:transparent;
                        font-family: "Noto Sans S Chinese";
                        font-weight: 500;
                        font-size: 24px;
                        color: #A1A8B3;
                    }
                )");
            // this->ui->label_title_FAR->setText(tr("欢迎登录"));
            this->ui->label_title_FAR->setStyleSheet(R"(
                    QLabel
                    {
                        background:transparent;
                        font-family: "Noto Sans S Chinese";
                        font-weight: 500;
                        font-size: 24px;
                        color: #A1A8B3;
                    }
                )");

            this->ui->pBt_SendVerifyCode_FAR->setText(tr("发送验证码"));
            this->ui->pBt_SendVerifyCode_FAR->setStyleSheet(R"(
                    QPushButton
                    {
                        background:transparent;
                        font-family: "Noto Sans S Chinese";
                        font-weight: 500;
                        font-size: 14px;
                        color: #4E8FB5;
                    }
                    QPushButton:hover
                    {
                        color: #3F6A88;
                    }
                    QPushButton:disabled
                    {
                        color: #A5A7AC;
                    }
                )");
        }
    }
    {
        // 登录加载动画
        clp_login_loading_ = new CustomQWidgetLoading(ui->widget);
        clp_login_loading_->setObjectName("LoginAndActivationCode_loading");
        clp_login_loading_->setFixedSize(17, 17);
        clp_login_loading_->move(ui->pBt_ELogin->x() + 184, ui->pBt_ELogin->y() + 15);
        clp_login_loading_->hide();
    }
}

/// 初始化 非 UI 成员
void LoginAndActivationCode::InitMember()
{
    {
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &LoginAndActivationCode::onTimeout);

        movie = new QMovie(":/Skin/Images/Login/wait.gif");
        ui->lab_wait->setMovie(movie); // 1. 设置要显示的 GIF 动画图片

        movie->setCacheMode(QMovie::CacheAll); // 缓存所有帧，避免重复加载
    }
}

/// 连接 必要的信号槽
void LoginAndActivationCode::InitConnect()
{
    ui->lEdit_email->installEventFilter(this);
    ui->lEdit_password->installEventFilter(this);

    // 登录成功 → 隐藏加载动画
    connect(this, &LoginAndActivationCode::MainPageChange, this,
            &LoginAndActivationCode::hideLoginLoading);
}
//若服务器存在多条该名称的数据，服务器会返回最新版本的数据
//点击微信登录按钮
void LoginAndActivationCode::on_pBt_wechat_clicked()
{
    g_user_information.network.login_type = "wechat";
    GetWechatCode();
}

//点击邮箱登录
void LoginAndActivationCode::on_pBt_ELogin_clicked()
{
    if (ui->lEdit_email->text().isEmpty()) {
        ui->lab_error->setText(tr("邮箱不能为空"));
        ui->lab_error->show();
        return;
    } else {
        if (ui->lEdit_password->text().isEmpty()) {
            ui->lab_error_p->setText(tr("密码不能为空"));
            ui->lab_error_p->show();
            return;
        } else {
            if (ui->lEdit_password->text().length() < 6) {
                ui->lab_error_p->setText(tr("密码长度小于六位数"));
                ui->lab_error_p->show();
                return;
            } else {
                showError(false);
            }
        }
    }

    // 登录 — 显示加载动画
    if (clp_login_loading_) {
        clp_login_loading_->start();
        clp_login_loading_->show();
    }

    DeSheng::UserLoginRequest t_login_request_info; // 登录请求信息
    t_login_request_info.email = ui->lEdit_email->text();
    t_login_request_info.password = ui->lEdit_password->text();

    QJsonDocument t_json_doc(DeSheng::UserLoginRequestToJson(t_login_request_info));
    QByteArray requestBody = t_json_doc.toJson();

    //***********后期删-测试用***********
    QString requestId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    // 记录开始时间
    QElapsedTimer timer;
    timer.start();

    // 记录请求时间字符串（精确到毫秒）
    QString requestTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

    // 发送 POST 请求
    QNetworkReply *reply = HttpClient::instance().post("/user/login",
        RequestOptions{}.withBody(t_json_doc.toJson()).withTag("user").noAuth());

    connect(
        reply,
        &QNetworkReply::finished,
        this,
        [=]() {
            // 计算耗时（毫秒）
            qint64 elapsedMs = timer.elapsed();

            // 错误处理
            if (reply->error() != QNetworkReply::NoError) {
                // QString errorMsg = "网络错误: " + reply->errorString();
                QString errorMsg = "网络请求失败，错误码: " + QString::number(reply->error());

                //***********后期删-测试用***********
                // 获取请求接口（URL）
                //QString api = reply->request().url().toString();

                // 获取请求头（从原始请求中提取）
                QList<QByteArray> reqHeaders = reply->request().rawHeaderList();
                QString requestHeadersStr;
                for (const QByteArray &header : reqHeaders) {
                    requestHeadersStr += QString("%1: %2\n")
                                             .arg(header.constData(),
                                                  reply->request().rawHeader(header).constData());
                }

                // 获取响应状态码
                int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

                // 获取响应头
                QList<QByteArray> respHeaders = reply->rawHeaderList();
                QString responseHeadersStr;
                for (const QByteArray &header : respHeaders) {
                    responseHeadersStr += QString("%1: %2\n")
                                              .arg(header.constData(),
                                                   reply->rawHeader(header).constData());
                }

                // 获取响应体（必须在任何可能导致数据被消耗的操作之前读取）
                QByteArray responseBody = reply->readAll();
                QString responseBodyStr = QString::fromUtf8(responseBody); // 假设响应为 UTF-8 文本

                // 构建详细日志
                QString logMsg = QString("\n========== 邮箱登录网络请求详情 ==========\n"
                                         "请求时间: %1\n"
                                         "Request-ID: %2\n" // 新增一行
                                         /*"接口: %3\n"*/
                                         "耗时: %3 ms\n"
                                         "HTTP状态码: %4\n"
                                         "-------- 请求头 --------\n%5"
                                         "-------- 请求体 --------\n%6\n"
                                         "-------- 响应头 --------\n%7"
                                         "-------- 响应体 --------\n%8\n"
                                         "==================================\n")
                                     .arg(requestTime,
                                          requestId /*, api*/,
                                          QString::number(elapsedMs),
                                          QString::number(statusCode),
                                          requestHeadersStr,
                                          logSafeBody(requestBody),
                                          responseHeadersStr,
                                          responseBodyStr);
                emit ApoManager::instance()->requestlogWithTime(QString("邮箱登录: %1").arg(logMsg));
                LOG_WARN("[Login] 邮箱登录网络失败, status={}, elapsed={}ms",
                         statusCode, elapsedMs);

                reply->deleteLater();
                hideLoginLoading();
                CheckServerMaintenanceSta(errorMsg);
                return;
            }

            QByteArray responseData = reply->readAll();

            QJsonParseError parseError;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
            if (parseError.error != QJsonParseError::NoError) {
                QString errorMsg = "JSON解析错误: " + parseError.errorString();
                qWarning() << errorMsg;
                reply->deleteLater();
                hideLoginLoading();
                return;
            }

            DeSheng::UserLoginResponse t_userLogin_resp;
            if (DeSheng::ProcessUserLoginResult(t_userLogin_resp, jsonDoc)) {
                if (t_userLogin_resp.code != "success") {
                    QString message = t_userLogin_resp.message;
                    QString errorMsg = QString("API error: code=%1, message=%2")
                                           .arg(t_userLogin_resp.code, message);
                    ui->lab_error_p->setText(tr("邮箱或密码错误"));
                    ui->lab_error_p->show();
                    qWarning() << errorMsg;
                    LOG_WARN("[Login] 邮箱登录失败, code={} msg={}",
                             t_userLogin_resp.code.toStdString(), message.toStdString());
                    reply->deleteLater();
                    hideLoginLoading();
                    return;
                }
                // 登录成功, 显示登录中页面
                // showWaitPage();//登录中移出

                // 保存回显信息 至 g_user_information
                g_user_information.network.access_token = t_userLogin_resp.data.access_token;
                AuthStore::instance().setToken(g_user_information.network.access_token);
                g_user_information.network.refresh_token = t_userLogin_resp.data.refresh_token;

                g_user_information.updateFromServer(t_userLogin_resp.data.user);
                g_user_information.local.user_psw = t_login_request_info.password;

                LOG_INFO("[Login] 邮箱登录成功, user={}",
                         g_user_information.network.username.toStdString());

                isLogin = true; ///< 登录成功后立即标记已登录，避免 LoginInEn 误判

                {
                    // 更新UI信息
                    //头像
                    ui->lab_Avatar->setStyleSheet("border-radius:0px;border-image: "
                                                  "url(:/Skin/Images/Login/Default Avatar.png);");
                    //用户名
                    ui->lab_username->setText(g_user_information.network.username);

                    //头像
                    ui->lab_Avatar_w->setStyleSheet(
                        "border-radius:0px;border-image: url(:/Skin/Images/Login/Default "
                        "Avatar.png);");
                    //用户名
                    ui->lab_username_w->setText(g_user_information.network.username);
                }

                globalSettings->setValue("Login/en", true); //已登录
                globalSettings->setValue("Login/type",
                                         g_user_information.network.login_type); //登录类型
                globalSettings->setValue("Login/nickname",
                                         g_user_information.network.username); //用户名
                globalSettings->setValue("Login/id",
                                         g_user_information.network.id); //用户ID(唯一且不可更改)

                QVariantMap map;
                map["user_email"] = g_user_information.network.email;
                map["user_psw"] = g_user_information.local.user_psw;
                map["access_token"] = g_user_information.network.access_token; //访问令牌
                globalSettings->setValue("Login/Account", map);                //用户ID(唯一且不可更改)

                // 获取当前登录用户完整信息
                if (g_user_information.local.is_get_userInfo_first) {
                    QNetworkReply *t_reply = HttpClient::instance().get("/user",
                        RequestOptions{}.withTag("user"));
                    connect(t_reply, &QNetworkReply::finished, this, [=]() {
                        if (t_reply->error() == QNetworkReply::NoError) {
                            QJsonDocument t_doc = QJsonDocument::fromJson(t_reply->readAll());
                            DeSheng::GetCurrentUserResponse t_resp;
                            if (DeSheng::ProcessGetCurrentUserResult(t_resp, t_doc)) {
                                if (t_resp.code == "success") {
                                    g_user_information.updateFromServer(t_resp.data);
                                }
                            }
                        }

                        initUserLocalDataAfterLogin();
                        g_user_information.local.is_get_userInfo_first.store(false);

                        // 更新 UI
                        ui->lab_username->setText(g_user_information.network.username);
                        ui->lab_username_w->setText(g_user_information.network.username);

                        emit MainPageChange(); ///< GET /user 失败时也进入主页（login 响应已有基本信息）

                        t_reply->deleteLater();
                    });
                }

                // UpdateDev();留
                // GetDevMsg();
                // emit MainPageChange();
                // 100ms后执行 MainPageChange，不阻塞界面
                // QTimer::singleShot(1000, this, &LoginAndActivationCode::MainPageChange);

                // qDebug("ShowActivationCode11111111");
                // //激活码页面
                // ShowActivationCode();

            } else {
                qDebug() << __FILE__ <<__FUNCTION__<< __LINE__ << " ProcessUserLoginResult error";
            };

            reply->deleteLater();
        });
}

//点击忘记密码
void LoginAndActivationCode::on_pBt_forgot_clicked()
{
    ui->label_title_FAR->setText(tr("忘记密码"));
    ui->lEdit_oldpsw_FAR->setPlaceholderText(tr("请输入新密码"));
    ui->lEdit_newpsw_FAR->setPlaceholderText(tr("请再次确认新密码"));
    ui->pBt_ok_FAR->setText(tr("重置密码"));

    this->ui->label_title_FAR->setStyleSheet(R"(
                    QLabel
                    {
                        background:transparent;
                        font-family: "Noto Sans S Chinese";
                        font-weight: 500;
                        font-size: 24px;
                        color: #A1A8B3;
                    }
                )");

    showTimer(false);
    ui->stackedWidget->setCurrentWidget(ui->page_FAR);
    SceneType = 1;
}

//点击注册
void LoginAndActivationCode::on_pBt_register_clicked()
{
    ui->label_title_FAR->setText(tr("注册新账号"));
    ui->lEdit_oldpsw_FAR->setPlaceholderText(tr("请输入密码"));
    ui->lEdit_newpsw_FAR->setPlaceholderText(tr("请再次确认密码"));
    ui->pBt_ok_FAR->setText(tr("注册"));

    this->ui->label_title_FAR->setStyleSheet(R"(
                    QLabel
                    {
                        background:transparent;
                        font-family: "Noto Sans S Chinese";
                font-weight: 500;
                        font-size: 20px;
                        color: #A1A8B3;
                    }
                )");

    showTimer(false);
    ui->stackedWidget->setCurrentWidget(ui->page_FAR);
    SceneType = 0;
}

//忘记密码、注册
void LoginAndActivationCode::on_pBt_ok_FAR_clicked()
{
    showErrorFAR(false);
    if (SceneType == 0) {
        g_user_information.network.email =ui->lEdit_email_FAR->text();
        user_psw_old = ui->lEdit_oldpsw_FAR->text();
        user_psw_new = ui->lEdit_newpsw_FAR->text();
        user_verifyCode = ui->lEdit_verifyCode_FAR->text();

        if (g_user_information.network.email.isEmpty()) {
            ui->lab_error_FAR_E->setText(tr("邮箱不能为空"));
            ui->lab_error_FAR_E->show();
            return;
        } else {
            if (user_psw_old.isEmpty()) {
                ui->lab_error_FAR_O->setText(tr("密码不能为空"));
                ui->lab_error_FAR_O->show();
                return;
            } else {
                if (user_psw_old.length() < 6) {
                    ui->lab_error_FAR_O->setText(tr("密码长度小于六位数"));
                    ui->lab_error_FAR_O->show();
                    return;
                } else {
                    if (user_psw_new != user_psw_old) {
                        ui->lab_error_FAR_N->setText(tr("密码不一致"));
                        ui->lab_error_FAR_N->show();
                        return;
                    } else {
                        if (user_verifyCode.isEmpty()) {
                            ui->lab_error_FAR_C->setText(tr("验证码不能为空"));
                            ui->lab_error_FAR_C->show();
                            return;
                        } else {
                            showErrorFAR(false);
                        }
                    }
                }
            }
        }

        // 构建 JSON 请求体（参数）
        QJsonObject json;
        json["username"] = g_user_information.network.email;
        json["email"] = g_user_information.network.email;
        json["password"] = user_psw_new;
        json["code"] = user_verifyCode;

        // 发送 POST 请求
        QNetworkReply *reply = HttpClient::instance().post("/user/signup",
            RequestOptions{}.withBody(QJsonDocument(json).toJson()).withTag("user").noAuth());

        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            // 错误处理
            if (reply->error() != QNetworkReply::NoError) {
                // QString errorMsg = "网络错误: " + reply->errorString();
                QString errorMsg = "网络请求失败，错误码: " + QString::number(reply->error());
                // 确保 reply 被正确释放
                reply->deleteLater();
                emit ApoManager::instance()->requestlogWithTime(
                    QString("注册: %1").arg(reply->errorString()));
                CheckServerMaintenanceSta(errorMsg);
                // msgBox.critical(NULL,tr("错误"),errorMsg);
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
                // 确保 reply 被正确释放
                reply->deleteLater();
                return;
            }

            QJsonObject rootObj = jsonDoc.object();

            // 检查业务状态码
            QString code = rootObj["code"].toString();
            if (code != "success") {
                QString message = rootObj["message"].toString();
                QString errorMsg = QString("API error: code=%1, message=%2").arg(code, message);
                qWarning() << errorMsg;

                if (code.contains("user_already_exist")) {
                    ui->lab_error_FAR_E->setText(tr("邮箱已被注册"));
                    ui->lab_error_FAR_E->show();
                } else if (code.contains("user_not_found")) {
                    ui->lab_error_FAR_E->setText(tr("用户名包含敏感词"));
                    ui->lab_error_FAR_E->show();
                } else {
                    ui->lab_error_FAR_C->setText(tr("无效验证码"));
                    ui->lab_error_FAR_C->show();
                }

                // 确保 reply 被正确释放
                reply->deleteLater();
                return;
            }

            showError(false);
            showErrorFAR(false);
            ui->pBt_SendVerifyCode_FAR->setEnabled(true);
            ui->lEdit_email->setText(g_user_information.network.email);
            ui->lEdit_password->setText(user_psw_new);

            reply->deleteLater();
            ui->stackedWidget->setCurrentWidget(ui->page_Login);
        });
    } else if (SceneType == 1) {
        g_user_information.network.email =ui->lEdit_email_FAR->text();
        user_psw_old = ui->lEdit_oldpsw_FAR->text();
        user_psw_new = ui->lEdit_newpsw_FAR->text();
        user_verifyCode = ui->lEdit_verifyCode_FAR->text();

        if (g_user_information.network.email.isEmpty()) {
            ui->lab_error_FAR_E->setText(tr("邮箱不能为空"));
            ui->lab_error_FAR_E->show();
            return;
        } else {
            if (user_psw_old.isEmpty()) {
                ui->lab_error_FAR_O->setText(tr("密码不能为空"));
                ui->lab_error_FAR_O->show();
                return;
            } else {
                if (user_psw_old.length() < 6) {
                    ui->lab_error_FAR_O->setText(tr("密码长度小于六位数"));
                    ui->lab_error_FAR_O->show();
                    return;
                } else {
                    if (user_psw_new != user_psw_old) {
                        ui->lab_error_FAR_N->setText(tr("密码不一致"));
                        ui->lab_error_FAR_N->show();
                        return;
                    } else {
                        if (user_verifyCode.isEmpty()) {
                            ui->lab_error_FAR_C->setText(tr("验证码不能为空"));
                            ui->lab_error_FAR_C->show();
                            return;
                        } else {
                            showErrorFAR(false);
                        }
                    }
                }
            }
        }

        // 构建 JSON 请求体（参数）
        QJsonObject json;
        json["email"] = g_user_information.network.email;
        json["code"] = user_verifyCode;
        json["new_password"] = user_psw_new;

        // 发送 POST 请求
        QNetworkReply *reply = HttpClient::instance().post("/user/forgot-password",
            RequestOptions{}.withBody(QJsonDocument(json).toJson()).withTag("user").noAuth());

        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            // 错误处理
            if (reply->error() != QNetworkReply::NoError) {
                // QString errorMsg = "网络错误: " + reply->errorString();
                QString errorMsg = "网络请求失败，错误码: " + QString::number(reply->error());
                // 确保 reply 被正确释放
                reply->deleteLater();
                emit ApoManager::instance()->requestlogWithTime(
                    QString("忘记密码: %1").arg(reply->errorString()));
                CheckServerMaintenanceSta(errorMsg);
                // msgBox.critical(NULL,tr("错误"),errorMsg);
                return;
            }

            // 读取响应数据
            QByteArray t_data = reply->readAll();
            QJsonDocument t_doc = QJsonDocument::fromJson(t_data);
            if (t_doc.isNull()) {
                reply->deleteLater();
                return;
            }

            QJsonObject t_root = t_doc.object();
            QString t_code = t_root["code"].toString();
            if (t_code != "success") {
                QString t_message = t_root["message"].toString();
                qWarning() << "API error: code=" << t_code << "message=" << t_message;

                if (t_code.contains("user_not_found")) {
                    ui->lab_error_FAR_E->setText(tr("用户不存在"));
                    ui->lab_error_FAR_E->show();
                } else {
                    ui->lab_error_FAR_C->setText(tr("验证码错误或已过期"));
                    ui->lab_error_FAR_C->show();
                }
                reply->deleteLater();
                return;
            }

            showError(false);
            showErrorFAR(false);
            ui->pBt_SendVerifyCode_FAR->setEnabled(true);
            ui->lEdit_email->setText(g_user_information.network.email);
            ui->lEdit_password->setText(user_psw_new);

            reply->deleteLater();
            ui->stackedWidget->setCurrentWidget(ui->page_Login);
        });
    }
}

//发送验证码
void LoginAndActivationCode::on_pBt_SendVerifyCode_FAR_clicked()
{
    showErrorFAR(false);
    QString email = ui->lEdit_email_FAR->text();
    if (email.isEmpty()) {
        ui->lab_error_FAR_E->setText(tr("邮箱不能为空"));
        ui->lab_error_FAR_E->show();
        return;
    } else {
        ui->pBt_SendVerifyCode_FAR->setEnabled(false);
        ui->lab_error_FAR_E->hide();
    }
    QString scene;
    if (SceneType == 0) {
        scene = "user_signup";
    } else if (SceneType == 1) {
        scene = "user_forgot_password";
    } else if (SceneType == 2) {
        scene = "user_change_password";
    }

    // 构建 JSON 请求体（参数）
    QJsonObject json;
    json["email"] = email;
    json["scene"] = scene;

    // 发送 POST 请求
    QNetworkReply *reply = HttpClient::instance().post("/email/send-code",
        RequestOptions{}.withBody(QJsonDocument(json).toJson()).withTag("user").noAuth());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        // 错误处理
        if (reply->error() != QNetworkReply::NoError) {
            ui->pBt_SendVerifyCode_FAR->setEnabled(true);
            // QString errorMsg = "网络错误: " + reply->errorString();
            QString errorMsg = "网络请求失败，错误码: " + QString::number(reply->error());
            ui->lab_error_FAR_C->setText(tr("网络错误"));
            ui->lab_error_FAR_C->show();
            // 确保 reply 被正确释放
            reply->deleteLater();
            emit ApoManager::instance()->requestlogWithTime(
                QString("发送验证码: %1").arg(reply->errorString()));
            CheckServerMaintenanceSta(errorMsg);
            // msgBox.critical(NULL,tr("错误"),errorMsg);
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
            // 确保 reply 被正确释放
            reply->deleteLater();
            return;
        }

        QJsonObject rootObj = jsonDoc.object();

        ui->pBt_SendVerifyCode_FAR->setEnabled(true);

        // 检查业务状态码
        QString code = rootObj["code"].toString();
        if (code != "success") {
            QString message = rootObj["message"].toString();
            QString errorMsg = QString("API error: code=%1, message=%2").arg(code, message);
            qWarning() << errorMsg;

            ui->lab_error_FAR_C->setText(tr("发送失败，无效邮箱"));
            ui->lab_error_FAR_C->show();

            // 确保 reply 被正确释放
            reply->deleteLater();
            return;
        } else {
            showTimer(true);
            Countdown = 60;
            m_timer->start(1000); // 每隔 1000 毫秒（1 秒）触发一次 timeout
        }

        // 确保 reply 被正确释放
        reply->deleteLater();
    });
}

void LoginAndActivationCode::onTimeout()
{
    Countdown--;
    ui->lab_timer->setText(QString::number(Countdown) + "S");
    if (Countdown <= 0) {
        m_timer->stop();
        showTimer(false);
    }
}

//返回登录页面
void LoginAndActivationCode::on_pBt_backLogin_FAR_clicked()
{
    showError(false);
    showErrorFAR(false); ///< 隐藏注册/忘记密码的错误提示
    ui->pBt_SendVerifyCode_FAR->setEnabled(true);
    clearText();
    ui->stackedWidget->setCurrentWidget(ui->page_Login);
}

//令牌是否失效,失效则重新登录
bool LoginAndActivationCode::TokenEn()
{
    // g_user_information.local.is_get_userInfo_first 默认为true
    if (g_user_information.local.is_get_userInfo_first) {
        // 记录时间，保留登录日志，方便排查Bug
        QString requestId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        QElapsedTimer timer; // 开始时间
        timer.start();
        QString requestTime = QDateTime::currentDateTime().toString(
            "yyyy-MM-dd hh:mm:ss.zzz"); // 记录请求时间字符串（精确到毫秒）

        // 发送请求
        QNetworkReply *t_reply = HttpClient::instance().get("/user",
            RequestOptions{}.withTag("user"));
        connect(t_reply,
                &QNetworkReply::finished,
                this,
                [=]() {
                    //***********后期删-测试用***********
                    // 计算耗时（毫秒）
                    qint64 elapsedMs = timer.elapsed();

                    if (t_reply->error() != QNetworkReply::NoError) {
                        QString errorMsg = "网络请求失败，错误码: "
                                           + QString::number(t_reply->error());

                        //***********后期删-测试用***********
                        // 获取请求接口（URL）
                        //QString api = reply->request().url().toString();

                        // 获取请求头（从原始请求中提取）
                        QList<QByteArray> reqHeaders = t_reply->request().rawHeaderList();
                        QString requestHeadersStr;
                        for (const QByteArray &header : reqHeaders) {
                            requestHeadersStr
                                += QString("%1: %2\n")
                                       .arg(header.constData(),
                                            t_reply->request().rawHeader(header).constData());
                        }

                        // 获取响应状态码
                        int statusCode
                            = t_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                        // 获取响应头
                        QList<QByteArray> respHeaders = t_reply->rawHeaderList();
                        QString responseHeadersStr;
                        for (const QByteArray &header : respHeaders) {
                            responseHeadersStr += QString("%1: %2\n")
                                                      .arg(header.constData(),
                                                           t_reply->rawHeader(header).constData());
                        }

                        // 获取响应体（必须在错误判断前读取）
                        QByteArray responseBody = t_reply->readAll();
                        QString responseBodyStr = QString::fromUtf8(
                            responseBody); // 假设响应为 UTF-8 文本

                        // 构建详细日志（GET 无请求体）
                        QString logMsg = QString("\n========== 网络请求详情 ==========\n"
                                                 "请求时间: %1\n"
                                                 "Request-ID: %2\n" // 新增一行
                                                 /*"接口: %3\n"*/
                                                 "耗时: %3 ms\n"
                                                 "HTTP状态码: %4\n"
                                                 "-------- 请求头 --------\n%5"
                                                 "-------- 响应头 --------\n%6"
                                                 "-------- 响应体 --------\n%7\n"
                                                 "==================================\n")
                                             .arg(requestTime,
                                                  requestId /*, api*/,
                                                  QString::number(elapsedMs),
                                                  QString::number(statusCode),
                                                  requestHeadersStr,
                                                  responseHeadersStr,
                                                  responseBodyStr);

                        emit ApoManager::instance()->requestlogWithTime(
                            QString("检验: %1").arg(logMsg));

                        // 确保 reply 被正确释放
                        t_reply->deleteLater();
                        CheckServerMaintenanceSta(errorMsg);
                        // msgBox.critical(NULL,tr("错误"),errorMsg);
                        //emit ApoManager::instance()->requestlogWithTime(QString("TokenEn4: %1").arg(reply->errorString()));
                        isLogin = false;
                        emit LoginAgain();
                        return;
                    }

                    if (t_reply->error() == QNetworkReply::NoError) {
                        // 解析 JSON
                        QJsonParseError parseError;
                        QJsonDocument t_doc = QJsonDocument::fromJson(t_reply->readAll(),
                                                                      &parseError);
                        if (parseError.error != QJsonParseError::NoError) {
                            QString errorMsg = "JSON解析错误: " + parseError.errorString();
                            qWarning() << errorMsg;
                            emit ApoManager::instance()->requestlogWithTime(
                                QString("TokenEn5: %1").arg(errorMsg));

                            isLogin = false;
                            emit LoginAgain();

                            t_reply->deleteLater();
                            return;
                        }

                        DeSheng::GetCurrentUserResponse t_resp;
                        if (DeSheng::ProcessGetCurrentUserResult(t_resp, t_doc)) {
                            // 请求失败
                            if (t_resp.code != "success") {
                                QString errorMsg = QString("API error: code=%1, message=%2")
                                                       .arg(t_resp.code, t_resp.message);

                                qWarning() << errorMsg;
                                emit ApoManager::instance()->requestlogWithTime(
                                    QString("TokenEn6: %1").arg(errorMsg));

                                if (t_resp.message.contains("未登录", Qt::CaseInsensitive)
                                    || t_resp.message.contains("无效的设备名称", Qt::CaseInsensitive)
                                    || t_resp.message.contains("token不存在", Qt::CaseInsensitive)
                                    || t_resp.message.contains("token已失效", Qt::CaseInsensitive)
                                    || t_resp.message.contains("token已过期", Qt::CaseInsensitive)) {
                                    emit ApoManager::instance()->requestlogWithTime("AAA token ok");
                                    qDebug("AAA登录状态不存在\n");
                                    //跳转到登录界面
                                    isLogin = false;
                                    emit LoginAgain();
                                } else {
                                    msgBox.critical(NULL, tr("错误"), t_resp.message);
                                }

                                t_reply->deleteLater();
                                return;
                            }

                            // 请求成功
                            {
                                initUserLocalDataAfterLogin();
                                g_user_information.updateFromServer(t_resp.data);
                                g_user_information.local.is_get_userInfo_first.store(false);
                                {
                                    // 实时获取
                                    // 网络上下载对应头像
                                    QString t_dir = g_user_information.userDirName();
                                    QDir().mkpath(t_dir);
                                    QString t_file_path = g_user_information.avatarFilePath();

                                    auto *t_nam = new QNetworkAccessManager(this);
                                    t_nam->setTransferTimeout(60000);
                                    QNetworkReply *t_reply = t_nam->get(QNetworkRequest(QUrl(g_user_information.network.avatar)));
                                    connect(t_reply, &QNetworkReply::finished, this, [this, t_reply, t_file_path, t_nam]() {
                                        t_nam->deleteLater();
                                        if (t_reply->error() == QNetworkReply::NoError) {
                                            QPixmap t_pixmap;
                                            t_pixmap.loadFromData(t_reply->readAll());
                                            if (!t_pixmap.isNull()) {
                                                t_pixmap.save(t_file_path, "PNG");
                                            }
                                        }
                                        t_reply->deleteLater();
                                    });
                                }

                                // 更新 UI
                                ui->lab_username->setText(g_user_information.network.username);
                                ui->lab_username_w->setText(g_user_information.network.username);

                                isLogin = true;
                                // qDebug("TokenEnAAA登录状态存在\n");

                                // 同步社区模块 token（Token 自动登录路径）
                                AuthStore::instance().setToken(g_user_information.network.access_token);

                                emit ApoManager::instance()->requestlogWithTime("AAA token ok");

                                // emit LoginAgain();
                                emit MainPageChange(); //进入主页面

                                // qDebug("UpdateDev\n");留
                                // UpdateDev();
                                // GetDevMsg();
                                // qDebug("GetDevMsg\n");
                            }
                        }
                    }
                    t_reply->deleteLater();
                });
    } else {
        isLogin = true;
        emit MainPageChange(); //进入主页面
    }

    return true;
}

//登录前或请求报错500后，判断是否存在维修提醒，若存在，则弹窗显示公告。若不存在则显示原本显示的错误弹窗
void LoginAndActivationCode::CheckServerMaintenanceSta(const QString &originalError)
{
    if (m_checkingMaintenance)
        return;
    m_checkingMaintenance = true;

    //构建 URL
    QString base = "http://118.190.150.95:8082/ServiceSuspensionNotice.json";
    QUrl url(base);
    qDebug() << "判断服务器是否维修url:" << url;

    //发送GET请求（使用局部 QNAM — 外部 HTTP 服务，不经过 BaseClient）
    auto *t_nam = new QNetworkAccessManager(this);
    t_nam->setTransferTimeout(60000);
    QNetworkReply *reply3 = t_nam->get(QNetworkRequest(url));

    qDebug() << "发起请求，reply:" << reply3;

    // 连接 finished 信号
    connect(reply3, &QNetworkReply::finished, this, [this, originalError, reply3, t_nam]() {
        t_nam->deleteLater();
        m_checkingMaintenance = false;
        // 检查错误
        if (reply3->error() != QNetworkReply::NoError) {
            if (!originalError.isEmpty()) {
                // 无公告或请求失败，则弹出原始错误
                msgBox.critical(nullptr, tr("错误"), originalError);
            }
            qWarning() << "获得维护信息请求失败:" << reply3->errorString();
            reply3->deleteLater();
            return;
        }

        // 读取响应数据
        QByteArray responseData = reply3->readAll();
        // // 去 BOM
        // if (responseData.startsWith("\xEF\xBB\xBF"))
        //     responseData.remove(0, 3);
        // qDebug() << "原始响应: " << responseData;   // 添加这行

        // 解析 JSON
        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            if (!originalError.isEmpty()) {
                // 无公告或请求失败，则弹出原始错误
                msgBox.critical(nullptr, tr("错误"), originalError);
            }
            QString errorMsg = "CheckServerMaintenanceSta JSON解析错误: "
                               + parseError.errorString();
            qWarning() << errorMsg;
            reply3->deleteLater();
            return;
        }

        QJsonObject rootObj = jsonDoc.object();

        // 检查业务状态码
        QString code = rootObj["code"].toString();
        if (code != "success") {
            QString message = rootObj["message"].toString();
            QString errorMsg = QString("CheckServerMaintenanceSta API error: code=%1, message=%2")
                                   .arg(code, message);
            qWarning() << errorMsg;
            reply3->deleteLater();

            if (!originalError.isEmpty()) {
                // 无公告或请求失败，则弹出原始错误
                msgBox.critical(nullptr, tr("错误"), originalError);
            }

            return;
        }

        // 提取 data 字段
        QJsonObject dataObj = rootObj["data"].toObject();

        if (!dataObj.isEmpty()) {
            QString Announcement_title = dataObj["title"].toString();
            qDebug() << "获得title:" << Announcement_title;

            QString Announcement_Content = dataObj["content"].toString();
            qDebug() << "获得content:" << Announcement_Content;

            msgBox.critical(NULL, Announcement_title, Announcement_Content);
        } else {
            if (!originalError.isEmpty()) {
                // 无公告或请求失败，则弹出原始错误
                msgBox.critical(nullptr, tr("错误"), originalError);
            }
        }

        reply3->deleteLater();
    });
}

// 辅助函数：为 QLineEdit 内部 QAction 对应的 QToolButton 安装 hover 事件过滤器
// QAction 不是 QWidget，CSS 的 :hover 对其无效，需要通过代码监听 Enter/Leave 事件来切换图标
void LoginAndActivationCode::setupActionHover(QLineEdit *lineEdit, QAction *action)
{
    if (!lineEdit || !action) {
        return;
    }
    // QLineEdit::addAction() 内部会创建 QLineEditIconButton（继承自 QToolButton），
    // 该按钮调用了 setDefaultAction(action)，因此可通过 defaultAction() 匹配到它
    QList<QToolButton *> buttons = lineEdit->findChildren<QToolButton *>();
    for (QToolButton *btn : buttons) {
        if (btn->defaultAction() == action) {
            btn->installEventFilter(this);
            btn->setCursor(Qt::PointingHandCursor);
            break;
        }
    }
}

// 事件过滤器：监听 action 内部按钮的 Enter/Leave 事件，实现 hover 图标切换
bool LoginAndActivationCode::eventFilter(QObject *watched, QEvent *event)
{
    // 邮箱输入框 — 失焦时校验非空
    if (watched == ui->lEdit_email && event->type() == QEvent::FocusOut) {
        if (ui->lEdit_email->text().trimmed().isEmpty()) {
            ui->lab_error->setText(tr("邮箱不能为空"));
            ui->lab_error->show();
        } else {
            ui->lab_error->hide();
        }
    }

    // 密码输入框 — 失焦时校验非空 + 长度 >= 6
    if (watched == ui->lEdit_password && event->type() == QEvent::FocusOut) {
        if (ui->lEdit_password->text().isEmpty()) {
            ui->lab_error_p->setText(tr("密码不能为空"));
            ui->lab_error_p->show();
        } else if (ui->lEdit_password->text().length() < 6) {
            ui->lab_error_p->setText(tr("密码长度小于六位数"));
            ui->lab_error_p->show();
        } else {
            ui->lab_error_p->hide();
        }
    }

    QToolButton *btn = qobject_cast<QToolButton *>(watched);
    if (btn && btn->defaultAction()) {
        QAction *action = btn->defaultAction();
        QLineEdit *le = qobject_cast<QLineEdit *>(btn->parentWidget());

        if (event->type() == QEvent::Enter) {
            // 鼠标进入 → 切换为 hover 图标
            if (action == lEdit_password_Action) {
                if (le && le->echoMode() == QLineEdit::Normal) {
                    // 密码可见状态 → 显示"可见"hover 图标
                    action->setIcon(QIcon(":/Skin/Images/Login/v_2_0/visible_hover.png"));
                } else {
                    // 密码隐藏状态 → 显示"不可见"hover 图标
                    action->setIcon(QIcon(":/Skin/Images/Login/v_2_0/invisible_hover.png"));
                }
            } else if (action == lEdit_oldpsw_FAR_Action) {
                if (le && le->echoMode() == QLineEdit::Normal) {
                    action->setIcon(QIcon(":/Skin/Images/Login/v_2_0/visible_hover.png"));
                } else {
                    action->setIcon(QIcon(":/Skin/Images/Login/v_2_0/invisible_hover.png"));
                }
            } else if (action == lEdit_newpsw_FAR_Action) {
                if (le && le->echoMode() == QLineEdit::Normal) {
                    action->setIcon(QIcon(":/Skin/Images/Login/v_2_0/visible_hover.png"));
                } else {
                    action->setIcon(QIcon(":/Skin/Images/Login/v_2_0/invisible_hover.png"));
                }
            }
        } else if (event->type() == QEvent::Leave) {
            // 鼠标离开 → 恢复为正常图标
            if (action == lEdit_password_Action) {
                if (le && le->echoMode() == QLineEdit::Normal) {
                    // 密码可见状态 → 显示"可见"图标
                    action->setIcon(QIcon(":/Skin/Images/Login/v_2_0/visible_normal.png"));
                } else {
                    // 密码隐藏状态 → 显示"不可见"图标
                    action->setIcon(QIcon(":/Skin/Images/Login/v_2_0/invisible_normal.png"));
                }
            } else if (action == lEdit_oldpsw_FAR_Action) {
                if (le && le->echoMode() == QLineEdit::Normal) {
                    action->setIcon(QIcon(":/Skin/Images/Login/v_2_0/visible_normal.png"));
                } else {
                    action->setIcon(QIcon(":/Skin/Images/Login/v_2_0/invisible_normal.png"));
                }
            } else if (action == lEdit_newpsw_FAR_Action) {
                if (le && le->echoMode() == QLineEdit::Normal) {
                    action->setIcon(QIcon(":/Skin/Images/Login/v_2_0/visible_normal.png"));
                } else {
                    action->setIcon(QIcon(":/Skin/Images/Login/v_2_0/invisible_normal.png"));
                }
            }
        }
    }
    return QDialog::eventFilter(watched, event);
}

/// \brief 登录后初始化/恢复用户本地数据
static void repairCustomWallpapersIfNeeded()
{
    if (g_user_information.local.custom_wallpaper_map
        && !g_user_information.local.custom_wallpaper_map->isEmpty()) {
        return;
    }

    const QString t_dir = g_user_information.customBackgroundDir();
    if (t_dir.isEmpty())
        return;

    QDir t_custom_dir(t_dir);
    if (!t_custom_dir.exists())
        return;

    const QStringList t_filters = {"*.png", "*.jpg", "*.jpeg", "*.bmp"};
    if (t_custom_dir.entryList(t_filters, QDir::Files, QDir::Name).isEmpty())
        return;

    g_user_information.initCustomWallpapers();
}

static void initWallpaperConfigAfterLogin()
{
    if (g_user_information.loadWallpaperConfig()) {
        g_user_information.local.validateWallpaperPaths();
        g_user_information.refreshSystemWallpapers();
        repairCustomWallpapersIfNeeded();
        g_user_information.saveWallpaperConfigAsync();
        return;
    }

    g_user_information.initSystemWallpapers();
    g_user_information.initCustomWallpapers();
    g_user_information.saveWallpaperConfigAsync();
}

static void initUserLocalDataAfterLogin()
{
    // 保存当前登录获得的 token，防止 loadFromDisk 用磁盘旧 token 覆盖
    const QString t_accessToken = g_user_information.network.access_token;
    const QString t_refreshToken = g_user_information.network.refresh_token;

    // 本地文件存在 → 从磁盘恢复；不存在 → 初始化为默认值
    if (QFile::exists(g_user_information.userInfoFilePath())) {
        if (!g_user_information.loadFromDisk()) {
            // 解析失败（加密/版本/损坏）→ 走重新扫描，避免空 map 写回覆盖有效数据
            g_user_information.initSystemWallpapers();
            g_user_information.initCustomWallpapers();
            g_user_information.saveToDiskAsync();
            initWallpaperConfigAfterLogin();
            // 恢复 token（loadFromDisk 可能部分覆盖），然后返回
            if (!t_accessToken.isEmpty()) {
                g_user_information.network.access_token = t_accessToken;
                g_user_information.network.refresh_token = t_refreshToken;
            }
            return;
        }
        // 清除三张 map 中磁盘已不存在的壁纸路径（默认/系统/自定义均校验）
        g_user_information.local.validateWallpaperPaths();
        // 检查系统壁纸是否与当前 exe 版本一致（追加新文件、删除磁盘不存在的）
        g_user_information.refreshSystemWallpapers();
        // 兼容历史异常：JSON 中自定义壁纸为空，但 CustomBackground 目录文件仍存在时重建索引。
        repairCustomWallpapersIfNeeded();
        g_user_information.saveToDiskAsync();
    } else {
        // 首次安装（无持久化文件）：初始化默认 + 系统 + 自定义壁纸
        g_user_information.initSystemWallpapers();
        g_user_information.initCustomWallpapers();
        g_user_information.saveToDiskAsync();
    }

    initWallpaperConfigAfterLogin();

    // 恢复登录获得的 token（loadFromDisk 可能用磁盘旧值覆盖了新 token）
    if (!t_accessToken.isEmpty()) {
        g_user_information.network.access_token = t_accessToken;
    }
    if (!t_refreshToken.isEmpty()) {
        g_user_information.network.refresh_token = t_refreshToken;
    }
}
