#ifndef USER_CONFIG_API_H
#define USER_CONFIG_API_H

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QUrlQuery>

namespace DeSheng {

/// 路径常量
namespace ApiPaths {
inline constexpr const char *kConfigBase         = "/user-configs";
/// 子路径使用 %1 占位 ID，如 QString(kConfigLike).arg(42) → "/user-configs/42/like"
inline constexpr const char *kConfigDetail       = "/user-configs/%1";
inline constexpr const char *kConfigLike         = "/user-configs/%1/like";
inline constexpr const char *kConfigDislike      = "/user-configs/%1/dislike";
inline constexpr const char *kConfigCollect      = "/user-configs/%1/collect";
inline constexpr const char *kConfigComments     = "/user-configs/%1/comments";
inline constexpr const char *kConfigShare        = "/user-configs/%1/share";
inline constexpr const char *kConfigDownload     = "/user-configs/%1/download";
inline constexpr const char *kConfigPin          = "/user-configs/%1/pin";
inline constexpr const char *kConfigMy           = "/user-configs/my";
inline constexpr const char *kConfigCollectsMy   = "/user-configs/collects/my";
inline constexpr const char *kConfigLikesMy      = "/user-configs/likes/my";
inline constexpr const char *kConfigTodayCount   = "/user-configs/today-count";
inline constexpr const char *kConfigPinnedCount  = "/user-configs/pinned-count";
inline constexpr const char *kConfigUser         = "/user-configs/user/%1";
inline constexpr const char *kConfigShareDownload = "/user-configs/share/%1/download";
} // namespace ApiPaths

/// 方案可见性取值 / Config visibility values
inline constexpr const char *kVisibilityPublic  = "public";
inline constexpr const char *kVisibilityPrivate = "private";

/// 用户角色取值 / User role values
inline constexpr const char *kRoleStreamer     = "streamer";
inline constexpr const char *kRoleOfficial     = "official";
inline constexpr const char *kRoleProfessional = "professional";

/// 用户头衔取值 / User title values
inline constexpr const char *kTitleExpert = "expert";


/*************************************************************************************  预设配置方案相关  ************************************************************************************************/
/// 创建配置请求结构
typedef struct UserConfigsCreateRequest
{
    QString device_id;        /// 设备ID (必填)
    QString drive_version;    ///驱动版本号 (必填)
    QString firmware_version; ///固件版本号 (必填)
    QString device_name;      ///设备名称 (必填)
    QString device_type;      ///设备类型：mouse/keyboard/headset (必填)
    QString title;            /// 方案名称 (必填)
    QString description;      /// 方案描述，最多1000字符 (可选)
    QList<QString> user_tags; /// 用户标签数组，最多10个，单标签最多50字符 (可选)
    QString config_url;       /// 配置文件下载URL (必填)
    QString language;         /// 配置语言：zh/en (可选，默认zh)
    QString visibility;       /// 可见性：public/private (可选，默认public)
} UserConfigsCreateRequest;

/// 创建配置应答结构
typedef struct UserConfigsCreateResponse
{
    QString code;
    QString message;
    typedef struct Author
    {
        int user_id;
        QString username;
        QString avatar;
        QString nickname;
        int level;
        QStringList roles;   /// 用户角色
        QStringList titles;  /// 用户头衔
    } Author;

    typedef struct ReturnData
    {
        int id;
        Author author;
        QString device_id;
        QString drive_version;
        QString firmware_version;
        QString device_name;
        QString device_type;
        QString title;
        QString description;
        QString language;
        QString visibility;
        QStringList user_tags;
        int download_count;
        int collect_count;
        int like_count;
        int share_count;
        int dislike_count;
        int like_dislike_score;
        int hot_score;
        QString share_code;
        QString status;
        QString created_at;
        QString updated_at;
        QString published_at;
        QString modified_at;
        bool is_official_tag;
        bool is_expert_tag;
    } ReturnData;

    ReturnData data;
} UserConfigsCreateResponse;

//创建配置接口，服务器回应解析
bool ProcessUserConfigsCreateResult(DeSheng::UserConfigsCreateResponse &responseData,
                                    QJsonDocument &jsonDocument);
//结构体传递得到QJsonObject
QJsonObject UserConfigsCreateRequestToJson(const UserConfigsCreateRequest &req);



/// 获取今日创建数量 应答结构体（GET /api/v1/user-configs/today-count）
typedef struct GetTodayCountListResponse
{
    QString code;
    QString message;
    typedef struct ReturnData
    {
        QString device_name;
        int today_count;
    } ReturnData;
    ReturnData data;
} GetTodayCountListResponse;
//获取今日创建数量接口，服务器回应解析
bool ProcessGetTodayCountListResult(DeSheng::GetTodayCountListResponse &responseData,
                                    QJsonDocument &jsonDocument);




/// 获取公开配置列表 请求结构体
typedef struct GetPublicConfigurationListRequest
{
    ///（全部非必填）
    QString keyword;         /// 关键词搜索（搜索 title + user_tags）
    QString username;        ///按作者用户名模糊搜索
    QString device_name;     ///设备名称筛选
    QString device_type;     ///设备类型：mouse/keyboard/headset
    QString language;        ///语言筛选：zh/en，不传则查全部
    QString user_tag;        ///用户标签精确筛选，匹配单个标签
    QString sort;            ///排序方式：hot(热门)/download(下载)/like(点赞)/collect(收藏)/share(分享)/score(得分)/new(最新)，默认 new
    QString start_time;      ///开始时间（RFC3339格式）
    QString end_time;        ///结束时间（RFC3339格式）
    bool is_official_tag;    ///筛选官方标签配置，true=只查官方配置
    bool is_expert_tag;      ///筛选大神标签配置，true=只查大神配置
    int page = 1;            /// 页码，默认 1
    int page_size = 20;      /// 每页数量，默认 20，最大 100
} GetPublicConfigurationListRequest;

/// 获取公开配置列表 应答结构体
typedef struct GetPublicConfigurationListResponse
{
    QString code;
    QString message;

    typedef struct Comment
    {
        int id;                  ///评论ID
        QString comment_text;    ///评论内容（中文）
        QString comment_text_en; ///评论内容（英文）
        int count;               ///该配置被点击此评论的次数
        bool is_clicked;         ///当前用户是否已点击该评论（未登录用户为 false）
    } Comment;

    typedef struct Author
    {
        int user_id;
        QString username;
        QString avatar;
        QString nickname;
        int level;
        QStringList roles;   /// 用户角色
        QStringList titles;  /// 用户头衔
    } Author;

    typedef struct ListItem
    {
        int id;
        Author author;
        QString device_id;
        QString drive_version;
        QString firmware_version;
        QString device_name;
        QString device_type;
        QString title;
        QString description;      ///< 方案描述
        QString language;         ///< 配置语言
        QString visibility;       ///< 可见性
        QStringList user_tags;
        int download_count;
        int collect_count;
        int like_count;
        int share_count;
        int dislike_count;      /// 踩数量
        int like_dislike_score; ///点赞踩得分（点赞数-踩数）
        int hot_score;
        QString status;
        QString created_at;
        QString updated_at;
        bool is_official_tag;    ///是否官方标签配置
        bool is_expert_tag;      ///是否大神标签配置
        bool is_collected;       ///当前用户是否已收藏该配置（未登录用户为 false）
        bool is_liked;           ///当前用户是否已点赞该配置（未登录用户为 false）
        bool is_disliked;        ///当前用户是否已踩该配置（未登录用户为 false）
        bool is_pinned;          ///是否已钉选（仅 my / user 列表接口返回，公开列表恒 false）
        QList<Comment> comments; ///评论列表，包含该配置的所有评论及点击统计
    } ListItem;

    typedef struct ReturnData
    {
        QList<ListItem> list;
        int total;
        int page;
        int page_size;
    } ReturnData;

    ReturnData data;
} GetPublicConfigurationListResponse;

// GET /api/v1/user-configs?keyword=游戏&device_type=mouse&sort=hot&page=1&page_size=20
// # 只查询官方推荐配置
// GET /api/v1/user-configs?is_official_tag=true
// # 只查询大神配置
// GET /api/v1/user-configs?is_expert_tag=true
// # 同时筛选官方和大神配置
// GET /api/v1/user-configs?is_official_tag=true&is_expert_tag=true

bool ProcessGetPublicConfigurationListResult(
    DeSheng::GetPublicConfigurationListResponse &responseData, QJsonDocument &jsonDocument);

QJsonObject GetPublicConfigurationListRequestToJson(const GetPublicConfigurationListRequest &req);

bool buildGetPublicConfigurationListQuery(const GetPublicConfigurationListRequest &req,
                                          QUrlQuery &query,
                                          QString &error);

/// 获取配置详细 请求结构
typedef struct GetConfigurationDetailsRequest
{
    QString id; /// 配置文件id (必填) ///接口路径: GET /api/v1/user-configs/:id
} GetConfigurationDetailsRequest;

/// 获取配置详细 应答结构
typedef struct GetConfigurationDetailsResponse
{
    QString code;
    QString message;

    typedef struct Comment
    {
        int id;                  ///评论ID
        QString comment_text;    ///评论内容（中文）
        QString comment_text_en; ///评论内容（英文）
        int count;               ///该配置被点击此评论的次数
        bool is_clicked;         ///当前用户是否已点击该评论（未登录用户为 false）
    } Comment;

    typedef struct Author
    {
        int user_id;
        QString username;
        QString avatar;
        QString nickname;
        int level;
        QStringList roles;   /// 用户角色
        QStringList titles;  /// 用户头衔
    } Author;

    typedef struct ReturnData
    {
        int id;
        Author author;
        QString device_id;
        QString drive_version;
        QString firmware_version;
        QString device_name;
        QString device_type;
        QString title;
        QString description;
        QString language;
        QString visibility;
        QStringList user_tags;
        int download_count;
        int collect_count;
        int like_count;
        int dislike_count;      ///踩数量
        int like_dislike_score; ///点赞踩得分（点赞数-踩数）
        int share_count;
        int hot_score;
        QString config_url;
        QString status;
        QString created_at;
        QString updated_at;
        QString published_at;
        QString modified_at;
        bool is_official_tag;    ///是否官方标签配置
        bool is_expert_tag;      ///是否大神标签配置
        bool is_collected;       ///当前用户是否已收藏该配置（未登录用户为 false）
        bool is_liked;           ///当前用户是否已点赞该配置（未登录用户为 false）
        bool is_disliked;        ///当前用户是否已踩该配置（未登录用户为 false）
        QList<Comment> comments; ///评论列表，包含该配置的所有评论及点击统计
    } ReturnData;

    ReturnData data;
} GetConfigurationDetailsResponse;

bool ProcessGetConfigurationDetailsResult(GetConfigurationDetailsResponse &responseData,
                                          QJsonDocument &jsonDocument);

/// 获取我的配置列表 请求结构
typedef struct GetMyConfigurationListRequest
{
    /// 全部非必填
    QString status;      /// 状态筛选：active/rejected，不传则返回 active 和 rejected
    QString language;    /// 语言筛选：zh/en，不传则查全部
    QString device_name; /// 设备名称筛选
    QString device_type; ///< 设备类型：mouse/keyboard/headset（本项目固定 headset）
    int page = 1;        /// 页码，默认 1
    int page_size = 20;  /// 每页数量，默认 20，最大 100

} GetMyConfigurationListRequest;

/// 获取我的配置列表 应答结构
typedef struct GetMyConfigurationListResponse
{
    QString code;
    QString message;

    typedef struct Author
    {
        int user_id;        ///< 用户ID
        QString username;   ///< 用户名
        QString avatar;     ///< 头像URL
        QString nickname;   ///< 用户昵称
        int level;          ///< 用户等级
        QStringList roles;  ///< 用户角色
        QStringList titles; ///< 用户头衔
    } Author;

    typedef struct ListItem
    {
        int id;
        Author author;                              ///< 作者信息
        QString device_id;
        QString drive_version;                      ///< 驱动版本号
        QString firmware_version;                   ///< 固件版本号
        QString device_name;
        QString device_type;
        QString title;
        QString description;                        ///< 方案描述
        QString language;                           ///< 配置语言
        QString visibility;                         ///< 可见性
        QStringList user_tags;
        int download_count;
        int collect_count;
        int like_count;
        int share_count;
        int dislike_count;                          ///< 踩数量
        int like_dislike_score;                     ///< 点赞踩得分
        int hot_score;
        QString config_url;                         ///< 配置文件下载URL（仅自己的配置返回）
        QString status;
        QString created_at;
        QString updated_at;
        bool is_official_tag;
        bool is_expert_tag;
        bool is_pinned;                             ///< 是否已钉选
        QString pinned_at;                          ///< 钉选时间
        bool is_collected;                          ///< 当前用户是否已收藏
        bool is_liked;                              ///< 当前用户是否已点赞
        bool is_disliked;                           ///< 当前用户是否已踩

        typedef struct CommentItem
        {
            int id;                  ///< 评论ID
            QString comment_text;    ///< 评论内容（中文）
            QString comment_text_en; ///< 评论内容（英文）
            int count;               ///< 该配置被点击此评论的次数
            bool is_clicked;         ///< 当前用户是否已点击
        } CommentItem;
        QList<CommentItem> comments;                ///< 评论列表
    } ListItem;

    typedef struct ReturnData
    {
        QList<ListItem> list;
        int total;
        int page;
        int page_size;
    } ReturnData;

    ReturnData data;
} GetMyConfigurationListResponse;

bool ProcessGetMyConfigurationListResult(DeSheng::GetMyConfigurationListResponse &responseData,
                                         QJsonDocument &jsonDocument);

QJsonObject GetMyConfigurationListRequestToJson(const GetMyConfigurationListRequest &req);

bool buildGetMyConfigurationListQuery(const GetMyConfigurationListRequest &req,
                                      QUrlQuery &query,
                                      QString &error);

// 调用示例（新栈）— 获取我的配置列表
// GetMyConfigurationListRequest req;
// req.status = "active";
// req.page = 1;
// req.page_size = 20;
// QUrlQuery query;
// QString error;
// buildGetMyConfigurationListQuery(req, query, error);

// auto &cli = HttpClient::instance();  /// network/http_client.h
// QNetworkReply *reply = cli.get(DeSheng::ApiPaths::kConfigBase, RequestOptions{}.withQuery(query).withTag("userConfig"));

/// 获取指定用户的配置列表 请求结构
typedef struct GetTargetUserConfigurationsRequest
{
    int64_t user_id;     /// 用户ID（路径参数，必填）
    QString keyword;     /// 关键词搜索（搜索 title + user_tags）
    QString device_name; /// 设备名称筛选
    QString device_type; /// 设备类型：mouse/keyboard/headset
    QString language;    /// 语言筛选：zh/en，不传则查全部
    QString sort;        /// 排序方式：hot/download/like/collect/share/score/new，默认 new
    int page = 1;        /// 页码，默认 1
    int page_size = 20;  /// 每页数量，默认 20，最大 100
} GetTargetUserConfigurationsRequest;

/// 获取指定用户的配置列表 应答结构
typedef struct GetTargetUserConfigurationsResponse
{
    QString code;
    QString message;

    typedef struct Author
    {
        int user_id;
        QString username;
        QString avatar;
        QString nickname;
        int level;
        QStringList roles;   /// 用户角色
        QStringList titles;  /// 用户头衔
    } Author;

    typedef struct ListItem
    {
        int id;
        Author author;
        QString device_id;
        QString drive_version;
        QString firmware_version;
        QString device_name;
        QString device_type;
        QString title;
        QString description;    ///< 方案描述
        QString language;       ///< 配置语言
        QString visibility;     ///< 可见性
        QStringList user_tags;
        int download_count;
        int collect_count;
        int like_count;
        int share_count;
        int dislike_count;      ///踩数量
        int like_dislike_score; ///点赞踩得分（点赞数-踩数）
        int hot_score;
        QString status;
        QString created_at;
        QString updated_at;
        bool is_official_tag; ///是否官方标签配置
        bool is_expert_tag;   ///是否大神标签配置
        bool is_pinned;       ///是否已钉选
        QString pinned_at;    ///钉选时间
        bool is_collected;    ///当前登录用户是否已收藏该配置
        bool is_liked;        ///当前登录用户是否已点赞该配置
        bool is_disliked;     ///当前登录用户是否已踩该配置

        typedef struct Comment
        {
            int id;                  ///< 评论ID
            QString comment_text;    ///< 评论内容（中文）
            QString comment_text_en; ///< 评论内容（英文）
            int count;               ///< 该配置被点击此评论的次数
            bool is_clicked;         ///< 当前用户是否已点击
        } Comment;
        QList<Comment> comments; ///< 评论列表
    } ListItem;

    typedef struct ReturnData
    {
        QList<ListItem> list;
        int total;
        int page;
        int page_size;
    } ReturnData;

    ReturnData data;
} GetTargetUserConfigurationsResponse;

bool ProcessGetTargetUserConfigurationsResult(GetTargetUserConfigurationsResponse &responseData,
                                              QJsonDocument &jsonDocument);

QJsonObject GetTargetUserConfigurationsRequestToJson(const GetTargetUserConfigurationsRequest &req);

bool buildGetTargetUserConfigurationsQuery(const GetTargetUserConfigurationsRequest &req,
                                           QUrlQuery &query,
                                           QString &error);

/// UpdateUserConfigRequest 更新配置 请求结构体（不允许修改 device_name / device_type）
typedef struct UpdateUserConfigRequest
{
    QString title;            ///< 方案名称 (可选)
    QString description;      ///< 方案描述，最多1000字符 (可选)
    QString language;         ///< 配置语言：zh/en (可选)
    QString visibility;       ///< 可见性：public/private (可选)
    QString config_url;       ///< 配置文件下载URL (可选)
    QStringList user_tags;    ///< 用户标签数组，最多10个 (可选)
} UpdateUserConfigRequest;

/// UpdateUserConfigResponse 更新配置 应答结构体
typedef struct UpdateUserConfigResponse
{
    QString code;
    QString message;

    typedef struct Author
    {
        int user_id;
        QString username;
        QString avatar;
        QString nickname;
        int level;
        QStringList roles;   /// 用户角色
        QStringList titles;  /// 用户头衔
    } Author;

    typedef struct ReturnData
    {
        int id;
        Author author;
        QString device_id;
        QString drive_version;
        QString firmware_version;
        QString device_name;
        QString device_type;
        QString title;
        QString description;
        QString language;
        QString visibility;
        QStringList user_tags;
        QString config_url;
        int download_count;
        int collect_count;
        int like_count;
        int share_count;
        int dislike_count;
        int like_dislike_score;
        int hot_score;
        QString status;
        QString created_at;
        QString updated_at;
        QString published_at;
        QString modified_at;
        bool is_official_tag;
        bool is_expert_tag;
    } ReturnData;
    ReturnData data;
} UpdateUserConfigResponse;

bool ProcessUpdateUserConfigResult(UpdateUserConfigResponse &responseData,
                                   QJsonDocument &jsonDocument);

QJsonObject UpdateUserConfigRequestToJson(const UpdateUserConfigRequest &req);

bool buildUpdateUserConfigQuery(const UpdateUserConfigRequest &req,
                                 QUrlQuery &query,
                                 QString &error);

/// 删除配置 请求结构
typedef struct DeleteUserConfigRequest
{
    int id; /// 配置ID（路径参数，必填）
} DeleteUserConfigRequest;

/// 删除配置 应答结构
typedef struct DeleteUserConfigResponse
{
    QString code;
    QString message;
    /// data 为 null，无需解析，根据状态码进行判断操作是否成功
} DeleteUserConfigResponse;

bool ProcessDeleteUserConfigResult(DeleteUserConfigResponse &responseData,
                                   QJsonDocument &jsonDocument);

QJsonObject DeleteUserConfigRequestToJson(const DeleteUserConfigRequest &req);

bool buildDeleteUserConfigQuery(const DeleteUserConfigRequest &req,
                                QUrlQuery &query,
                                QString &error);
// 调用示例（新栈）— 删除配置
// DeleteUserConfigRequest req;
// req.id = 123;
// auto &cli = HttpClient::instance();  /// network/http_client.h
// QNetworkReply *reply = cli.del(QString(DeSheng::ApiPaths::kConfigDetail).arg(req.id), RequestOptions{}.withTag("userConfig"));

/// 下载指定配置 请求结构
typedef struct DownloadTargetConfigurationRequest
{
    int id; /// 配置ID（路径参数，必填）
} DownloadTargetConfigurationRequest;
///  接口路径: GET /api/v1/user-configs/:id/download    ///携带 user_token

/// 下载指定配置 应答结构
typedef struct DownloadTargetConfigurationResponse
{
    QString code;
    QString message;

    typedef struct ReturnData
    {
        QString config_url; /// 配置文件下载地址
    } ReturnData;

    ReturnData data;
} DownloadTargetConfigurationResponse;

bool ProcessDownloadTargetConfigurationResult(DownloadTargetConfigurationResponse &responseData,
                                              QJsonDocument &jsonDocument);

QJsonObject DownloadTargetConfigurationRequestToJson(const DownloadTargetConfigurationRequest &req);

bool buildDownloadTargetConfigurationQuery(const DownloadTargetConfigurationRequest &req,
                                           QUrlQuery &query,
                                           QString &error);

/// 通过分享码下载配置 请求结构
typedef struct DownloadConfigByShareCodeRequest
{
    QString share_code; /// 分享码（Base64 编码字符串，路径参数，必填）
} DownloadConfigByShareCodeRequest;

/// 通过分享码下载配置 应答结构
typedef struct DownloadConfigByShareCodeResponse
{
    QString code;
    QString message;

    typedef struct ReturnData
    {
        QString config_url; /// 配置文件下载地址
    } ReturnData;

    ReturnData data;
} DownloadConfigByShareCodeResponse;

/**
 * @brief 处理通过分享码下载配置接口的响应结果
 * @param responseData 响应数据
 * @param jsonDocument JSON文档
 * @return true 成功, false 失败
 */
bool ProcessDownloadConfigByShareCodeResult(DownloadConfigByShareCodeResponse &responseData,
                                            QJsonDocument &jsonDocument);

/**
 * @brief 将通过分享码下载配置请求结构体转换为JSON对象
 * @param req 请求结构体
 * @return QJsonObject（GET请求无Body，返回空对象）
 */
QJsonObject DownloadConfigByShareCodeRequestToJson(const DownloadConfigByShareCodeRequest &req);

/**
 * @brief 构建通过分享码下载配置接口的URL查询参数
 * @param req 请求结构体
 * @param query URL查询参数
 * @param error 错误信息
 * @return true 成功, false 失败
 */
bool buildDownloadConfigByShareCodeQuery(const DownloadConfigByShareCodeRequest &req,
                                         QUrlQuery &query,
                                         QString &error);

/// 分享配置 请求结构
typedef struct ShareConfigurationRequest
{
    int id; ///< 配置ID（路径参数，必填）
} ShareConfigurationRequest;

/// 分享配置 应答结构
typedef struct ShareConfigurationResponse
{
    QString code;
    QString message;

    typedef struct ReturnData
    {
        QString share_code; ///< 分享码
        int share_count;    ///< 当前分享次数
    } ReturnData;

    ReturnData data;
} ShareConfigurationResponse;

/**
 * @brief 处理分享配置接口的响应结果
 * @param responseData 响应数据
 * @param jsonDocument JSON文档
 * @return true 成功, false 失败
 */
bool ProcessShareConfigurationResult(ShareConfigurationResponse &responseData,
                                     QJsonDocument &jsonDocument);

/**
 * @brief 将分享配置请求结构体转换为JSON对象
 * @param req 请求结构体
 * @return QJsonObject（POST请求无Body，返回空对象）
 */
QJsonObject ShareConfigurationRequestToJson(const ShareConfigurationRequest &req);

/**
 * @brief 构建分享配置接口的URL查询参数
 * @param req 请求结构体
 * @param query URL查询参数
 * @param error 错误信息
 * @return true 成功, false 失败
 */
bool buildShareConfigurationQuery(const ShareConfigurationRequest &req,
                                  QUrlQuery &query,
                                  QString &error);

/// 点赞配置 请求结构
typedef struct LikeConfigurationRequest
{
    int id; ///< 配置ID（路径参数，必填）
} LikeConfigurationRequest;

/// 点赞配置 应答结构
typedef struct LikeConfigurationResponse
{
    QString code;
    QString message;
    // data 为 null，无需解析
} LikeConfigurationResponse;

// 函数声明
/**
 * @brief 处理点赞配置接口的响应结果
 * @param responseData 响应数据
 * @param jsonDocument JSON文档
 * @return true 成功, false 失败
 */
bool ProcessLikeConfigurationResult(LikeConfigurationResponse &responseData,
                                    QJsonDocument &jsonDocument);

/**
 * @brief 将点赞配置请求结构体转换为JSON对象
 * @param req 请求结构体
 * @return QJsonObject（POST请求无Body，返回空对象）
 */
QJsonObject LikeConfigurationRequestToJson(const LikeConfigurationRequest &req);

/**
 * @brief 构建点赞配置接口的URL查询参数
 * @param req 请求结构体
 * @param query URL查询参数
 * @param error 错误信息
 * @return true 成功, false 失败
 */
bool buildLikeConfigurationQuery(const LikeConfigurationRequest &req,
                                 QUrlQuery &query,
                                 QString &error);

// 调用示例（新栈）— 点赞配置
// LikeConfigurationRequest req;
// req.id = 123;

// auto &cli = HttpClient::instance();  /// network/http_client.h
// QNetworkReply *reply = cli.post(QString(DeSheng::ApiPaths::kConfigLike).arg(req.id), RequestOptions{}.withBody(QByteArray("{}")).withTag("userConfig"));

/// 取消点赞配置 请求结构
typedef struct CancelLikeConfigurationRequest
{
    int id; ///< 配置ID（路径参数，必填）
} CancelLikeConfigurationRequest;

/// 取消点赞配置 应答结构
typedef struct CancelLikeConfigurationResponse
{
    QString code;
    QString message;
    // data 为 null，无需解析
} CancelLikeConfigurationResponse;

// 函数声明
/**
 * @brief 处理取消点赞配置接口的响应结果
 * @param responseData 响应数据
 * @param jsonDocument JSON文档
 * @return true 成功, false 失败
 */
bool ProcessCancelLikeConfigurationResult(CancelLikeConfigurationResponse &responseData,
                                          QJsonDocument &jsonDocument);

/**
 * @brief 将取消点赞配置请求结构体转换为JSON对象
 * @param req 请求结构体
 * @return QJsonObject（DELETE请求无Body，返回空对象）
 */
QJsonObject CancelLikeConfigurationRequestToJson(const CancelLikeConfigurationRequest &req);

/**
 * @brief 构建取消点赞配置接口的URL查询参数
 * @param req 请求结构体
 * @param query URL查询参数
 * @param error 错误信息
 * @return true 成功, false 失败
 */
bool buildCancelLikeConfigurationQuery(const CancelLikeConfigurationRequest &req,
                                       QUrlQuery &query,
                                       QString &error);

// 调用示例（新栈）— 取消点赞配置
// CancelLikeConfigurationRequest req;
// req.id = 123;

// auto &cli = HttpClient::instance();  /// network/http_client.h
// QNetworkReply *reply = cli.del(QString(DeSheng::ApiPaths::kConfigLike).arg(req.id), RequestOptions{}.withTag("userConfig"));

/// 踩配置 请求结构
typedef struct DislikeConfigurationRequest
{
    int id; ///< 配置ID（路径参数，必填）
} DislikeConfigurationRequest;

/// 踩配置 应答结构
typedef struct DislikeConfigurationResponse
{
    QString code;
    QString message;
    // data 为 null，无需解析
} DislikeConfigurationResponse;

// 函数声明
/**
 * @brief 处理踩配置接口的响应结果
 * @param responseData 响应数据
 * @param jsonDocument JSON文档
 * @return true 成功, false 失败
 */
bool ProcessDislikeConfigurationResult(DislikeConfigurationResponse &responseData,
                                       QJsonDocument &jsonDocument);

/**
 * @brief 将踩配置请求结构体转换为JSON对象
 * @param req 请求结构体
 * @return QJsonObject（POST请求无Body，返回空对象）
 */
QJsonObject DislikeConfigurationRequestToJson(const DislikeConfigurationRequest &req);

/**
 * @brief 构建踩配置接口的URL查询参数
 * @param req 请求结构体
 * @param query URL查询参数
 * @param error 错误信息
 * @return true 成功, false 失败
 */
bool buildDislikeConfigurationQuery(const DislikeConfigurationRequest &req,
                                    QUrlQuery &query,
                                    QString &error);

// 调用示例（新栈）— 踩配置
// DislikeConfigurationRequest req;
// req.id = 123;

// auto &cli = HttpClient::instance();  /// network/http_client.h
// QNetworkReply *reply = cli.post(QString(DeSheng::ApiPaths::kConfigDislike).arg(req.id), RequestOptions{}.withBody(QByteArray("{}")).withTag("userConfig"));

/// 取消踩配置 请求结构
typedef struct CancelDislikeConfigurationRequest
{
    int id; ///< 配置ID（路径参数，必填）
} CancelDislikeConfigurationRequest;

/// 取消踩配置 应答结构
typedef struct CancelDislikeConfigurationResponse
{
    QString code;
    QString message;
    // data 为 null，无需解析
} CancelDislikeConfigurationResponse;

// 函数声明
/**
 * @brief 处理取消踩配置接口的响应结果
 * @param responseData 响应数据
 * @param jsonDocument JSON文档
 * @return true 成功, false 失败
 */
bool ProcessCancelDislikeConfigurationResult(CancelDislikeConfigurationResponse &responseData,
                                             QJsonDocument &jsonDocument);

/**
 * @brief 将取消踩配置请求结构体转换为JSON对象
 * @param req 请求结构体
 * @return QJsonObject（DELETE请求无Body，返回空对象）
 */
QJsonObject CancelDislikeConfigurationRequestToJson(const CancelDislikeConfigurationRequest &req);

/**
 * @brief 构建取消踩配置接口的URL查询参数
 * @param req 请求结构体
 * @param query URL查询参数
 * @param error 错误信息
 * @return true 成功, false 失败
 */
bool buildCancelDislikeConfigurationQuery(const CancelDislikeConfigurationRequest &req,
                                          QUrlQuery &query,
                                          QString &error);

// 调用示例（新栈）— 取消踩配置
// CancelDislikeConfigurationRequest req;
// req.id = 123;

// auto &cli = HttpClient::instance();  /// network/http_client.h
// QNetworkReply *reply = cli.del(QString(DeSheng::ApiPaths::kConfigDislike).arg(req.id), RequestOptions{}.withTag("userConfig"));

/// 收藏配置 请求结构
typedef struct CollectConfigurationRequest
{
    int id; ///< 配置ID（路径参数，必填）
} CollectConfigurationRequest;

/// 收藏配置 应答结构
typedef struct CollectConfigurationResponse
{
    QString code;
    QString message;
    // data 为 null，无需解析
} CollectConfigurationResponse;

// 函数声明
/**
 * @brief 处理收藏配置接口的响应结果
 * @param responseData 响应数据
 * @param jsonDocument JSON文档
 * @return true 成功, false 失败
 */
bool ProcessCollectConfigurationResult(CollectConfigurationResponse &responseData,
                                       QJsonDocument &jsonDocument);

/**
 * @brief 将收藏配置请求结构体转换为JSON对象
 * @param req 请求结构体
 * @return QJsonObject（POST请求无Body，返回空对象）
 */
QJsonObject CollectConfigurationRequestToJson(const CollectConfigurationRequest &req);

/**
 * @brief 构建收藏配置接口的URL查询参数
 * @param req 请求结构体
 * @param query URL查询参数
 * @param error 错误信息
 * @return true 成功, false 失败
 */
bool buildCollectConfigurationQuery(const CollectConfigurationRequest &req,
                                    QUrlQuery &query,
                                    QString &error);

// 调用示例（新栈）— 收藏配置
// CollectConfigurationRequest req;
// req.id = 123;

// auto &cli = HttpClient::instance();  /// network/http_client.h
// QNetworkReply *reply = cli.post(QString(DeSheng::ApiPaths::kConfigCollect).arg(req.id), RequestOptions{}.withBody(QByteArray("{}")).withTag("userConfig"));

/// 取消收藏配置 请求结构
typedef struct CancelCollectConfigurationRequest
{
    int id; ///< 配置ID（路径参数，必填）
} CancelCollectConfigurationRequest;

/// 取消收藏配置 应答结构
typedef struct CancelCollectConfigurationResponse
{
    QString code;
    QString message;
    // data 为 null，无需解析
} CancelCollectConfigurationResponse;

// 函数声明
/**
 * @brief 处理取消收藏配置接口的响应结果
 * @param responseData 响应数据
 * @param jsonDocument JSON文档
 * @return true 成功, false 失败
 */
bool ProcessCancelCollectConfigurationResult(CancelCollectConfigurationResponse &responseData,
                                             QJsonDocument &jsonDocument);

/**
 * @brief 将取消收藏配置请求结构体转换为JSON对象
 * @param req 请求结构体
 * @return QJsonObject（DELETE请求无Body，返回空对象）
 */
QJsonObject CancelCollectConfigurationRequestToJson(const CancelCollectConfigurationRequest &req);

/**
 * @brief 构建取消收藏配置接口的URL查询参数
 * @param req 请求结构体
 * @param query URL查询参数
 * @param error 错误信息
 * @return true 成功, false 失败
 */
bool buildCancelCollectConfigurationQuery(const CancelCollectConfigurationRequest &req,
                                          QUrlQuery &query,
                                          QString &error);

// 调用示例（新栈）— 取消收藏配置
// CancelCollectConfigurationRequest req;
// req.id = 123;

// auto &cli = HttpClient::instance();  /// network/http_client.h
// QNetworkReply *reply = cli.del(QString(DeSheng::ApiPaths::kConfigCollect).arg(req.id), RequestOptions{}.withTag("userConfig"));

/// 获取我的收藏列表 请求结构
typedef struct GetMyCollectionListRequest
{
    // 全部非必填
    QString device_name; ///< 设备名称筛选
    QString device_type; ///< 设备类型：mouse/keyboard/headset（本项目固定 headset）
    int page = 1;        ///< 页码，默认 1
    int page_size = 20;  ///< 每页数量，默认 20，最大 100
} GetMyCollectionListRequest;

/// 获取我的收藏列表 应答结构
typedef struct GetMyCollectionListResponse
{
    QString code;
    QString message;

    typedef struct Author
    {
        int user_id;        ///< 用户ID
        QString username;   ///< 用户名
        QString avatar;     ///< 头像URL
        QString nickname;   ///< 用户昵称
        int level;          ///< 用户等级
        QStringList roles;  ///< 用户角色
        QStringList titles; ///< 用户头衔
    } Author;

    typedef struct ListItem
    {
        int id;                   ///< 配置ID
        Author author;            ///< 作者信息
        QString device_id;        ///< 设备ID
        QString drive_version;    ///< 驱动版本
        QString firmware_version; ///< 固件版本
        QString device_name;      ///< 设备名称
        QString device_type;      ///< 设备类型：mouse/keyboard/headset
        QString title;            ///< 配置标题
        QString description;      ///< 方案描述
        QString language;         ///< 配置语言
        QString visibility;       ///< 可见性
        QStringList user_tags;    ///< 用户标签
        int download_count;       ///< 下载次数
        int collect_count;        ///< 收藏次数
        int like_count;           ///< 点赞次数
        int share_count;          ///< 分享次数
        int dislike_count;        ///< 踩数量
        int like_dislike_score;   ///< 点赞踩得分
        int hot_score;            ///< 热度得分
        QString status;           ///< 状态：active/rejected
        QString created_at;       ///< 创建时间
        QString updated_at;       ///< 更新时间
        bool is_official_tag;     ///< 是否官方标签配置
        bool is_expert_tag;       ///< 是否大神标签配置
        bool is_liked;            ///< 当前用户是否已点赞
        bool is_disliked;         ///< 当前用户是否已踩

        typedef struct CommentItem
        {
            int id;                  ///< 评论ID
            QString comment_text;    ///< 评论内容（中文）
            QString comment_text_en; ///< 评论内容（英文）
            int count;               ///< 该配置被点击此评论的次数
            bool is_clicked;         ///< 当前用户是否已点击
        } CommentItem;
        QList<CommentItem> comments; ///< 评论列表
    } ListItem;

    typedef struct ReturnData
    {
        QList<ListItem> list; ///< 收藏列表
        int total;            ///< 总数
        int page;             ///< 当前页码
        int page_size;        ///< 每页数量
    } ReturnData;

    ReturnData data;
} GetMyCollectionListResponse;

// 函数声明
/**
 * @brief 处理获取我的收藏列表接口的响应结果
 * @param responseData 响应数据
 * @param jsonDocument JSON文档
 * @return true 成功, false 失败
 */
bool ProcessGetMyCollectionListResult(GetMyCollectionListResponse &responseData,
                                      QJsonDocument &jsonDocument);

/**
 * @brief 将获取我的收藏列表请求结构体转换为JSON对象
 * @param req 请求结构体
 * @return QJsonObject（GET请求用Query参数，返回空对象）
 */
QJsonObject GetMyCollectionListRequestToJson(const GetMyCollectionListRequest &req);

/**
 * @brief 构建获取我的收藏列表接口的URL查询参数
 * @param req 请求结构体
 * @param query URL查询参数
 * @param error 错误信息
 * @return true 成功, false 失败
 */
bool buildGetMyCollectionListQuery(const GetMyCollectionListRequest &req,
                                   QUrlQuery &query,
                                   QString &error);

// 调用示例（新栈）— 获取我的收藏列表
// GetMyCollectionListRequest req;
// req.page = 1;
// QUrlQuery query;
// QString error;
// buildGetMyCollectionListQuery(req, query, error);

// auto &cli = HttpClient::instance();  /// network/http_client.h
// QNetworkReply *reply = cli.get(DeSheng::ApiPaths::kConfigCollectsMy, RequestOptions{}.withQuery(query).withTag("userConfig"));

/// 获取我的点赞列表 请求结构
typedef struct GetMyLikeListRequest
{
    // 全部非必填
    QString device_name; ///< 设备名称筛选
    QString device_type; ///< 设备类型：mouse/keyboard/headset（本项目固定 headset）
    int page = 1;        ///< 页码，默认 1
    int page_size = 20;  ///< 每页数量，默认 20，最大 100
} GetMyLikeListRequest;

/// 获取我的点赞列表 应答结构
typedef struct GetMyLikeListResponse
{
    QString code;
    QString message;

    typedef struct Author
    {
        int user_id;        ///< 用户ID
        QString username;   ///< 用户名
        QString avatar;     ///< 头像URL
        QString nickname;   ///< 用户昵称
        int level;          ///< 用户等级
        QStringList roles;  ///< 用户角色
        QStringList titles; ///< 用户头衔
    } Author;

    typedef struct ListItem
    {
        int id;                   ///< 配置ID
        Author author;            ///< 作者信息
        QString device_id;        ///< 设备ID
        QString drive_version;    ///< 驱动版本
        QString firmware_version; ///< 固件版本
        QString device_name;      ///< 设备名称
        QString device_type;      ///< 设备类型：mouse/keyboard/headset
        QString title;            ///< 配置标题
        QString description;      ///< 方案描述
        QString language;         ///< 配置语言
        QString visibility;       ///< 可见性
        QStringList user_tags;    ///< 用户标签
        int download_count;       ///< 下载次数
        int collect_count;        ///< 收藏次数
        int like_count;           ///< 点赞次数
        int share_count;          ///< 分享次数
        int dislike_count;        ///< 踩数量
        int like_dislike_score;   ///< 点赞踩得分
        int hot_score;            ///< 热度得分
        QString status;           ///< 状态：active/rejected
        QString created_at;       ///< 创建时间
        QString updated_at;       ///< 更新时间
        bool is_official_tag;     ///< 是否官方标签配置
        bool is_expert_tag;       ///< 是否大神标签配置
        bool is_collected;        ///< 当前用户是否已收藏
        bool is_liked;            ///< 当前用户是否已点赞（始终为 true）
        bool is_disliked;         ///< 当前用户是否已踩

        typedef struct CommentItem
        {
            int id;                  ///< 评论ID
            QString comment_text;    ///< 评论内容（中文）
            QString comment_text_en; ///< 评论内容（英文）
            int count;               ///< 该配置被点击此评论的次数
            bool is_clicked;         ///< 当前用户是否已点击
        } CommentItem;
        QList<CommentItem> comments; ///< 评论列表
    } ListItem;

    typedef struct ReturnData
    {
        QList<ListItem> list; ///< 点赞列表
        int total;            ///< 总数
        int page;             ///< 当前页码
        int page_size;        ///< 每页数量
    } ReturnData;

    ReturnData data;
} GetMyLikeListResponse;

// 函数声明
bool ProcessGetMyLikeListResult(GetMyLikeListResponse &responseData,
                                QJsonDocument &jsonDocument);

QJsonObject GetMyLikeListRequestToJson(const GetMyLikeListRequest &req);

bool buildGetMyLikeListQuery(const GetMyLikeListRequest &req,
                             QUrlQuery &query,
                             QString &error);

/// 获取配置评论列表 请求结构
typedef struct GetConfigCommentsRequest
{
    int64_t id; ///< 用户配置ID（user_configs.id），指定要获取哪个配置的评论列表
} GetConfigCommentsRequest;

/// 获取配置评论列表 应答结构
typedef struct GetConfigCommentsResponse
{
    QString code;
    QString message;

    typedef struct CommentItem
    {
        int id;                  ///< 评论ID
        QString comment_text;    ///< 评论内容（中文）
        QString comment_text_en; ///< 评论内容（英文）
        int count;               ///< 该配置被点击此评论的次数
        bool is_clicked;         ///< 当前用户是否已点击（未登录为false）
    } CommentItem;

    QList<CommentItem> data; ///< 评论列表（直接是数组，无分页）
} GetConfigCommentsResponse;

/**
 * @brief 处理获取配置评论列表接口的响应结果
 * @param responseData 响应数据
 * @param jsonDocument JSON文档
 * @return true 成功, false 失败
 */
bool ProcessGetConfigCommentsResult(GetConfigCommentsResponse &responseData,
                                    QJsonDocument &jsonDocument);

/**
 * @brief 将获取配置评论列表请求结构体转换为JSON对象
 * @param req 请求结构体
 * @return QJsonObject（GET请求无Body，返回空对象）
 */
QJsonObject GetConfigCommentsRequestToJson(const GetConfigCommentsRequest &req);

/**
 * @brief 构建获取配置评论列表接口的URL查询参数
 * @param req 请求结构体
 * @param query URL查询参数
 * @param error 错误信息
 * @return true 成功, false 失败
 */
bool buildGetConfigCommentsQuery(const GetConfigCommentsRequest &req,
                                 QUrlQuery &query,
                                 QString &error);

// 调用示例（新栈）— 获取配置评论列表
// GetConfigCommentsRequest req;
// req.id = 123;

// auto &cli = HttpClient::instance();  /// network/http_client.h
// QNetworkReply *reply = cli.get(QString(DeSheng::ApiPaths::kConfigComments).arg(req.id), RequestOptions{}.withTag("userConfig"));
// connect(reply, &QNetworkReply::finished, [reply]() {
//     GetConfigCommentsResponse resp;
//     QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
//     if (ProcessGetConfigCommentsResult(resp, doc)) {
//         for (const auto &comment : resp.data) {
//             qDebug() << comment.comment_text << comment.count;
//         }
//     }
//     reply->deleteLater();
// });

/// ClickCommentRequest 点击评论 请求结构体
typedef struct ClickCommentRequest
{
    int id;             ///< 配置ID（路径参数，必填）
    int64_t comment_id; ///< 评论ID（请求体，必填）
} ClickCommentRequest;

/// ClickCommentResponse 点击评论 应答结构体
typedef struct ClickCommentResponse
{
    QString code;
    QString message;
    /// data 为 null，无需解析
} ClickCommentResponse;

bool ProcessClickCommentResult(ClickCommentResponse &responseData,
                               QJsonDocument &jsonDocument);

QJsonObject ClickCommentRequestToJson(const ClickCommentRequest &req);

bool buildClickCommentQuery(const ClickCommentRequest &req,
                             QUrlQuery &query,
                             QString &error);

/// CancelClickCommentRequest 取消点击评论 请求结构体
typedef struct CancelClickCommentRequest
{
    int id;             ///< 配置ID（路径参数，必填）
    int64_t comment_id; ///< 评论ID（请求体，必填）
} CancelClickCommentRequest;

/// CancelClickCommentResponse 取消点击评论 应答结构体
typedef struct CancelClickCommentResponse
{
    QString code;
    QString message;
    /// data 为 null，无需解析
} CancelClickCommentResponse;

bool ProcessCancelClickCommentResult(CancelClickCommentResponse &responseData,
                                     QJsonDocument &jsonDocument);

QJsonObject CancelClickCommentRequestToJson(const CancelClickCommentRequest &req);

bool buildCancelClickCommentQuery(const CancelClickCommentRequest &req,
                                   QUrlQuery &query,
                                   QString &error);

// ──────────────────────────────────────── 钉选 ────────────────────────────────────────

/// 获取钉选数量 请求结构（GET /api/v1/user-configs/pinned-count?device_type=headset）
typedef struct GetPinnedCountRequest
{
    QString device_type; ///< 设备类型：mouse/keyboard/headset（必填）
} GetPinnedCountRequest;

/// 获取钉选数量 应答结构
typedef struct GetPinnedCountResponse
{
    QString code;
    QString message;

    typedef struct ReturnData
    {
        int count; ///< 当前已钉选数量
        int limit; ///< 钉选上限
    } ReturnData;

    ReturnData data;
} GetPinnedCountResponse;

bool ProcessGetPinnedCountResult(GetPinnedCountResponse &responseData,
                                  QJsonDocument &jsonDocument);

QJsonObject GetPinnedCountRequestToJson(const GetPinnedCountRequest &req);

bool buildGetPinnedCountQuery(const GetPinnedCountRequest &req,
                               QUrlQuery &query,
                               QString &error);

/// 钉选配置 请求结构（POST /api/v1/user-configs/:id/pin）
typedef struct PinConfigurationRequest
{
    int id; ///< 配置ID（路径参数，必填）
} PinConfigurationRequest;

/// 钉选配置 应答结构
typedef struct PinConfigurationResponse
{
    QString code;
    QString message;
    /// data 为 null，无需解析
} PinConfigurationResponse;

bool ProcessPinConfigurationResult(PinConfigurationResponse &responseData,
                                    QJsonDocument &jsonDocument);

QJsonObject PinConfigurationRequestToJson(const PinConfigurationRequest &req);

bool buildPinConfigurationQuery(const PinConfigurationRequest &req,
                                 QUrlQuery &query,
                                 QString &error);

/// 取消钉选配置 请求结构（DELETE /api/v1/user-configs/:id/pin）
typedef struct CancelPinConfigurationRequest
{
    int id; ///< 配置ID（路径参数，必填）
} CancelPinConfigurationRequest;

/// 取消钉选配置 应答结构
typedef struct CancelPinConfigurationResponse
{
    QString code;
    QString message;
    /// data 为 null，无需解析
} CancelPinConfigurationResponse;

bool ProcessCancelPinConfigurationResult(CancelPinConfigurationResponse &responseData,
                                          QJsonDocument &jsonDocument);

QJsonObject CancelPinConfigurationRequestToJson(const CancelPinConfigurationRequest &req);

bool buildCancelPinConfigurationQuery(const CancelPinConfigurationRequest &req,
                                       QUrlQuery &query,
                                       QString &error);

// ──────────────────────────────────────── 管理端配置 ────────────────────────────────────────

/// 管理端获取配置列表 请求结构（GET /api/v1/admin/user-configs）
typedef struct AdminGetUserConfigsRequest
{
    int64_t user_id = 0;       ///< 用户ID筛选
    QString status;             ///< 状态筛选：all/active/rejected，默认 all
    QString visibility;         ///< 可见性筛选：public/private
    QString language;           ///< 语言筛选：zh/en
    QString device_type;        ///< 设备类型：mouse/keyboard/headset
    QString device_name;        ///< 设备名称筛选（模糊匹配）
    QString keyword;            ///< 关键词搜索（搜索 title + user_tags）
    QString username;           ///< 按作者用户名模糊搜索
    QString user_tag;           ///< 用户标签精确筛选
    QString sort;               ///< 排序方式：hot/download/like/collect/share/score/new，默认 new
    QString start_time;         ///< 开始时间
    QString end_time;           ///< 结束时间
    bool is_official_tag;       ///< 筛选官方标签配置
    bool is_expert_tag;         ///< 筛选大神标签配置
    int page = 1;               ///< 页码，默认 1
    int page_size = 20;         ///< 每页数量，默认 20，最大 100
} AdminGetUserConfigsRequest;

/// 管理端获取配置列表 应答结构
typedef struct AdminGetUserConfigsResponse
{
    QString code;
    QString message;

    typedef struct Author
    {
        int user_id;
        QString username;
        QString avatar;
        QString nickname;
        int level;
        QStringList roles;
        QStringList titles;
    } Author;

    typedef struct ListItem
    {
        int id;
        Author author;
        QString device_id;
        QString drive_version;
        QString firmware_version;
        QString device_name;
        QString device_type;
        QString title;
        QString description;
        QString language;
        QString visibility;
        QStringList user_tags;
        int download_count;
        int collect_count;
        int like_count;
        int share_count;
        int hot_score;
        QString config_url;
        QString status;
        QString created_at;
        QString updated_at;
        bool is_official_tag;
        bool is_expert_tag;
    } ListItem;

    typedef struct ReturnData
    {
        QList<ListItem> list;
        int total;
        int page;
        int page_size;
    } ReturnData;

    ReturnData data;
} AdminGetUserConfigsResponse;

bool ProcessAdminGetUserConfigsResult(AdminGetUserConfigsResponse &responseData,
                                       QJsonDocument &jsonDocument);

QJsonObject AdminGetUserConfigsRequestToJson(const AdminGetUserConfigsRequest &req);

bool buildAdminGetUserConfigsQuery(const AdminGetUserConfigsRequest &req,
                                    QUrlQuery &query,
                                    QString &error);

/// 管理端获取配置详情 请求结构（GET /api/v1/admin/user-configs/:id）
typedef struct AdminGetUserConfigDetailRequest
{
    int id; ///< 配置ID（路径参数，必填）
} AdminGetUserConfigDetailRequest;

/// 管理端获取配置详情 应答结构
typedef struct AdminGetUserConfigDetailResponse
{
    QString code;
    QString message;

    typedef struct Comment
    {
        int id;
        QString comment_text;
        QString comment_text_en;
        int count;
        bool is_clicked;
    } Comment;

    typedef struct Author
    {
        int user_id;
        QString username;
        QString avatar;
        QString nickname;
        int level;
        QStringList roles;
        QStringList titles;
    } Author;

    typedef struct ReturnData
    {
        int id;
        Author author;
        QString device_id;
        QString drive_version;
        QString firmware_version;
        QString device_name;
        QString device_type;
        QString title;
        QString description;
        QString language;
        QString visibility;
        QStringList user_tags;
        int download_count;
        int collect_count;
        int like_count;
        int share_count;
        int hot_score;
        QString config_url;
        QString status;
        QString created_at;
        QString updated_at;
        QString published_at;
        QString modified_at;
        bool is_official_tag;
        bool is_expert_tag;
        QList<Comment> comments;
    } ReturnData;

    ReturnData data;
} AdminGetUserConfigDetailResponse;

bool ProcessAdminGetUserConfigDetailResult(AdminGetUserConfigDetailResponse &responseData,
                                            QJsonDocument &jsonDocument);

QJsonObject AdminGetUserConfigDetailRequestToJson(const AdminGetUserConfigDetailRequest &req);

bool buildAdminGetUserConfigDetailQuery(const AdminGetUserConfigDetailRequest &req,
                                         QUrlQuery &query,
                                         QString &error);

/// 管理端更新配置状态 请求结构（PUT /api/v1/admin/user-configs/:id/status）
typedef struct AdminUpdateUserConfigStatusRequest
{
    int id;         ///< 配置ID（路径参数，必填）
    QString status; ///< 新状态：active/rejected（必填）
} AdminUpdateUserConfigStatusRequest;

/// 管理端更新配置状态 应答结构
typedef struct AdminUpdateUserConfigStatusResponse
{
    QString code;
    QString message;
    /// data 为 null
} AdminUpdateUserConfigStatusResponse;

bool ProcessAdminUpdateUserConfigStatusResult(AdminUpdateUserConfigStatusResponse &responseData,
                                               QJsonDocument &jsonDocument);

QJsonObject AdminUpdateUserConfigStatusRequestToJson(const AdminUpdateUserConfigStatusRequest &req);

bool buildAdminUpdateUserConfigStatusQuery(const AdminUpdateUserConfigStatusRequest &req,
                                            QUrlQuery &query,
                                            QString &error);

/// 管理端删除配置 请求结构（DELETE /api/v1/admin/user-configs/:id）
typedef struct AdminDeleteUserConfigRequest
{
    int id; ///< 配置ID（路径参数，必填）
} AdminDeleteUserConfigRequest;

/// 管理端删除配置 应答结构
typedef struct AdminDeleteUserConfigResponse
{
    QString code;
    QString message;
    /// data 为 null
} AdminDeleteUserConfigResponse;

bool ProcessAdminDeleteUserConfigResult(AdminDeleteUserConfigResponse &responseData,
                                         QJsonDocument &jsonDocument);

QJsonObject AdminDeleteUserConfigRequestToJson(const AdminDeleteUserConfigRequest &req);

bool buildAdminDeleteUserConfigQuery(const AdminDeleteUserConfigRequest &req,
                                      QUrlQuery &query,
                                      QString &error);

/// 管理端设置配置标签 请求结构（PUT /api/v1/admin/user-configs/:id/tags）
typedef struct AdminSetUserConfigTagsRequest
{
    int id;               ///< 配置ID（路径参数，必填）
    bool is_official_tag; ///< 是否设为官方标签
    bool is_expert_tag;   ///< 是否设为大神标签
} AdminSetUserConfigTagsRequest;

/// 管理端设置配置标签 应答结构
typedef struct AdminSetUserConfigTagsResponse
{
    QString code;
    QString message;
    /// data 为 null
} AdminSetUserConfigTagsResponse;

bool ProcessAdminSetUserConfigTagsResult(AdminSetUserConfigTagsResponse &responseData,
                                          QJsonDocument &jsonDocument);

QJsonObject AdminSetUserConfigTagsRequestToJson(const AdminSetUserConfigTagsRequest &req);

bool buildAdminSetUserConfigTagsQuery(const AdminSetUserConfigTagsRequest &req,
                                       QUrlQuery &query,
                                       QString &error);

// ──────────────────────────────────────── 管理端评论 ────────────────────────────────────────

/// 管理端获取评论列表 请求结构（GET /api/v1/admin/comments）
typedef struct AdminGetCommentsRequest
{
    int64_t device_id = 0; ///< 设备ID筛选
    int page = 1;          ///< 页码，默认 1
    int page_size = 20;    ///< 每页数量，默认 20，最大 100
} AdminGetCommentsRequest;

/// 管理端获取评论列表 应答结构
typedef struct AdminGetCommentsResponse
{
    QString code;
    QString message;

    typedef struct ListItem
    {
        int id;
        int device_id;
        QString device_name;
        QString comment_text;
        QString comment_text_en;
        QString status;
        int use_count;
        QString created_at;
    } ListItem;

    typedef struct ReturnData
    {
        QList<ListItem> list;
        int total;
        int page;
        int page_size;
    } ReturnData;

    ReturnData data;
} AdminGetCommentsResponse;

bool ProcessAdminGetCommentsResult(AdminGetCommentsResponse &responseData,
                                    QJsonDocument &jsonDocument);

QJsonObject AdminGetCommentsRequestToJson(const AdminGetCommentsRequest &req);

bool buildAdminGetCommentsQuery(const AdminGetCommentsRequest &req,
                                 QUrlQuery &query,
                                 QString &error);

/// 管理端创建设备评论 请求结构（POST /api/v1/admin/comments）
typedef struct AdminCreateCommentRequest
{
    int64_t device_id;        ///< 设备ID（必填）
    QString comment_text;     ///< 评论内容，最多50字符（必填）
    QString comment_text_en;  ///< 英文评论内容，最多150字符（可选）
} AdminCreateCommentRequest;

/// 管理端创建设备评论 应答结构
typedef struct AdminCreateCommentResponse
{
    QString code;
    QString message;

    typedef struct ReturnData
    {
        int id;
        int device_id;
        QString comment_text;
        QString comment_text_en;
        QString status;
        QString created_at;
    } ReturnData;

    ReturnData data;
} AdminCreateCommentResponse;

bool ProcessAdminCreateCommentResult(AdminCreateCommentResponse &responseData,
                                      QJsonDocument &jsonDocument);

QJsonObject AdminCreateCommentRequestToJson(const AdminCreateCommentRequest &req);

bool buildAdminCreateCommentQuery(const AdminCreateCommentRequest &req,
                                   QUrlQuery &query,
                                   QString &error);

/// 管理端更新评论 请求结构（PUT /api/v1/admin/comments/:id）
typedef struct AdminUpdateCommentRequest
{
    int id;                   ///< 评论ID（路径参数，必填）
    QString comment_text;     ///< 新评论内容（必填）
    QString comment_text_en;  ///< 英文评论内容（可选）
} AdminUpdateCommentRequest;

/// 管理端更新评论 应答结构
typedef struct AdminUpdateCommentResponse
{
    QString code;
    QString message;
    /// data 为 null
} AdminUpdateCommentResponse;

bool ProcessAdminUpdateCommentResult(AdminUpdateCommentResponse &responseData,
                                      QJsonDocument &jsonDocument);

QJsonObject AdminUpdateCommentRequestToJson(const AdminUpdateCommentRequest &req);

bool buildAdminUpdateCommentQuery(const AdminUpdateCommentRequest &req,
                                   QUrlQuery &query,
                                   QString &error);

/// 管理端删除评论 请求结构（DELETE /api/v1/admin/comments/:id）
typedef struct AdminDeleteCommentRequest
{
    int id; ///< 评论ID（路径参数，必填）
} AdminDeleteCommentRequest;

/// 管理端删除评论 应答结构
typedef struct AdminDeleteCommentResponse
{
    QString code;
    QString message;
    /// data 为 null
} AdminDeleteCommentResponse;

bool ProcessAdminDeleteCommentResult(AdminDeleteCommentResponse &responseData,
                                      QJsonDocument &jsonDocument);

QJsonObject AdminDeleteCommentRequestToJson(const AdminDeleteCommentRequest &req);

bool buildAdminDeleteCommentQuery(const AdminDeleteCommentRequest &req,
                                   QUrlQuery &query,
                                   QString &error);

/// 管理端更新评论状态 请求结构（PUT /api/v1/admin/comments/:id/status）
typedef struct AdminUpdateCommentStatusRequest
{
    int id;          ///< 评论ID（路径参数，必填）
    QString status;  ///< 新状态：active/inactive（必填）
} AdminUpdateCommentStatusRequest;

/// 管理端更新评论状态 应答结构
typedef struct AdminUpdateCommentStatusResponse
{
    QString code;
    QString message;
    /// data 为 null
} AdminUpdateCommentStatusResponse;

bool ProcessAdminUpdateCommentStatusResult(AdminUpdateCommentStatusResponse &responseData,
                                            QJsonDocument &jsonDocument);

QJsonObject AdminUpdateCommentStatusRequestToJson(const AdminUpdateCommentStatusRequest &req);

bool buildAdminUpdateCommentStatusQuery(const AdminUpdateCommentStatusRequest &req,
                                         QUrlQuery &query,
                                         QString &error);

} // namespace DeSheng

// 推荐写法（HttpClient + RequestOptions）
// auto &cli = HttpClient::instance();
// QUrlQuery t_q; t_q.addQueryItem("page", "1");
// QNetworkReply *r = cli.get(DeSheng::ApiPaths::kConfigBase, RequestOptions{}.withQuery(t_q).withTag("userConfig"));
// QNetworkReply *r2 = cli.post(DeSheng::ApiPaths::kConfigBase, RequestOptions{}.withBody(t_body).withTag("userConfig"));
// QString t_path = QString(DeSheng::ApiPaths::kConfigLike).arg(42);
// QNetworkReply *r3 = cli.post(t_path, RequestOptions{}.withBody(QByteArray("{}")).withTag("userConfig"));
// connect(r, &QNetworkReply::finished, [r]() { ... ; r->deleteLater(); });

#endif // USER_CONFIG_API_H
