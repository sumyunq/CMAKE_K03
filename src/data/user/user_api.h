#ifndef USER_API_H
#define USER_API_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QUrlQuery>

namespace DeSheng {

/// 路径常量
namespace ApiPaths {
inline constexpr const char *kUserSendCode       = "/email/send-code";
inline constexpr const char *kUserSignUp         = "/user/signup";
inline constexpr const char *kUserLogin          = "/user/login";
inline constexpr const char *kUserForgotPassword = "/user/forgot-password";
inline constexpr const char *kUserChangePassword = "/user/change-password";
inline constexpr const char *kUserMe             = "/user";          ///< GET 获取当前用户 / PUT 更新用户
inline constexpr const char *kUserUpdate         = "/user";
inline constexpr const char *kUserLogout         = "/user/logout";
inline constexpr const char *kUserUploads        = "/user/uploads";
inline constexpr const char *kPublicUserInfo     = "/user/%1";          ///< GET 其他用户公开信息（%1 = user_id）
// 管理端
inline constexpr const char *kAdminUser        = "/admin/user";          ///< GET 用户列表
inline constexpr const char *kAdminUserDetail  = "/admin/user/%1";       ///< GET 详情 / PUT 更新 / DELETE 删除（%1 = user_id）
inline constexpr const char *kAdminUserLevel   = "/admin/user/%1/level"; ///< GET 用户等级（%1 = user_id）。文档已收录（userLevel 模块），响应含 user_id；客户端当前不调用 admin 接口。
} // namespace ApiPaths

/// 其他用户公开信息路径别名（契约写法 DeSheng::kPublicUserInfo，等价于 ApiPaths::kPublicUserInfo）
inline constexpr const char *kPublicUserInfo = ApiPaths::kPublicUserInfo;

/*************************************************************************************  用户系统  ************************************************************************************************/

/// EmailSendCode 验证码 请求结构体信息
typedef struct EmailSendCodeRequest
{
    QString email; ///< 邮箱
    QString scene; ///< scene 字段：user_signup / user_forgot_password / user_change_password
} EmailSendCodeRequest;

/// EmailSendCode 验证码 应答结构体信息
typedef struct EmailSendCodeResponse
{
    QString code;    ///< 成功:"success"
    QString message; ///< 成功:"success"
    typedef struct ReturnData
    {
        QString message; ///< 回显信息
    } returnData;
    returnData data;
} EmailSendCodeResponse;

bool ProcessEmailSendCodeResult(DeSheng::EmailSendCodeResponse &responseData,
                                QJsonDocument &jsonDocument);

QJsonObject EmailSendCodeRequestToJson(const EmailSendCodeRequest &req);

bool buildEmailSendCodeQuery(const EmailSendCodeRequest &req, QUrlQuery &query, QString &error);

/// SignUpRequest 用户注册 请求结构体信息
typedef struct UserSignUpRequest
{
    QString username; ///< 用户名，3-50 字符 (必填)
    QString email;    ///< 邮箱地址 (必填)
    QString password; ///< 密码，至少 6 位 (必填)
    QString nickname; ///< 昵称，最多 50 字符 (可选)
    QString code;     ///< 邮箱验证码，6 位数字 (必填)
} UserSignUpRequest;

/// SignUpResponse 用户注册 应答结构体信息
typedef struct UserSignUpResponse
{
    QString code;    ///< 成功:"success"
    QString message; ///< 成功:"success"
    typedef struct ReturnData
    {
        QString id;              ///< 用户 ID
        QString username;        ///< 用户名
        QString email;           ///< 邮箱地址
        QString nickname;        ///< 用户昵称 (可选)
        QString avatar;          ///< 头像 URL (可选)
        QString status;          ///< 状态: "active"
        QString login_type;      ///< 登录类型: 邮箱"account" 微信"wechat"
        QString created_at;      ///< 创建时间 (可选)
        QString last_login_at;   ///< 最后登录时间 (可选)
        QString favorite_games;  ///< 喜爱游戏 (可选)
        QString activation_code; ///< 激活码 (可选)
        QString city;            ///< 城市 (可选)
        QString login_ip;        ///< 登录 IP (可选)
        QString os_info;         ///< 操作系统信息 (可选)
    } returnData;
    returnData data;
} UserSignUpResponse;

bool ProcessUserSignUpResult(DeSheng::UserSignUpResponse &responseData, QJsonDocument &jsonDocument);

QJsonObject UserSignUpRequestToJson(const DeSheng::UserSignUpRequest &req);

bool buildUserSignUpQuery(const UserSignUpRequest &req, QUrlQuery &query, QString &error);

/// UserLoginRequest 用户登录 请求结构体信息
typedef struct UserLoginRequest
{
    QString email;           ///< 邮箱地址 (必填)
    QString password;        ///< 密码 (必填)
    bool rememberMe = false; ///< 是否记住我 (可选，默认 false)
} UserLoginRequest;

/// LoginResponse 用户登录 应答结构体信息
typedef struct UserLoginResponse
{
    QString code;    ///< 成功:"success"
    QString message; ///< 成功:"success"
    typedef struct ReturnData
    {
        QString access_token;  ///< 访问令牌
        QString refresh_token; ///< 刷新令牌
        typedef struct UserInfo
        {
            QString id;              ///< 用户 ID
            QString username;        ///< 用户名
            QString email;           ///< 邮箱地址
            QString nickname;        ///< 用户昵称 (可选)
            QString avatar;          ///< 头像 URL (可选)
            QString status;          ///< 状态: "active"
            QString login_type;      ///< 登录类型: "account"
            QString created_at;      ///< 创建时间 (可选)
            QString last_login_at;   ///< 最后登录时间 (可选)
            QString favorite_games;  ///< 喜爱游戏 (可选)
            QString activation_code; ///< 激活码 (可选)
            QString city;            ///< 城市 (可选)
            QString login_ip;        ///< 登录 IP (可选)
            QString os_info;         ///< 操作系统信息 (可选)
            QStringList roles;       ///< 用户角色（如 streamer / professional）(可选)
            QStringList titles;      ///< 用户头衔 (可选)
        } UserInfo;
        UserInfo user;
    } returnData;
    returnData data;
} UserLoginResponse;

bool ProcessUserLoginResult(DeSheng::UserLoginResponse &responseData,
                            const QJsonDocument &jsonDocument);

QJsonObject UserLoginRequestToJson(const DeSheng::UserLoginRequest &req);

bool buildUserLoginUpQuery(const UserLoginRequest &req, QUrlQuery &query, QString &error);

/// UserLogoutRequest 用户退出 请求结构体信息（无 body，仅需 Authorization header）
typedef struct UserLogoutRequest
{
} UserLogoutRequest;

/// UserLogoutResponse 用户退出 应答结构体信息
typedef struct UserLogoutResponse
{
    QString code;    ///< 成功:"success"
    QString message; ///< 成功:"success"
} UserLogoutResponse;

bool ProcessUserLogoutResult(UserLogoutResponse &responseData, const QJsonDocument &jsonDocument);

QJsonObject UserLogoutRequestToJson(const UserLogoutRequest &req);

bool buildUserLogoutQuery(const UserLogoutRequest &req, QUrlQuery &query, QString &error);

/// GetCurrentUserRequest 获取当前用户信息 请求（无 body，GET 请求）
typedef struct GetCurrentUserRequest
{
} GetCurrentUserRequest;

/// GetCurrentUserResponse 获取当前用户信息 应答
typedef struct GetCurrentUserResponse
{
    QString code;    ///< 成功:"success"
    QString message; ///< 成功:"success"
    typedef struct ReturnData
    {
        QString id;              ///< 用户 ID
        QString username;        ///< 用户名
        QString email;           ///< 邮箱地址
        QString nickname;        ///< 用户昵称
        QString avatar;          ///< 头像 URL
        QString status;          ///< 状态
        QString login_type;      ///< 登录类型
        QString created_at;      ///< 创建时间
        QString last_login_at;   ///< 最后登录时间
        QString favorite_games;  ///< 喜爱游戏
        QString activation_code; ///< 激活码
        QString city;            ///< 城市
        QString login_ip;        ///< 登录 IP
        QString os_info;         ///< 操作系统信息
        QString bio;             ///< 个性签名
        QStringList roles;       ///< 用户角色（如 streamer / professional）
        QStringList titles;      ///< 用户头衔
    } returnData;
    returnData data;
} GetCurrentUserResponse;

bool ProcessGetCurrentUserResult(GetCurrentUserResponse &responseData,
                                 const QJsonDocument &jsonDocument);

QJsonObject GetCurrentUserRequestToJson(const GetCurrentUserRequest &req);

bool buildGetCurrentUserQuery(const GetCurrentUserRequest &req, QUrlQuery &query, QString &error);

/// GetPublicUserInfoResponse 获取其他用户公开信息 应答（GET /user/%1，仅公开字段，无 email/login_ip 等敏感信息）
typedef struct GetPublicUserInfoResponse
{
    QString code;    ///< 成功:"success"
    QString message; ///< 成功:"success"
    typedef struct ReturnData
    {
        QString id;             ///< 用户 ID
        QString username;       ///< 用户名（展示推荐优先）
        QString nickname;       ///< 用户昵称（可能为空）
        QString avatar;         ///< 头像 URL
        QStringList roles;      ///< 用户角色（如 official / streamer / professional）
        QStringList titles;     ///< 用户头衔（如 expert）
        QString bio;            ///< 个性签名
        QString favorite_games; ///< 喜爱的游戏
        QString city;           ///< 所在城市
        QString created_at;     ///< 注册年月（如 2025-12，精确到月）
    } returnData;
    returnData data;
} GetPublicUserInfoResponse;

bool ProcessGetPublicUserInfoResult(GetPublicUserInfoResponse &responseData,
                                    const QJsonDocument &jsonDocument);

/// UpdateUserRequest 更新用户信息 请求结构体
typedef struct UpdateUserRequest
{
    QString username;        ///< 用户名，3-50 字符 (可选)
    QString nickname;        ///< 昵称，最多 50 字符 (可选)
    QString avatar;          ///< 头像 URL (可选)
    QString email;           ///< 邮箱地址 (可选)
    QString status;          ///< 状态：active/disabled (可选)
    QString favorite_games;  ///< 喜爱游戏，最多 255 字符 (可选)
    QString activation_code; ///< 激活码，最多 100 字符 (可选)
    QString bio;             ///< 个性签名，最多 100 字符 (可选)
} UpdateUserRequest;

/// UpdateUserResponse 更新用户信息 应答结构体（复用 GetCurrentUserResponse 的 data 结构）
/// 与 GetCurrentUser 响应结构一致
typedef GetCurrentUserResponse UpdateUserResponse;

bool ProcessUpdateUserResult(UpdateUserResponse &responseData, const QJsonDocument &jsonDocument);

QJsonObject UpdateUserRequestToJson(const UpdateUserRequest &req);

bool buildUpdateUserQuery(const UpdateUserRequest &req, QUrlQuery &query, QString &error);

/// ChangePasswordRequest 修改密码 请求结构体
typedef struct ChangePasswordRequest
{
    QString old_password; ///< 原密码 (必填)
    QString new_password; ///< 新密码，至少 6 位 (必填)
    QString email;        ///< 邮箱地址 (必填)
    QString code;         ///< 邮箱验证码，6 位数字 (必填)
} ChangePasswordRequest;

/// ChangePasswordResponse 修改密码 应答结构体
typedef struct ChangePasswordResponse
{
    QString code;    ///< 成功:"success"
    QString message; ///< 成功:"success"
} ChangePasswordResponse;

bool ProcessChangePasswordResult(ChangePasswordResponse &responseData,
                                 const QJsonDocument &jsonDocument);

QJsonObject ChangePasswordRequestToJson(const ChangePasswordRequest &req);

bool buildChangePasswordQuery(const ChangePasswordRequest &req, QUrlQuery &query, QString &error);

/// ForgotPasswordRequest 找回密码 请求结构体
typedef struct ForgotPasswordRequest
{
    QString email;        ///< 邮箱地址 (必填)
    QString code;         ///< 邮箱验证码，6 位数字 (必填)
    QString new_password; ///< 新密码，至少 6 位 (必填)
} ForgotPasswordRequest;

/// ForgotPasswordResponse 找回密码 应答结构体
typedef struct ForgotPasswordResponse
{
    QString code;    ///< 成功:"success"
    QString message; ///< 成功:"success"
} ForgotPasswordResponse;

bool ProcessForgotPasswordResult(ForgotPasswordResponse &responseData,
                                 const QJsonDocument &jsonDocument);

QJsonObject ForgotPasswordRequestToJson(const ForgotPasswordRequest &req);

bool buildForgotPasswordQuery(const ForgotPasswordRequest &req, QUrlQuery &query, QString &error);

/*************************************************************************************  管理端用户系统  ************************************************************************************************/

/// 管理端用户信息（无 bio 字段，与普通用户接口区分）
typedef struct AdminUserInfo
{
    QString id;              ///< 用户 ID
    QString username;        ///< 用户名
    QString email;           ///< 邮箱地址
    QString nickname;        ///< 用户昵称
    QString avatar;          ///< 头像 URL
    QString status;          ///< 状态：active/disabled
    QString login_type;      ///< 登录方式：account/wechat/google
    QString created_at;      ///< 注册时间
    QString last_login_at;   ///< 最后登录时间
    QString favorite_games;  ///< 喜爱的游戏
    QString activation_code; ///< 激活码
    QString city;            ///< 所在城市
    QString login_ip;        ///< 登录 IP
    QString os_info;         ///< 操作系统信息
} AdminUserInfo;

/// AdminUserListRequest 管理端获取用户列表 请求结构体（GET）
typedef struct AdminUserListRequest
{
    QString id;          ///< 用户ID筛选（可选）
    QString username;    ///< 用户名筛选，模糊匹配（可选）
    QString email;       ///< 邮箱筛选，模糊匹配（可选）
    QString nickname;    ///< 昵称筛选，模糊匹配（可选）
    QString status;      ///< 状态筛选：active/disabled（可选）
    QString device_type; ///< 设备类型筛选（可选）
    QString sort_by;     ///< 排序字段：created_at/updated_at，默认 created_at（可选）
    QString order_by;    ///< 排序方式：asc/desc，默认 desc（可选）
    int page = 1;        ///< 页码，默认 1
    int page_size = 10;  ///< 每页数量，默认 10，最大 100
} AdminUserListRequest;

/// AdminUserListResponse 管理端获取用户列表 应答结构体
typedef struct AdminUserListResponse
{
    QString code;    ///< 成功:"success"
    QString message; ///< 成功:"success"
    typedef struct ReturnData
    {
        QList<AdminUserInfo> list; ///< 用户列表
        int total = 0;             ///< 总记录数
        int page = 0;              ///< 当前页码
        int size = 0;              ///< 每页数量
    } ReturnData;
    ReturnData data;
} AdminUserListResponse;

bool ProcessAdminUserListResult(AdminUserListResponse &responseData,
                                 const QJsonDocument &jsonDocument);

QJsonObject AdminUserListRequestToJson(const AdminUserListRequest &req);

bool buildAdminUserListQuery(const AdminUserListRequest &req,
                              QUrlQuery &query,
                              QString &error);

/// AdminUserDetailRequest 管理端获取用户详情 请求结构体（GET /admin/user/:id）
typedef struct AdminUserDetailRequest
{
    QString user_id; ///< 用户 ID（必填，路径参数）
} AdminUserDetailRequest;

/// AdminUserDetailResponse 管理端获取用户详情 应答结构体
typedef struct AdminUserDetailResponse
{
    QString code;    ///< 成功:"success"
    QString message; ///< 成功:"success"
    typedef struct ReturnData
    {
        QString id;              ///< 用户 ID
        QString username;        ///< 用户名
        QString email;           ///< 邮箱地址
        QString nickname;        ///< 用户昵称
        QString avatar;          ///< 头像 URL
        QString status;          ///< 状态：active/disabled
        QString login_type;      ///< 登录方式
        QString created_at;      ///< 注册时间
        QString last_login_at;   ///< 最后登录时间
        QString favorite_games;  ///< 喜爱的游戏
        QString activation_code; ///< 激活码
        QString city;            ///< 所在城市
        QString login_ip;        ///< 登录 IP
        QString os_info;         ///< 操作系统信息
    } ReturnData;
    ReturnData data;
} AdminUserDetailResponse;

bool ProcessAdminUserDetailResult(AdminUserDetailResponse &responseData,
                                   const QJsonDocument &jsonDocument);

QJsonObject AdminUserDetailRequestToJson(const AdminUserDetailRequest &req);

bool buildAdminUserDetailQuery(const AdminUserDetailRequest &req,
                                QUrlQuery &query,
                                QString &error);

/// AdminUserUpdateRequest 管理端更新用户信息 请求结构体（PUT /admin/user/:id）
typedef struct AdminUserUpdateRequest
{
    QString user_id;         ///< 用户 ID（必填，路径参数）
    QString username;        ///< 用户名，3-50 字符（可选）
    QString nickname;        ///< 昵称，最多 50 字符（可选）
    QString avatar;          ///< 头像 URL（可选）
    QString email;           ///< 邮箱地址（可选）
    QString status;          ///< 状态：active/disabled（可选）
    QString favorite_games;  ///< 喜爱游戏，最多 255 字符（可选）
    QString activation_code; ///< 激活码，最多 100 字符（可选）
} AdminUserUpdateRequest;

/// AdminUserUpdateResponse 管理端更新用户信息 应答（复用 AdminUserDetailResponse）
typedef AdminUserDetailResponse AdminUserUpdateResponse;

bool ProcessAdminUserUpdateResult(AdminUserUpdateResponse &responseData,
                                   const QJsonDocument &jsonDocument);

QJsonObject AdminUserUpdateRequestToJson(const AdminUserUpdateRequest &req);

bool buildAdminUserUpdateQuery(const AdminUserUpdateRequest &req,
                                QUrlQuery &query,
                                QString &error);

/// AdminUserDeleteRequest 管理端删除用户 请求结构体（DELETE /admin/user/:id，软删除）
typedef struct AdminUserDeleteRequest
{
    QString user_id; ///< 用户 ID（必填，路径参数）
} AdminUserDeleteRequest;

/// AdminUserDeleteResponse 管理端删除用户 应答结构体
typedef struct AdminUserDeleteResponse
{
    QString code;    ///< 成功:"success"
    QString message; ///< 成功:"success"
} AdminUserDeleteResponse;

bool ProcessAdminUserDeleteResult(AdminUserDeleteResponse &responseData,
                                   const QJsonDocument &jsonDocument);

QJsonObject AdminUserDeleteRequestToJson(const AdminUserDeleteRequest &req);

bool buildAdminUserDeleteQuery(const AdminUserDeleteRequest &req,
                                QUrlQuery &query,
                                QString &error);

/// AdminUserLevelRequest 管理端获取用户等级 请求结构体（GET /admin/user/:id/level）
typedef struct AdminUserLevelRequest
{
    QString user_id; ///< 用户 ID（必填，路径参数）
} AdminUserLevelRequest;

/// AdminUserLevelResponse 管理端获取用户等级 应答结构体
typedef struct AdminUserLevelResponse
{
    QString code;    ///< 成功:"success" 或 200
    QString message; ///< 成功:"success"
    typedef struct ReturnData
    {
        int user_id = 0;            ///< 用户 ID（文档 2026-08-05 新增）
        int level = 0;              ///< 当前等级（0-15）
        int total_experience = 0;   ///< 历史累计总经验
        int current_experience = 0; ///< 当前等级已获经验
        int exp_cap = 0;            ///< 当前等级经验上限
    } ReturnData;
    ReturnData data;
} AdminUserLevelResponse;

bool ProcessAdminUserLevelResult(AdminUserLevelResponse &responseData,
                                  const QJsonDocument &jsonDocument);

QJsonObject AdminUserLevelRequestToJson(const AdminUserLevelRequest &req);

bool buildAdminUserLevelQuery(const AdminUserLevelRequest &req,
                               QUrlQuery &query,
                               QString &error);

} // namespace DeSheng

// 调用示例（新栈）— 用户登录
// auto &cli = HttpClient::instance();  /// network/http_client.h
// UserLoginRequest t_req;
// t_req.email = "user@example.com"; t_req.password = "123456";
// QByteArray t_body = QJsonDocument(UserLoginRequestToJson(t_req)).toJson();
// QNetworkReply *r = cli.post(DeSheng::ApiPaths::kUserLogin, RequestOptions{}.withBody(t_body).withTag("user"));
// connect(r, &QNetworkReply::finished, [r]() {
//     UserLoginResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(r->readAll());
//     if (ProcessUserLoginResult(t_resp, t_doc) && t_resp.code == "success") {
//         // t_resp.data.access_token ...
//     }
//     r->deleteLater();
// });
//
// 调用示例（新栈）— 获取当前用户信息
// QNetworkReply *r2 = cli.get(DeSheng::ApiPaths::kUserMe, RequestOptions{}.withTag("user"));
// connect(r2, &QNetworkReply::finished, [r2]() {
//     GetCurrentUserResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(r2->readAll());
//     if (ProcessGetCurrentUserResult(t_resp, t_doc) && t_resp.code == "success") {
//         // t_resp.data.nickname ...
//     }
//     r2->deleteLater();
// });
//
// 调用示例（新栈）— 用户退出
// QNetworkReply *r3 = cli.post(DeSheng::ApiPaths::kUserLogout, RequestOptions{}.withBody(QByteArray("{}")).withTag("user"));
// connect(r3, &QNetworkReply::finished, [r3]() {
//     UserLogoutResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(r3->readAll());
//     if (ProcessUserLogoutResult(t_resp, t_doc) && t_resp.code == "success") {}
//     r3->deleteLater();
// });
//
// 调用示例（新栈）— 文件上传
// QHttpMultiPart *t_mp = new QHttpMultiPart(QHttpMultiPart::FormDataType);
// QHttpPart t_filePart;
// t_filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
//     "form-data; name=\"file\"; filename=\"avatar.png\"");
// QFile *t_file = new QFile("avatar.png");
// t_file->open(QIODevice::ReadOnly);
// t_filePart.setBodyDevice(t_file);
// t_file->setParent(t_mp);
// t_mp->append(t_filePart);
// QNetworkReply *r4 = cli.upload(DeSheng::ApiPaths::kUserUploads, t_mp, RequestOptions{}.withTag("user"));
// t_mp->setParent(r4);
// connect(r4, &QNetworkReply::finished, [r4]() {
//     FileUploadsResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(r4->readAll());
//     if (ProcessFileUploadsResult(t_resp, t_doc) && t_resp.code == "success") {}
//     r4->deleteLater();
// });
//
// 推荐写法（HttpClient + RequestOptions）
// auto &cli = HttpClient::instance();
// QNetworkReply *r = cli.post(DeSheng::ApiPaths::kUserLogin, RequestOptions{}.withBody(t_body).withTag("user"));
// QNetworkReply *r2 = cli.get(DeSheng::ApiPaths::kUserMe, RequestOptions{}.withTag("user"));
// // multipart 上传: cli.upload(DeSheng::ApiPaths::kUserUploads, t_mp, RequestOptions{}.withTag("user"));
// connect(r, &QNetworkReply::finished, [r]() { ... ; r->deleteLater(); });
//
// 调用示例（新栈）— 管理端获取用户列表
// AdminUserListRequest t_req;
// t_req.status = "active";
// t_req.page = 1; t_req.page_size = 20;
// QUrlQuery t_query;
// QString t_err;
// buildAdminUserListQuery(t_req, t_query, t_err);
// auto &cli = HttpClient::instance();  /// network/http_client.h
// QNetworkReply *r = cli.get(DeSheng::ApiPaths::kAdminUser, RequestOptions{}.withQuery(t_query).withTag("user"));
// connect(r, &QNetworkReply::finished, [r]() {
//     AdminUserListResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(r->readAll());
//     if (ProcessAdminUserListResult(t_resp, t_doc) && t_resp.code == "success") {
//         // t_resp.data.list / t_resp.data.total ...
//     }
//     r->deleteLater();
// });
//
// 调用示例 — 管理端获取用户详情
// AdminUserDetailRequest t_req2;
// t_req2.user_id = "1";
// QString t_path2 = QString(DeSheng::ApiPaths::kAdminUserDetail).arg(t_req2.user_id);
// auto &cli = HttpClient::instance();
// QNetworkReply *reply2 = cli.get(t_path2, RequestOptions{}.withTag("user"));
// connect(t_reply2, &QNetworkReply::finished, [t_reply2]() {
//     AdminUserDetailResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(t_reply2->readAll());
//     if (ProcessAdminUserDetailResult(t_resp, t_doc) && t_resp.code == "success") {}
//     t_reply2->deleteLater();
// });
//
// 调用示例（新栈）— 管理端更新用户
// AdminUserUpdateRequest t_req3;
// t_req3.user_id = "1";
// t_req3.status = "disabled";
// QString t_path3 = QString(DeSheng::ApiPaths::kAdminUserDetail).arg(t_req3.user_id);
// QByteArray t_body3 = QJsonDocument(DeSheng::AdminUserUpdateRequestToJson(t_req3)).toJson();
// auto &cli = HttpClient::instance();  /// network/http_client.h
// QNetworkReply *r3 = cli.put(t_path3, RequestOptions{}.withBody(t_body3).withTag("user"));
// connect(r3, &QNetworkReply::finished, [r3]() { ... ; r3->deleteLater(); });
//
// 调用示例（新栈）— 管理端删除用户
// AdminUserDeleteRequest t_req4;
// t_req4.user_id = "1";
// QString t_path4 = QString(DeSheng::ApiPaths::kAdminUserDetail).arg(t_req4.user_id);
// QNetworkReply *r4 = cli.del(t_path4, RequestOptions{}.withTag("user"));
// connect(r4, &QNetworkReply::finished, [r4]() { ... ; r4->deleteLater(); });
//
// 调用示例（新栈）— 管理端获取用户等级
// AdminUserLevelRequest t_req5;
// t_req5.user_id = "1";
// QString t_path5 = QString(DeSheng::ApiPaths::kAdminUserLevel).arg(t_req5.user_id);
// QNetworkReply *r5 = cli.get(t_path5, RequestOptions{}.withTag("user"));
// connect(r5, &QNetworkReply::finished, [r5]() { ... ; r5->deleteLater(); });

#endif // USER_API_H
