#include "data/audition/audition_api.h"
#include "data/api_global.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

bool DeSheng::ProcessAuditionsListResult(AuditionsListResponse &responseData,
                                         QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.list.clear();
    responseData.total = 0;
    responseData.page = 0;
    responseData.page_size = 0;

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "试听视频列表 应答信息 JSON文档为空";
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

        if (tmp_jsonObj.contains("data") && tmp_jsonObj["data"].isObject()) {
            QJsonObject tmp_dataObj = tmp_jsonObj["data"].toObject();

            // list 数组
            if (tmp_dataObj.contains("list") && tmp_dataObj["list"].isArray()) {
                QJsonArray listArr = tmp_dataObj["list"].toArray();
                for (const QJsonValue &val : listArr) {
                    if (val.isObject()) {
                        responseData.data.list.append(VideoItem::fromJson(val.toObject()));
                    }
                }
            } else {
                qDebug() << "data 内 list 数据不存在 或 类型异常";
                return false;
            }

            // total / page / page_size（可为 0，非必失败）
            if (tmp_dataObj.contains("total"))
                responseData.total = tmp_dataObj["total"].toInt();
            if (tmp_dataObj.contains("page"))
                responseData.page = tmp_dataObj["page"].toInt();
            if (tmp_dataObj.contains("page_size"))
                responseData.page_size = tmp_dataObj["page_size"].toInt();

        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "试听视频列表 应答信息解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AuditionsListRequestToJson(const AuditionsListRequest &req)
{
    QJsonObject obj;
    obj["scene"] = req.scene;
    if (!req.device_type.isEmpty())
        obj["device_type"] = req.device_type;
    if (!req.device_name.isEmpty())
        obj["device_name"] = req.device_name;
    if (req.page != 1)
        obj["page"] = req.page;
    if (req.page_size != 10)
        obj["page_size"] = req.page_size;
    return obj;
}

bool DeSheng::buildAuditionsListQuery(const AuditionsListRequest &req,
                                      QUrlQuery &query,
                                      QString &error)
{
    // scene 必填
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.scene, "scene", error))
        return false;

    // 校验 page / page_size 范围
    if (req.page < 1) {
        error = "page 必须 >= 1";
        return false;
    }
    if (req.page_size < 1 || req.page_size > 100) {
        error = "page_size 必须在 1~100 之间";
        return false;
    }

    // 构建 query
    query.addQueryItem("scene", req.scene);
    if (!req.device_type.isEmpty())
        query.addQueryItem("device_type", req.device_type);

    if (!req.device_name.isEmpty())
        query.addQueryItem("device_name", req.device_name);
    if (req.page != 1)
        query.addQueryItem("page", QString::number(req.page));
    if (req.page_size != 10)
        query.addQueryItem("page_size", QString::number(req.page_size));

    return true;
}

bool DeSheng::ProcessGetAuditionDetailResult(GetAuditionDetailResponse &responseData,
                                             QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "试听详情 应答信息 JSON文档为空";
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
            responseData.data.title = dataObj.value("title").toString();
            responseData.data.scene = dataObj.value("scene").toString();
            responseData.data.scene_name = dataObj.value("scene_name").toString();
            responseData.data.video_desc = dataObj.value("video_desc").toString();
            responseData.data.img_url = dataObj.value("img_url").toString();
            responseData.data.video_url = dataObj.value("video_url").toString();
            responseData.data.device_name = dataObj.value("device_name").toString();
            responseData.data.device_type = dataObj.value("device_type").toString();
            responseData.data.status = dataObj.value("status").toString();
            responseData.data.created_at = dataObj.value("created_at").toString();
            responseData.data.updated_at = dataObj.value("updated_at").toString();
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "试听详情 解析异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::GetAuditionDetailRequestToJson(const GetAuditionDetailRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildGetAuditionDetailQuery(const GetAuditionDetailRequest &req,
                                           QUrlQuery &query,
                                           QString &error)
{
    Q_UNUSED(query);
    if (req.id <= 0) {
        error = "试听 ID 不能为空或小于等于 0";
        return false;
    }
    return true;
}


/*************************************************************************************  管理端试听实现  ************************************************************************************************/

/// 从 JSON 对象解析单个管理端试听条目
static void parseAdminAuditionItem(const QJsonObject &obj, DeSheng::AdminAuditionItem &item)
{
    item.id = obj["id"].toInt();
    item.title = obj["title"].toString();
    item.scene = obj["scene"].toString();
    item.scene_name = obj["scene_name"].toString();
    item.video_desc = obj["video_desc"].toString();
    item.img_url = obj["img_url"].toString();
    item.video_url = obj["video_url"].toString();
    item.device_name = obj["device_name"].toString();
    item.device_type = obj["device_type"].toString();
    item.status = obj["status"].toString();
    item.created_at = obj["created_at"].toString();
    item.updated_at = obj["updated_at"].toString();
}

// ---- 管理端试听列表 ----

bool DeSheng::ProcessAdminAuditionListResult(AdminAuditionListResponse &responseData,
                                             const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.list.clear();
    responseData.data.total = 0;
    responseData.data.page = 0;
    responseData.data.page_size = 0;

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端试听列表 应答信息 JSON文档为空";
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

        if (!root.contains("data") || !root["data"].isObject()) {
            qDebug() << "data 不存在或类型异常";
            return false;
        }
        QJsonObject dataObj = root["data"].toObject();

        if (!dataObj.contains("list") || !dataObj["list"].isArray()) {
            qDebug() << "data.list 不存在或类型异常";
            return false;
        }
        QJsonArray listArr = dataObj["list"].toArray();
        for (const QJsonValue &val : listArr) {
            if (!val.isObject()) continue;
            AdminAuditionItem item;
            parseAdminAuditionItem(val.toObject(), item);
            responseData.data.list.append(item);
        }

        responseData.data.total = dataObj["total"].toInt();
        responseData.data.page = dataObj["page"].toInt();
        responseData.data.page_size = dataObj["page_size"].toInt();

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析管理端试听列表异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminAuditionListRequestToJson(const AdminAuditionListRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildAdminAuditionListQuery(const AdminAuditionListRequest &req,
                                          QUrlQuery &query,
                                          QString &error)
{
    // 所有参数均为可选

    // 校验分页范围
    if (req.page < 1) {
        error = "page 必须 >= 1";
        return false;
    }
    if (req.page_size < 1 || req.page_size > 100) {
        error = "page_size 必须在 1~100 之间";
        return false;
    }

    // 构建 query
    if (!req.scene.isEmpty())
        query.addQueryItem("scene", req.scene);
    if (!req.status.isEmpty())
        query.addQueryItem("status", req.status);
    if (!req.device_type.isEmpty())
        query.addQueryItem("device_type", req.device_type);
    if (!req.device_name.isEmpty())
        query.addQueryItem("device_name", req.device_name);
    if (!req.keyword.isEmpty())
        query.addQueryItem("keyword", req.keyword);
    if (req.page != 1)
        query.addQueryItem("page", QString::number(req.page));
    if (req.page_size != 10)
        query.addQueryItem("page_size", QString::number(req.page_size));

    return true;
}

// ---- 管理端试听详情 ----

bool DeSheng::ProcessAdminAuditionDetailResult(AdminAuditionDetailResponse &responseData,
                                               const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端试听详情 应答信息 JSON文档为空";
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
            parseAdminAuditionItem(root["data"].toObject(), responseData.data);
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析管理端试听详情异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminAuditionDetailRequestToJson(const AdminAuditionDetailRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildAdminAuditionDetailQuery(const AdminAuditionDetailRequest &req,
                                            QUrlQuery &query,
                                            QString &error)
{
    Q_UNUSED(query);
    if (req.id <= 0) {
        error = "试听 ID 不能为空或小于等于 0";
        return false;
    }
    return true;
}

// ---- 管理端创建试听 ----

bool DeSheng::ProcessAdminAuditionCreateResult(AdminAuditionCreateResponse &responseData,
                                               const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端创建试听 应答信息 JSON文档为空";
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
            parseAdminAuditionItem(root["data"].toObject(), responseData.data);
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析管理端创建试听异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminAuditionCreateRequestToJson(const AdminAuditionCreateRequest &req)
{
    QJsonObject obj;
    obj["title"] = req.title;
    obj["scene"] = req.scene;
    obj["scene_name"] = req.scene_name;
    if (!req.video_desc.isEmpty())
        obj["video_desc"] = req.video_desc;
    obj["img_url"] = req.img_url;
    obj["video_url"] = req.video_url;
    if (!req.device_name.isEmpty())
        obj["device_name"] = req.device_name;
    if (!req.device_type.isEmpty())
        obj["device_type"] = req.device_type;
    if (!req.status.isEmpty())
        obj["status"] = req.status;
    return obj;
}

bool DeSheng::buildAdminAuditionCreateQuery(const AdminAuditionCreateRequest &req,
                                            QUrlQuery &query,
                                            QString &error)
{
    Q_UNUSED(query);
    // title 必填
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.title, "title", error))
        return false;
    // scene 必填
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.scene, "scene", error))
        return false;
    // scene_name 必填
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.scene_name, "scene_name", error))
        return false;
    // img_url 必填
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.img_url, "img_url", error))
        return false;
    // video_url 必填
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.video_url, "video_url", error))
        return false;
    return true;
}

// ---- 管理端更新试听 ----

bool DeSheng::ProcessAdminAuditionUpdateResult(AdminAuditionUpdateResponse &responseData,
                                               const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端更新试听 应答信息 JSON文档为空";
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
            parseAdminAuditionItem(root["data"].toObject(), responseData.data);
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析管理端更新试听异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminAuditionUpdateRequestToJson(const AdminAuditionUpdateRequest &req)
{
    QJsonObject obj;
    // id 为路径参数，不放进 body
    if (!req.title.isEmpty())
        obj["title"] = req.title;
    if (!req.scene.isEmpty())
        obj["scene"] = req.scene;
    if (!req.scene_name.isEmpty())
        obj["scene_name"] = req.scene_name;
    // video_desc 允许传空字符串清空，故无条件写入
    obj["video_desc"] = req.video_desc;
    if (!req.img_url.isEmpty())
        obj["img_url"] = req.img_url;
    if (!req.video_url.isEmpty())
        obj["video_url"] = req.video_url;
    if (!req.device_name.isEmpty())
        obj["device_name"] = req.device_name;
    if (!req.device_type.isEmpty())
        obj["device_type"] = req.device_type;
    if (!req.status.isEmpty())
        obj["status"] = req.status;
    return obj;
}

bool DeSheng::buildAdminAuditionUpdateQuery(const AdminAuditionUpdateRequest &req,
                                            QUrlQuery &query,
                                            QString &error)
{
    Q_UNUSED(query);
    if (req.id <= 0) {
        error = "试听 ID 不能为空或小于等于 0";
        return false;
    }
    return true;
}

// ---- 管理端删除试听 ----

bool DeSheng::ProcessAdminAuditionDeleteResult(AdminAuditionDeleteResponse &responseData,
                                               const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端删除试听 应答信息 JSON文档为空";
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
        qDebug() << __FILE__ << __FUNCTION__ << "解析管理端删除试听异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminAuditionDeleteRequestToJson(const AdminAuditionDeleteRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildAdminAuditionDeleteQuery(const AdminAuditionDeleteRequest &req,
                                            QUrlQuery &query,
                                            QString &error)
{
    Q_UNUSED(query);
    if (req.id <= 0) {
        error = "试听 ID 不能为空或小于等于 0";
        return false;
    }
    return true;
}
