#ifndef FIRMWARE_API_H
#define FIRMWARE_API_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QUrlQuery>

namespace DeSheng {

/// 路径常量
namespace ApiPaths {
inline constexpr const char *kFirmwareInfo = "/firmware/info";
inline constexpr const char *kAdminFirmwares      = "/admin/firmwares";
inline constexpr const char *kAdminFirmwareDetail = "/admin/firmwares/{id}";
} // namespace ApiPaths

/*************************************************************************************  固件系统  ************************************************************************************************/

/// FirmwareInfoRequest 查询固件信息 请求结构体（客户端用）
typedef struct FirmwareInfoRequest
{
    QString device_id;   ///< 设备ID，与 device_name 二选一 (可选)
    QString device_name; ///< 设备名称 (可选)
    QString version;     ///< 指定版本号，不传返回最新 (可选)
    QString status;      ///< 指定状态：enabled/disabled (可选)
} FirmwareInfoRequest;

/// FirmwareInfoResponse 查询固件信息 应答结构体（客户端用）
typedef struct FirmwareInfoResponse
{
    QString code;
    QString message;
    typedef struct DownloadSource
    {
        QString source; ///< 下载源名称
        QString type;   ///< 类型
        QString url;    ///< 下载地址
    } DownloadSource;
    typedef struct ReturnData
    {
        QString id;                       ///< 固件ID
        QString device_id;                ///< 设备ID
        QString device_name;              ///< 设备名称
        QString version;                  ///< 版本号
        QString release_date;             ///< 发布日期
        QString download_url;             ///< 下载地址
        QString type;                     ///< 类型
        QString status;                   ///< 状态
        bool is_latest;                   ///< 是否最新版本
        QList<DownloadSource> download_sources; ///< 多下载源列表
        QString created_at;               ///< 创建时间
    } ReturnData;
    ReturnData data;
} FirmwareInfoResponse;

bool ProcessFirmwareInfoResult(FirmwareInfoResponse &responseData,
                               const QJsonDocument &jsonDocument);

QJsonObject FirmwareInfoRequestToJson(const FirmwareInfoRequest &req);

bool buildFirmwareInfoQuery(const FirmwareInfoRequest &req,
                             QUrlQuery &query,
                             QString &error);

/*************************************************************************************  固件后台管理  ************************************************************************************************/

/// 固件下载源（管理端）
typedef struct AdminFirmwareDownloadSource
{
    QString source; ///< 下载源名称：minio / aliyun / cf-r2
    QString type;   ///< 类型：file(上传文件) / web(外链)
    QString url;    ///< 下载地址
} AdminFirmwareDownloadSource;

/// 创建固件 请求结构体（管理端）
typedef struct AdminFirmwareCreateRequest
{
    QString device_id;   ///< 设备ID，与 device_name 二选一 (可选)
    QString device_name; ///< 设备名称 (可选)
    QString version;     ///< 版本号（必填）
    QString file_name;   ///< 文件名 (可选)
    QString release_date;///< 发布日期（必填）
    QString release_notes;///< 发行说明（必填）
    QString download_url;///< 国内下载地址（必填）
    QString type;        ///< file(上传文件) / web(外链)（必填）
    qint64 size = 0;     ///< 文件大小 (可选)
    QList<AdminFirmwareDownloadSource> download_sources; ///< 下载源 (可选)
} AdminFirmwareCreateRequest;

/// 创建固件 应答结构体（管理端）
typedef struct AdminFirmwareCreateResponse
{
    QString code;
    QString message;
    typedef struct ReturnData
    {
        QString id;           ///< 固件ID
        QString version;      ///< 版本号
        QString file_name;    ///< 文件名
        QString release_date; ///< 发布日期
        QString release_notes;///< 发行说明
        QString download_url; ///< 下载地址
        int downloads = 0;    ///< 下载次数
        qint64 size = 0;      ///< 文件大小
        QString type;         ///< 类型
        QString device_id;    ///< 设备ID
        QString device_name;  ///< 设备名称
        QString device_type;  ///< 设备类型
        QString status;       ///< 状态：enabled / disabled
        QList<AdminFirmwareDownloadSource> download_sources; ///< 多下载源列表
        QString created_at;   ///< 创建时间
        QString updated_at;   ///< 更新时间
    } ReturnData;
    ReturnData data;
} AdminFirmwareCreateResponse;

bool ProcessAdminFirmwareCreateResult(AdminFirmwareCreateResponse &responseData,
                                       const QJsonDocument &jsonDocument);

QJsonObject AdminFirmwareCreateRequestToJson(const AdminFirmwareCreateRequest &req);

bool buildAdminFirmwareCreateQuery(const AdminFirmwareCreateRequest &req,
                                    QUrlQuery &query,
                                    QString &error);

/// 固件列表 请求结构体（管理端）
typedef struct AdminFirmwareListRequest
{
    int page = 1;        ///< 页码，默认 1 (可选)
    int page_size = 10;  ///< 每页条数，默认 10，最大 100 (可选)
    QString device_id;   ///< 按设备ID筛选 (可选)
    QString device_name; ///< 按设备名称模糊搜索 (可选)
    QString version;     ///< 按版本号模糊搜索 (可选)
    QString status;      ///< 按状态筛选：enabled / disabled (可选)
    QString sort_by;     ///< 排序字段 (可选)
    QString order_by;    ///< 排序方向：asc / desc (可选)
} AdminFirmwareListRequest;

/// 固件列表 应答结构体（管理端）
typedef struct AdminFirmwareListResponse
{
    QString code;
    QString message;
    typedef struct FirmwareListItem
    {
        QString id;           ///< 固件ID
        QString device_id;    ///< 设备ID
        QString device_name;  ///< 设备名称
        QString version;      ///< 版本号
        QString download_url; ///< 下载地址
        QString type;         ///< 类型
        QString status;       ///< 状态
        int downloads = 0;    ///< 下载次数
        QList<AdminFirmwareDownloadSource> download_sources; ///< 多下载源列表
        QString created_at;   ///< 创建时间
    } FirmwareListItem;
    typedef struct ReturnData
    {
        QList<FirmwareListItem> list;
        int total = 0;
        int page = 0;
        int page_size = 0;
    } ReturnData;
    ReturnData data;
} AdminFirmwareListResponse;

bool ProcessAdminFirmwareListResult(AdminFirmwareListResponse &responseData,
                                     const QJsonDocument &jsonDocument);

QJsonObject AdminFirmwareListRequestToJson(const AdminFirmwareListRequest &req);

bool buildAdminFirmwareListQuery(const AdminFirmwareListRequest &req,
                                  QUrlQuery &query,
                                  QString &error);

/// 查询固件详情 请求结构体（管理端）
typedef struct AdminFirmwareDetailRequest
{
    QString id; ///< 固件ID（路径参数，必填）
} AdminFirmwareDetailRequest;

/// 查询固件详情 应答复用 AdminFirmwareCreateResponse，格式一致

bool ProcessAdminFirmwareDetailResult(AdminFirmwareCreateResponse &responseData,
                                       const QJsonDocument &jsonDocument);

QJsonObject AdminFirmwareDetailRequestToJson(const AdminFirmwareDetailRequest &req);

bool buildAdminFirmwareDetailQuery(const AdminFirmwareDetailRequest &req,
                                    QUrlQuery &query,
                                    QString &error);

/// 更新固件 请求结构体（管理端，所有字段可选）
typedef struct AdminFirmwareUpdateRequest
{
    QString id;           ///< 固件ID（路径参数，必填）
    QString version;      ///< 版本号 (可选)
    QString file_name;    ///< 文件名 (可选)
    QString release_date; ///< 发布日期 (可选)
    QString release_notes;///< 发行说明 (可选)
    QString download_url; ///< 下载地址 (可选)
    QString type;         ///< 类型 (可选)
    QString status;       ///< 状态 (可选)
    qint64 size = -1;     ///< 文件大小，-1 表示未设置 (可选)
    QList<AdminFirmwareDownloadSource> download_sources; ///< 下载源 (可选)
} AdminFirmwareUpdateRequest;

/// 更新固件 应答复用 AdminFirmwareCreateResponse，格式一致

bool ProcessAdminFirmwareUpdateResult(AdminFirmwareCreateResponse &responseData,
                                       const QJsonDocument &jsonDocument);

QJsonObject AdminFirmwareUpdateRequestToJson(const AdminFirmwareUpdateRequest &req);

bool buildAdminFirmwareUpdateQuery(const AdminFirmwareUpdateRequest &req,
                                    QUrlQuery &query,
                                    QString &error);

/// 删除固件 请求结构体（管理端）
typedef struct AdminFirmwareDeleteRequest
{
    QString id; ///< 固件ID（路径参数，必填）
} AdminFirmwareDeleteRequest;

/// 删除固件 应答结构体（管理端）
typedef struct AdminFirmwareDeleteResponse
{
    QString code;
    QString message;
} AdminFirmwareDeleteResponse;

bool ProcessAdminFirmwareDeleteResult(AdminFirmwareDeleteResponse &responseData,
                                       const QJsonDocument &jsonDocument);

QJsonObject AdminFirmwareDeleteRequestToJson(const AdminFirmwareDeleteRequest &req);

bool buildAdminFirmwareDeleteQuery(const AdminFirmwareDeleteRequest &req,
                                    QUrlQuery &query,
                                    QString &error);

} // namespace DeSheng

// 调用示例（新栈）— 查询固件信息
// auto &cli = HttpClient::instance();  /// network/http_client.h
// FirmwareInfoRequest t_req;
// t_req.device_name = "K03S";
// QUrlQuery t_query;
// QString t_err;
// buildFirmwareInfoQuery(t_req, t_query, t_err);
// QNetworkReply *r = cli.get(DeSheng::ApiPaths::kFirmwareInfo, RequestOptions{}.withQuery(t_query).withTag("firmware"));
// connect(r, &QNetworkReply::finished, [r]() {
//     FirmwareInfoResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(r->readAll());
//     if (ProcessFirmwareInfoResult(t_resp, t_doc) && t_resp.code == "success") {
//         // t_resp.data.download_url ...
//     }
//     r->deleteLater();
// });
//
// 推荐写法（HttpClient + RequestOptions）
// auto &cli = HttpClient::instance();
// QUrlQuery t_q; t_q.addQueryItem("device_name", "K03S");
// QNetworkReply *r = cli.get(DeSheng::ApiPaths::kFirmwareInfo, RequestOptions{}.withQuery(t_q).withTag("firmware"));
// connect(r, &QNetworkReply::finished, [r]() { ... ; r->deleteLater(); });

#endif // FIRMWARE_API_H
