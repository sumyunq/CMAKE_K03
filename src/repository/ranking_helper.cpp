#include "repository/ranking_helper.h"

#include <QDate>
#include <QDateTime>
#include <QTime>
#include <QUrlQuery>

#include "network/http_client.h"
#include "network/request_options.h"

namespace RankingHelper {

void fetchTop(const QString& sort, int limit, bool monthly, QObject* ctx,
              std::function<void(const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>&,
                                 bool)> cb) {
  QUrlQuery t_query;
  t_query.addQueryItem("sort", sort);
  t_query.addQueryItem("device_type", "headset");  ///< 排行榜只统计耳机设备方案
  t_query.addQueryItem("page_size", QString::number(limit));
  if (monthly) {
    // RFC3339：本月 1 号 00:00:00（本地时区，带真实 UTC 偏移，非硬编码 +08:00）
    // 文档要求 + 编码为 %2B（否则服务端按 urlencoded 解析把 + 当空格 → 时间格式非法）
    const QDate t_today = QDate::currentDate();
    const QDateTime t_monthStart(QDate(t_today.year(), t_today.month(), 1), QTime(0, 0));
    const int t_offset = t_monthStart.offsetFromUtc();  // 秒
    const QString t_offsetStr = QString("%1%2:%3")
                                    .arg(t_offset >= 0 ? '+' : '-')
                                    .arg(qAbs(t_offset) / 3600, 2, 10, QChar('0'))
                                    .arg((qAbs(t_offset) % 3600) / 60, 2, 10, QChar('0'));
    QString t_start = t_monthStart.toString("yyyy-MM-ddTHH:mm:ss") + t_offsetStr;
    t_query.addQueryItem("start_time", t_start.replace("+", "%2B"));
  }

  QNetworkReply *t_reply = HttpClient::instance().get("/user-configs",
      RequestOptions{}.withQuery(t_query).withTag("userConfig"));
  QObject::connect(t_reply, &QNetworkReply::finished, ctx, [t_reply, cb]() {
    t_reply->deleteLater();
    QList<DeSheng::GetPublicConfigurationListResponse::ListItem> t_list;
    bool t_ok = false;
    if (t_reply->error() == QNetworkReply::NoError) {
      QJsonDocument t_doc = QJsonDocument::fromJson(t_reply->readAll());
      DeSheng::GetPublicConfigurationListResponse t_resp;
      if (DeSheng::ProcessGetPublicConfigurationListResult(t_resp, t_doc)
          && t_resp.code == "success") {
        t_list = t_resp.data.list;
        t_ok = true;
      }
    }
    if (cb) cb(t_list, t_ok);
  });
}

}  // namespace RankingHelper
