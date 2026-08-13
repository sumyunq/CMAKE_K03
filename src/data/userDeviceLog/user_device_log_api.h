#ifndef USER_DEVICE_LOG_API_H
#define USER_DEVICE_LOG_API_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QUrlQuery>

namespace DeSheng {

/// 路径常量
namespace ApiPaths {
inline constexpr const char *kDeviceLogCreate     = "/user-device-logs";
inline constexpr const char *kDeviceLogList       = "/user-device-logs";
inline constexpr const char *kDeviceLogBasicStats = "/user-device-logs/basic-stats";
inline constexpr const char *kDeviceLogCityActive = "/user-device-logs/city-active";
inline constexpr const char *kDeviceLogCountryActive = "/user-device-logs/country-active";
inline constexpr const char *kDeviceLogDeviceActive  = "/user-device-logs/device-active";
inline constexpr const char *kDeviceLogUserActive    = "/user-device-logs/user-active";
} // namespace ApiPaths

/*************************************************************************************  用户设备日志系统  ************************************************************************************************/

/// 1. 创建设备日志
typedef struct CreateDeviceLogRequest
{
    QString device_name;     ///< 设备名称，1-100字符 (必填)
    QString device_type;     ///< 设备类型：mouse/keyboard/headset (必填)
    QString city;            ///< 城市/省份 (可选)
    QString drive_id;        ///< 驱动ID (可选)
    QString drive_version;   ///< 驱动版本号 (可选)
    QString firmware_id;     ///< 固件ID (可选)
    QString firmware_version;///< 固件版本号 (可选)
} CreateDeviceLogRequest;

typedef struct CreateDeviceLogResponse
{
    QString code;
    QString message;
    typedef struct ReturnData
    {
        QString id;            ///< 日志ID
        QString user_id;       ///< 用户ID
        QString device_name;   ///< 设备名称
        QString device_type;   ///< 设备类型
        QString city;          ///< 城市/省份
        QString country;       ///< ISO 国家代码
        QString country_name;  ///< 中文国家名
        bool first_register;   ///< 是否首次注册
        QString drive_id;      ///< 驱动ID
        QString drive_version; ///< 驱动版本号
        QString firmware_id;   ///< 固件ID
        QString firmware_version;///< 固件版本号
        QString login_ip;      ///< 客户端IP
        QString os_info;       ///< 操作系统信息
        QString created_at;    ///< 创建时间
    } ReturnData;
    ReturnData data;
} CreateDeviceLogResponse;

bool ProcessCreateDeviceLogResult(CreateDeviceLogResponse &responseData,
                                  QJsonDocument &jsonDocument);
QJsonObject CreateDeviceLogRequestToJson(const CreateDeviceLogRequest &req);
bool buildCreateDeviceLogQuery(const CreateDeviceLogRequest &req,
                                QUrlQuery &query, QString &error);

/// 2. 获取设备日志列表
typedef struct GetDeviceLogListRequest
{
    QString user_id;       ///< 用户ID筛选 (可选)
    QString device_id;     ///< 设备ID筛选 (可选)
    QString device_name;   ///< 设备名称模糊搜索 (可选)
    QString device_type;   ///< 设备类型 (可选)
    QString city;          ///< 城市/省份模糊搜索 (可选)
    bool first_register;   ///< 是否只查首次注册 (可选)
    QString start_date;    ///< 开始日期 (可选)
    QString end_date;      ///< 结束日期 (可选)
    int page = 1;          ///< 页码，默认 1
    int page_size = 10;    ///< 每页数量，默认 10，最大 100
    QString sort_by;       ///< 排序字段：created_at (可选)
    QString order_by;      ///< 排序方式：asc/desc，默认 desc (可选)
} GetDeviceLogListRequest;

/// 共用日志条目
typedef struct DeviceLogItem
{
    QString id;
    QString user_id;
    QString device_name;
    QString device_type;
    QString city;
    QString country;
    QString country_name;
    bool first_register;
    QString drive_id;
    QString drive_version;
    QString firmware_id;
    QString firmware_version;
    QString login_ip;
    QString os_info;
    QString created_at;
} DeviceLogItem;

typedef struct GetDeviceLogListResponse
{
    QString code;
    QString message;
    typedef struct ReturnData
    {
        QList<DeviceLogItem> list;
        int total;
        int page;
        int page_size;
    } ReturnData;
    ReturnData data;
} GetDeviceLogListResponse;

bool ProcessGetDeviceLogListResult(GetDeviceLogListResponse &responseData,
                                   QJsonDocument &jsonDocument);
QJsonObject GetDeviceLogListRequestToJson(const GetDeviceLogListRequest &req);
bool buildGetDeviceLogListQuery(const GetDeviceLogListRequest &req,
                                 QUrlQuery &query, QString &error);

/// 3. 获取基础统计
typedef struct GetBasicStatsRequest
{
} GetBasicStatsRequest;

typedef struct GetBasicStatsResponse
{
    QString code;
    QString message;
    typedef struct ReturnData
    {
        int64_t total_users;     ///< 累计总用户数
        int64_t total_devices;   ///< 累计总设备数（代码扩展字段，非API文档定义）
        int64_t daily_active;    ///< 日活跃
        int64_t monthly_active;  ///< 月活跃
        int64_t daily_register;  ///< 日注册
    } ReturnData;
    ReturnData data;
} GetBasicStatsResponse;

bool ProcessGetBasicStatsResult(GetBasicStatsResponse &responseData,
                                QJsonDocument &jsonDocument);
QJsonObject GetBasicStatsRequestToJson(const GetBasicStatsRequest &req);
bool buildGetBasicStatsQuery(const GetBasicStatsRequest &req,
                              QUrlQuery &query, QString &error);

/// 4. 获取城市活跃统计
typedef struct GetCityActiveRequest
{
    QString start_time; ///< 开始时间 ISO8601（文档参数名 start_time）
    QString end_time;   ///< 结束时间 ISO8601（文档参数名 end_time）
} GetCityActiveRequest;

typedef struct CityActiveItem
{
    QString city;
    int64_t count;
} CityActiveItem;

typedef struct GetCityActiveResponse
{
    QString code;
    QString message;
    QList<CityActiveItem> data;
} GetCityActiveResponse;

bool ProcessGetCityActiveResult(GetCityActiveResponse &responseData,
                                QJsonDocument &jsonDocument);
QJsonObject GetCityActiveRequestToJson(const GetCityActiveRequest &req);
bool buildGetCityActiveQuery(const GetCityActiveRequest &req,
                              QUrlQuery &query, QString &error);

/// 5. 获取国家活跃统计
typedef struct GetCountryActiveRequest
{
    QString start_time; ///< 开始时间 ISO8601（文档参数名 start_time）
    QString end_time;   ///< 结束时间 ISO8601（文档参数名 end_time）
} GetCountryActiveRequest;

typedef struct CountryActiveItem
{
    QString country;
    QString country_name;
    int64_t count;
} CountryActiveItem;

typedef struct GetCountryActiveResponse
{
    QString code;
    QString message;
    QList<CountryActiveItem> data;
} GetCountryActiveResponse;

bool ProcessGetCountryActiveResult(GetCountryActiveResponse &responseData,
                                   QJsonDocument &jsonDocument);
QJsonObject GetCountryActiveRequestToJson(const GetCountryActiveRequest &req);
bool buildGetCountryActiveQuery(const GetCountryActiveRequest &req,
                                 QUrlQuery &query, QString &error);

/// 6. 获取设备活跃统计
typedef struct GetDeviceActiveRequest
{
    QString group_by;     ///< 分组方式：day/month (必填)
    QString device_type;  ///< 设备类型 (可选)
    QString device_id;    ///< 设备ID (可选)
    QString device_name;  ///< 设备名称 (可选)
    QString start_date;   ///< 开始日期 (可选)
    QString end_date;     ///< 结束日期 (可选)
} GetDeviceActiveRequest;

typedef struct DeviceActiveItem
{
    QString date;        ///< 日期
    QString device_name;
    QString device_type;
    int64_t count;
} DeviceActiveItem;

typedef struct GetDeviceActiveResponse
{
    QString code;
    QString message;
    QList<DeviceActiveItem> data;
} GetDeviceActiveResponse;

bool ProcessGetDeviceActiveResult(GetDeviceActiveResponse &responseData,
                                  QJsonDocument &jsonDocument);
QJsonObject GetDeviceActiveRequestToJson(const GetDeviceActiveRequest &req);
bool buildGetDeviceActiveQuery(const GetDeviceActiveRequest &req,
                                QUrlQuery &query, QString &error);

/// 7. 获取用户活跃统计
typedef struct GetUserActiveRequest
{
    QString group_by;     ///< 分组方式：day/month (必填)
    QString device_type;  ///< 设备类型 (可选)
    QString device_id;    ///< 设备ID (可选)
    QString device_name;  ///< 设备名称 (可选)
    QString start_date;   ///< 开始日期 (可选)
    QString end_date;     ///< 结束日期 (可选)
} GetUserActiveRequest;

typedef struct UserActiveItem
{
    QString date;
    int64_t count;
} UserActiveItem;

typedef struct GetUserActiveResponse
{
    QString code;
    QString message;
    QList<UserActiveItem> data;
} GetUserActiveResponse;

bool ProcessGetUserActiveResult(GetUserActiveResponse &responseData,
                                QJsonDocument &jsonDocument);
QJsonObject GetUserActiveRequestToJson(const GetUserActiveRequest &req);
bool buildGetUserActiveQuery(const GetUserActiveRequest &req,
                              QUrlQuery &query, QString &error);

} // namespace DeSheng

// 调用示例（新栈）— 创建设备日志
// auto &cli = HttpClient::instance();  /// network/http_client.h
// CreateDeviceLogRequest t_req;
// t_req.device_name = "K03S"; t_req.device_type = "headset";
// QByteArray t_body = QJsonDocument(CreateDeviceLogRequestToJson(t_req)).toJson();
// QNetworkReply *r = cli.post(DeSheng::ApiPaths::kDeviceLogCreate, RequestOptions{}.withBody(t_body).withTag("userDeviceLog"));
// connect(r, &QNetworkReply::finished, [r]() {
//     CreateDeviceLogResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(r->readAll());
//     if (ProcessCreateDeviceLogResult(t_resp, t_doc) && t_resp.code == "success") {}
//     r->deleteLater();
// });
//
// 调用示例（新栈）— 获取设备日志列表
// GetDeviceLogListRequest t_req2;
// t_req2.page = 1; t_req2.page_size = 10;
// QUrlQuery t_query; QString t_err;
// buildGetDeviceLogListQuery(t_req2, t_query, t_err);
// QNetworkReply *r2 = cli.get(DeSheng::ApiPaths::kDeviceLogList, RequestOptions{}.withQuery(t_query).withTag("userDeviceLog"));
// connect(r2, &QNetworkReply::finished, [r2]() {
//     GetDeviceLogListResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(r2->readAll());
//     if (ProcessGetDeviceLogListResult(t_resp, t_doc) && t_resp.code == "success") {}
//     r2->deleteLater();
// });
//
// 推荐写法（HttpClient + RequestOptions）
// auto &cli = HttpClient::instance();
// QNetworkReply *r = cli.post(DeSheng::ApiPaths::kDeviceLogCreate, RequestOptions{}.withBody(t_body).withTag("userDeviceLog"));
// QUrlQuery t_q; t_q.addQueryItem("page", "1");
// QNetworkReply *r2 = cli.get(DeSheng::ApiPaths::kDeviceLogList, RequestOptions{}.withQuery(t_q).withTag("userDeviceLog"));
// connect(r, &QNetworkReply::finished, [r]() { ... ; r->deleteLater(); });

#endif // USER_DEVICE_LOG_API_H
