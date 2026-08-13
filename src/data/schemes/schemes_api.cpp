#include "data/schemes/schemes_api.h"
#include "data/api_global.h"


#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>



bool DeSheng::ProcessCreateShareCodeResult(CreateShareCodeResponse &responseData,
                                           QJsonDocument &jsonDocument)
{
    // 清空旧数据
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    // 校验JSON文档是否为空
    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "创建分享码 应答信息 JSON文档为空";
        return false;
    }

    try {
        // 校验根节点是否为对象
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        // 解析 code
        if (root.contains("code") && root["code"].isString()) {
            responseData.code = root["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        // 解析 message
        if (root.contains("message") && root["message"].isString()) {
            responseData.message = root["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

        // 解析 data
        if (root.contains("data") && root["data"].isObject()) {
            QJsonObject dataObj = root["data"].toObject();
            responseData.data.id = dataObj.value("id").toVariant().toLongLong();
            responseData.data.share_code = dataObj.value("share_code").toString();
            responseData.data.url = dataObj.value("url").toString();
            responseData.data.title = dataObj.value("title").toString();
            responseData.data.description = dataObj.value("description").toString();
            responseData.data.device_name = dataObj.value("device_name").toString();
            responseData.data.device_type = dataObj.value("device_type").toString();
            responseData.data.status = dataObj.value("status").toString();
            responseData.data.created_at = dataObj.value("created_at").toString();
        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析异常:" << e.what();
        return false;
    }
    return true;
}

/// 将创建分享码请求结构体转换为JSON对象
QJsonObject DeSheng::CreateShareCodeRequestToJson(const CreateShareCodeRequest &req)
{
    QJsonObject obj;
    obj["url"] = req.url;
    obj["device_name"] = req.device_name;
    obj["device_type"] = req.device_type;

    if (!req.title.isEmpty())
        obj["title"] = req.title;
    if (!req.description.isEmpty())
        obj["description"] = req.description;

    return obj;
}

/// 构建创建分享码接口的URL查询参数
bool DeSheng::buildCreateShareCodeQuery(const CreateShareCodeRequest &req,
                                        QUrlQuery &query,
                                        QString &error)
{
    Q_UNUSED(req);
    Q_UNUSED(query);
    Q_UNUSED(error);
    return true;
}

/// 处理解析分享码接口的响应结果
bool DeSheng::ProcessResolveShareCodeResult(ResolveShareCodeResponse &responseData,
                                            QJsonDocument &jsonDocument)
{
    // 清空旧数据
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    // 校验JSON文档是否为空
    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析分享码 应答信息 JSON文档为空";
        return false;
    }

    try {
        // 校验根节点是否为对象
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        // 解析 code
        if (root.contains("code") && root["code"].isString()) {
            responseData.code = root["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        // 解析 message
        if (root.contains("message") && root["message"].isString()) {
            responseData.message = root["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

        // 解析 data
        if (root.contains("data") && root["data"].isObject()) {
            QJsonObject dataObj = root["data"].toObject();
            responseData.data.share_code = dataObj.value("share_code").toString();
            responseData.data.url = dataObj.value("url").toString();
            responseData.data.title = dataObj.value("title").toString();
            responseData.data.description = dataObj.value("description").toString();
            responseData.data.device_name = dataObj.value("device_name").toString();
            responseData.data.device_type = dataObj.value("device_type").toString();
            responseData.data.status = dataObj.value("status").toString();
        } else {
            qDebug() << "data 数据不存在 或 类型异常";
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析异常:" << e.what();
        return false;
    }
    return true;
}

/// 将解析分享码请求结构体转换为JSON对象
QJsonObject DeSheng::ResolveShareCodeRequestToJson(const ResolveShareCodeRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

/// 构建解析分享码接口的URL查询参数
bool DeSheng::buildResolveShareCodeQuery(const ResolveShareCodeRequest &req,
                                         QUrlQuery &query,
                                         QString &error)
{
    Q_UNUSED(query);

    // 校验分享码
    if (req.share_code.isEmpty()) {
        error = "分享码 不能为空";
        return false;
    }

    return true;
}

/// 处理用户端更新方案接口的响应结果
bool DeSheng::ProcessUpdateSchemeResult(UpdateSchemeResponse &responseData,
                                        QJsonDocument &jsonDocument)
{
    // 清空旧数据
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    // 校验JSON文档是否为空
    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "更新方案 应答信息 JSON文档为空";
        return false;
    }

    try {
        // 校验根节点是否为对象
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        // 解析 code
        if (root.contains("code") && root["code"].isString()) {
            responseData.code = root["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        // 解析 message
        if (root.contains("message") && root["message"].isString()) {
            responseData.message = root["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

        // 解析 data
        if (root.contains("data") && root["data"].isObject()) {
            QJsonObject dataObj = root["data"].toObject();
            responseData.data.id = dataObj.value("id").toVariant().toLongLong();
            responseData.data.share_code = dataObj.value("share_code").toString();
            responseData.data.url = dataObj.value("url").toString();
            responseData.data.title = dataObj.value("title").toString();
            responseData.data.description = dataObj.value("description").toString();
            responseData.data.device_name = dataObj.value("device_name").toString();
            responseData.data.device_type = dataObj.value("device_type").toString();
            responseData.data.status = dataObj.value("status").toString();
            responseData.data.created_at = dataObj.value("created_at").toString();
            responseData.data.updated_at = dataObj.value("updated_at").toString();
        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析异常:" << e.what();
        return false;
    }
    return true;
}

/// 将用户端更新方案请求结构体转换为JSON对象
QJsonObject DeSheng::UpdateSchemeRequestToJson(const UpdateSchemeRequest &req)
{
    QJsonObject obj;

    // 只传入非空字段
    if (!req.title.isEmpty())
        obj["title"] = req.title;
    if (!req.description.isEmpty())
        obj["description"] = req.description;
    if (!req.url.isEmpty())
        obj["url"] = req.url;
    if (!req.device_name.isEmpty())
        obj["device_name"] = req.device_name;
    if (!req.device_type.isEmpty())
        obj["device_type"] = req.device_type;

    return obj;
}

/// 构建用户端更新方案接口的URL查询参数
bool DeSheng::buildUpdateSchemeQuery(const UpdateSchemeRequest &req,
                                     QUrlQuery &query,
                                     QString &error)
{
    Q_UNUSED(query);

    // 校验方案ID
    if (req.id <= 0) {
        error = "方案 ID 不能为空或小于等于 0";
        return false;
    }

    return true;
}

// ============================ 管理端-方案管理 ============================

bool DeSheng::ProcessAdminSchemeListResult(AdminSchemeListResponse &responseData,
                                            QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.list.clear();
    responseData.data.total = 0;
    responseData.data.page = 0;
    responseData.data.page_size = 0;

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端方案列表 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        if (!root.contains("code") || !root["code"].isString()) {
            qDebug() << "code 不存在或类型异常";
            return false;
        }
        responseData.code = root["code"].toString();

        if (!root.contains("message") || !root["message"].isString()) {
            qDebug() << "message 不存在或类型异常";
            return false;
        }
        responseData.message = root["message"].toString();

        if (root.contains("data") && root["data"].isObject()) {
            QJsonObject dataObj = root["data"].toObject();

            if (dataObj.contains("list") && dataObj["list"].isArray()) {
                QJsonArray listArr = dataObj["list"].toArray();
                for (const QJsonValue &val : listArr) {
                    if (!val.isObject())
                        continue;
                    QJsonObject itemObj = val.toObject();
                    AdminSchemeListResponse::ListItem item;
                    item.id = itemObj.value("id").toVariant().toLongLong();
                    item.share_code = itemObj.value("share_code").toString();
                    item.url = itemObj.value("url").toString();
                    item.title = itemObj.value("title").toString();
                    item.description = itemObj.value("description").toString();
                    item.device_name = itemObj.value("device_name").toString();
                    item.device_type = itemObj.value("device_type").toString();
                    item.status = itemObj.value("status").toString();
                    item.user_id = itemObj.value("user_id").toString();
                    item.created_at = itemObj.value("created_at").toString();
                    item.updated_at = itemObj.value("updated_at").toString();
                    responseData.data.list.append(item);
                }
            }

            responseData.data.total = dataObj.value("total").toInt();
            responseData.data.page = dataObj.value("page").toInt();
            responseData.data.page_size = dataObj.value("page_size").toInt();
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端方案列表 解析异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminSchemeListRequestToJson(const AdminSchemeListRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildAdminSchemeListQuery(const AdminSchemeListRequest &req,
                                         QUrlQuery &query,
                                         QString &error)
{
    Q_UNUSED(error);
    if (req.page != 1)
        query.addQueryItem("page", QString::number(req.page));
    if (req.page_size != 20)
        query.addQueryItem("page_size", QString::number(req.page_size));
    if (!req.keyword.isEmpty())
        query.addQueryItem("keyword", req.keyword);
    if (!req.status.isEmpty())
        query.addQueryItem("status", req.status);
    return true;
}

bool DeSheng::ProcessAdminSchemeUpdateResult(AdminSchemeUpdateResponse &responseData,
                                              QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端更新方案 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        if (!root.contains("code") || !root["code"].isString()) {
            qDebug() << "code 不存在或类型异常";
            return false;
        }
        responseData.code = root["code"].toString();

        if (!root.contains("message") || !root["message"].isString()) {
            qDebug() << "message 不存在或类型异常";
            return false;
        }
        responseData.message = root["message"].toString();

        if (root.contains("data") && root["data"].isObject()) {
            QJsonObject dataObj = root["data"].toObject();
            responseData.data.id = dataObj.value("id").toVariant().toLongLong();
            responseData.data.share_code = dataObj.value("share_code").toString();
            responseData.data.url = dataObj.value("url").toString();
            responseData.data.title = dataObj.value("title").toString();
            responseData.data.description = dataObj.value("description").toString();
            responseData.data.device_name = dataObj.value("device_name").toString();
            responseData.data.device_type = dataObj.value("device_type").toString();
            responseData.data.status = dataObj.value("status").toString();
            responseData.data.user_id = dataObj.value("user_id").toString();
            responseData.data.created_at = dataObj.value("created_at").toString();
            responseData.data.updated_at = dataObj.value("updated_at").toString();
        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端更新方案 解析异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminSchemeUpdateRequestToJson(const AdminSchemeUpdateRequest &req)
{
    QJsonObject obj;

    if (!req.title.isEmpty())
        obj["title"] = req.title;
    if (!req.description.isEmpty())
        obj["description"] = req.description;
    if (!req.url.isEmpty())
        obj["url"] = req.url;
    if (!req.device_name.isEmpty())
        obj["device_name"] = req.device_name;
    if (!req.device_type.isEmpty())
        obj["device_type"] = req.device_type;
    if (!req.status.isEmpty())
        obj["status"] = req.status;

    return obj;
}

bool DeSheng::buildAdminSchemeUpdateQuery(const AdminSchemeUpdateRequest &req,
                                           QUrlQuery &query,
                                           QString &error)
{
    Q_UNUSED(query);
    if (req.id <= 0) {
        error = "方案 ID 不能为空或小于等于 0";
        return false;
    }
    return true;
}

bool DeSheng::ProcessAdminSchemeDeleteResult(AdminSchemeDeleteResponse &responseData,
                                              QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端删除方案 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        if (!root.contains("code") || !root["code"].isString()) {
            qDebug() << "code 不存在或类型异常";
            return false;
        }
        responseData.code = root["code"].toString();

        if (!root.contains("message") || !root["message"].isString()) {
            qDebug() << "message 不存在或类型异常";
            return false;
        }
        responseData.message = root["message"].toString();
        // data 为 null，无需解析
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端删除方案 解析异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminSchemeDeleteRequestToJson(const AdminSchemeDeleteRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildAdminSchemeDeleteQuery(const AdminSchemeDeleteRequest &req,
                                           QUrlQuery &query,
                                           QString &error)
{
    Q_UNUSED(query);
    if (req.id <= 0) {
        error = "方案 ID 不能为空或小于等于 0";
        return false;
    }
    return true;
}

