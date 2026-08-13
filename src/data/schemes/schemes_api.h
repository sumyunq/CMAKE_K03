#ifndef SCHEMES_API_H
#define SCHEMES_API_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QUrlQuery>

namespace DeSheng {

/// 路径常量
namespace ApiPaths {
// 用户端
/// @deprecated 文档中无 POST /schemes 端点，创建分享码请用 kSchemeShare（/schemes/share-code）。
///             保留以兼容可能的外部引用，新代码不应使用。
inline constexpr const char *kSchemeCreate  = "/schemes";
inline constexpr const char *kSchemeShare   = "/schemes/share-code";   ///< POST 创建分享码
inline constexpr const char *kSchemeResolve = "/schemes/resolve";      ///< GET 解析分享码（:share_code 拼入路径）
inline constexpr const char *kSchemeUpdate  = "/schemes/%1";           ///< PUT 更新方案（id 拼入路径）
// 管理端
inline constexpr const char *kAdminSchemeList   = "/admin/schemes";      ///< GET 管理端方案列表
inline constexpr const char *kAdminSchemeUpdate = "/admin/schemes/%1";   ///< PUT 管理端更新方案
inline constexpr const char *kAdminSchemeDelete = "/admin/schemes/%1";   ///< DELETE 管理端软删除方案
} // namespace ApiPaths


/*************************************************************************************  方案库分享相关  ************************************************************************************************/
/// 创建分享码 请求结构
typedef struct CreateShareCodeRequest
{
    QString url;         ///< 文件下载链接，必须是http或https协议，最多500字符（必填）
    QString title;       ///< 方案标题，最多200字符
    QString description; ///< 方案描述，最多1000字符
    QString device_name; ///< 设备名称，最多100字符（必填）
    QString device_type; ///< 设备类型：mouse/keyboard/headset（必填）
} CreateShareCodeRequest;

/// 创建分享码 应答结构
typedef struct CreateShareCodeResponse
{
    QString code;
    QString message;

    typedef struct ReturnData
    {
        int64_t id;          ///< 方案ID
        QString share_code;  ///< 8位分享码，大写字母+数字组合
        QString url;         ///< 文件下载链接
        QString title;       ///< 方案标题
        QString description; ///< 方案描述
        QString device_name; ///< 设备名称
        QString device_type; ///< 设备类型：mouse/keyboard/headset
        QString status;      ///< 状态：active-启用，inactive-禁用
        QString created_at;  ///< 创建时间
    } ReturnData;

    ReturnData data;
} CreateShareCodeResponse;

/**
 * @brief 处理创建分享码接口的响应结果
 * @param responseData 响应数据
 * @param jsonDocument JSON文档
 * @return true 成功, false 失败
 */
bool ProcessCreateShareCodeResult(CreateShareCodeResponse &responseData,
                                  QJsonDocument &jsonDocument);

/**
 * @brief 将创建分享码请求结构体转换为JSON对象
 * @param req 请求结构体
 * @return QJsonObject
 */
QJsonObject CreateShareCodeRequestToJson(const CreateShareCodeRequest &req);

/**
 * @brief 构建创建分享码接口的URL查询参数
 * @param req 请求结构体
 * @param query URL查询参数
 * @param error 错误信息
 * @return true 成功, false 失败
 */
bool buildCreateShareCodeQuery(const CreateShareCodeRequest &req, QUrlQuery &query, QString &error);

// 调用示例（新栈）— 创建分享码
// auto &cli = HttpClient::instance();  /// network/http_client.h
// CreateShareCodeRequest req;
// req.url = "https://example.com/config.json";
// req.device_name = "雷柏VT950";
// req.device_type = "mouse";
// req.title = "游戏配置方案";
// QByteArray body = QJsonDocument(CreateShareCodeRequestToJson(req)).toJson();

// QNetworkReply *reply = cli.post(DeSheng::ApiPaths::kSchemeShare, RequestOptions{}.withBody(body).withTag("schemes"));
// connect(reply, &QNetworkReply::finished, [reply]() {
//     CreateShareCodeResponse resp;
//     QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
//     if (ProcessCreateShareCodeResult(resp, doc)) {
//         QString shareCode = resp.data.share_code;
//     }
//     reply->deleteLater();
// });

/// 解析分享码 请求结构
typedef struct ResolveShareCodeRequest
{
    QString share_code; ///< 分享码，8位大写字母+数字组合（路径参数，必填）
} ResolveShareCodeRequest;

/// 解析分享码 应答结构
typedef struct ResolveShareCodeResponse
{
    QString code;
    QString message;

    typedef struct ReturnData
    {
        QString share_code;  ///< 分享码
        QString url;         ///< 文件下载链接
        QString title;       ///< 方案标题
        QString description; ///< 方案描述
        QString device_name; ///< 设备名称
        QString device_type; ///< 设备类型：mouse/keyboard/headset
        QString status;      ///< 状态：active-启用，inactive-禁用
    } ReturnData;

    ReturnData data;
} ResolveShareCodeResponse;

// 函数声明
/**
 * @brief 处理解析分享码接口的响应结果
 * @param responseData 响应数据
 * @param jsonDocument JSON文档
 * @return true 成功, false 失败
 */
bool ProcessResolveShareCodeResult(ResolveShareCodeResponse &responseData,
                                   QJsonDocument &jsonDocument);

/**
 * @brief 将解析分享码请求结构体转换为JSON对象
 * @param req 请求结构体
 * @return QJsonObject（GET请求无Body，返回空对象）
 */
QJsonObject ResolveShareCodeRequestToJson(const ResolveShareCodeRequest &req);

/**
 * @brief 构建解析分享码接口的URL查询参数
 * @param req 请求结构体
 * @param query URL查询参数
 * @param error 错误信息
 * @return true 成功, false 失败
 */
bool buildResolveShareCodeQuery(const ResolveShareCodeRequest &req,
                                QUrlQuery &query,
                                QString &error);

// 调用示例（新栈）— 解析分享码
// ResolveShareCodeRequest req;
// req.share_code = "A3B7K9P2";
// QString encodedShareCode = QUrl::toPercentEncoding(req.share_code);

// auto &cli = HttpClient::instance();  /// network/http_client.h
// QNetworkReply *reply = cli.get(QString(DeSheng::ApiPaths::kSchemeResolve) + "/" + encodedShareCode, RequestOptions{}.withTag("schemes"));
// connect(reply, &QNetworkReply::finished, [reply]() {
//     ResolveShareCodeResponse resp;
//     QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
//     if (ProcessResolveShareCodeResult(resp, doc)) {
//         if (resp.data.status == "active") {
//             QString downloadUrl = resp.data.url;
//         }
//     }
//     reply->deleteLater();
// });

/// 用户端更新方案 请求结构
typedef struct UpdateSchemeRequest
{
    int64_t id;          ///< 方案ID（路径参数，必填）
    QString title;       ///< 方案标题，最多200字符
    QString description; ///< 方案描述，最多1000字符
    QString url;         ///< 文件下载链接，必须是http或https协议，最多500字符
    QString device_name; ///< 设备名称，最多100字符
    QString device_type; ///< 设备类型：mouse/keyboard/headset
} UpdateSchemeRequest;

/// 用户端更新方案 应答结构
typedef struct UpdateSchemeResponse
{
    QString code;
    QString message;

    typedef struct ReturnData
    {
        int64_t id;          ///< 方案ID
        QString share_code;  ///< 分享码
        QString url;         ///< 文件下载链接
        QString title;       ///< 方案标题
        QString description; ///< 方案描述
        QString device_name; ///< 设备名称
        QString device_type; ///< 设备类型：mouse/keyboard/headset
        QString status;      ///< 状态：active-启用，inactive-禁用
        QString created_at;  ///< 创建时间
        QString updated_at;  ///< 更新时间
    } ReturnData;

    ReturnData data;
} UpdateSchemeResponse;

// 函数声明
/**
 * @brief 处理用户端更新方案接口的响应结果
 * @param responseData 响应数据
 * @param jsonDocument JSON文档
 * @return true 成功, false 失败
 */
bool ProcessUpdateSchemeResult(UpdateSchemeResponse &responseData, QJsonDocument &jsonDocument);

/**
 * @brief 将用户端更新方案请求结构体转换为JSON对象
 * @param req 请求结构体
 * @return QJsonObject
 */
QJsonObject UpdateSchemeRequestToJson(const UpdateSchemeRequest &req);

/**
 * @brief 构建用户端更新方案接口的URL查询参数
 * @param req 请求结构体
 * @param query URL查询参数
 * @param error 错误信息
 * @return true 成功, false 失败
 */
bool buildUpdateSchemeQuery(const UpdateSchemeRequest &req, QUrlQuery &query, QString &error);

// 调用示例（新栈）— 更新方案
// UpdateSchemeRequest req;
// req.id = 1;
// req.title = "修改后的标题";
// req.description = "更新后的描述";
// QByteArray body = QJsonDocument(UpdateSchemeRequestToJson(req)).toJson();

// auto &cli = HttpClient::instance();  /// network/http_client.h
// QNetworkReply *reply = cli.put(QString(DeSheng::ApiPaths::kSchemeUpdate).arg(req.id), RequestOptions{}.withBody(body).withTag("schemes"));
// connect(reply, &QNetworkReply::finished, [reply]() {
//     UpdateSchemeResponse resp;
//     QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
//     if (ProcessUpdateSchemeResult(resp, doc)) {
//         qDebug() << "方案更新成功" << resp.data.updated_at;
//     }
//     reply->deleteLater();
// });

/*************************************************************************************  管理端-方案管理  ************************************************************************************************/

/// AdminSchemeListRequest 管理端获取方案列表 请求结构体
typedef struct AdminSchemeListRequest
{
    int page        = 1;       ///< 页码，默认 1（可选）
    int page_size   = 20;      ///< 每页数量，默认 20，最大 100（可选）
    QString keyword;           ///< 关键词搜索（标题、描述、分享码）（可选）
    QString status;            ///< 状态筛选：active/inactive（可选）
} AdminSchemeListRequest;

/// AdminSchemeListResponse 管理端获取方案列表 应答结构体
typedef struct AdminSchemeListResponse
{
    QString code;
    QString message;
    typedef struct ListItem
    {
        int64_t id;
        QString share_code;
        QString url;
        QString title;
        QString description;
        QString device_name;
        QString device_type;
        QString status;
        QString user_id;
        QString created_at;
        QString updated_at;
    } ListItem;
    typedef struct ReturnData
    {
        QList<ListItem> list;
        int total;
        int page;
        int page_size;
    } ReturnData;
    ReturnData data;
} AdminSchemeListResponse;

bool ProcessAdminSchemeListResult(AdminSchemeListResponse &responseData,
                                   QJsonDocument &jsonDocument);

QJsonObject AdminSchemeListRequestToJson(const AdminSchemeListRequest &req);

bool buildAdminSchemeListQuery(const AdminSchemeListRequest &req,
                                QUrlQuery &query,
                                QString &error);

/// AdminSchemeUpdateRequest 管理端更新方案 请求结构体（比用户端多 status 字段）
typedef struct AdminSchemeUpdateRequest
{
    int64_t id;          ///< 方案ID（路径参数，必填）
    QString title;       ///< 方案标题，最多200字符
    QString description; ///< 方案描述，最多1000字符
    QString url;         ///< 文件下载链接
    QString device_name; ///< 设备名称，最多100字符
    QString device_type; ///< 设备类型：mouse/keyboard/headset
    QString status;      ///< 状态：active/inactive（管理端可修改）
} AdminSchemeUpdateRequest;

/// AdminSchemeUpdateResponse 管理端更新方案 应答结构体（比用户端多 user_id）
typedef struct AdminSchemeUpdateResponse
{
    QString code;
    QString message;
    typedef struct ReturnData
    {
        int64_t id;
        QString share_code;
        QString url;
        QString title;
        QString description;
        QString device_name;
        QString device_type;
        QString status;
        QString user_id;
        QString created_at;
        QString updated_at;
    } ReturnData;
    ReturnData data;
} AdminSchemeUpdateResponse;

bool ProcessAdminSchemeUpdateResult(AdminSchemeUpdateResponse &responseData,
                                     QJsonDocument &jsonDocument);

QJsonObject AdminSchemeUpdateRequestToJson(const AdminSchemeUpdateRequest &req);

bool buildAdminSchemeUpdateQuery(const AdminSchemeUpdateRequest &req,
                                  QUrlQuery &query,
                                  QString &error);

/// AdminSchemeDeleteRequest 管理端删除方案 请求结构体
typedef struct AdminSchemeDeleteRequest
{
    int64_t id; ///< 方案ID（路径参数，必填）
} AdminSchemeDeleteRequest;

/// AdminSchemeDeleteResponse 管理端删除方案 应答结构体（软删除）
typedef struct AdminSchemeDeleteResponse
{
    QString code;
    QString message;
    /// data 为 null，无需解析
} AdminSchemeDeleteResponse;

bool ProcessAdminSchemeDeleteResult(AdminSchemeDeleteResponse &responseData,
                                     QJsonDocument &jsonDocument);

QJsonObject AdminSchemeDeleteRequestToJson(const AdminSchemeDeleteRequest &req);

bool buildAdminSchemeDeleteQuery(const AdminSchemeDeleteRequest &req,
                                  QUrlQuery &query,
                                  QString &error);


} // namespace DeSheng

// 推荐写法（HttpClient + RequestOptions）
// auto &cli = HttpClient::instance();
// QNetworkReply *r = cli.get(QString(DeSheng::ApiPaths::kSchemeResolve) + "?share_code=abc123", RequestOptions{}.withTag("schemes"));
// QNetworkReply *r2 = cli.post(DeSheng::ApiPaths::kSchemeCreate, RequestOptions{}.withBody(t_body).withTag("schemes"));
// connect(r, &QNetworkReply::finished, [r]() { ... ; r->deleteLater(); });

#endif // SCHEMES_API_H
