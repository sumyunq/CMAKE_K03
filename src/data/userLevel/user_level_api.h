#ifndef USER_LEVEL_API_H
#define USER_LEVEL_API_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QUrlQuery>

namespace DeSheng {

/// 路径常量
namespace ApiPaths {
inline constexpr const char *kOnlineReport = "/user/online-report";
inline constexpr const char *kUserLevel   = "/user/level";
} // namespace ApiPaths

/*************************************************************************************  用户等级系统  ************************************************************************************************/
/***在线时长上报***/
/// OnlineReportRequest 在线时长达标上报 请求结构体（无 body，仅需 Authorization header） `POST /api/v1/user/online-report`
typedef struct OnlineReportRequest
{
} OnlineReportRequest;

/// OnlineReportResponse 在线时长达标上报 应答结构体
typedef struct OnlineReportResponse
{
    QString code;
    QString message;
    typedef struct ReturnData
    {
        QString status;   ///< 状态：success/cooling/limit_reached
        int gained_exp;   ///< 实际获得的经验值（0或5）
    } ReturnData;
    ReturnData data;
} OnlineReportResponse;

bool ProcessOnlineReportResult(OnlineReportResponse &responseData,
                               const QJsonDocument &jsonDocument);

QJsonObject OnlineReportRequestToJson(const OnlineReportRequest &req);

bool buildOnlineReportQuery(const OnlineReportRequest &req,
                             QUrlQuery &query,
                             QString &error);

/***获取用户等级***/
/// GetUserLevelRequest 获取用户等级信息 请求结构体（无 body，GET 请求） `GET /api/v1/user/level`
typedef struct GetUserLevelRequest
{
} GetUserLevelRequest;

/// GetUserLevelResponse 获取用户等级信息 应答结构体
typedef struct GetUserLevelResponse
{
    QString code;
    QString message;
    typedef struct ReturnData
    {
        int level;              ///< 当前等级（0-15）
        int total_experience;   ///< 历史累计总经验
        int current_experience; ///< 当前等级已获经验
        int exp_cap;            ///< 当前等级经验上限
    } ReturnData;
    ReturnData data;
} GetUserLevelResponse;

bool ProcessGetUserLevelResult(GetUserLevelResponse &responseData,
                               const QJsonDocument &jsonDocument);

QJsonObject GetUserLevelRequestToJson(const GetUserLevelRequest &req);

bool buildGetUserLevelQuery(const GetUserLevelRequest &req,
                             QUrlQuery &query,
                             QString &error);

} // namespace DeSheng

// 调用示例（新栈）— 在线时长达标上报
// auto &cli = HttpClient::instance();  /// network/http_client.h
// OnlineReportRequest t_req;
// QNetworkReply *r = cli.post(DeSheng::ApiPaths::kOnlineReport, RequestOptions{}.withBody(QByteArray("{}")).withTag("userLevel"));
// connect(r, &QNetworkReply::finished, [r]() {
//     OnlineReportResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(r->readAll());
//     if (ProcessOnlineReportResult(t_resp, t_doc) && t_resp.code == "success") {}
//     r->deleteLater();
// });
//
// 调用示例（新栈）— 获取用户等级
// QNetworkReply *r = cli.get(DeSheng::ApiPaths::kUserLevel, RequestOptions{}.withTag("userLevel"));
// connect(r, &QNetworkReply::finished, [r]() {
//     GetUserLevelResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(r->readAll());
//     if (ProcessGetUserLevelResult(t_resp, t_doc) && t_resp.code == "success") {
//         // t_resp.data.level / t_resp.data.current_experience ...
//     }
//     r->deleteLater();
// });
//
// 推荐写法（HttpClient + RequestOptions）
// auto &cli = HttpClient::instance();
// QNetworkReply *r = cli.get(DeSheng::ApiPaths::kUserLevel, RequestOptions{}.withTag("userLevel"));
// connect(r, &QNetworkReply::finished, [r]() { ... ; r->deleteLater(); });

#endif // USER_LEVEL_API_H
