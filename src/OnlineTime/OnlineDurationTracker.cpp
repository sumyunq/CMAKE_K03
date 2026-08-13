#include "OnlineTime/OnlineDurationTracker.h"
#include <QSettings>
#include <QDebug>
#include "LoadLib.h"
#include "data/api_global.h"
#include "network/http_client.h"
#include "network/request_options.h"
#include "APOThread/ApoManager.h"
#include "modules/GeneralCustomUI/custom_QWidget_notification.h"
// 阈值（单位：分钟）
static const int FIRST_THRESHOLD_MIN  = 30;     // 0.5 小时
static const int SECOND_THRESHOLD_MIN = 125;    //120;  // 2 小时（0.5h + 1.5h）,多五分钟，防止和服务器的时间有误差

OnlineDurationTracker::OnlineDurationTracker(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
    , m_todayMinutes(0)
    , m_requestCount(0)
    , m_lastDate(QDate::currentDate())
    , cl_network_manager_(new QNetworkAccessManager(this))
    , OnlineResponse(new DeSheng::OnlineReportResponse())
{
    connect(m_timer, &QTimer::timeout, this, &OnlineDurationTracker::onTimeout);
    // 每分钟触发一次（1000ms * 60）
    m_timer->setInterval(60 * 1000);

    // 启动时加载持久化状态
    loadState();
    resetIfNewDay();
}

OnlineDurationTracker::~OnlineDurationTracker()
{
    stop();      // 停止定时器并保存
}
//启动计时（应用启动时调用）
void OnlineDurationTracker::start()
{
    resetIfNewDay();   // 防止跨天未重置
    if (!m_timer->isActive()) {
        m_timer->start();
        qDebug() << "Online tracker started, today minutes:" << m_todayMinutes
                 << ", requests sent:" << m_requestCount;
        emit ApoManager::instance()->requestlogWithTime("在线时长开始");

    }else
    {
        emit ApoManager::instance()->requestlogWithTime("在线时长开始失败");
    }
    // 立即检查一次，以防因启动前已超时需补发请求
    checkAndTrigger();
}
//停止计时（应用退出时调用，析构时也会自动保存）
void OnlineDurationTracker::stop()
{
    if (m_timer->isActive()) {
        m_timer->stop();
        saveState();
        qDebug() << "Online tracker stopped, today minutes:" << m_todayMinutes;
    }
}
// 获取当天已在线分钟数
int OnlineDurationTracker::todayMinutes() const
{
    return m_todayMinutes;
}
// 当天已发送的请求次数
int OnlineDurationTracker::requestCount() const
{
    return m_requestCount;
}

//定时器
void OnlineDurationTracker::onTimeout()
{
    resetIfNewDay();   // 检查日期

    // 累计 1 分钟
    m_todayMinutes++;
    saveState();       // 每分钟保存一次状态

    checkAndTrigger();
}
// 检查是否达到触发条件
void OnlineDurationTracker::checkAndTrigger()
{
    // 已发两次，不再检查
    if (m_requestCount >= 2)
    {
        // ReportRequest();//测试，执行第三次，是否正常报错，后续需删除
        return;
    }
    // 第一次：达到 30 分钟且未发送过
    if (m_requestCount == 0 && m_todayMinutes >= FIRST_THRESHOLD_MIN) {
        m_requestCount = 1;
        saveState();
        qDebug() << "Trigger first request at" << m_todayMinutes << "minutes";
        emit ApoManager::instance()->requestlogWithTime(QString("在线时长Trigger first request at").arg(m_todayMinutes));
        ReportRequest();
    }
    // 第二次：达到 120 分钟且只发送过一次
    // 这里对应“第一次触发后重新计时 1.5 小时”，总时长 2 小时
    else if (m_requestCount == 1 && m_todayMinutes >= SECOND_THRESHOLD_MIN) {
        m_requestCount = 2;
        saveState();
        qDebug() << "Trigger second request at" << m_todayMinutes << "minutes";
        emit ApoManager::instance()->requestlogWithTime(QString("在线时长Trigger second request at").arg(m_todayMinutes));
        ReportRequest();
    }
}
// 从 QSettings 加载状态
void OnlineDurationTracker::loadState()
{
    QSettings settings("XIBERIA", "XIBERIA X HUB");
    m_lastDate     = settings.value("online/date", QDate::currentDate()).toDate();
    m_todayMinutes = settings.value("online/minutes", 0).toInt();
    m_requestCount = settings.value("online/reqCount", 0).toInt();
}
// 保存状态到 注册表（QSettings）（计算机\HKEY_CURRENT_USER\Software\XIBERIA\XIBERIA X HUB\online）
void OnlineDurationTracker::saveState()
{
    QSettings settings("XIBERIA", "XIBERIA X HUB");
    settings.setValue("online/date", m_lastDate);
    settings.setValue("online/minutes", m_todayMinutes);
    settings.setValue("online/reqCount", m_requestCount);
}
// 检测日期变化并重置
void OnlineDurationTracker::resetIfNewDay()
{
    QDate today = QDate::currentDate();
    if (m_lastDate != today) {
        qDebug() << "New day, resetting online state.";
        m_lastDate     = today;
        m_todayMinutes = 0;
        m_requestCount = 0;
        saveState();
        emit ApoManager::instance()->requestlogWithTime(QString("在线时长,%1,%2").arg(m_lastDate.toString()).arg(today.toString()));
    }
}

//在线时长达标上报
void OnlineDurationTracker::ReportRequest()
{
    user_token = globalSettings->value("Login/Account")
    .toMap()
        .value("access_token")
        .toString(); ///用户tonken
    // 在线上报 → ApiClient
    QNetworkReply *reply = HttpClient::instance().post(
        "/user/online-report",
        RequestOptions{}.withTag("userLevel"));
    connect(reply, &QNetworkReply::finished, this, [=]() mutable {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit ApoManager::instance()->requestlogWithTime(QString("%1 %2 netWorkReply error:%3")
                                                                .arg(__FUNCTION__)
                                                                .arg(__LINE__)
                                                                .arg(reply->errorString()));

            return;
        }
        if (reply->error() == QNetworkReply::NoError) {
            // 读取响应数据
            QByteArray responseData = reply->readAll();
            qDebug() << "创建配置请求 回显原始数据:" << QString::fromUtf8(responseData);



            // 解析 JSON
            QJsonParseError parseError;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
            if (parseError.error != QJsonParseError::NoError) {
                QString errorMsg = "JSON解析错误: " + parseError.errorString();
                emit ApoManager::instance()->requestlogWithTime(
                    QString("%1 %2 %3").arg(__FUNCTION__).arg(__LINE__).arg(errorMsg));

                return;
            }

            //UploadPlansResponse传递空值
            bool result = DeSheng::ProcessOnlineReportResult(*OnlineResponse,jsonDoc);
            if(result)
            {
                qDebug("在线时长达标上报成功\n");
                emit ApoManager::instance()->requestlogWithTime("在线时长上报成功");
            }else
            {
                qDebug("在线时长达标上报失败\n");
                if(OnlineResponse->data.status.contains("limit_reached"))
                {
                    emit ApoManager::instance()->requestlogWithTime("在线时长上报失败，已达到今日上限");
                    // auto *t_notif = new CustomQWidgetNotification(tr("在线时长上报失败，已达到今日上限"), QString(), nullptr);
                    // QObject::connect(t_notif, &CustomQWidgetNotification::accepted,
                    //                  t_notif, &QWidget::deleteLater);
                    // t_notif->show();
                }else if(OnlineResponse->data.status.contains("cooling"))
                {
                    auto *t_notif = new CustomQWidgetNotification(tr("在线时长上报失败，半个小时内重复请求"), QString(), nullptr);
                    QObject::connect(t_notif, &CustomQWidgetNotification::accepted,
                                     t_notif, &QWidget::deleteLater);
                    t_notif->show();
                }
            }

        }


    });

}
