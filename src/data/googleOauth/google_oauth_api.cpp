#include "data/googleOauth/google_oauth_api.h"
#include "data/api_global.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

/// 生成授权链接（GET，302 跳转）
bool DeSheng::ProcessGoogleOauthAuthResult(GoogleOauthAuthResponse &responseData,
                                            const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        // 302 跳转无 JSON body，不算错误
        return true;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject tmp_jsonObj = jsonDocument.object();

        // code（302 跳转成功时无此字段，仅错误时返回）
        if (tmp_jsonObj.contains("code") && tmp_jsonObj["code"].isString()) {
            responseData.code = tmp_jsonObj["code"].toString();
        }
        if (tmp_jsonObj.contains("message") && tmp_jsonObj["message"].isString()) {
            responseData.message = tmp_jsonObj["message"].toString();
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "Google OAuth 授权链接 应答解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::GoogleOauthAuthRequestToJson(const GoogleOauthAuthRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject(); // GET 请求无 body
}

bool DeSheng::buildGoogleOauthAuthQuery(const GoogleOauthAuthRequest &req,
                                         QUrlQuery &query,
                                         QString &error)
{
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.app_id, "app_id", error))
        return false;

    if (!req.redirect_url.isEmpty())
        query.addQueryItem("redirect_url", req.redirect_url);

    return true;
}

/// OAuth 回调（GET，302 跳转）
bool DeSheng::ProcessGoogleOauthCallbackResult(GoogleOauthCallbackResponse &responseData,
                                                const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        // 302 跳转无 JSON body，不算错误
        return true;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject tmp_jsonObj = jsonDocument.object();

        // code（302 跳转成功时无此字段，仅错误时返回）
        if (tmp_jsonObj.contains("code") && tmp_jsonObj["code"].isString()) {
            responseData.code = tmp_jsonObj["code"].toString();
        }
        if (tmp_jsonObj.contains("message") && tmp_jsonObj["message"].isString()) {
            responseData.message = tmp_jsonObj["message"].toString();
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "Google OAuth 回调 应答解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::GoogleOauthCallbackRequestToJson(const GoogleOauthCallbackRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject(); // GET 请求无 body
}

bool DeSheng::buildGoogleOauthCallbackQuery(const GoogleOauthCallbackRequest &req,
                                             QUrlQuery &query,
                                             QString &error)
{
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.app_id, "app_id", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.code, "code", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.state, "state", error))
        return false;

    query.addQueryItem("code", req.code);
    query.addQueryItem("state", req.state);

    return true;
}

/// 预授权（桌面端）
bool DeSheng::ProcessGoogleOauthPreAuthResult(GoogleOauthPreAuthResponse &responseData,
                                               const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "Google OAuth 预授权 应答 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject tmp_jsonObj = jsonDocument.object();

        // code
        if (tmp_jsonObj.contains("code") && tmp_jsonObj["code"].isString()) {
            responseData.code = tmp_jsonObj["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        // message
        if (tmp_jsonObj.contains("message") && tmp_jsonObj["message"].isString()) {
            responseData.message = tmp_jsonObj["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

        // data
        if (tmp_jsonObj.contains("data") && tmp_jsonObj["data"].isObject()) {
            QJsonObject tmp_dataObj = tmp_jsonObj["data"].toObject();
            responseData.data.client_id = tmp_dataObj.value("client_id").toString();
            responseData.data.state = tmp_dataObj.value("state").toString();
            responseData.data.redirect_uri = tmp_dataObj.value("redirect_uri").toString();
            responseData.data.scope = tmp_dataObj.value("scope").toString();
            responseData.data.auth_url = tmp_dataObj.value("auth_url").toString();
        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "Google OAuth 预授权 应答解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::GoogleOauthPreAuthRequestToJson(const GoogleOauthPreAuthRequest &req)
{
    QJsonObject obj;
    if (!req.redirect_url.isEmpty())
        obj["redirect_url"] = req.redirect_url;
    return obj;
}

bool DeSheng::buildGoogleOauthPreAuthQuery(const GoogleOauthPreAuthRequest &req,
                                            QUrlQuery &query,
                                            QString &error)
{
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.app_id, "app_id", error))
        return false;

    Q_UNUSED(query);
    return true;
}

/// 查询登录状态
bool DeSheng::ProcessGoogleOauthLoginStatusResult(GoogleOauthLoginStatusResponse &responseData,
                                                   const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "Google OAuth 登录状态查询 应答 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject tmp_jsonObj = jsonDocument.object();

        // code
        if (tmp_jsonObj.contains("code") && tmp_jsonObj["code"].isString()) {
            responseData.code = tmp_jsonObj["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        // message
        if (tmp_jsonObj.contains("message") && tmp_jsonObj["message"].isString()) {
            responseData.message = tmp_jsonObj["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

        // data
        if (tmp_jsonObj.contains("data") && tmp_jsonObj["data"].isObject()) {
            QJsonObject tmp_dataObj = tmp_jsonObj["data"].toObject();
            responseData.data.status = tmp_dataObj.value("status").toString();

            // status=success 时才有 access_token 和 user
            responseData.data.access_token = tmp_dataObj.value("access_token").toString();

            if (tmp_dataObj.contains("user") && tmp_dataObj["user"].isObject()) {
                QJsonObject t_user = tmp_dataObj["user"].toObject();
                responseData.data.user.id = t_user.value("id").toString();
                responseData.data.user.username = t_user.value("username").toString();
                responseData.data.user.nickname = t_user.value("nickname").toString();
                responseData.data.user.avatar = t_user.value("avatar").toString();
                responseData.data.user.login_type = t_user.value("login_type").toString();
            }
        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "Google OAuth 登录状态查询 应答解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::GoogleOauthLoginStatusRequestToJson(const GoogleOauthLoginStatusRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject(); // GET 请求无 body
}

bool DeSheng::buildGoogleOauthLoginStatusQuery(const GoogleOauthLoginStatusRequest &req,
                                                QUrlQuery &query,
                                                QString &error)
{
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.evidence, "evidence", error))
        return false;

    Q_UNUSED(query);
    return true;
}
