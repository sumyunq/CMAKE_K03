#ifndef USER_DEVICE_API_H
#define USER_DEVICE_API_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QUrlQuery>

namespace DeSheng {

/// 路径常量
namespace ApiPaths {
inline constexpr const char *kDeviceMonthlyStats    = "/user-devices/statistics/monthly";
inline constexpr const char *kDeviceCumulativeStats = "/user-devices/statistics/cumulative";
} // namespace ApiPaths

/*************************************************************************************  设备绑定统计系统（用户端） ************************************************************************************************/

/// 设备类型统计项（mouse / keyboard / headset 通用）
typedef struct DeviceCountItem
{
    int64_t count; ///< 绑定人数
} DeviceCountItem;

/// 设备统计汇总（三种设备类型）
typedef struct DeviceStatistics
{
    DeviceCountItem mouse;
    DeviceCountItem keyboard;
    DeviceCountItem headset;
} DeviceStatistics;

/// 1. 获取月度统计数据 — GET /user-devices/statistics/monthly?month=YYYY-MM
typedef struct GetDeviceMonthlyStatsRequest
{
    QString month; ///< 月份，格式：YYYY-MM (必填)
} GetDeviceMonthlyStatsRequest;

typedef struct GetDeviceMonthlyStatsResponse
{
    QString code;
    QString message;
    typedef struct ReturnData
    {
        QString period;              ///< 统计周期（月份）
        QString period_type;         ///< 统计类型：monthly
        DeviceStatistics statistics; ///< 各设备类型统计数据
        int64_t total;               ///< 总计绑定人数
    } ReturnData;
    ReturnData data;
} GetDeviceMonthlyStatsResponse;

bool ProcessGetDeviceMonthlyStatsResult(GetDeviceMonthlyStatsResponse &responseData,
                                        QJsonDocument &jsonDocument);
QJsonObject GetDeviceMonthlyStatsRequestToJson(const GetDeviceMonthlyStatsRequest &req);
bool buildGetDeviceMonthlyStatsQuery(const GetDeviceMonthlyStatsRequest &req,
                                     QUrlQuery &query, QString &error);

/// 2. 获取累计统计数据 — GET /user-devices/statistics/cumulative（无参数）
typedef struct GetDeviceCumulativeStatsRequest
{
} GetDeviceCumulativeStatsRequest;

typedef struct GetDeviceCumulativeStatsResponse
{
    QString code;
    QString message;
    typedef struct ReturnData
    {
        QString period;              ///< 统计周期（all 表示全部时间）
        QString period_type;         ///< 统计类型：cumulative
        DeviceStatistics statistics; ///< 各设备类型统计数据
        int64_t total;               ///< 总计绑定人数
    } ReturnData;
    ReturnData data;
} GetDeviceCumulativeStatsResponse;

bool ProcessGetDeviceCumulativeStatsResult(GetDeviceCumulativeStatsResponse &responseData,
                                           QJsonDocument &jsonDocument);
QJsonObject GetDeviceCumulativeStatsRequestToJson(const GetDeviceCumulativeStatsRequest &req);
bool buildGetDeviceCumulativeStatsQuery(const GetDeviceCumulativeStatsRequest &req,
                                        QUrlQuery &query, QString &error);

} // namespace DeSheng

// 调用示例（新栈）— 获取月度统计数据
// auto &cli = HttpClient::instance();  /// network/http_client.h
// GetDeviceMonthlyStatsRequest t_req;
// t_req.month = "2024-12";
// QUrlQuery t_query; QString t_err;
// buildGetDeviceMonthlyStatsQuery(t_req, t_query, t_err);
// QNetworkReply *r = cli.get(DeSheng::ApiPaths::kDeviceMonthlyStats, RequestOptions{}.withQuery(t_query).withTag("userDevice"));
// connect(r, &QNetworkReply::finished, [r]() {
//     GetDeviceMonthlyStatsResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(r->readAll());
//     if (ProcessGetDeviceMonthlyStatsResult(t_resp, t_doc) && t_resp.code == "success") {}
//     r->deleteLater();
// });
//
// 调用示例（新栈）— 获取累计统计数据
// GetDeviceCumulativeStatsRequest t_req2;
// QUrlQuery t_query2; QString t_err2;
// buildGetDeviceCumulativeStatsQuery(t_req2, t_query2, t_err2);
// QNetworkReply *r2 = cli.get(DeSheng::ApiPaths::kDeviceCumulativeStats, RequestOptions{}.withQuery(t_query2).withTag("userDevice"));
// connect(r2, &QNetworkReply::finished, [r2]() {
//     GetDeviceCumulativeStatsResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(r2->readAll());
//     if (ProcessGetDeviceCumulativeStatsResult(t_resp, t_doc) && t_resp.code == "success") {}
//     r2->deleteLater();
// });
//
// 推荐写法（HttpClient + RequestOptions）
// auto &cli = HttpClient::instance();
// QUrlQuery t_q; t_q.addQueryItem("month", "2024-12");
// QNetworkReply *r = cli.get(DeSheng::ApiPaths::kDeviceMonthlyStats, RequestOptions{}.withQuery(t_q).withTag("userDevice"));
// QNetworkReply *r2 = cli.get(DeSheng::ApiPaths::kDeviceCumulativeStats, RequestOptions{}.withTag("userDevice"));
// connect(r, &QNetworkReply::finished, [r]() { ... ; r->deleteLater(); });

#endif // USER_DEVICE_API_H
