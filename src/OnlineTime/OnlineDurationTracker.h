#ifndef ONLINEDURATIONTRACKER_H
#define ONLINEDURATIONTRACKER_H

#include "qnetworkaccessmanager.h"
#include "qnetworkrequest.h"
#include <QObject>
#include <QTimer>
#include <QDate>
#include "data/api_global.h"

/********************* 计算用户在线时长（当天满0.5h上报一次，满2h上报一次，下一天重新开始计算时长；一天只能上报两次，服务器有做次数校验） *********************/

class OnlineDurationTracker : public QObject
{
    Q_OBJECT

public:
    explicit OnlineDurationTracker(QObject *parent = nullptr);
    ~OnlineDurationTracker();

    // 启动计时（应用启动时调用）
    void start();
    // 停止计时（应用退出时调用，析构时也会自动保存）
    void stop();

    // 获取当天已在线分钟数
    int todayMinutes() const;
    // 当天已发送的请求次数
    int requestCount() const;

private slots:
    void onTimeout();          // 每分钟触发

private:
    void loadState();          // 从 QSettings 加载状态
    void saveState();          // 保存状态到 QSettings
    void resetIfNewDay();      // 检测日期变化并重置
    void checkAndTrigger();    // 检查是否达到触发条件

    QTimer *m_timer;
    int m_todayMinutes;        // 当天累计在线分钟数
    int m_requestCount;        // 当天已发送请求次数 (0/1/2)
    QDate m_lastDate;          // 上次记录的日期

    //一天只上报两次请求： 第一次请求信号（累计达 0.5 小时）,第二次请求信号（累计达 2 小时）
    void ReportRequest();

    QString user_token = "";
    QNetworkAccessManager *cl_network_manager_;
    DeSheng::OnlineReportResponse *OnlineResponse = {};//上传方案回应 结构体
};

#endif // ONLINEDURATIONTRACKER_H
