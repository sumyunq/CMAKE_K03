#ifndef WECHAT_OAUTH_API_H
#define WECHAT_OAUTH_API_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QUrlQuery>

namespace DeSheng {

/// 路径常量
namespace ApiPaths {
inline constexpr const char *kWechatPreAuth     = "/wechat/oauth/preauth";
inline constexpr const char *kWechatLoginStatus = "/wechat/login-status";
} // namespace ApiPaths

/*************************************************************************************  微信OAuth系统  ************************************************************************************************/

/// PreAuthRequest 预授权请求（内嵌二维码）
typedef struct WechatPreAuthRequest
{
    QString redirect_url; ///< 登录成功后跳转地址 (必填)
} WechatPreAuthRequest;

/// PreAuthResponse 预授权应答
typedef struct WechatPreAuthResponse
{
    QString code;
    QString message;
    typedef struct ReturnData
    {
        QString auth_url;     ///< 微信授权链接
        QString app_id;       ///< 应用ID
        QString redirect_uri; ///< 回调地址
        QString scope;        ///< 授权范围
        QString state;        ///< 会话状态码
    } ReturnData;
    ReturnData data;
} WechatPreAuthResponse;

bool ProcessWechatPreAuthResult(WechatPreAuthResponse &responseData,
                                const QJsonDocument &jsonDocument);

QJsonObject WechatPreAuthRequestToJson(const WechatPreAuthRequest &req);

bool buildWechatPreAuthQuery(const WechatPreAuthRequest &req,
                              QUrlQuery &query,
                              QString &error);

/// LoginStatusRequest 查询登录状态 请求
typedef struct WechatLoginStatusRequest
{
    QString evidence; ///< 会话凭证 (必填)
} WechatLoginStatusRequest;

/// LoginStatusResponse 查询登录状态 应答
typedef struct WechatLoginStatusResponse
{
    QString code;
    QString message;
    typedef struct UserInfo
    {
        QString id;          ///< 用户ID
        QString username;    ///< 用户名
        QString nickname;    ///< 用户昵称
        QString avatar;      ///< 头像URL
        QString login_type;  ///< 登录类型
    } UserInfo;

    typedef struct ReturnData
    {
        QString status;       ///< 登录状态：pending/success/expired
        QString access_token; ///< 访问令牌（status=success 时）
        UserInfo user;        ///< 用户信息（status=success 时）
    } ReturnData;
    ReturnData data;
} WechatLoginStatusResponse;

bool ProcessWechatLoginStatusResult(WechatLoginStatusResponse &responseData,
                                    const QJsonDocument &jsonDocument);

QJsonObject WechatLoginStatusRequestToJson(const WechatLoginStatusRequest &req);

bool buildWechatLoginStatusQuery(const WechatLoginStatusRequest &req,
                                  QUrlQuery &query,
                                  QString &error);

} // namespace DeSheng

// 调用示例（新栈）— 微信预授权
// auto &cli = HttpClient::instance();  /// network/http_client.h
// PreAuthRequest t_req;
// QByteArray t_body = QJsonDocument(PreAuthRequestToJson(t_req)).toJson();
// QNetworkReply *r = cli.post(DeSheng::ApiPaths::kWechatPreAuth, RequestOptions{}.withBody(t_body).withTag("wechatOauth"));
// connect(r, &QNetworkReply::finished, [r]() {
//     PreAuthResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(r->readAll());
//     if (ProcessPreAuthResult(t_resp, t_doc) && t_resp.code == "success") {
//         // t_resp.data.qr_code_url / t_resp.data.state ...
//     }
//     r->deleteLater();
// });
//
// 调用示例（新栈）— 微信登录状态查询
// WechatLoginStatusRequest t_req;
// t_req.state = "sess_xxx";
// QUrlQuery t_query;
// QString t_err;
// buildWechatLoginStatusQuery(t_req, t_query, t_err);
// QNetworkReply *r = cli.get(DeSheng::ApiPaths::kWechatLoginStatus, RequestOptions{}.withQuery(t_query).withTag("wechatOauth"));
// connect(r, &QNetworkReply::finished, [r]() {
//     WechatLoginStatusResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(r->readAll());
//     if (ProcessWechatLoginStatusResult(t_resp, t_doc) && t_resp.code == "success") {
//         // t_resp.data.access_token ...
//     }
//     r->deleteLater();
// });
//
// 推荐写法（HttpClient + RequestOptions）
// auto &cli = HttpClient::instance();
// QByteArray t_body = QJsonDocument(DeSheng::PreAuthRequestToJson(t_req)).toJson();
// QNetworkReply *r = cli.post(DeSheng::ApiPaths::kWechatPreAuth, RequestOptions{}.withBody(t_body).withTag("wechatOauth"));
// connect(r, &QNetworkReply::finished, [r]() { ... ; r->deleteLater(); });

#endif // WECHAT_OAUTH_API_H
