#ifndef FEEDBACK_API_H
#define FEEDBACK_API_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QUrlQuery>

namespace DeSheng {

/// 路径常量
namespace ApiPaths {
// 用户端
inline constexpr const char *kFeedbackList   = "/feedbacks";       ///< GET 列表 / POST 提交
inline constexpr const char *kFeedbackDetail = "/feedbacks/%1";    ///< GET 详情（ticket_no 拼入路径）
inline constexpr const char *kFeedbackUpload = "/user/uploads";    ///< POST 文件上传
// 管理端
inline constexpr const char *kAdminFeedbackList   = "/admin/feedbacks";         ///< GET 管理端反馈列表
inline constexpr const char *kAdminFeedbackDetail = "/admin/feedbacks/%1";      ///< GET 管理端反馈详情
inline constexpr const char *kAdminFeedbackStatus = "/admin/feedbacks/%1/status"; ///< PUT 更新反馈状态
} // namespace ApiPaths

/*************************************************************************************  反馈系统  ************************************************************************************************/

/// FileUploadsRequest 文件上传 请求结构体信息
typedef struct FileUploadsRequest
{
    QString device_id; ///< 设备ID（必填）
    QString drive_id;  ///< 驱动ID（必填）
} FileUploadsRequest;

/// FileUploadsResponse 文件上传 应答结构体信息
typedef struct FileUploadsResponse
{
    QString code;    ///< 成功:"success"
    QString message; ///< 成功:"success"
    typedef struct ReturnData
    {
        QString id;        ///< 文件ID
        QString driver;    ///< 存储驱动
        QString name;      ///< 文件名
        QString mime_type; ///< MIME类型
        QString size;      ///< 文件大小（存储为QString）
        QString url;       ///< 文件URL
        QString user_id;   ///< 用户ID
    } returnData;
    returnData data;
} FileUploadsResponse;

/// 解析上传文件反馈结果
bool ProcessFileUploadsResult(DeSheng::FileUploadsResponse &responseData,
                              QJsonDocument &jsonDocument);

///
/// \brief UserFeedBacksRequest
/// 用户反馈 请求结构体信息
typedef struct UserFeedBacksRequest
{
    QString device_id;        ///< 设备ID（必填）
    QString drive_id;         ///< 驱动ID（必填）
    QString firmware_id;      ///< 固件ID（必填）
    QString drive_version;    ///< 驱动版本号(必填)
    QString firmware_version; ///< 固件版本号(必填)
    QString receiver_version; ///< 接收器版本号(可选)
    QString device_name;      ///< 设备名称(必填)
    QString device_type;      ///< 设备类型:mouse/keyboard/headset (必填)
    QString title;            ///< 反馈标题(必填)
    QString description;      ///< 反馈详细内容(必填)
    QList<QString> images;    ///< 图片URL数组，最多3张(可选)
    QString type;             ///< 反馈类型:bug/feature/other(必填)
    QString contact_info;     ///< 联系方式(必填)
    QString os_info;          ///< 操作系统信息(必填)
    QString config_url;       ///< 配置文件下载URL(必填)
} UserFeedBacksRequest;

///
/// \brief UserFeedbackResponse
/// 用户反馈 应答结构体信息
typedef struct UserFeedbackResponse
{
    QString code;    ///< 成功:"success"
    QString message; ///< 成功:"success"
    typedef struct ReturnData
    {
        QString ticket_no; ///< 工单号
    } returnData;
    returnData data;

} UserFeedbackResponse;

///
/// \brief ProcessUserFeedbackResult    解析用户反馈请求的 应答信息(Json格式)
/// \param responseData 数据结构体
/// \param jsonDocument 应答的json数据
/// \return 解析成功返回 true,失败返回 false
///
bool ProcessUserFeedbackResult(DeSheng::UserFeedbackResponse &responseData,
                               QJsonDocument &jsonDocument);
///
/// \brief userFeedbackRequestToJson    构造反馈信息结构体的Json对象
/// \param req  请求信息结构体
/// \return 请求结构体的 QJsonObject 对象,
///
QJsonObject userFeedbackRequestToJson(const UserFeedBacksRequest &req);

///
/// \brief buildFeedbackQuery 校验并构建查询参数
/// \deprecated 提交反馈走 POST JSON body（非 query string），此函数将 POST 字段错误放入 query。
///             现有调用方仅用其做字段校验（query 未实际使用），保留以兼容调用方，新代码不应使用。
/// \param req      [in] 请求数据
/// \param query    [out] 构建好的 QUrlQuery 对象（仅当返回 true 时有效）
/// \param error    [out] 错误信息（当返回 false 时描述缺失字段）
/// \return         表示校验通过且 query 已填充；false 表示存在必填字段缺失
///
bool buildFeedbackQuery(const UserFeedBacksRequest &req, QUrlQuery &query, QString &error);

/// GetFeedbackListRequest 获取反馈列表 请求结构体
typedef struct GetFeedbackListRequest
{
    int page = 1;       ///< 页码，默认 1 (可选)
    int page_size = 10; ///< 每页数量，默认 10 (可选)
} GetFeedbackListRequest;

/// GetFeedbackListResponse 获取反馈列表 应答结构体
typedef struct GetFeedbackListResponse
{
    QString code;
    QString message;
    typedef struct ListItem
    {
        int id;                ///< 反馈 ID
        QString ticket_no;     ///< 工单号
        QString title;         ///< 反馈标题
        QString type;          ///< 反馈类型
        QString status;        ///< 状态
        QString device_type;   ///< 设备类型
        QString created_at;    ///< 创建时间
    } ListItem;
    typedef struct ReturnData
    {
        QList<ListItem> list;
        int total;
        int page;
    } ReturnData;
    ReturnData data;
} GetFeedbackListResponse;

bool ProcessGetFeedbackListResult(GetFeedbackListResponse &responseData,
                                  QJsonDocument &jsonDocument);

QJsonObject GetFeedbackListRequestToJson(const GetFeedbackListRequest &req);

bool buildGetFeedbackListQuery(const GetFeedbackListRequest &req,
                                QUrlQuery &query,
                                QString &error);

/// GetFeedbackDetailRequest 获取反馈详情 请求结构体
typedef struct GetFeedbackDetailRequest
{
    QString ticket_no; ///< 工单号（路径参数，必填）
} GetFeedbackDetailRequest;

/// GetFeedbackDetailResponse 获取反馈详情 应答结构体
typedef struct GetFeedbackDetailResponse
{
    QString code;
    QString message;
    typedef struct ReturnData
    {
        int id;                  ///< 反馈 ID
        QString ticket_no;       ///< 工单号
        QString title;           ///< 反馈标题
        QString description;     ///< 详细描述
        QStringList images;      ///< 图片 URL 数组
        QString type;            ///< 反馈类型
        QString status;          ///< 状态
        QString device_id;       ///< 设备 ID
        QString drive_id;        ///< 驱动 ID
        QString firmware_id;     ///< 固件 ID
        QString drive_version;   ///< 驱动版本号
        QString firmware_version;///< 固件版本号
        QString receiver_version;///< 接收器版本号
        QString device_name;     ///< 设备名称
        QString device_type;     ///< 设备类型
        QString created_at;      ///< 创建时间
        QString updated_at;      ///< 更新时间
    } ReturnData;
    ReturnData data;
} GetFeedbackDetailResponse;

bool ProcessGetFeedbackDetailResult(GetFeedbackDetailResponse &responseData,
                                    QJsonDocument &jsonDocument);

QJsonObject GetFeedbackDetailRequestToJson(const GetFeedbackDetailRequest &req);

bool buildGetFeedbackDetailQuery(const GetFeedbackDetailRequest &req,
                                  QUrlQuery &query,
                                  QString &error);

/*************************************************************************************  管理端-反馈管理  ************************************************************************************************/

/// AdminFeedbackListRequest 管理端获取反馈列表 请求结构体
typedef struct AdminFeedbackListRequest
{
    int page        = 1;       ///< 页码，默认 1（可选）
    int page_size   = 10;      ///< 每页数量，默认 10，最大 100（可选）
    QString status;            ///< 状态筛选：pending/processing/resolved/closed（可选）
    QString type;              ///< 类型筛选：bug/feature/other（可选）
    QString keyword;           ///< 关键词搜索标题（可选）
} AdminFeedbackListRequest;

/// AdminFeedbackListResponse 管理端获取反馈列表 应答结构体
typedef struct AdminFeedbackListResponse
{
    QString code;
    QString message;
    typedef struct ListItem
    {
        int id;
        QString ticket_no;
        QString title;
        QString description;
        QStringList images;
        QString type;
        QString status;
        QString user_id;
        QString device_name;
        QString device_type;
        QString drive_version;
        QString firmware_version;
        QString receiver_version;
        QString contact_info;
        QString os_info;
        QString created_at;
        QString updated_at;
    } ListItem;
    typedef struct ReturnData
    {
        QList<ListItem> list;
        int total;
        int page;
    } ReturnData;
    ReturnData data;
} AdminFeedbackListResponse;

bool ProcessAdminFeedbackListResult(AdminFeedbackListResponse &responseData,
                                     QJsonDocument &jsonDocument);

QJsonObject AdminFeedbackListRequestToJson(const AdminFeedbackListRequest &req);

bool buildAdminFeedbackListQuery(const AdminFeedbackListRequest &req,
                                  QUrlQuery &query,
                                  QString &error);

/// AdminFeedbackDetailResponse 管理端获取反馈详情 应答结构体
typedef struct AdminFeedbackDetailResponse
{
    QString code;
    QString message;
    typedef struct ReturnData
    {
        int id;
        QString ticket_no;
        QString user_id;
        QString title;
        QString description;
        QStringList images;
        QString type;
        QString status;
        QString device_id;
        QString drive_id;
        QString firmware_id;
        QString drive_version;
        QString firmware_version;
        QString receiver_version;
        QString device_name;
        QString device_type;
        QString contact_info;
        QString login_ip;
        QString os_info;
        QString config_url;
        QString created_at;
        QString updated_at;
        QString resolved_at;
    } ReturnData;
    ReturnData data;
} AdminFeedbackDetailResponse;

bool ProcessAdminFeedbackDetailResult(AdminFeedbackDetailResponse &responseData,
                                       QJsonDocument &jsonDocument);

QJsonObject AdminFeedbackDetailRequestToJson(const QString &ticket_no);

/// AdminFeedbackStatusRequest 管理端更新反馈状态 请求结构体
typedef struct AdminFeedbackStatusRequest
{
    QString ticket_no; ///< 工单号（路径参数，必填）
    QString status;    ///< 新状态：pending/processing/resolved/closed（必填）
} AdminFeedbackStatusRequest;

/// AdminFeedbackStatusResponse 管理端更新反馈状态 应答结构体
typedef struct AdminFeedbackStatusResponse
{
    QString code;
    QString message;
    /// data 为 null，无需解析
} AdminFeedbackStatusResponse;

bool ProcessAdminFeedbackStatusResult(AdminFeedbackStatusResponse &responseData,
                                       QJsonDocument &jsonDocument);

QJsonObject AdminFeedbackStatusRequestToJson(const AdminFeedbackStatusRequest &req);

bool buildAdminFeedbackStatusQuery(const AdminFeedbackStatusRequest &req,
                                    QUrlQuery &query,
                                    QString &error);

} // namespace DeSheng

// 调用示例（新栈）— 文件上传
// QHttpMultiPart *t_mp = new QHttpMultiPart(QHttpMultiPart::FormDataType);
// QHttpPart t_filePart;
// t_filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
//     "form-data; name=\"file\"; filename=\"screenshot.png\"");
// t_filePart.setHeader(QNetworkRequest::ContentTypeHeader, "image/png");
// QFile *t_file = new QFile("screenshot.png");
// t_file->open(QIODevice::ReadOnly);
// t_filePart.setBodyDevice(t_file);
// t_file->setParent(t_mp);
// t_mp->append(t_filePart);
// auto &cli = HttpClient::instance();  /// network/http_client.h
// QNetworkReply *r = cli.upload(DeSheng::ApiPaths::kFeedbackUpload, t_mp, RequestOptions{}.withTag("feedback"));
// connect(r, &QNetworkReply::finished, [r]() {
//     FileUploadsResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(r->readAll());
//     if (ProcessFileUploadsResult(t_resp, t_doc) && t_resp.code == "success") {
//         // t_resp.data.url ...
//     }
//     r->deleteLater();
// });
//
// 调用示例（新栈）— 提交用户反馈
// UserFeedBacksRequest t_req;
// t_req.device_id = "xxx"; t_req.title = "Bug report";
// QByteArray t_body = QJsonDocument(userFeedbackRequestToJson(t_req)).toJson();
// QNetworkReply *r2 = cli.post(DeSheng::ApiPaths::kFeedbackList, RequestOptions{}.withBody(t_body).withTag("feedback"));
// connect(r2, &QNetworkReply::finished, [r2]() {
//     UserFeedbackResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(r2->readAll());
//     if (ProcessUserFeedbackResult(t_resp, t_doc) && t_resp.code == "success") {
//         // t_resp.data.ticket_no ...
//     }
//     r2->deleteLater();
// });
//
// 调用示例（新栈）— 获取反馈列表
// GetFeedbackListRequest t_req3;
// t_req3.page = 1; t_req3.page_size = 10;
// QUrlQuery t_query;
// QString t_err;
// buildGetFeedbackListQuery(t_req3, t_query, t_err);
// QNetworkReply *r3 = cli.get(DeSheng::ApiPaths::kFeedbackList, RequestOptions{}.withQuery(t_query).withTag("feedback"));
// connect(r3, &QNetworkReply::finished, [r3]() {
//     GetFeedbackListResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(r3->readAll());
//     if (ProcessGetFeedbackListResult(t_resp, t_doc) && t_resp.code == "success") {
//         // t_resp.data.list ...
//     }
//     r3->deleteLater();
// });
//
// 调用示例（新栈）— 获取反馈详情
// GetFeedbackDetailRequest t_req4;
// t_req4.ticket_no = "TK-001";
// QNetworkReply *r4 = cli.get(QString(DeSheng::ApiPaths::kFeedbackDetail).arg(t_req4.ticket_no), RequestOptions{}.withTag("feedback"));
// connect(r4, &QNetworkReply::finished, [r4]() {
//     GetFeedbackDetailResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(r4->readAll());
//     if (ProcessGetFeedbackDetailResult(t_resp, t_doc) && t_resp.code == "success") {
//         // t_resp.data.description ...
//     }
//     r4->deleteLater();
// });
//
// 推荐写法（HttpClient + RequestOptions）
// auto &cli = HttpClient::instance();
// QNetworkReply *r = cli.post(DeSheng::ApiPaths::kFeedbackList, RequestOptions{}.withBody(t_body).withTag("feedback"));
// // multipart 上传: cli.upload(DeSheng::ApiPaths::kFeedbackUpload, t_mp, RequestOptions{}.withTag("feedback"));
// connect(r, &QNetworkReply::finished, [r]() { ... ; r->deleteLater(); });

#endif // FEEDBACK_API_H
