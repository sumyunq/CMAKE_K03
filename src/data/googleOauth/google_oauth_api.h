#ifndef GOOGLE_OAUTH_API_H
#define GOOGLE_OAUTH_API_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QUrlQuery>

namespace DeSheng {

/// 路径常量（%1 = app_id，如 google_web / google_desktop）
namespace ApiPaths {
inline constexpr const char *kGoogleOauthAuth        = "/oauth/%1/auth";
inline constexpr const char *kGoogleOauthCallback    = "/oauth/%1/callback";
inline constexpr const char *kGoogleOauthPreAuth     = "/oauth/%1/preauth";
inline constexpr const char *kGoogleOauthLoginStatus = "/oauth/login-status/%1";
} // namespace ApiPaths

/*************************************************************************************  Google OAuth 统一登录系统  ************************************************************************************************/

/// GoogleOauthAuthRequest 生成授权链接 请求结构体（GET，302 跳转）
typedef struct GoogleOauthAuthRequest
{
    QString app_id;       ///< 应用标识，如 google_web、google_desktop（必填，路径参数）
    QString redirect_url; ///< 登录成功后跳转的前端页面地址（可选，查询参数）
} GoogleOauthAuthRequest;

/// GoogleOauthAuthResponse 生成授权链接 应答结构体（302 跳转，无 JSON body）
typedef struct GoogleOauthAuthResponse
{
    QString code;    ///< 成功:"success"
    QString message; ///< 成功:"success"
} GoogleOauthAuthResponse;

bool ProcessGoogleOauthAuthResult(GoogleOauthAuthResponse &responseData,
                                   const QJsonDocument &jsonDocument);

QJsonObject GoogleOauthAuthRequestToJson(const GoogleOauthAuthRequest &req);

bool buildGoogleOauthAuthQuery(const GoogleOauthAuthRequest &req,
                                QUrlQuery &query,
                                QString &error);

/// GoogleOauthCallbackRequest OAuth 回调 请求结构体（GET，302 跳转，由泰国服务回调）
typedef struct GoogleOauthCallbackRequest
{
    QString app_id; ///< 应用标识（必填，路径参数）
    QString code;   ///< 泰国 OAuth2 授权服务器返回的授权码（必填，查询参数）
    QString state;  ///< 防 CSRF 的状态码 = 会话 ID（必填，查询参数）
} GoogleOauthCallbackRequest;

/// GoogleOauthCallbackResponse OAuth 回调 应答结构体（302 跳转）
typedef struct GoogleOauthCallbackResponse
{
    QString code;    ///< 成功:"success"
    QString message; ///< 成功:"success"
} GoogleOauthCallbackResponse;

bool ProcessGoogleOauthCallbackResult(GoogleOauthCallbackResponse &responseData,
                                       const QJsonDocument &jsonDocument);

QJsonObject GoogleOauthCallbackRequestToJson(const GoogleOauthCallbackRequest &req);

bool buildGoogleOauthCallbackQuery(const GoogleOauthCallbackRequest &req,
                                    QUrlQuery &query,
                                    QString &error);

/// GoogleOauthPreAuthRequest 预授权 请求结构体（POST，桌面端/内嵌场景）
typedef struct GoogleOauthPreAuthRequest
{
    QString app_id;       ///< 应用标识（必填，路径参数）
    QString redirect_url; ///< 登录成功后跳转地址（可选）
} GoogleOauthPreAuthRequest;

/// GoogleOauthPreAuthResponse 预授权 应答结构体
typedef struct GoogleOauthPreAuthResponse
{
    QString code;
    QString message;
    typedef struct ReturnData
    {
        QString client_id;    ///< 泰国 OAuth2 服务注册的 client_id
        QString state;        ///< 会话 ID，回调时需要携带
        QString redirect_uri; ///< OAuth 回调地址
        QString scope;        ///< 授权作用域
        QString auth_url;     ///< 完整的泰国 OAuth2 授权 URL（最终跳转至 Google）
    } ReturnData;
    ReturnData data;
} GoogleOauthPreAuthResponse;

bool ProcessGoogleOauthPreAuthResult(GoogleOauthPreAuthResponse &responseData,
                                      const QJsonDocument &jsonDocument);

QJsonObject GoogleOauthPreAuthRequestToJson(const GoogleOauthPreAuthRequest &req);

bool buildGoogleOauthPreAuthQuery(const GoogleOauthPreAuthRequest &req,
                                   QUrlQuery &query,
                                   QString &error);

/// GoogleOauthLoginStatusRequest 查询登录状态 请求结构体（GET）
typedef struct GoogleOauthLoginStatusRequest
{
    QString evidence; ///< 回调 302 跳转时 URL 中携带的会话 ID = session.ID（必填，路径参数）
} GoogleOauthLoginStatusRequest;

/// GoogleOauthLoginStatusResponse 查询登录状态 应答结构体
typedef struct GoogleOauthLoginStatusResponse
{
    QString code;
    QString message;
    typedef struct UserInfo
    {
        QString id;         ///< 用户 ID
        QString username;   ///< 系统生成的唯一用户名，用于登录标识
        QString nickname;   ///< Google 返回的昵称，可能为空
        QString avatar;     ///< 头像 URL
        QString login_type; ///< 登录类型："google"
    } UserInfo;

    typedef struct ReturnData
    {
        QString status;       ///< 登录状态：pending / success / expired
        QString access_token; ///< 访问令牌（status=success 时）
        UserInfo user;        ///< 用户信息（status=success 时）
    } ReturnData;
    ReturnData data;
} GoogleOauthLoginStatusResponse;

bool ProcessGoogleOauthLoginStatusResult(GoogleOauthLoginStatusResponse &responseData,
                                          const QJsonDocument &jsonDocument);

QJsonObject GoogleOauthLoginStatusRequestToJson(const GoogleOauthLoginStatusRequest &req);

bool buildGoogleOauthLoginStatusQuery(const GoogleOauthLoginStatusRequest &req,
                                       QUrlQuery &query,
                                       QString &error);

} // namespace DeSheng

// 调用示例 — Google 预授权（桌面端）
// GoogleOauthPreAuthRequest t_req;
// t_req.app_id = "google_desktop";
// QString t_path = QString(DeSheng::ApiPaths::kGoogleOauthPreAuth).arg(t_req.app_id);
// QByteArray t_body = QJsonDocument(DeSheng::GoogleOauthPreAuthRequestToJson(t_req)).toJson();
// auto &cli = HttpClient::instance();
// QNetworkReply *reply = cli.post(t_path, RequestOptions{}.withBody(t_body).withTag("googleOauth"));
// connect(t_reply, &QNetworkReply::finished, [t_reply]() {
//     GoogleOauthPreAuthResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(t_reply->readAll());
//     if (ProcessGoogleOauthPreAuthResult(t_resp, t_doc) && t_resp.code == "success") {
//         // t_resp.data.auth_url / t_resp.data.state ...
//     }
//     t_reply->deleteLater();
// });
//
// 调用示例 — Google 查询登录状态
// GoogleOauthLoginStatusRequest t_req2;
// t_req2.evidence = "sess_19b18bd6f04dded5a1ab7032da887a4e";
// QString t_path2 = QString(DeSheng::ApiPaths::kGoogleOauthLoginStatus).arg(t_req2.evidence);
// auto &cli2 = HttpClient::instance();
// QNetworkReply *reply2 = cli2.get(t_path2, RequestOptions{}.withTag("googleOauth"));
// connect(t_reply2, &QNetworkReply::finished, [t_reply2]() {
//     GoogleOauthLoginStatusResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(t_reply2->readAll());
//     if (ProcessGoogleOauthLoginStatusResult(t_resp, t_doc) && t_resp.code == "success") {
//         // t_resp.data.status / t_resp.data.access_token ...
//     }
//     t_reply2->deleteLater();
// });
//
// 调用示例（新栈）— Google 生成授权链接（Web 端，302 跳转）
// GoogleOauthAuthRequest t_req3;
// t_req3.app_id = "google_web";
// t_req3.redirect_url = "https://example.com/login-callback";
// QString t_path3 = QString(DeSheng::ApiPaths::kGoogleOauthAuth).arg(t_req3.app_id);
// QUrlQuery t_query3;
// QString t_err3;
// buildGoogleOauthAuthQuery(t_req3, t_query3, t_err3);
// auto &cli3 = HttpClient::instance();  /// network/http_client.h
// QNetworkReply *reply3 = cli3.get(t_path3, RequestOptions{}.withQuery(t_query3).withTag("googleOauth"));
// // 服务端返回 302 跳转到 Google 授权页，客户端跟随跳转或从响应中提取授权地址
//
// 调用示例 — Google 授权回调（桌面端，GET）
// GoogleOauthCallbackRequest t_req4;
// t_req4.app_id = "google_desktop";
// t_req4.code = "xxx";
// t_req4.state = "sess_xxx";
// QString t_path4 = QString(DeSheng::ApiPaths::kGoogleOauthCallback).arg(t_req4.app_id);
// QUrlQuery t_query4;
// QString t_err4;
// buildGoogleOauthCallbackQuery(t_req4, t_query4, t_err4);
// QUrl t_url4 = ServerRouter::instance().resolveUrl(t_path4, "", "googleOauth"); ///< 旧硬编码 baseUrl 已废弃
// t_url4.setQuery(t_query4);
// auto &cli4 = HttpClient::instance();
// QNetworkReply *reply4 = cli4.get(t_url4, RequestOptions{}.withTag("googleOauth"));
// connect(reply4, &QNetworkReply::finished, [reply4]() {
//     // 302 跳转 → 客户端跟随跳转 or 从 redirect 提取 evidence
//     t_reply4->deleteLater();
// });

#endif // GOOGLE_OAUTH_API_H
