#ifndef AD_API_H
#define AD_API_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QUrlQuery>

namespace DeSheng {

/// 路径常量
namespace ApiPaths {
inline constexpr const char *kAdList        = "/advertisements";
inline constexpr const char *kAdClick       = "/advertisements/{id}/click";
inline constexpr const char *kAdminAdList   = "/admin/advertisements";
inline constexpr const char *kAdminAdDetail = "/admin/advertisements/%1"; ///< 注：文档对删除语义自相矛盾（§7 称"不单独提供 DELETE，删除通过 PUT status=inactive 实现"，状态说明节又称"GORM 软删除"）。此处按通用 REST 提供 DELETE 端点；客户端当前不调用 admin 接口，实际路由以后端为准。
} // namespace ApiPaths

/*************************************************************************************  广告相关  ************************************************************************************************/

/// 获取广告列表 请求结构
typedef struct AdvertisementsListRequest
{
    QString scene;       ///< 场景标识，如：home_banner、bbs_sidebar（必填）
    QString device_type; ///< 设备类型：mouse/keyboard/headset（可选）
    QString device_name; ///< 设备名称，精确匹配（可选）
    int page = 1;        ///< 页码，默认 1（可选）
    int page_size = 10;  ///< 每页数量，默认 10，最大 100（可选）
} AdvertisementsListRequest;

/// 获取广告列表 应答结构中的单个广告项
typedef struct AdvertisementItem
{
    int id;              ///< 广告ID
    QString title;       ///< 广告标题
    QString scene;       ///< 场景标识
    QString media_type;  ///< 媒体类型：image-图片, video-视频
    QString img_url;     ///< 图片URL，media_type=image时必填
    QString video_url;   ///< 视频URL，media_type=video时必填
    QString jump_url;    ///< 点击跳转链接
    QString device_name; ///< 设备名称
    QString device_type; ///< 设备类型：mouse/keyboard/headset
    int sort_order;      ///< 排序权重，值越大越靠前
    QString start_time;  ///< 开始展示时间，RFC3339格式
    QString end_time;    ///< 结束展示时间，RFC3339格式
} AdvertisementItem;

/// 获取广告列表 应答结构
typedef struct AdvertisementsListResponse
{
    QString code;
    QString message;
    typedef struct ReturnData
    {
        QList<AdvertisementItem> list;
        int total;
        int page;
        int page_size;
    } ReturnData;
    ReturnData data;
} AdvertisementsListResponse;

/**
 * @brief 处理获取广告列表接口的响应结果
 * @param responseData 响应数据
 * @param jsonDocument JSON文档
 * @return true 成功, false 失败
 */
bool ProcessAdvertisementsListResult(AdvertisementsListResponse &responseData,
                                     const QJsonDocument &jsonDocument);

/**
 * @brief 将获取广告列表请求结构体转换为JSON对象
 * @param req 请求结构体
 * @return QJsonObject（GET请求无Body，返回空对象）
 */
QJsonObject AdvertisementsListRequestToJson(const AdvertisementsListRequest &req);

/**
 * @brief 构建获取广告列表接口的URL查询参数
 * @param req 请求结构体
 * @param query URL查询参数
 * @param error 错误信息
 * @return true 成功, false 失败
 */
bool buildAdvertisementsListQuery(const AdvertisementsListRequest &req,
                                  QUrlQuery &query,
                                  QString &error);

/// 记录广告点击 请求结构
typedef struct AdClickRequest
{
    int id; ///< 广告ID（路径参数，必填）
} AdClickRequest;

/// 记录广告点击 应答结构
typedef struct AdClickResponse
{
    QString code;
    QString message;
    // data 为 null，无需解析
} AdClickResponse;

/**
 * @brief 处理记录广告点击接口的响应结果
 * @param responseData 响应数据
 * @param jsonDocument JSON文档
 * @return true 成功, false 失败
 */
bool ProcessAdClickResult(AdClickResponse &responseData, const QJsonDocument &jsonDocument);

/**
 * @brief 将记录广告点击请求结构体转换为JSON对象
 * @param req 请求结构体
 * @return QJsonObject（POST请求无Body，返回空对象）
 */
QJsonObject AdClickRequestToJson(const AdClickRequest &req);

/**
 * @brief 构建记录广告点击接口的URL查询参数
 * @param req 请求结构体
 * @param query URL查询参数
 * @param error 错误信息
 * @return true 成功, false 失败
 */
bool buildAdClickQuery(const AdClickRequest &req, QUrlQuery &query, QString &error);


/*************************************************************************************  管理端广告  ************************************************************************************************/

/// 管理端广告条目（含 status/click_count/created_at 字段）
typedef struct AdminAdItem
{
    int id;              ///< 广告ID
    QString title;       ///< 广告标题
    QString scene;       ///< 场景标识
    QString media_type;  ///< 媒体类型：image/video
    QString img_url;     ///< 图片URL
    QString video_url;   ///< 视频URL
    QString jump_url;    ///< 跳转链接
    QString device_name; ///< 设备名称
    QString device_type; ///< 设备类型
    int sort_order;      ///< 排序权重
    QString status;      ///< 状态：active/inactive
    int click_count;     ///< 点击次数
    QString start_time;  ///< 开始时间，RFC3339格式
    QString end_time;    ///< 结束时间，RFC3339格式
    QString created_at;  ///< 创建时间
} AdminAdItem;

// ---- 管理端广告列表 ----

typedef struct AdminAdListRequest
{
    QString scene;       ///< 场景标识（必填）
    QString status;      ///< 状态筛选：active/inactive（可选）
    QString device_type; ///< 设备类型：mouse/keyboard/headset（可选）
    QString device_name; ///< 设备名称，精确匹配（可选）
    QString media_type;  ///< 媒体类型：image/video（可选）
    QString keyword;     ///< 关键词搜索标题（可选）
    QString sort_by;     ///< 排序字段：sort_order/created_at/click_count，默认 sort_order（可选）
    QString order_by;    ///< 排序方向：asc/desc，默认 desc（可选）
    int page = 1;        ///< 页码，默认 1
    int page_size = 10;  ///< 每页数量，默认 10，最大 100
} AdminAdListRequest;

typedef struct AdminAdListResponse
{
    QString code;
    QString message;
    typedef struct ReturnData
    {
        QList<AdminAdItem> list;
        int total;
        int page;
        int page_size;
    } ReturnData;
    ReturnData data;
} AdminAdListResponse;

bool ProcessAdminAdListResult(AdminAdListResponse &responseData,
                              const QJsonDocument &jsonDocument);
QJsonObject AdminAdListRequestToJson(const AdminAdListRequest &req);
bool buildAdminAdListQuery(const AdminAdListRequest &req,
                           QUrlQuery &query,
                           QString &error);

// ---- 管理端广告详情 ----

typedef struct AdminAdDetailRequest
{
    int id; ///< 广告ID（路径参数，必填）
} AdminAdDetailRequest;

typedef struct AdminAdDetailResponse
{
    QString code;
    QString message;
    AdminAdItem data;
} AdminAdDetailResponse;

bool ProcessAdminAdDetailResult(AdminAdDetailResponse &responseData,
                                const QJsonDocument &jsonDocument);
QJsonObject AdminAdDetailRequestToJson(const AdminAdDetailRequest &req);
bool buildAdminAdDetailQuery(const AdminAdDetailRequest &req,
                             QUrlQuery &query,
                             QString &error);

// ---- 管理端创建广告 ----

typedef struct AdminAdCreateRequest
{
    QString title;       ///< 广告标题，最多200字符（必填）
    QString scene;       ///< 场景标识（必填）
    QString media_type;  ///< 媒体类型：image/video，默认 image（可选）
    QString img_url;     ///< 图片URL，最多500字符（可选）
    QString video_url;   ///< 视频URL，最多500字符（可选）
    QString jump_url;    ///< 跳转链接，最多500字符（必填）
    QString device_name; ///< 关联设备名称（可选）
    QString device_type; ///< 设备类型：mouse/keyboard/headset（可选）
    int sort_order;      ///< 排序权重，0-9999，默认 0（可选）
    QString status;      ///< 状态：active/inactive，默认 active（可选）
    QString start_time;  ///< 开始时间，RFC3339格式（可选）
    QString end_time;    ///< 结束时间，RFC3339格式（可选）
} AdminAdCreateRequest;

typedef struct AdminAdCreateResponse
{
    QString code;
    QString message;
    AdminAdItem data;
} AdminAdCreateResponse;

bool ProcessAdminAdCreateResult(AdminAdCreateResponse &responseData,
                                const QJsonDocument &jsonDocument);
QJsonObject AdminAdCreateRequestToJson(const AdminAdCreateRequest &req);
bool buildAdminAdCreateQuery(const AdminAdCreateRequest &req,
                             QUrlQuery &query,
                             QString &error);

// ---- 管理端更新广告 ----

typedef struct AdminAdUpdateRequest
{
    int id;              ///< 广告ID（路径参数，必填）
    QString title;       ///< 广告标题，最多200字符（可选）
    QString scene;       ///< 场景标识（可选）
    QString media_type;  ///< 媒体类型：image/video（可选）
    QString img_url;     ///< 图片URL，最多500字符（可选）
    QString video_url;   ///< 视频URL，最多500字符（可选）
    QString jump_url;    ///< 跳转链接，最多500字符（可选）
    QString device_name; ///< 关联设备名称（可选）
    QString device_type; ///< 设备类型：mouse/keyboard/headset（可选）
    int sort_order;      ///< 排序权重，0-9999（可选）
    QString status;      ///< 状态：active/inactive（可选）
    QString start_time;  ///< 开始时间，RFC3339格式（可选）
    QString end_time;    ///< 结束时间，RFC3339格式（可选）
} AdminAdUpdateRequest;

typedef struct AdminAdUpdateResponse
{
    QString code;
    QString message;
    AdminAdItem data;
} AdminAdUpdateResponse;

bool ProcessAdminAdUpdateResult(AdminAdUpdateResponse &responseData,
                                const QJsonDocument &jsonDocument);
QJsonObject AdminAdUpdateRequestToJson(const AdminAdUpdateRequest &req);
bool buildAdminAdUpdateQuery(const AdminAdUpdateRequest &req,
                             QUrlQuery &query,
                             QString &error);

// ---- 管理端删除广告 ----

typedef struct AdminAdDeleteRequest
{
    int id; ///< 广告ID（路径参数，必填）
} AdminAdDeleteRequest;

typedef struct AdminAdDeleteResponse
{
    QString code;
    QString message;
} AdminAdDeleteResponse;

bool ProcessAdminAdDeleteResult(AdminAdDeleteResponse &responseData,
                                const QJsonDocument &jsonDocument);
QJsonObject AdminAdDeleteRequestToJson(const AdminAdDeleteRequest &req);
bool buildAdminAdDeleteQuery(const AdminAdDeleteRequest &req,
                             QUrlQuery &query,
                             QString &error);


} // namespace DeSheng

// 调用示例（新栈）— 获取广告列表
// auto &cli = HttpClient::instance();  /// network/http_client.h
// AdvertisementsListRequest t_req;
// t_req.page = 1; t_req.page_size = 20;
// QUrlQuery t_query;
// QString t_err;
// buildAdvertisementsListQuery(t_req, t_query, t_err);
// QNetworkReply *r = cli.get(DeSheng::ApiPaths::kAdList, RequestOptions{}.withQuery(t_query).withTag("ad"));
// connect(r, &QNetworkReply::finished, [r]() {
//     AdvertisementsListResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(r->readAll());
//     if (ProcessAdvertisementsListResult(t_resp, t_doc) && t_resp.code == "success") {
//         // t_resp.data.list ...
//     }
//     r->deleteLater();
// });
//
// 调用示例（新栈）— 广告点击上报
// AdClickRequest t_req2;
// t_req2.id = "123";
// QNetworkReply *r2 = cli.post(QString(DeSheng::ApiPaths::kAdClick).arg(t_req2.id), RequestOptions{}.withBody(QByteArray("{}")).withTag("ad"));
// connect(r2, &QNetworkReply::finished, [r2]() {
//     AdClickResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(r2->readAll());
//     if (ProcessAdClickResult(t_resp, t_doc) && t_resp.code == "success") {}
//     r2->deleteLater();
// });
//
// 推荐写法（HttpClient + RequestOptions）
// auto &cli = HttpClient::instance();
// QNetworkReply *r = cli.get(DeSheng::ApiPaths::kAdList, RequestOptions{}.withQuery(t_query).withTag("ad"));
// QNetworkReply *r2 = cli.post(QString(DeSheng::ApiPaths::kAdClick).arg("123"), RequestOptions{}.withTag("ad"));
// connect(r, &QNetworkReply::finished, [r]() { ... ; r->deleteLater(); });

Q_DECLARE_METATYPE(DeSheng::AdvertisementItem) ///< 广告数据

#endif // AD_API_H

