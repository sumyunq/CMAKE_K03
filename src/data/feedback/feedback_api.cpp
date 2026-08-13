#include "data/feedback/feedback_api.h"
#include "data/api_global.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>


bool DeSheng::ProcessFileUploadsResult(FileUploadsResponse &responseData,
                                       QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.id.clear();
    responseData.data.driver.clear();
    responseData.data.name.clear();
    responseData.data.mime_type.clear();
    // responseData.data.path.clear();
    responseData.data.size.clear();
    responseData.data.url.clear();
    responseData.data.user_id.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "用户上传文件请求 应答信息 JSON文档为空";
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

            if (tmp_dataObj.contains("driver") && tmp_dataObj["driver"].isString()) {
                responseData.data.driver = tmp_dataObj["driver"].toString();
            } else {
                qDebug() << "data 内 driver 数据不存在 或 类型异常";
                return false;
            }

            if (tmp_dataObj.contains("name") && tmp_dataObj["name"].isString()) {
                responseData.data.name = tmp_dataObj["name"].toString();
            } else {
                qDebug() << "data 内 name 数据不存在 或 类型异常";
                return false;
            }
            if (tmp_dataObj.contains("mime_type") && tmp_dataObj["mime_type"].isString()) {
                responseData.data.mime_type = tmp_dataObj["mime_type"].toString();
            } else {
                qDebug() << "data 内 mime_type 数据不存在 或 类型异常";
                return false;
            }
            // if (tmp_dataObj.contains("path") && tmp_dataObj["path"].isString()) {
            //     responseData.data.path = tmp_dataObj["path"].toString();
            // } else {
            //     qDebug() << "data 内 path 数据不存在 或 类型异常";
            //     return false;
            // }
            if (tmp_dataObj.contains("size") && tmp_dataObj["size"].isDouble()) {
                int sizeValue = tmp_dataObj["size"].toInt();
                responseData.data.size = QString::number(sizeValue); // 转存为 QString
            } else {
                qDebug() << "data 内 size 数据不存在 或 类型异常";
                return false;
            }

            if (tmp_dataObj.contains("url") && tmp_dataObj["url"].isString()) {
                responseData.data.url = tmp_dataObj["url"].toString();
            } else {
                qDebug() << "data 内 url 数据不存在 或 类型异常";
                return false;
            }
            if (tmp_dataObj.contains("user_id") && tmp_dataObj["user_id"].isString()) {
                responseData.data.user_id = tmp_dataObj["user_id"].toString();
            } else {
                qDebug() << "data 内 user_id 数据不存在 或 类型异常";
                return false;
            }
        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "处理用户文件上传应答信息 JSON数据时发生异常:" << e.what();
        return false;
    }
    return true;
}
bool DeSheng::ProcessUserFeedbackResult(UserFeedbackResponse &responseData,
                                        QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.ticket_no.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "用户反馈应答信息 JSON文档为空";
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
            if (tmp_dataObj.contains("ticket_no") && tmp_dataObj["ticket_no"].isString()) {
                responseData.data.ticket_no = tmp_dataObj["ticket_no"].toString();
            } else {
                qDebug() << "data 内 ticket_no 数据不存在 或 类型异常";
                return false;
            }

        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "处理用户反馈应答信息 JSON数据时发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::userFeedbackRequestToJson(const UserFeedBacksRequest &req)
{
    QJsonObject obj;
    obj["device_id"] = req.device_id;
    obj["drive_id"] = req.drive_id;
    obj["firmware_id"] = req.firmware_id;
    obj["drive_version"] = req.drive_version;
    obj["firmware_version"] = req.firmware_version;
    if (!req.receiver_version.isEmpty())
        obj["receiver_version"] = req.receiver_version;
    obj["device_name"] = req.device_name;
    obj["device_type"] = req.device_type;
    obj["title"] = req.title;
    obj["description"] = req.description;

    QJsonArray imagesArray;
    for (const QString &img : req.images)
        imagesArray.append(img);
    obj["images"] = imagesArray;

    obj["type"] = req.type;
    obj["contact_info"] = req.contact_info;
    obj["os_info"] = req.os_info;
    obj["config_url"] = req.config_url;
    return obj;
}

bool DeSheng::buildFeedbackQuery(const UserFeedBacksRequest &req, QUrlQuery &query, QString &error)
{
    // 1. 校验所有必填字段
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.device_id, "device_id", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.drive_id, "drive_id", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.firmware_id, "firmware_id", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.drive_version, "drive_version", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.firmware_version, "firmware_version", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.device_name, "device_name", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.device_type, "device_type", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.title, "title", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.description, "description", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.type, "type", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.contact_info, "contact_info", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.os_info, "os_info", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.config_url, "config_url", error))
        return false;

    // 2. 校验 device_type 的取值
    if (req.device_type != "mouse" && req.device_type != "keyboard"
        && req.device_type != "headset") {
        error = "device_type 必须是 mouse, keyboard 或 headset";
        return false;
    }

    // 3. 校验 type 的取值
    if (req.type != "bug" && req.type != "feature" && req.type != "other") {
        error = "type 必须是 bug, feature 或 other";
        return false;
    }

    // 4. 校验 images 数量（最多3张）
    if (req.images.size() > 3) {
        error = "images 最多支持3张图片";
        return false;
    }

    // 5.开始构建 query
    query.addQueryItem("device_id", req.device_id);
    query.addQueryItem("drive_id", req.drive_id);
    query.addQueryItem("firmware_id", req.firmware_id);
    query.addQueryItem("drive_version", req.drive_version);
    query.addQueryItem("firmware_version", req.firmware_version);
    query.addQueryItem("device_name", req.device_name);
    query.addQueryItem("device_type", req.device_type);
    query.addQueryItem("title", req.title);
    query.addQueryItem("description", req.description);
    query.addQueryItem("type", req.type);
    query.addQueryItem("contact_info", req.contact_info);
    query.addQueryItem("os_info", req.os_info);
    query.addQueryItem("config_url", req.config_url);

    // 可选字段：receiver_version
    if (!req.receiver_version.isEmpty()) {
        query.addQueryItem("receiver_version", req.receiver_version);
    }

    // 可选字段：images（允许多个同名参数）
    for (const QString &img : req.images) {
        if (!img.isEmpty()) {
            query.addQueryItem("images", img);
        }
    }

    return true;
}


bool DeSheng::ProcessGetFeedbackListResult(GetFeedbackListResponse &responseData,
                                           QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.list.clear();
    responseData.data.total = 0;
    responseData.data.page = 0;

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "反馈列表 应答信息 JSON文档为空";
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
                    GetFeedbackListResponse::ListItem item;
                    item.id = itemObj.value("id").toInt();
                    item.ticket_no = itemObj.value("ticket_no").toString();
                    item.title = itemObj.value("title").toString();
                    item.type = itemObj.value("type").toString();
                    item.status = itemObj.value("status").toString();
                    item.device_type = itemObj.value("device_type").toString();
                    item.created_at = itemObj.value("created_at").toString();
                    responseData.data.list.append(item);
                }
            }

            responseData.data.total = dataObj.value("total").toInt();
            responseData.data.page = dataObj.value("page").toInt();
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "反馈列表 解析异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::GetFeedbackListRequestToJson(const GetFeedbackListRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildGetFeedbackListQuery(const GetFeedbackListRequest &req,
                                         QUrlQuery &query,
                                         QString &error)
{
    Q_UNUSED(error);
    if (req.page != 1)
        query.addQueryItem("page", QString::number(req.page));
    if (req.page_size != 10)
        query.addQueryItem("page_size", QString::number(req.page_size));
    return true;
}

bool DeSheng::ProcessGetFeedbackDetailResult(GetFeedbackDetailResponse &responseData,
                                             QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "反馈详情 应答信息 JSON文档为空";
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
            responseData.data.id = dataObj.value("id").toInt();
            responseData.data.ticket_no = dataObj.value("ticket_no").toString();
            responseData.data.title = dataObj.value("title").toString();
            responseData.data.description = dataObj.value("description").toString();
            responseData.data.type = dataObj.value("type").toString();
            responseData.data.status = dataObj.value("status").toString();
            responseData.data.device_id = dataObj.value("device_id").toString();
            responseData.data.drive_id = dataObj.value("drive_id").toString();
            responseData.data.firmware_id = dataObj.value("firmware_id").toString();
            responseData.data.drive_version = dataObj.value("drive_version").toString();
            responseData.data.firmware_version = dataObj.value("firmware_version").toString();
            responseData.data.receiver_version = dataObj.value("receiver_version").toString();
            responseData.data.device_name = dataObj.value("device_name").toString();
            responseData.data.device_type = dataObj.value("device_type").toString();
            responseData.data.created_at = dataObj.value("created_at").toString();
            responseData.data.updated_at = dataObj.value("updated_at").toString();

            if (dataObj.contains("images") && dataObj["images"].isArray()) {
                QJsonArray imagesArr = dataObj["images"].toArray();
                for (const QJsonValue &val : imagesArr) {
                    responseData.data.images.append(val.toString());
                }
            }
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "反馈详情 解析异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::GetFeedbackDetailRequestToJson(const GetFeedbackDetailRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildGetFeedbackDetailQuery(const GetFeedbackDetailRequest &req,
                                           QUrlQuery &query,
                                           QString &error)
{
    Q_UNUSED(query);
    if (req.ticket_no.isEmpty()) {
        error = "工单号 不能为空";
        return false;
    }
    return true;
}

// ============================ 管理端-反馈管理 ============================

bool DeSheng::ProcessAdminFeedbackListResult(AdminFeedbackListResponse &responseData,
                                              QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.list.clear();
    responseData.data.total = 0;
    responseData.data.page = 0;

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端反馈列表 应答信息 JSON文档为空";
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
                    AdminFeedbackListResponse::ListItem item;
                    item.id = itemObj.value("id").toInt();
                    item.ticket_no = itemObj.value("ticket_no").toString();
                    item.title = itemObj.value("title").toString();
                    item.description = itemObj.value("description").toString();
                    item.type = itemObj.value("type").toString();
                    item.status = itemObj.value("status").toString();
                    item.user_id = itemObj.value("user_id").toString();
                    item.device_name = itemObj.value("device_name").toString();
                    item.device_type = itemObj.value("device_type").toString();
                    item.drive_version = itemObj.value("drive_version").toString();
                    item.firmware_version = itemObj.value("firmware_version").toString();
                    item.receiver_version = itemObj.value("receiver_version").toString();
                    item.contact_info = itemObj.value("contact_info").toString();
                    item.os_info = itemObj.value("os_info").toString();
                    item.created_at = itemObj.value("created_at").toString();
                    item.updated_at = itemObj.value("updated_at").toString();

                    if (itemObj.contains("images") && itemObj["images"].isArray()) {
                        QJsonArray imagesArr = itemObj["images"].toArray();
                        for (const QJsonValue &imgVal : imagesArr)
                            item.images.append(imgVal.toString());
                    }
                    responseData.data.list.append(item);
                }
            }

            responseData.data.total = dataObj.value("total").toInt();
            responseData.data.page = dataObj.value("page").toInt();
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端反馈列表 解析异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminFeedbackListRequestToJson(const AdminFeedbackListRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildAdminFeedbackListQuery(const AdminFeedbackListRequest &req,
                                           QUrlQuery &query,
                                           QString &error)
{
    Q_UNUSED(error);
    if (req.page != 1)
        query.addQueryItem("page", QString::number(req.page));
    if (req.page_size != 10)
        query.addQueryItem("page_size", QString::number(req.page_size));
    if (!req.status.isEmpty())
        query.addQueryItem("status", req.status);
    if (!req.type.isEmpty())
        query.addQueryItem("type", req.type);
    if (!req.keyword.isEmpty())
        query.addQueryItem("keyword", req.keyword);
    return true;
}

bool DeSheng::ProcessAdminFeedbackDetailResult(AdminFeedbackDetailResponse &responseData,
                                                QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端反馈详情 应答信息 JSON文档为空";
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
            responseData.data.id = dataObj.value("id").toInt();
            responseData.data.ticket_no = dataObj.value("ticket_no").toString();
            responseData.data.user_id = dataObj.value("user_id").toString();
            responseData.data.title = dataObj.value("title").toString();
            responseData.data.description = dataObj.value("description").toString();
            responseData.data.type = dataObj.value("type").toString();
            responseData.data.status = dataObj.value("status").toString();
            responseData.data.device_id = dataObj.value("device_id").toString();
            responseData.data.drive_id = dataObj.value("drive_id").toString();
            responseData.data.firmware_id = dataObj.value("firmware_id").toString();
            responseData.data.drive_version = dataObj.value("drive_version").toString();
            responseData.data.firmware_version = dataObj.value("firmware_version").toString();
            responseData.data.receiver_version = dataObj.value("receiver_version").toString();
            responseData.data.device_name = dataObj.value("device_name").toString();
            responseData.data.device_type = dataObj.value("device_type").toString();
            responseData.data.contact_info = dataObj.value("contact_info").toString();
            responseData.data.login_ip = dataObj.value("login_ip").toString();
            responseData.data.os_info = dataObj.value("os_info").toString();
            responseData.data.config_url = dataObj.value("config_url").toString();
            responseData.data.created_at = dataObj.value("created_at").toString();
            responseData.data.updated_at = dataObj.value("updated_at").toString();
            responseData.data.resolved_at = dataObj.value("resolved_at").toString();

            if (dataObj.contains("images") && dataObj["images"].isArray()) {
                QJsonArray imagesArr = dataObj["images"].toArray();
                for (const QJsonValue &val : imagesArr)
                    responseData.data.images.append(val.toString());
            }
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端反馈详情 解析异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminFeedbackDetailRequestToJson(const QString &ticket_no)
{
    Q_UNUSED(ticket_no);
    return QJsonObject();
}

bool DeSheng::ProcessAdminFeedbackStatusResult(AdminFeedbackStatusResponse &responseData,
                                                QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "更新反馈状态 应答信息 JSON文档为空";
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
        qDebug() << __FILE__ << __FUNCTION__ << "更新反馈状态 解析异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminFeedbackStatusRequestToJson(const AdminFeedbackStatusRequest &req)
{
    QJsonObject obj;
    obj["status"] = req.status;
    return obj;
}

bool DeSheng::buildAdminFeedbackStatusQuery(const AdminFeedbackStatusRequest &req,
                                             QUrlQuery &query,
                                             QString &error)
{
    Q_UNUSED(query);
    if (req.ticket_no.isEmpty()) {
        error = "工单号 不能为空";
        return false;
    }
    if (req.status.isEmpty()) {
        error = "反馈状态 不能为空";
        return false;
    }
    if (req.status != "pending" && req.status != "processing"
        && req.status != "resolved" && req.status != "closed") {
        error = "status 必须是 pending, processing, resolved 或 closed";
        return false;
    }
    return true;
}
