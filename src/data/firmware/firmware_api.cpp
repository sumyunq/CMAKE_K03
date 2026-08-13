#include "data/firmware/firmware_api.h"
#include "data/api_global.h"


#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>


bool DeSheng::ProcessFirmwareInfoResult(FirmwareInfoResponse &responseData,
                                         const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "固件信息 应答 JSON文档为空";
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

            responseData.data.id = tmp_dataObj.value("id").toString();
            responseData.data.device_id = tmp_dataObj.value("device_id").toString();
            responseData.data.device_name = tmp_dataObj.value("device_name").toString();
            responseData.data.version = tmp_dataObj.value("version").toString();
            responseData.data.release_date = tmp_dataObj.value("release_date").toString();
            responseData.data.download_url = tmp_dataObj.value("download_url").toString();
            responseData.data.type = tmp_dataObj.value("type").toString();
            responseData.data.status = tmp_dataObj.value("status").toString();
            responseData.data.is_latest = tmp_dataObj.value("is_latest").toBool();
            responseData.data.created_at = tmp_dataObj.value("created_at").toString();

            // download_sources 数组
            if (tmp_dataObj.contains("download_sources") && tmp_dataObj["download_sources"].isArray()) {
                QJsonArray arr = tmp_dataObj["download_sources"].toArray();
                for (const QJsonValue &val : arr) {
                    if (!val.isObject())
                        continue;
                    QJsonObject srcObj = val.toObject();
                    FirmwareInfoResponse::DownloadSource ds;
                    ds.source = srcObj.value("source").toString();
                    ds.type = srcObj.value("type").toString();
                    ds.url = srcObj.value("url").toString();
                    responseData.data.download_sources.append(ds);
                }
            }

        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "固件信息 应答解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::FirmwareInfoRequestToJson(const FirmwareInfoRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildFirmwareInfoQuery(const FirmwareInfoRequest &req,
                                      QUrlQuery &query,
                                      QString &error)
{
    Q_UNUSED(error);

    if (!req.device_id.isEmpty())
        query.addQueryItem("device_id", req.device_id);
    if (!req.device_name.isEmpty())
        query.addQueryItem("device_name", req.device_name);
    if (!req.version.isEmpty())
        query.addQueryItem("version", req.version);
    if (!req.status.isEmpty())
        query.addQueryItem("status", req.status);

    return true;
}

/*************************************************************************************  固件后台管理 实现  ************************************************************************************************/

/// 处理创建固件应答
bool DeSheng::ProcessAdminFirmwareCreateResult(AdminFirmwareCreateResponse &responseData,
                                                const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "创建固件 应答 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject tmp_jsonObj = jsonDocument.object();

        // code — 兼容数字 200 与字符串
        responseData.code = tmp_jsonObj.value("code").toString();
        if (responseData.code.isEmpty() && tmp_jsonObj.value("code").isDouble()) {
            responseData.code = QString::number(static_cast<int>(tmp_jsonObj.value("code").toDouble()));
        }
        if (responseData.code.isEmpty()) {
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

            responseData.data.id            = tmp_dataObj.value("id").toString();
            responseData.data.version       = tmp_dataObj.value("version").toString();
            responseData.data.file_name     = tmp_dataObj.value("file_name").toString();
            responseData.data.release_date  = tmp_dataObj.value("release_date").toString();
            responseData.data.release_notes = tmp_dataObj.value("release_notes").toString();
            responseData.data.download_url  = tmp_dataObj.value("download_url").toString();
            responseData.data.downloads     = tmp_dataObj.value("downloads").toInt();
            responseData.data.size          = static_cast<qint64>(tmp_dataObj.value("size").toDouble());
            responseData.data.type          = tmp_dataObj.value("type").toString();
            responseData.data.device_id     = tmp_dataObj.value("device_id").toString();
            responseData.data.device_name   = tmp_dataObj.value("device_name").toString();
            responseData.data.device_type   = tmp_dataObj.value("device_type").toString();
            responseData.data.status        = tmp_dataObj.value("status").toString();
            responseData.data.created_at    = tmp_dataObj.value("created_at").toString();
            responseData.data.updated_at    = tmp_dataObj.value("updated_at").toString();

            // download_sources
            if (tmp_dataObj.contains("download_sources") && tmp_dataObj["download_sources"].isArray()) {
                QJsonArray arr = tmp_dataObj["download_sources"].toArray();
                for (const QJsonValue &val : arr) {
                    if (!val.isObject())
                        continue;
                    QJsonObject srcObj = val.toObject();
                    AdminFirmwareDownloadSource ds;
                    ds.source = srcObj.value("source").toString();
                    ds.type   = srcObj.value("type").toString();
                    ds.url    = srcObj.value("url").toString();
                    responseData.data.download_sources.append(ds);
                }
            }

        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "创建固件 应答解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

/// 将创建固件请求转换为 JSON
QJsonObject DeSheng::AdminFirmwareCreateRequestToJson(const AdminFirmwareCreateRequest &req)
{
    QJsonObject obj;
    if (!req.device_id.isEmpty())
        obj["device_id"] = req.device_id;
    if (!req.device_name.isEmpty())
        obj["device_name"] = req.device_name;
    obj["version"]      = req.version;
    if (!req.file_name.isEmpty())
        obj["file_name"] = req.file_name;
    obj["release_date"] = req.release_date;
    obj["release_notes"] = req.release_notes;
    obj["download_url"] = req.download_url;
    obj["type"]         = req.type;
    if (req.size > 0)
        obj["size"] = req.size;
    if (!req.download_sources.isEmpty()) {
        QJsonArray arr;
        for (const auto &ds : req.download_sources) {
            QJsonObject srcObj;
            srcObj["source"] = ds.source;
            srcObj["type"]   = ds.type;
            srcObj["url"]    = ds.url;
            arr.append(srcObj);
        }
        obj["download_sources"] = arr;
    }
    return obj;
}

/// 构建创建固件 URL 查询参数
bool DeSheng::buildAdminFirmwareCreateQuery(const AdminFirmwareCreateRequest &req,
                                             QUrlQuery &query,
                                             QString &error)
{
    Q_UNUSED(query);

    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.version, "version", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.release_date, "release_date", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.release_notes, "release_notes", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.download_url, "download_url", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.type, "type", error))
        return false;
    if (req.device_id.isEmpty() && req.device_name.isEmpty()) {
        error = "device_id 与 device_name 至少填一个";
        return false;
    }
    return true;
}

/// 处理固件列表应答
bool DeSheng::ProcessAdminFirmwareListResult(AdminFirmwareListResponse &responseData,
                                              const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.list.clear();
    responseData.data.total     = 0;
    responseData.data.page      = 0;
    responseData.data.page_size = 0;

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "固件列表 应答 JSON文档为空";
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

            responseData.data.total     = tmp_dataObj.value("total").toInt();
            responseData.data.page      = tmp_dataObj.value("page").toInt();
            responseData.data.page_size = tmp_dataObj.value("page_size").toInt();

            if (tmp_dataObj.contains("list") && tmp_dataObj["list"].isArray()) {
                QJsonArray arr = tmp_dataObj["list"].toArray();
                for (const QJsonValue &val : arr) {
                    if (!val.isObject())
                        continue;
                    QJsonObject itemObj = val.toObject();
                    AdminFirmwareListResponse::FirmwareListItem item;
                    item.id           = itemObj.value("id").toString();
                    item.device_id    = itemObj.value("device_id").toString();
                    item.device_name  = itemObj.value("device_name").toString();
                    item.version      = itemObj.value("version").toString();
                    item.download_url = itemObj.value("download_url").toString();
                    item.type         = itemObj.value("type").toString();
                    item.status       = itemObj.value("status").toString();
                    item.downloads    = itemObj.value("downloads").toInt();
                    item.created_at   = itemObj.value("created_at").toString();

                    // download_sources
                    if (itemObj.contains("download_sources") && itemObj["download_sources"].isArray()) {
                        QJsonArray srcArr = itemObj["download_sources"].toArray();
                        for (const QJsonValue &sv : srcArr) {
                            if (!sv.isObject())
                                continue;
                            QJsonObject srcObj = sv.toObject();
                            AdminFirmwareDownloadSource ds;
                            ds.source = srcObj.value("source").toString();
                            ds.type   = srcObj.value("type").toString();
                            ds.url    = srcObj.value("url").toString();
                            item.download_sources.append(ds);
                        }
                    }
                    responseData.data.list.append(item);
                }
            }

        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "固件列表 应答解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

/// 将固件列表请求转换为 JSON（GET 无 Body）
QJsonObject DeSheng::AdminFirmwareListRequestToJson(const AdminFirmwareListRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

/// 构建固件列表 URL 查询参数
bool DeSheng::buildAdminFirmwareListQuery(const AdminFirmwareListRequest &req,
                                           QUrlQuery &query,
                                           QString &error)
{
    if (req.page < 1) {
        error = "page 必须 >= 1";
        return false;
    }
    if (req.page_size < 1 || req.page_size > 100) {
        error = "page_size 必须在 1~100 之间";
        return false;
    }

    if (req.page != 1)
        query.addQueryItem("page", QString::number(req.page));
    if (req.page_size != 10)
        query.addQueryItem("page_size", QString::number(req.page_size));
    if (!req.device_id.isEmpty())
        query.addQueryItem("device_id", req.device_id);
    if (!req.device_name.isEmpty())
        query.addQueryItem("device_name", req.device_name);
    if (!req.version.isEmpty())
        query.addQueryItem("version", req.version);
    if (!req.status.isEmpty())
        query.addQueryItem("status", req.status);
    if (!req.sort_by.isEmpty())
        query.addQueryItem("sort_by", req.sort_by);
    if (!req.order_by.isEmpty())
        query.addQueryItem("order_by", req.order_by);

    return true;
}

/// 处理查询固件详情应答
bool DeSheng::ProcessAdminFirmwareDetailResult(AdminFirmwareCreateResponse &responseData,
                                                const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "固件详情 应答 JSON文档为空";
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

            responseData.data.id            = tmp_dataObj.value("id").toString();
            responseData.data.version       = tmp_dataObj.value("version").toString();
            responseData.data.file_name     = tmp_dataObj.value("file_name").toString();
            responseData.data.release_date  = tmp_dataObj.value("release_date").toString();
            responseData.data.release_notes = tmp_dataObj.value("release_notes").toString();
            responseData.data.download_url  = tmp_dataObj.value("download_url").toString();
            responseData.data.downloads     = tmp_dataObj.value("downloads").toInt();
            responseData.data.size          = static_cast<qint64>(tmp_dataObj.value("size").toDouble());
            responseData.data.type          = tmp_dataObj.value("type").toString();
            responseData.data.device_id     = tmp_dataObj.value("device_id").toString();
            responseData.data.device_name   = tmp_dataObj.value("device_name").toString();
            responseData.data.device_type   = tmp_dataObj.value("device_type").toString();
            responseData.data.status        = tmp_dataObj.value("status").toString();
            responseData.data.created_at    = tmp_dataObj.value("created_at").toString();
            responseData.data.updated_at    = tmp_dataObj.value("updated_at").toString();

            // download_sources
            if (tmp_dataObj.contains("download_sources") && tmp_dataObj["download_sources"].isArray()) {
                QJsonArray arr = tmp_dataObj["download_sources"].toArray();
                for (const QJsonValue &val : arr) {
                    if (!val.isObject())
                        continue;
                    QJsonObject srcObj = val.toObject();
                    AdminFirmwareDownloadSource ds;
                    ds.source = srcObj.value("source").toString();
                    ds.type   = srcObj.value("type").toString();
                    ds.url    = srcObj.value("url").toString();
                    responseData.data.download_sources.append(ds);
                }
            }

        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "固件详情 应答解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

/// 将固件详情请求转换为 JSON（GET 无 Body）
QJsonObject DeSheng::AdminFirmwareDetailRequestToJson(const AdminFirmwareDetailRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

/// 构建固件详情 URL 查询参数
bool DeSheng::buildAdminFirmwareDetailQuery(const AdminFirmwareDetailRequest &req,
                                             QUrlQuery &query,
                                             QString &error)
{
    Q_UNUSED(query);

    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.id, "id", error))
        return false;
    return true;
}

/// 处理更新固件应答
bool DeSheng::ProcessAdminFirmwareUpdateResult(AdminFirmwareCreateResponse &responseData,
                                                const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "更新固件 应答 JSON文档为空";
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

            responseData.data.id            = tmp_dataObj.value("id").toString();
            responseData.data.version       = tmp_dataObj.value("version").toString();
            responseData.data.file_name     = tmp_dataObj.value("file_name").toString();
            responseData.data.release_date  = tmp_dataObj.value("release_date").toString();
            responseData.data.release_notes = tmp_dataObj.value("release_notes").toString();
            responseData.data.download_url  = tmp_dataObj.value("download_url").toString();
            responseData.data.downloads     = tmp_dataObj.value("downloads").toInt();
            responseData.data.size          = static_cast<qint64>(tmp_dataObj.value("size").toDouble());
            responseData.data.type          = tmp_dataObj.value("type").toString();
            responseData.data.device_id     = tmp_dataObj.value("device_id").toString();
            responseData.data.device_name   = tmp_dataObj.value("device_name").toString();
            responseData.data.device_type   = tmp_dataObj.value("device_type").toString();
            responseData.data.status        = tmp_dataObj.value("status").toString();
            responseData.data.created_at    = tmp_dataObj.value("created_at").toString();
            responseData.data.updated_at    = tmp_dataObj.value("updated_at").toString();

            // download_sources
            if (tmp_dataObj.contains("download_sources") && tmp_dataObj["download_sources"].isArray()) {
                QJsonArray arr = tmp_dataObj["download_sources"].toArray();
                for (const QJsonValue &val : arr) {
                    if (!val.isObject())
                        continue;
                    QJsonObject srcObj = val.toObject();
                    AdminFirmwareDownloadSource ds;
                    ds.source = srcObj.value("source").toString();
                    ds.type   = srcObj.value("type").toString();
                    ds.url    = srcObj.value("url").toString();
                    responseData.data.download_sources.append(ds);
                }
            }

        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "更新固件 应答解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

/// 将更新固件请求转换为 JSON
QJsonObject DeSheng::AdminFirmwareUpdateRequestToJson(const AdminFirmwareUpdateRequest &req)
{
    QJsonObject obj;

    if (!req.version.isEmpty())
        obj["version"] = req.version;
    if (!req.file_name.isEmpty())
        obj["file_name"] = req.file_name;
    if (!req.release_date.isEmpty())
        obj["release_date"] = req.release_date;
    if (!req.release_notes.isEmpty())
        obj["release_notes"] = req.release_notes;
    if (!req.download_url.isEmpty())
        obj["download_url"] = req.download_url;
    if (!req.type.isEmpty())
        obj["type"] = req.type;
    if (!req.status.isEmpty())
        obj["status"] = req.status;
    if (req.size >= 0)
        obj["size"] = req.size;
    if (!req.download_sources.isEmpty()) {
        QJsonArray arr;
        for (const auto &ds : req.download_sources) {
            QJsonObject srcObj;
            srcObj["source"] = ds.source;
            srcObj["type"]   = ds.type;
            srcObj["url"]    = ds.url;
            arr.append(srcObj);
        }
        obj["download_sources"] = arr;
    }
    return obj;
}

/// 构建更新固件 URL 查询参数
bool DeSheng::buildAdminFirmwareUpdateQuery(const AdminFirmwareUpdateRequest &req,
                                             QUrlQuery &query,
                                             QString &error)
{
    Q_UNUSED(query);

    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.id, "id", error))
        return false;
    return true;
}

/// 处理删除固件应答
bool DeSheng::ProcessAdminFirmwareDeleteResult(AdminFirmwareDeleteResponse &responseData,
                                                const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "删除固件 应答 JSON文档为空";
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

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "删除固件 应答解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

/// 将删除固件请求转换为 JSON（DELETE 无 Body）
QJsonObject DeSheng::AdminFirmwareDeleteRequestToJson(const AdminFirmwareDeleteRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

/// 构建删除固件 URL 查询参数
bool DeSheng::buildAdminFirmwareDeleteQuery(const AdminFirmwareDeleteRequest &req,
                                             QUrlQuery &query,
                                             QString &error)
{
    Q_UNUSED(query);

    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.id, "id", error))
        return false;
    return true;
}
