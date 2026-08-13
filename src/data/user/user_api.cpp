#include "data/user/user_api.h"
#include "data/api_global.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

/// 验证码 — 发送邮箱验证码
bool DeSheng::ProcessEmailSendCodeResult(EmailSendCodeResponse &responseData,
                                         QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.message.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "用户请求验证码 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject tmp_jsonObj = jsonDocument.object();
        if (tmp_jsonObj.contains("code") && tmp_jsonObj["code"].isString()) {
            responseData.code = tmp_jsonObj["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }
        if (tmp_jsonObj.contains("message") && tmp_jsonObj["message"].isString()) {
            responseData.message = tmp_jsonObj["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

        if (tmp_jsonObj.contains("data") && tmp_jsonObj["data"].isObject()) {
            QJsonObject tmp_dataObj = tmp_jsonObj["data"].toObject();
            if (tmp_dataObj.contains("message") && tmp_dataObj["message"].isString()) {
                responseData.data.message = tmp_dataObj["message"].toString();
            } else {
                qDebug() << "data 内 message 数据不存在 或 类型异常";
                return false;
            }

        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "处理验证码 应答信息时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::EmailSendCodeRequestToJson(const EmailSendCodeRequest &req)
{
    QJsonObject obj;
    obj["email"] = req.email;
    obj["scene"] = req.scene;
    return obj;
}

bool DeSheng::buildEmailSendCodeQuery(const EmailSendCodeRequest &req,
                                      QUrlQuery &query,
                                      QString &error)
{
    /// 校验所有必填字段
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.email, "email", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.scene, "scene", error))
        return false;

    /// 构建 query
    query.addQueryItem("email", req.email);
    query.addQueryItem("scene", req.scene);

    return true;
}

/// 用户注册
bool DeSheng::ProcessUserSignUpResult(UserSignUpResponse &responseData, QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.id.clear();
    responseData.data.username.clear();
    responseData.data.email.clear();
    responseData.data.nickname.clear();
    responseData.data.avatar.clear();
    responseData.data.status.clear();
    responseData.data.login_type.clear();
    responseData.data.created_at.clear();
    responseData.data.last_login_at.clear();
    responseData.data.favorite_games.clear();
    responseData.data.activation_code.clear();
    responseData.data.city.clear();
    responseData.data.login_ip.clear();
    responseData.data.os_info.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "注册请求 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }

        QJsonObject tmp_jsonObj = jsonDocument.object();
        if (tmp_jsonObj.contains("code") && tmp_jsonObj["code"].isString()) {
            responseData.code = tmp_jsonObj["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }
        if (tmp_jsonObj.contains("message") && tmp_jsonObj["message"].isString()) {
            responseData.message = tmp_jsonObj["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

        if (tmp_jsonObj.contains("data") && tmp_jsonObj["data"].isObject()) {
            QJsonObject tmp_dataObj = tmp_jsonObj["data"].toObject();
            if (tmp_dataObj.contains("id") && tmp_dataObj["id"].isString()) {
                responseData.data.id = tmp_dataObj["id"].toString();
            } else {
                qDebug() << "data 内 id 数据不存在 或 类型异常";
                return false;
            }
            if (tmp_dataObj.contains("username") && tmp_dataObj["username"].isString()) {
                responseData.data.username = tmp_dataObj["username"].toString();
            } else {
                qDebug() << "data 内 username 数据不存在 或 类型异常";
                return false;
            }
            if (tmp_dataObj.contains("email") && tmp_dataObj["email"].isString()) {
                responseData.data.email = tmp_dataObj["email"].toString();
            } else {
                qDebug() << "data 内 email 数据不存在 或 类型异常";
                return false;
            }
            if (tmp_dataObj.contains("status") && tmp_dataObj["status"].isString()) {
                responseData.data.status = tmp_dataObj["status"].toString();
            } else {
                qDebug() << "data 内 status 数据不存在 或 类型异常";
                return false;
            }
            if (tmp_dataObj.contains("login_type") && tmp_dataObj["login_type"].isString()) {
                responseData.data.login_type = tmp_dataObj["login_type"].toString();
            } else {
                qDebug() << "data 内 login_type 数据不存在 或 类型异常";
                return false;
            }
            // 以下为可选字段
            if (tmp_dataObj.contains("nickname") && tmp_dataObj["nickname"].isString()) {
                responseData.data.nickname = tmp_dataObj["nickname"].toString();
            }
            if (tmp_dataObj.contains("avatar") && tmp_dataObj["avatar"].isString()) {
                responseData.data.avatar = tmp_dataObj["avatar"].toString();
            }
            if (tmp_dataObj.contains("created_at") && tmp_dataObj["created_at"].isString()) {
                responseData.data.created_at = tmp_dataObj["created_at"].toString();
            }
            if (tmp_dataObj.contains("last_login_at")
                && tmp_dataObj["last_login_at"].isString()) {
                responseData.data.last_login_at = tmp_dataObj["last_login_at"].toString();
            }
            if (tmp_dataObj.contains("favorite_games")
                && tmp_dataObj["favorite_games"].isString()) {
                responseData.data.favorite_games = tmp_dataObj["favorite_games"].toString();
            }
            if (tmp_dataObj.contains("activation_code")
                && tmp_dataObj["activation_code"].isString()) {
                responseData.data.activation_code = tmp_dataObj["activation_code"].toString();
            }
            if (tmp_dataObj.contains("city") && tmp_dataObj["city"].isString()) {
                responseData.data.city = tmp_dataObj["city"].toString();
            }
            if (tmp_dataObj.contains("login_ip") && tmp_dataObj["login_ip"].isString()) {
                responseData.data.login_ip = tmp_dataObj["login_ip"].toString();
            }
            if (tmp_dataObj.contains("os_info") && tmp_dataObj["os_info"].isString()) {
                responseData.data.os_info = tmp_dataObj["os_info"].toString();
            }

        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "用户请求注册 应答信息时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::UserSignUpRequestToJson(const DeSheng::UserSignUpRequest &req)
{
    QJsonObject obj;
    obj["username"] = req.username;
    obj["email"] = req.email;
    obj["password"] = req.password;
    if (!req.nickname.isEmpty()) {
        obj["nickname"] = req.nickname;
    }
    obj["code"] = req.code;
    return obj;
}

bool DeSheng::buildUserSignUpQuery(const UserSignUpRequest &req, QUrlQuery &query, QString &error)
{
    /// 校验所有必填字段
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.username, "username", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.email, "email", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.password, "password", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.code, "code", error))
        return false;

    /// 构建 query
    query.addQueryItem("username", req.username);
    query.addQueryItem("email", req.email);
    query.addQueryItem("password", req.password);
    if (!req.nickname.isEmpty()) {
        query.addQueryItem("nickname", req.nickname);
    }
    query.addQueryItem("code", req.code);

    return true;
}

/// 用户登录
bool DeSheng::ProcessUserLoginResult(UserLoginResponse &responseData,
                                     const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.access_token.clear();
    responseData.data.refresh_token.clear();
    responseData.data.user.id.clear();
    responseData.data.user.username.clear();
    responseData.data.user.email.clear();
    responseData.data.user.nickname.clear();
    responseData.data.user.avatar.clear();
    responseData.data.user.status.clear();
    responseData.data.user.login_type.clear();
    responseData.data.user.created_at.clear();
    responseData.data.user.last_login_at.clear();
    responseData.data.user.favorite_games.clear();
    responseData.data.user.activation_code.clear();
    responseData.data.user.city.clear();
    responseData.data.user.login_ip.clear();
    responseData.data.user.os_info.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "用户登录请求 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject rootObj = jsonDocument.object();

        if (!rootObj.contains("code") || !rootObj["code"].isString()) {
            qDebug() << "code 不存在或类型异常";
            return false;
        }
        responseData.code = rootObj["code"].toString();

        if (!rootObj.contains("message") || !rootObj["message"].isString()) {
            qDebug() << "message 不存在或类型异常";
            return false;
        }
        responseData.message = rootObj["message"].toString();

        // data 可能为 null（登录失败时），仅当是 object 时才解析
        if (!rootObj.contains("data") || !rootObj["data"].isObject()) {
            return true;
        }
        QJsonObject dataObj = rootObj["data"].toObject();

        if (!dataObj.contains("access_token") || !dataObj["access_token"].isString()) {
            qDebug() << "access_token 不存在或类型异常";
            return false;
        }
        responseData.data.access_token = dataObj["access_token"].toString();

        // refresh_token 仅在 rememberMe=true 时返回，为可选字段
        if (dataObj.contains("refresh_token") && dataObj["refresh_token"].isString()) {
            responseData.data.refresh_token = dataObj["refresh_token"].toString();
        }

        if (!dataObj.contains("user") || !dataObj["user"].isObject()) {
            qDebug() << "user 不存在或类型异常";
            return false;
        }
        QJsonObject userObj = dataObj["user"].toObject();

        // 解析 user 各字段
        if (userObj.contains("id") && userObj["id"].isString())
            responseData.data.user.id = userObj["id"].toString();
        else {
            qDebug() << "user.id 不存在或类型异常";
            return false;
        }

        if (userObj.contains("username") && userObj["username"].isString())
            responseData.data.user.username = userObj["username"].toString();
        else {
            qDebug() << "user.username 不存在或类型异常";
            return false;
        }
        if (userObj.contains("email") && userObj["email"].isString())
            responseData.data.user.email = userObj["email"].toString();
        else {
            qDebug() << "user.email 不存在或类型异常";
            return false;
        }
        if (userObj.contains("nickname") && userObj["nickname"].isString())
            responseData.data.user.nickname = userObj["nickname"].toString();
        if (userObj.contains("status") && userObj["status"].isString())
            responseData.data.user.status = userObj["status"].toString();
        else {
            qDebug() << "user.status 不存在或类型异常";
            return false;
        }
        if (userObj.contains("login_type") && userObj["login_type"].isString())
            responseData.data.user.login_type = userObj["login_type"].toString();
        else {
            qDebug() << "user.login_type 不存在或类型异常";
            return false;
        }
        // 以下为可选字段
        if (userObj.contains("avatar") && userObj["avatar"].isString())
            responseData.data.user.avatar = userObj["avatar"].toString();
        if (userObj.contains("created_at") && userObj["created_at"].isString())
            responseData.data.user.created_at = userObj["created_at"].toString();
        if (userObj.contains("last_login_at") && userObj["last_login_at"].isString())
            responseData.data.user.last_login_at = userObj["last_login_at"].toString();
        if (userObj.contains("favorite_games") && userObj["favorite_games"].isString())
            responseData.data.user.favorite_games = userObj["favorite_games"].toString();
        if (userObj.contains("activation_code") && userObj["activation_code"].isString())
            responseData.data.user.activation_code = userObj["activation_code"].toString();
        if (userObj.contains("city") && userObj["city"].isString())
            responseData.data.user.city = userObj["city"].toString();
        if (userObj.contains("login_ip") && userObj["login_ip"].isString())
            responseData.data.user.login_ip = userObj["login_ip"].toString();
        if (userObj.contains("os_info") && userObj["os_info"].isString())
            responseData.data.user.os_info = userObj["os_info"].toString();
        // 用户角色/头衔（数组）
        if (userObj.contains("roles") && userObj["roles"].isArray()) {
            const QJsonArray t_roles = userObj["roles"].toArray();
            for (const QJsonValue &v : t_roles)
                responseData.data.user.roles.append(v.toString());
        }
        if (userObj.contains("titles") && userObj["titles"].isArray()) {
            const QJsonArray t_titles = userObj["titles"].toArray();
            for (const QJsonValue &v : t_titles)
                responseData.data.user.titles.append(v.toString());
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "用户登录请求 应答信息解析时发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::UserLoginRequestToJson(const UserLoginRequest &req)
{
    QJsonObject obj;
    obj["email"] = req.email;
    obj["password"] = req.password;
    obj["rememberMe"] = req.rememberMe;
    return obj;
}

bool DeSheng::buildUserLoginUpQuery(const UserLoginRequest &req, QUrlQuery &query, QString &error)
{
    /// 校验所有必填字段
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.email, "email", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.password, "password", error))
        return false;

    /// 构建 query
    query.addQueryItem("email", req.email);
    query.addQueryItem("password", req.password);
    query.addQueryItem("rememberMe", req.rememberMe ? "true" : "false");

    return true;
}

/// 用户退出
bool DeSheng::ProcessUserLogoutResult(UserLogoutResponse &responseData,
                                      const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "用户退出 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject rootObj = jsonDocument.object();

        if (!rootObj.contains("code") || !rootObj["code"].isString()) {
            qDebug() << "code 不存在或类型异常";
            return false;
        }
        responseData.code = rootObj["code"].toString();

        if (!rootObj.contains("message") || !rootObj["message"].isString()) {
            qDebug() << "message 不存在或类型异常";
            return false;
        }
        responseData.message = rootObj["message"].toString();

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "用户退出 应答解析异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::UserLogoutRequestToJson(const UserLogoutRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildUserLogoutQuery(const UserLogoutRequest &req,
                                   QUrlQuery &query,
                                   QString &error)
{
    Q_UNUSED(req);
    Q_UNUSED(query);
    Q_UNUSED(error);
    return true;
}

/// 获取当前用户信息
bool DeSheng::ProcessGetCurrentUserResult(GetCurrentUserResponse &responseData,
                                          const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "获取当前用户信息 应答 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject rootObj = jsonDocument.object();

        if (!rootObj.contains("code") || !rootObj["code"].isString()) {
            qDebug() << "code 不存在或类型异常";
            return false;
        }
        responseData.code = rootObj["code"].toString();

        if (!rootObj.contains("message") || !rootObj["message"].isString()) {
            qDebug() << "message 不存在或类型异常";
            return false;
        }
        responseData.message = rootObj["message"].toString();

        if (rootObj.contains("data") && rootObj["data"].isObject()) {
            QJsonObject dataObj = rootObj["data"].toObject();
            responseData.data.id = dataObj.value("id").toString();
            responseData.data.username = dataObj.value("username").toString();
            responseData.data.email = dataObj.value("email").toString();
            responseData.data.nickname = dataObj.value("nickname").toString();
            responseData.data.avatar = dataObj.value("avatar").toString();
            responseData.data.status = dataObj.value("status").toString();
            responseData.data.login_type = dataObj.value("login_type").toString();
            responseData.data.created_at = dataObj.value("created_at").toString();
            responseData.data.last_login_at = dataObj.value("last_login_at").toString();
            responseData.data.favorite_games = dataObj.value("favorite_games").toString();
            responseData.data.activation_code = dataObj.value("activation_code").toString();
            responseData.data.city = dataObj.value("city").toString();
            responseData.data.login_ip = dataObj.value("login_ip").toString();
            responseData.data.os_info = dataObj.value("os_info").toString();
            responseData.data.bio = dataObj.value("bio").toString();
            // 用户角色/头衔（数组）
            if (dataObj.contains("roles") && dataObj["roles"].isArray()) {
                const QJsonArray t_roles = dataObj["roles"].toArray();
                for (const QJsonValue &v : t_roles)
                    responseData.data.roles.append(v.toString());
            }
            if (dataObj.contains("titles") && dataObj["titles"].isArray()) {
                const QJsonArray t_titles = dataObj["titles"].toArray();
                for (const QJsonValue &v : t_titles)
                    responseData.data.titles.append(v.toString());
            }
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "获取当前用户信息 解析异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::GetCurrentUserRequestToJson(const GetCurrentUserRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildGetCurrentUserQuery(const GetCurrentUserRequest &req,
                                       QUrlQuery &query,
                                       QString &error)
{
    Q_UNUSED(req);
    Q_UNUSED(query);
    Q_UNUSED(error);
    return true;
}

/// 获取其他用户公开信息
bool DeSheng::ProcessGetPublicUserInfoResult(GetPublicUserInfoResponse &responseData,
                                             const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "获取其他用户公开信息 应答 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject rootObj = jsonDocument.object();

        if (!rootObj.contains("code") || !rootObj["code"].isString()) {
            qDebug() << "code 不存在或类型异常";
            return false;
        }
        responseData.code = rootObj["code"].toString();

        if (!rootObj.contains("message") || !rootObj["message"].isString()) {
            qDebug() << "message 不存在或类型异常";
            return false;
        }
        responseData.message = rootObj["message"].toString();

        if (rootObj.contains("data") && rootObj["data"].isObject()) {
            QJsonObject dataObj = rootObj["data"].toObject();
            responseData.data.id = dataObj.value("id").toString();
            responseData.data.username = dataObj.value("username").toString();
            responseData.data.nickname = dataObj.value("nickname").toString();
            responseData.data.avatar = dataObj.value("avatar").toString();
            responseData.data.bio = dataObj.value("bio").toString();
            responseData.data.favorite_games = dataObj.value("favorite_games").toString();
            responseData.data.city = dataObj.value("city").toString();
            responseData.data.created_at = dataObj.value("created_at").toString();
            // 用户角色/头衔（数组）
            if (dataObj.contains("roles") && dataObj["roles"].isArray()) {
                const QJsonArray t_roles = dataObj["roles"].toArray();
                for (const QJsonValue &v : t_roles)
                    responseData.data.roles.append(v.toString());
            }
            if (dataObj.contains("titles") && dataObj["titles"].isArray()) {
                const QJsonArray t_titles = dataObj["titles"].toArray();
                for (const QJsonValue &v : t_titles)
                    responseData.data.titles.append(v.toString());
            }
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "获取其他用户公开信息 解析异常:" << e.what();
        return false;
    }
    return true;
}

/// 更新用户信息
bool DeSheng::ProcessUpdateUserResult(UpdateUserResponse &responseData,
                                      const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "更新用户信息 应答 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject rootObj = jsonDocument.object();

        if (!rootObj.contains("code") || !rootObj["code"].isString()) {
            qDebug() << "code 不存在或类型异常";
            return false;
        }
        responseData.code = rootObj["code"].toString();

        if (!rootObj.contains("message") || !rootObj["message"].isString()) {
            qDebug() << "message 不存在或类型异常";
            return false;
        }
        responseData.message = rootObj["message"].toString();

        if (rootObj.contains("data") && rootObj["data"].isObject()) {
            QJsonObject dataObj = rootObj["data"].toObject();
            responseData.data.id = dataObj.value("id").toString();
            responseData.data.username = dataObj.value("username").toString();
            responseData.data.email = dataObj.value("email").toString();
            responseData.data.nickname = dataObj.value("nickname").toString();
            responseData.data.avatar = dataObj.value("avatar").toString();
            responseData.data.status = dataObj.value("status").toString();
            responseData.data.login_type = dataObj.value("login_type").toString();
            responseData.data.created_at = dataObj.value("created_at").toString();
            responseData.data.last_login_at = dataObj.value("last_login_at").toString();
            responseData.data.favorite_games = dataObj.value("favorite_games").toString();
            responseData.data.activation_code = dataObj.value("activation_code").toString();
            responseData.data.city = dataObj.value("city").toString();
            responseData.data.login_ip = dataObj.value("login_ip").toString();
            responseData.data.os_info = dataObj.value("os_info").toString();
            responseData.data.bio = dataObj.value("bio").toString();
            // 用户角色/头衔（数组）
            if (dataObj.contains("roles") && dataObj["roles"].isArray()) {
                const QJsonArray t_roles = dataObj["roles"].toArray();
                for (const QJsonValue &v : t_roles)
                    responseData.data.roles.append(v.toString());
            }
            if (dataObj.contains("titles") && dataObj["titles"].isArray()) {
                const QJsonArray t_titles = dataObj["titles"].toArray();
                for (const QJsonValue &v : t_titles)
                    responseData.data.titles.append(v.toString());
            }
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "更新用户信息 解析异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::UpdateUserRequestToJson(const UpdateUserRequest &req)
{
    QJsonObject obj;
    if (!req.username.isEmpty())
        obj["username"] = req.username;
    if (!req.nickname.isEmpty())
        obj["nickname"] = req.nickname;
    if (!req.avatar.isEmpty())
        obj["avatar"] = req.avatar;
    if (!req.email.isEmpty())
        obj["email"] = req.email;
    if (!req.status.isEmpty())
        obj["status"] = req.status;
    if (!req.favorite_games.isEmpty())
        obj["favorite_games"] = req.favorite_games;
    if (!req.activation_code.isEmpty())
        obj["activation_code"] = req.activation_code;
    if (!req.bio.isEmpty())
        obj["bio"] = req.bio;
    return obj;
}

bool DeSheng::buildUpdateUserQuery(const UpdateUserRequest &req,
                                   QUrlQuery &query,
                                   QString &error)
{
    Q_UNUSED(req);
    Q_UNUSED(query);
    Q_UNUSED(error);
    return true;
}

/// 修改密码
bool DeSheng::ProcessChangePasswordResult(ChangePasswordResponse &responseData,
                                          const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "修改密码 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject rootObj = jsonDocument.object();

        if (!rootObj.contains("code") || !rootObj["code"].isString()) {
            qDebug() << "code 不存在或类型异常";
            return false;
        }
        responseData.code = rootObj["code"].toString();

        if (!rootObj.contains("message") || !rootObj["message"].isString()) {
            qDebug() << "message 不存在或类型异常";
            return false;
        }
        responseData.message = rootObj["message"].toString();

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "修改密码 解析异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::ChangePasswordRequestToJson(const ChangePasswordRequest &req)
{
    QJsonObject obj;
    obj["old_password"] = req.old_password;
    obj["new_password"] = req.new_password;
    obj["email"] = req.email;
    obj["code"] = req.code;
    return obj;
}

bool DeSheng::buildChangePasswordQuery(const ChangePasswordRequest &req,
                                       QUrlQuery &query,
                                       QString &error)
{
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.old_password, "old_password", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.new_password, "new_password", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.email, "email", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.code, "code", error))
        return false;

    query.addQueryItem("old_password", req.old_password);
    query.addQueryItem("new_password", req.new_password);
    query.addQueryItem("email", req.email);
    query.addQueryItem("code", req.code);

    return true;
}

/// 找回密码
bool DeSheng::ProcessForgotPasswordResult(ForgotPasswordResponse &responseData,
                                          const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "找回密码 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject rootObj = jsonDocument.object();

        if (!rootObj.contains("code") || !rootObj["code"].isString()) {
            qDebug() << "code 不存在或类型异常";
            return false;
        }
        responseData.code = rootObj["code"].toString();

        if (!rootObj.contains("message") || !rootObj["message"].isString()) {
            qDebug() << "message 不存在或类型异常";
            return false;
        }
        responseData.message = rootObj["message"].toString();

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "找回密码 解析异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::ForgotPasswordRequestToJson(const ForgotPasswordRequest &req)
{
    QJsonObject obj;
    obj["email"] = req.email;
    obj["code"] = req.code;
    obj["new_password"] = req.new_password;
    return obj;
}

bool DeSheng::buildForgotPasswordQuery(const ForgotPasswordRequest &req,
                                       QUrlQuery &query,
                                       QString &error)
{
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.email, "email", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.code, "code", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.new_password, "new_password", error))
        return false;

    query.addQueryItem("email", req.email);
    query.addQueryItem("code", req.code);
    query.addQueryItem("new_password", req.new_password);

    return true;
}

/// 管理端 — 获取用户列表
bool DeSheng::ProcessAdminUserListResult(AdminUserListResponse &responseData,
                                          const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端用户列表 应答 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject rootObj = jsonDocument.object();

        // code（管理端兼容字符串 "success" 与数字 200）
        if (rootObj.contains("code")) {
            if (rootObj["code"].isString())
                responseData.code = rootObj["code"].toString();
            else if (rootObj["code"].isDouble())
                responseData.code = (rootObj["code"].toInt() == 200) ? "success"
                                                                      : QString::number(rootObj["code"].toInt());
            else {
                qDebug() << "code 数据不存在 或 类型异常";
                return false;
            }
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        if (rootObj.contains("message") && rootObj["message"].isString()) {
            responseData.message = rootObj["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

        if (rootObj.contains("data") && rootObj["data"].isObject()) {
            QJsonObject dataObj = rootObj["data"].toObject();

            responseData.data.total = dataObj.value("total").toInt(0);
            responseData.data.page = dataObj.value("page").toInt(0);
            responseData.data.size = dataObj.value("size").toInt(0);

            // 解析 list 数组
            if (dataObj.contains("list") && dataObj["list"].isArray()) {
                QJsonArray listArr = dataObj["list"].toArray();
                for (const QJsonValue &val : listArr) {
                    if (!val.isObject())
                        continue;
                    QJsonObject userObj = val.toObject();
                    AdminUserInfo info;
                    info.id = userObj.value("id").toString();
                    info.username = userObj.value("username").toString();
                    info.email = userObj.value("email").toString();
                    info.nickname = userObj.value("nickname").toString();
                    info.avatar = userObj.value("avatar").toString();
                    info.status = userObj.value("status").toString();
                    info.login_type = userObj.value("login_type").toString();
                    info.created_at = userObj.value("created_at").toString();
                    info.last_login_at = userObj.value("last_login_at").toString();
                    info.favorite_games = userObj.value("favorite_games").toString();
                    info.activation_code = userObj.value("activation_code").toString();
                    info.city = userObj.value("city").toString();
                    info.login_ip = userObj.value("login_ip").toString();
                    info.os_info = userObj.value("os_info").toString();
                    responseData.data.list.append(info);
                }
            }
        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "管理端用户列表 应答解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminUserListRequestToJson(const AdminUserListRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject(); // GET 请求无 body
}

bool DeSheng::buildAdminUserListQuery(const AdminUserListRequest &req,
                                       QUrlQuery &query,
                                       QString &error)
{
    Q_UNUSED(error);

    if (!req.id.isEmpty())
        query.addQueryItem("id", req.id);
    if (!req.username.isEmpty())
        query.addQueryItem("username", req.username);
    if (!req.email.isEmpty())
        query.addQueryItem("email", req.email);
    if (!req.nickname.isEmpty())
        query.addQueryItem("nickname", req.nickname);
    if (!req.status.isEmpty())
        query.addQueryItem("status", req.status);
    if (!req.device_type.isEmpty())
        query.addQueryItem("device_type", req.device_type);
    if (!req.sort_by.isEmpty())
        query.addQueryItem("sort_by", req.sort_by);
    if (!req.order_by.isEmpty())
        query.addQueryItem("order_by", req.order_by);

    query.addQueryItem("page", QString::number(req.page));
    query.addQueryItem("page_size", QString::number(req.page_size));

    return true;
}

/// 管理端 — 获取用户详情
bool DeSheng::ProcessAdminUserDetailResult(AdminUserDetailResponse &responseData,
                                            const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端用户详情 应答 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject rootObj = jsonDocument.object();

        if (rootObj.contains("code")) {
            if (rootObj["code"].isString())
                responseData.code = rootObj["code"].toString();
            else if (rootObj["code"].isDouble())
                responseData.code = (rootObj["code"].toInt() == 200) ? "success"
                                                                      : QString::number(rootObj["code"].toInt());
            else {
                qDebug() << "code 数据不存在 或 类型异常";
                return false;
            }
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        if (rootObj.contains("message") && rootObj["message"].isString()) {
            responseData.message = rootObj["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

        if (rootObj.contains("data") && rootObj["data"].isObject()) {
            QJsonObject dataObj = rootObj["data"].toObject();
            responseData.data.id = dataObj.value("id").toString();
            responseData.data.username = dataObj.value("username").toString();
            responseData.data.email = dataObj.value("email").toString();
            responseData.data.nickname = dataObj.value("nickname").toString();
            responseData.data.avatar = dataObj.value("avatar").toString();
            responseData.data.status = dataObj.value("status").toString();
            responseData.data.login_type = dataObj.value("login_type").toString();
            responseData.data.created_at = dataObj.value("created_at").toString();
            responseData.data.last_login_at = dataObj.value("last_login_at").toString();
            responseData.data.favorite_games = dataObj.value("favorite_games").toString();
            responseData.data.activation_code = dataObj.value("activation_code").toString();
            responseData.data.city = dataObj.value("city").toString();
            responseData.data.login_ip = dataObj.value("login_ip").toString();
            responseData.data.os_info = dataObj.value("os_info").toString();
        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "管理端用户详情 应答解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminUserDetailRequestToJson(const AdminUserDetailRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject(); // GET 请求无 body
}

bool DeSheng::buildAdminUserDetailQuery(const AdminUserDetailRequest &req,
                                         QUrlQuery &query,
                                         QString &error)
{
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.user_id, "user_id", error))
        return false;
    Q_UNUSED(query);
    return true;
}

/// 管理端 — 更新用户信息
bool DeSheng::ProcessAdminUserUpdateResult(AdminUserUpdateResponse &responseData,
                                            const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端更新用户 应答 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject rootObj = jsonDocument.object();

        if (rootObj.contains("code")) {
            if (rootObj["code"].isString())
                responseData.code = rootObj["code"].toString();
            else if (rootObj["code"].isDouble())
                responseData.code = (rootObj["code"].toInt() == 200) ? "success"
                                                                      : QString::number(rootObj["code"].toInt());
            else {
                qDebug() << "code 数据不存在 或 类型异常";
                return false;
            }
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        if (rootObj.contains("message") && rootObj["message"].isString()) {
            responseData.message = rootObj["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

        if (rootObj.contains("data") && rootObj["data"].isObject()) {
            QJsonObject dataObj = rootObj["data"].toObject();
            responseData.data.id = dataObj.value("id").toString();
            responseData.data.username = dataObj.value("username").toString();
            responseData.data.email = dataObj.value("email").toString();
            responseData.data.nickname = dataObj.value("nickname").toString();
            responseData.data.avatar = dataObj.value("avatar").toString();
            responseData.data.status = dataObj.value("status").toString();
            responseData.data.login_type = dataObj.value("login_type").toString();
            responseData.data.created_at = dataObj.value("created_at").toString();
            responseData.data.last_login_at = dataObj.value("last_login_at").toString();
            responseData.data.favorite_games = dataObj.value("favorite_games").toString();
            responseData.data.activation_code = dataObj.value("activation_code").toString();
            responseData.data.city = dataObj.value("city").toString();
            responseData.data.login_ip = dataObj.value("login_ip").toString();
            responseData.data.os_info = dataObj.value("os_info").toString();
        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "管理端更新用户 应答解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminUserUpdateRequestToJson(const AdminUserUpdateRequest &req)
{
    QJsonObject obj;
    if (!req.username.isEmpty())
        obj["username"] = req.username;
    if (!req.nickname.isEmpty())
        obj["nickname"] = req.nickname;
    if (!req.avatar.isEmpty())
        obj["avatar"] = req.avatar;
    if (!req.email.isEmpty())
        obj["email"] = req.email;
    if (!req.status.isEmpty())
        obj["status"] = req.status;
    if (!req.favorite_games.isEmpty())
        obj["favorite_games"] = req.favorite_games;
    if (!req.activation_code.isEmpty())
        obj["activation_code"] = req.activation_code;
    return obj;
}

bool DeSheng::buildAdminUserUpdateQuery(const AdminUserUpdateRequest &req,
                                         QUrlQuery &query,
                                         QString &error)
{
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.user_id, "user_id", error))
        return false;
    Q_UNUSED(query);
    return true;
}

/// 管理端 — 删除用户（软删除）
bool DeSheng::ProcessAdminUserDeleteResult(AdminUserDeleteResponse &responseData,
                                            const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端删除用户 应答 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject rootObj = jsonDocument.object();

        if (rootObj.contains("code")) {
            if (rootObj["code"].isString())
                responseData.code = rootObj["code"].toString();
            else if (rootObj["code"].isDouble())
                responseData.code = (rootObj["code"].toInt() == 200) ? "success"
                                                                      : QString::number(rootObj["code"].toInt());
            else {
                qDebug() << "code 数据不存在 或 类型异常";
                return false;
            }
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        if (rootObj.contains("message") && rootObj["message"].isString()) {
            responseData.message = rootObj["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "管理端删除用户 应答解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminUserDeleteRequestToJson(const AdminUserDeleteRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject(); // DELETE 请求无 body
}

bool DeSheng::buildAdminUserDeleteQuery(const AdminUserDeleteRequest &req,
                                         QUrlQuery &query,
                                         QString &error)
{
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.user_id, "user_id", error))
        return false;
    Q_UNUSED(query);
    return true;
}

/// 管理端 — 获取用户等级
bool DeSheng::ProcessAdminUserLevelResult(AdminUserLevelResponse &responseData,
                                           const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端用户等级 应答 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject rootObj = jsonDocument.object();

        if (rootObj.contains("code")) {
            if (rootObj["code"].isString())
                responseData.code = rootObj["code"].toString();
            else if (rootObj["code"].isDouble())
                responseData.code = (rootObj["code"].toInt() == 200) ? "success"
                                                                      : QString::number(rootObj["code"].toInt());
            else {
                qDebug() << "code 数据不存在 或 类型异常";
                return false;
            }
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        if (rootObj.contains("message") && rootObj["message"].isString()) {
            responseData.message = rootObj["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

        if (rootObj.contains("data") && rootObj["data"].isObject()) {
            QJsonObject dataObj = rootObj["data"].toObject();
            responseData.data.user_id = dataObj.value("user_id").toInt(0);
            responseData.data.level = dataObj.value("level").toInt(0);
            responseData.data.total_experience = dataObj.value("total_experience").toInt(0);
            responseData.data.current_experience = dataObj.value("current_experience").toInt(0);
            responseData.data.exp_cap = dataObj.value("exp_cap").toInt(0);
        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "管理端用户等级 应答解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminUserLevelRequestToJson(const AdminUserLevelRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject(); // GET 请求无 body
}

bool DeSheng::buildAdminUserLevelQuery(const AdminUserLevelRequest &req,
                                        QUrlQuery &query,
                                        QString &error)
{
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.user_id, "user_id", error))
        return false;
    Q_UNUSED(query);
    return true;
}
