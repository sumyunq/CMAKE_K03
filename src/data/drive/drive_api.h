#ifndef DRIVE_API_H
#define DRIVE_API_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QUrlQuery>

namespace DeSheng {

/// 路径常量
namespace ApiPaths {
inline constexpr const char *kDriveInfo = "/drive/info";
inline constexpr const char *kAdminDrives      = "/admin/drives";
inline constexpr const char *kAdminDriveDetail = "/admin/drives/{id}";
} // namespace ApiPaths

/*************************************************************************************  驱动系统  ************************************************************************************************/

/// DriveInfoRequest 查询驱动信息 请求结构体（客户端用）
typedef struct DriveInfoRequest
{
    QString device_id;   ///< 设备ID，与 device_name 二选一 (可选)
    QString device_name; ///< 设备名称 (可选)
    QString version;     ///< 指定版本号，不传返回最新 (可选)
    QString status;      ///< 指定状态：prod/test/demo/spare (可选)
} DriveInfoRequest;

/// DriveInfoResponse 查询驱动信息 应答结构体（客户端用）
typedef struct DriveInfoResponse
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
        QString id;                       ///< 驱动ID
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
} DriveInfoResponse;

bool ProcessDriveInfoResult(DriveInfoResponse &responseData,
                            const QJsonDocument &jsonDocument);

QJsonObject DriveInfoRequestToJson(const DriveInfoRequest &req);

bool buildDriveInfoQuery(const DriveInfoRequest &req,
                          QUrlQuery &query,
                          QString &error);

/*************************************************************************************  驱动后台管理  ************************************************************************************************/

/// 驱动下载源（管理端）
typedef struct AdminDriveDownloadSource
{
    QString source; ///< 下载源名称：minio / aliyun / cf-r2
    QString type;   ///< 类型：file(上传文件) / web(外链)
    QString url;    ///< 下载地址
} AdminDriveDownloadSource;

/// 创建驱动 请求结构体（管理端）
typedef struct AdminDriveCreateRequest
{
    QString device_id;   ///< 设备ID，与 device_name 二选一 (可选)
    QString device_name; ///< 设备名称 (可选)
    QString version;     ///< 版本号（必填）
    QString release_date;///< 发布日期（必填）
    QString release_notes;///< 发行说明 (可选)
    QString download_url;///< 国内下载地址（必填）
    QString type;        ///< file(上传文件) / web(外链)（必填）
    QList<AdminDriveDownloadSource> download_sources; ///< 下载源 (可选)
} AdminDriveCreateRequest;

/// 创建驱动 应答结构体（管理端）
typedef struct AdminDriveCreateResponse
{
    QString code;
    QString message;
    typedef struct ReturnData
    {
        QString id;           ///< 驱动ID
        QString device_id;    ///< 设备ID
        QString device_name;  ///< 设备名称
        QString version;      ///< 版本号
        QString release_date; ///< 发布日期
        QString release_notes;///< 发行说明
        QString download_url; ///< 下载地址
        int downloads = 0;    ///< 下载次数
        QString type;         ///< 类型
        QString status;       ///< 状态：test / prod / demo / spare
        QList<AdminDriveDownloadSource> download_sources; ///< 多下载源列表
        QString created_at;   ///< 创建时间
        QString updated_at;   ///< 更新时间
    } ReturnData;
    ReturnData data;
} AdminDriveCreateResponse;

bool ProcessAdminDriveCreateResult(AdminDriveCreateResponse &responseData,
                                    const QJsonDocument &jsonDocument);

QJsonObject AdminDriveCreateRequestToJson(const AdminDriveCreateRequest &req);

bool buildAdminDriveCreateQuery(const AdminDriveCreateRequest &req,
                                 QUrlQuery &query,
                                 QString &error);

/// 驱动列表 请求结构体（管理端）
typedef struct AdminDriveListRequest
{
    int page = 1;        ///< 页码，默认 1 (可选)
    int page_size = 10;  ///< 每页条数，默认 10，最大 100 (可选)
    QString device_id;   ///< 按设备ID筛选 (可选)
    QString device_name; ///< 按设备名称模糊搜索 (可选)
    QString version;     ///< 按版本号模糊搜索 (可选)
    QString status;      ///< 按状态筛选：prod / test / demo / spare (可选)
    QString sort_by;     ///< 排序字段 (可选)
    QString order_by;    ///< 排序方向：asc / desc (可选)
} AdminDriveListRequest;

/// 驱动列表 应答结构体（管理端）
typedef struct AdminDriveListResponse
{
    QString code;
    QString message;
    typedef struct DriveListItem
    {
        QString id;           ///< 驱动ID
        QString device_id;    ///< 设备ID
        QString device_name;  ///< 设备名称
        QString version;      ///< 版本号
        QString download_url; ///< 下载地址
        QString type;         ///< 类型
        QString status;       ///< 状态
        int downloads = 0;    ///< 下载次数
        QList<AdminDriveDownloadSource> download_sources; ///< 多下载源列表
        QString created_at;   ///< 创建时间
    } DriveListItem;
    typedef struct ReturnData
    {
        QList<DriveListItem> list;
        int total = 0;
        int page = 0;
        int page_size = 0;
    } ReturnData;
    ReturnData data;
} AdminDriveListResponse;

bool ProcessAdminDriveListResult(AdminDriveListResponse &responseData,
                                  const QJsonDocument &jsonDocument);

QJsonObject AdminDriveListRequestToJson(const AdminDriveListRequest &req);

bool buildAdminDriveListQuery(const AdminDriveListRequest &req,
                               QUrlQuery &query,
                               QString &error);

/// 查询驱动详情 请求结构体（管理端）
typedef struct AdminDriveDetailRequest
{
    QString id; ///< 驱动ID（路径参数，必填）
} AdminDriveDetailRequest;

/// 查询驱动详情 应答复用 AdminDriveCreateResponse，格式一致

bool ProcessAdminDriveDetailResult(AdminDriveCreateResponse &responseData,
                                    const QJsonDocument &jsonDocument);

QJsonObject AdminDriveDetailRequestToJson(const AdminDriveDetailRequest &req);

bool buildAdminDriveDetailQuery(const AdminDriveDetailRequest &req,
                                 QUrlQuery &query,
                                 QString &error);

/// 更新驱动 请求结构体（管理端，所有字段可选）
typedef struct AdminDriveUpdateRequest
{
    QString id;           ///< 驱动ID（路径参数，必填）
    QString version;      ///< 版本号 (可选)
    QString release_date; ///< 发布日期 (可选)
    QString release_notes;///< 发行说明 (可选)
    QString download_url; ///< 下载地址 (可选)
    QString type;         ///< 类型 (可选)
    QString status;       ///< 状态 (可选)
    QList<AdminDriveDownloadSource> download_sources; ///< 下载源 (可选)
} AdminDriveUpdateRequest;

/// 更新驱动 应答复用 AdminDriveCreateResponse，格式一致

bool ProcessAdminDriveUpdateResult(AdminDriveCreateResponse &responseData,
                                    const QJsonDocument &jsonDocument);

QJsonObject AdminDriveUpdateRequestToJson(const AdminDriveUpdateRequest &req);

bool buildAdminDriveUpdateQuery(const AdminDriveUpdateRequest &req,
                                 QUrlQuery &query,
                                 QString &error);

/// 删除驱动 请求结构体（管理端）
typedef struct AdminDriveDeleteRequest
{
    QString id; ///< 驱动ID（路径参数，必填）
} AdminDriveDeleteRequest;

/// 删除驱动 应答结构体（管理端）
typedef struct AdminDriveDeleteResponse
{
    QString code;
    QString message;
} AdminDriveDeleteResponse;

bool ProcessAdminDriveDeleteResult(AdminDriveDeleteResponse &responseData,
                                    const QJsonDocument &jsonDocument);

QJsonObject AdminDriveDeleteRequestToJson(const AdminDriveDeleteRequest &req);

bool buildAdminDriveDeleteQuery(const AdminDriveDeleteRequest &req,
                                 QUrlQuery &query,
                                 QString &error);

} // namespace DeSheng

// 调用示例（新栈）— 查询驱动信息
// auto &cli = HttpClient::instance();  /// network/http_client.h
// DriveInfoRequest t_req;
// t_req.device_name = "K03S";
// QUrlQuery t_query;
// QString t_err;
// buildDriveInfoQuery(t_req, t_query, t_err);
// QNetworkReply *r = cli.get(DeSheng::ApiPaths::kDriveInfo, RequestOptions{}.withQuery(t_query).withTag("drive"));
// connect(r, &QNetworkReply::finished, [r]() {
//     DriveInfoResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(r->readAll());
//     if (ProcessDriveInfoResult(t_resp, t_doc) && t_resp.code == "success") {
//         // t_resp.data.download_url ...
//     }
//     r->deleteLater();
// });
//
// 推荐写法（HttpClient + RequestOptions）
// auto &cli = HttpClient::instance();
// QUrlQuery t_q; t_q.addQueryItem("device_name", "K03S");
// QNetworkReply *r = cli.get(DeSheng::ApiPaths::kDriveInfo, RequestOptions{}.withQuery(t_q).withTag("drive"));
// connect(r, &QNetworkReply::finished, [r]() { ... ; r->deleteLater(); });

#endif // DRIVE_API_H
