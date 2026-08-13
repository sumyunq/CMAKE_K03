#include "data/ad/ad_api.h"
#include "data/api_global.h"


#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>



bool DeSheng::ProcessAdvertisementsListResult(AdvertisementsListResponse &responseData,
                                              const QJsonDocument &jsonDocument)
{
    // 清空旧数据
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.list.clear();
    responseData.data.total = 0;
    responseData.data.page = 0;
    responseData.data.page_size = 0;

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "广告列表 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        // 解析 code
        if (!root.contains("code") || !root["code"].isString()) {
            qDebug() << "code 不存在或类型异常";
            return false;
        }
        responseData.code = root["code"].toString();

        // 解析 message
        if (!root.contains("message") || !root["message"].isString()) {
            qDebug() << "message 不存在或类型异常";
            return false;
        }
        responseData.message = root["message"].toString();

        // 解析 data
        if (!root.contains("data") || !root["data"].isObject()) {
            qDebug() << "data 不存在或类型异常";
            return false;
        }
        QJsonObject dataObj = root["data"].toObject();

        // 解析 list 数组
        if (!dataObj.contains("list") || !dataObj["list"].isArray()) {
            qDebug() << "data.list 不存在或类型异常";
            return false;
        }
        QJsonArray listArr = dataObj["list"].toArray();
        for (const QJsonValue &val : listArr) {
            if (!val.isObject()) continue;
            QJsonObject itemObj = val.toObject();

            AdvertisementItem item;
            item.id = itemObj["id"].toInt();
            item.title = itemObj["title"].toString();
            item.scene = itemObj["scene"].toString();
            item.media_type = itemObj["media_type"].toString();
            item.img_url = itemObj["img_url"].toString();
            item.video_url = itemObj["video_url"].toString();
            item.jump_url = itemObj["jump_url"].toString();
            item.device_name = itemObj["device_name"].toString();
            item.device_type = itemObj["device_type"].toString();
            item.sort_order = itemObj["sort_order"].toInt();
            item.start_time = itemObj["start_time"].toString();
            item.end_time = itemObj["end_time"].toString();

            responseData.data.list.append(item);
        }

        // 解析分页信息
        responseData.data.total = dataObj["total"].toInt();
        responseData.data.page = dataObj["page"].toInt();
        responseData.data.page_size = dataObj["page_size"].toInt();

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析广告列表异常:" << e.what();
        return false;
    }
    return true;
}

/// 将获取广告列表请求结构体转换为JSON对象（GET请求无Body）
QJsonObject DeSheng::AdvertisementsListRequestToJson(const AdvertisementsListRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

/// 构建获取广告列表接口的URL查询参数
bool DeSheng::buildAdvertisementsListQuery(const AdvertisementsListRequest &req,
                                           QUrlQuery &query,
                                           QString &error)
{
    // 校验必填字段
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.scene, "scene", error))
        return false;

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

/// 处理记录广告点击接口的响应结果
bool DeSheng::ProcessAdClickResult(AdClickResponse &responseData,
                                   const QJsonDocument &jsonDocument)
{
    // 清空旧数据
    responseData.code.clear();
    responseData.message.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "广告点击 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        // 解析 code
        if (!root.contains("code") || !root["code"].isString()) {
            qDebug() << "code 不存在或类型异常";
            return false;
        }
        responseData.code = root["code"].toString();

        // 解析 message
        if (!root.contains("message") || !root["message"].isString()) {
            qDebug() << "message 不存在或类型异常";
            return false;
        }
        responseData.message = root["message"].toString();

        // data 为 null，无需解析

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析广告点击异常:" << e.what();
        return false;
    }
    return true;
}

/// 将记录广告点击请求结构体转换为JSON对象（POST请求无Body）
QJsonObject DeSheng::AdClickRequestToJson(const AdClickRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

/// 构建记录广告点击接口的URL查询参数（无Query参数，仅校验ID）
bool DeSheng::buildAdClickQuery(const AdClickRequest &req,
                                QUrlQuery &query,
                                QString &error)
{
    Q_UNUSED(query);

    if (req.id <= 0) {
        error = "广告 ID 不能为空或小于等于 0";
        return false;
    }
    return true;
}


/*************************************************************************************  管理端广告实现  ************************************************************************************************/

/// 从 JSON 对象解析单个管理端广告条目
static void parseAdminAdItem(const QJsonObject &obj, DeSheng::AdminAdItem &item)
{
    item.id = obj["id"].toInt();
    item.title = obj["title"].toString();
    item.scene = obj["scene"].toString();
    item.media_type = obj["media_type"].toString();
    item.img_url = obj["img_url"].toString();
    item.video_url = obj["video_url"].toString();
    item.jump_url = obj["jump_url"].toString();
    item.device_name = obj["device_name"].toString();
    item.device_type = obj["device_type"].toString();
    item.sort_order = obj["sort_order"].toInt();
    item.status = obj["status"].toString();
    item.click_count = obj["click_count"].toInt();
    item.start_time = obj["start_time"].toString();
    item.end_time = obj["end_time"].toString();
    item.created_at = obj["created_at"].toString();
}

// ---- 管理端广告列表 ----

bool DeSheng::ProcessAdminAdListResult(AdminAdListResponse &responseData,
                                       const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.list.clear();
    responseData.data.total = 0;
    responseData.data.page = 0;
    responseData.data.page_size = 0;

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端广告列表 应答信息 JSON文档为空";
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
            AdminAdItem item;
            parseAdminAdItem(val.toObject(), item);
            responseData.data.list.append(item);
        }

        responseData.data.total = dataObj["total"].toInt();
        responseData.data.page = dataObj["page"].toInt();
        responseData.data.page_size = dataObj["page_size"].toInt();

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析管理端广告列表异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminAdListRequestToJson(const AdminAdListRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildAdminAdListQuery(const AdminAdListRequest &req,
                                    QUrlQuery &query,
                                    QString &error)
{
    // scene 必填
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.scene, "scene", error))
        return false;

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
    query.addQueryItem("scene", req.scene);
    if (!req.status.isEmpty())
        query.addQueryItem("status", req.status);
    if (!req.device_type.isEmpty())
        query.addQueryItem("device_type", req.device_type);
    if (!req.device_name.isEmpty())
        query.addQueryItem("device_name", req.device_name);
    if (!req.media_type.isEmpty())
        query.addQueryItem("media_type", req.media_type);
    if (!req.keyword.isEmpty())
        query.addQueryItem("keyword", req.keyword);
    if (!req.sort_by.isEmpty())
        query.addQueryItem("sort_by", req.sort_by);
    if (!req.order_by.isEmpty())
        query.addQueryItem("order_by", req.order_by);
    if (req.page != 1)
        query.addQueryItem("page", QString::number(req.page));
    if (req.page_size != 10)
        query.addQueryItem("page_size", QString::number(req.page_size));

    return true;
}

// ---- 管理端广告详情 ----

bool DeSheng::ProcessAdminAdDetailResult(AdminAdDetailResponse &responseData,
                                         const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端广告详情 应答信息 JSON文档为空";
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
            parseAdminAdItem(root["data"].toObject(), responseData.data);
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析管理端广告详情异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminAdDetailRequestToJson(const AdminAdDetailRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildAdminAdDetailQuery(const AdminAdDetailRequest &req,
                                      QUrlQuery &query,
                                      QString &error)
{
    Q_UNUSED(query);
    if (req.id <= 0) {
        error = "广告 ID 不能为空或小于等于 0";
        return false;
    }
    return true;
}

// ---- 管理端创建广告 ----

bool DeSheng::ProcessAdminAdCreateResult(AdminAdCreateResponse &responseData,
                                         const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端创建广告 应答信息 JSON文档为空";
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
            parseAdminAdItem(root["data"].toObject(), responseData.data);
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析管理端创建广告异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminAdCreateRequestToJson(const AdminAdCreateRequest &req)
{
    QJsonObject obj;
    obj["title"] = req.title;
    obj["scene"] = req.scene;
    if (!req.media_type.isEmpty())
        obj["media_type"] = req.media_type;
    if (!req.img_url.isEmpty())
        obj["img_url"] = req.img_url;
    if (!req.video_url.isEmpty())
        obj["video_url"] = req.video_url;
    obj["jump_url"] = req.jump_url;
    if (!req.device_name.isEmpty())
        obj["device_name"] = req.device_name;
    if (!req.device_type.isEmpty())
        obj["device_type"] = req.device_type;
    obj["sort_order"] = req.sort_order;
    if (!req.status.isEmpty())
        obj["status"] = req.status;
    if (!req.start_time.isEmpty())
        obj["start_time"] = req.start_time;
    if (!req.end_time.isEmpty())
        obj["end_time"] = req.end_time;
    return obj;
}

bool DeSheng::buildAdminAdCreateQuery(const AdminAdCreateRequest &req,
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
    // jump_url 必填
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.jump_url, "jump_url", error))
        return false;
    return true;
}

// ---- 管理端更新广告 ----

bool DeSheng::ProcessAdminAdUpdateResult(AdminAdUpdateResponse &responseData,
                                         const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端更新广告 应答信息 JSON文档为空";
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
            parseAdminAdItem(root["data"].toObject(), responseData.data);
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析管理端更新广告异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminAdUpdateRequestToJson(const AdminAdUpdateRequest &req)
{
    QJsonObject obj;
    // id 为路径参数，不放进 body
    if (!req.title.isEmpty())
        obj["title"] = req.title;
    if (!req.scene.isEmpty())
        obj["scene"] = req.scene;
    if (!req.media_type.isEmpty())
        obj["media_type"] = req.media_type;
    if (!req.img_url.isEmpty())
        obj["img_url"] = req.img_url;
    if (!req.video_url.isEmpty())
        obj["video_url"] = req.video_url;
    if (!req.jump_url.isEmpty())
        obj["jump_url"] = req.jump_url;
    if (!req.device_name.isEmpty())
        obj["device_name"] = req.device_name;
    if (!req.device_type.isEmpty())
        obj["device_type"] = req.device_type;
    obj["sort_order"] = req.sort_order;
    if (!req.status.isEmpty())
        obj["status"] = req.status;
    if (!req.start_time.isEmpty())
        obj["start_time"] = req.start_time;
    if (!req.end_time.isEmpty())
        obj["end_time"] = req.end_time;
    return obj;
}

bool DeSheng::buildAdminAdUpdateQuery(const AdminAdUpdateRequest &req,
                                      QUrlQuery &query,
                                      QString &error)
{
    Q_UNUSED(query);
    if (req.id <= 0) {
        error = "广告 ID 不能为空或小于等于 0";
        return false;
    }
    return true;
}

// ---- 管理端删除广告 ----

bool DeSheng::ProcessAdminAdDeleteResult(AdminAdDeleteResponse &responseData,
                                         const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端删除广告 应答信息 JSON文档为空";
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
        qDebug() << __FILE__ << __FUNCTION__ << "解析管理端删除广告异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminAdDeleteRequestToJson(const AdminAdDeleteRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildAdminAdDeleteQuery(const AdminAdDeleteRequest &req,
                                      QUrlQuery &query,
                                      QString &error)
{
    Q_UNUSED(query);
    if (req.id <= 0) {
        error = "广告 ID 不能为空或小于等于 0";
        return false;
    }
    return true;
}