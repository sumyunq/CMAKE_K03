#include "data/wechatOauth/wechat_oauth_api.h"
#include "data/api_global.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

/// 预授权 — 获取微信二维码
bool DeSheng::ProcessWechatPreAuthResult(WechatPreAuthResponse &responseData,
                                          const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "微信预授权 应答 JSON文档为空";
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
            responseData.data.auth_url = tmp_dataObj.value("auth_url").toString();
            responseData.data.app_id = tmp_dataObj.value("app_id").toString();
            responseData.data.redirect_uri = tmp_dataObj.value("redirect_uri").toString();
            responseData.data.scope = tmp_dataObj.value("scope").toString();
            responseData.data.state = tmp_dataObj.value("state").toString();
        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "微信预授权 应答解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::WechatPreAuthRequestToJson(const WechatPreAuthRequest &req)
{
    QJsonObject obj;
    obj["redirect_url"] = req.redirect_url;
    return obj;
}

bool DeSheng::buildWechatPreAuthQuery(const WechatPreAuthRequest &req,
                                       QUrlQuery &query,
                                       QString &error)
{
    Q_UNUSED(req);
    Q_UNUSED(query);
    Q_UNUSED(error);
    return true;
}

/// 查询登录状态
bool DeSheng::ProcessWechatLoginStatusResult(WechatLoginStatusResponse &responseData,
                                              const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "微信登录状态查询 应答 JSON文档为空";
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
            responseData.data.access_token = tmp_dataObj.value("access_token").toString();

            // user
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
                 << "微信登录状态查询 应答解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::WechatLoginStatusRequestToJson(const WechatLoginStatusRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildWechatLoginStatusQuery(const WechatLoginStatusRequest &req,
                                           QUrlQuery &query,
                                           QString &error)
{
    if (!req.evidence.isEmpty())
        query.addQueryItem("evidence", req.evidence);
    return true;
}
