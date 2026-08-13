#include "data/userDeviceLog/user_device_log_api.h"
#include "data/api_global.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>



/// 创建设备日志
bool DeSheng::ProcessCreateDeviceLogResult(CreateDeviceLogResponse &responseData,
                                           QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "创建设备日志 应答信息 JSON文档为空";
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
            responseData.data.user_id = tmp_dataObj.value("user_id").toString();
            responseData.data.device_name = tmp_dataObj.value("device_name").toString();
            responseData.data.device_type = tmp_dataObj.value("device_type").toString();
            responseData.data.city = tmp_dataObj.value("city").toString();
            responseData.data.country = tmp_dataObj.value("country").toString();
            responseData.data.country_name = tmp_dataObj.value("country_name").toString();
            responseData.data.first_register = tmp_dataObj.value("first_register").toBool();
            responseData.data.drive_id = tmp_dataObj.value("drive_id").toString();
            responseData.data.drive_version = tmp_dataObj.value("drive_version").toString();
            responseData.data.firmware_id = tmp_dataObj.value("firmware_id").toString();
            responseData.data.firmware_version = tmp_dataObj.value("firmware_version").toString();
            responseData.data.login_ip = tmp_dataObj.value("login_ip").toString();
            responseData.data.os_info = tmp_dataObj.value("os_info").toString();
            responseData.data.created_at = tmp_dataObj.value("created_at").toString();
        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "创建设备日志 应答信息解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::CreateDeviceLogRequestToJson(const CreateDeviceLogRequest &req)
{
    QJsonObject obj;
    obj["device_name"] = req.device_name;
    obj["device_type"] = req.device_type;
    if (!req.city.isEmpty())
        obj["city"] = req.city;
    if (!req.drive_id.isEmpty())
        obj["drive_id"] = req.drive_id;
    if (!req.drive_version.isEmpty())
        obj["drive_version"] = req.drive_version;
    if (!req.firmware_id.isEmpty())
        obj["firmware_id"] = req.firmware_id;
    if (!req.firmware_version.isEmpty())
        obj["firmware_version"] = req.firmware_version;
    return obj;
}

bool DeSheng::buildCreateDeviceLogQuery(const CreateDeviceLogRequest &req,
                                         QUrlQuery &query,
                                         QString &error)
{
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.device_name, "device_name", error))
        return false;
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.device_type, "device_type", error))
        return false;

    Q_UNUSED(query);
    return true;
}

/// 获取设备日志列表
bool DeSheng::ProcessGetDeviceLogListResult(GetDeviceLogListResponse &responseData,
                                            QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.list.clear();
    responseData.data.total = 0;
    responseData.data.page = 0;
    responseData.data.page_size = 0;

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "设备日志列表 应答信息 JSON文档为空";
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

            // list 数组
            if (tmp_dataObj.contains("list") && tmp_dataObj["list"].isArray()) {
                QJsonArray listArr = tmp_dataObj["list"].toArray();
                for (const QJsonValue &val : listArr) {
                    if (!val.isObject())
                        continue;
                    QJsonObject itemObj = val.toObject();
                    DeviceLogItem item;
                    item.id = itemObj.value("id").toString();
                    item.user_id = itemObj.value("user_id").toString();
                    item.device_name = itemObj.value("device_name").toString();
                    item.device_type = itemObj.value("device_type").toString();
                    item.city = itemObj.value("city").toString();
                    item.country = itemObj.value("country").toString();
                    item.country_name = itemObj.value("country_name").toString();
                    item.first_register = itemObj.value("first_register").toBool();
                    item.drive_id = itemObj.value("drive_id").toString();
                    item.drive_version = itemObj.value("drive_version").toString();
                    item.firmware_id = itemObj.value("firmware_id").toString();
                    item.firmware_version = itemObj.value("firmware_version").toString();
                    item.login_ip = itemObj.value("login_ip").toString();
                    item.os_info = itemObj.value("os_info").toString();
                    item.created_at = itemObj.value("created_at").toString();
                    responseData.data.list.append(item);
                }
            }

            responseData.data.total = tmp_dataObj.value("total").toInt();
            responseData.data.page = tmp_dataObj.value("page").toInt();
            responseData.data.page_size = tmp_dataObj.value("page_size").toInt();

        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "设备日志列表 应答信息解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::GetDeviceLogListRequestToJson(const GetDeviceLogListRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildGetDeviceLogListQuery(const GetDeviceLogListRequest &req,
                                          QUrlQuery &query,
                                          QString &error)
{
    Q_UNUSED(error);

    if (!req.user_id.isEmpty())
        query.addQueryItem("user_id", req.user_id);
    if (!req.device_id.isEmpty())
        query.addQueryItem("device_id", req.device_id);
    if (!req.device_name.isEmpty())
        query.addQueryItem("device_name", req.device_name);
    if (!req.device_type.isEmpty())
        query.addQueryItem("device_type", req.device_type);
    if (!req.city.isEmpty())
        query.addQueryItem("city", req.city);
    if (req.first_register)
        query.addQueryItem("first_register", "true");
    if (!req.start_date.isEmpty())
        query.addQueryItem("start_date", req.start_date);
    if (!req.end_date.isEmpty())
        query.addQueryItem("end_date", req.end_date);
    if (req.page != 1)
        query.addQueryItem("page", QString::number(req.page));
    if (req.page_size != 10)
        query.addQueryItem("page_size", QString::number(req.page_size));
    if (!req.sort_by.isEmpty())
        query.addQueryItem("sort_by", req.sort_by);
    if (!req.order_by.isEmpty())
        query.addQueryItem("order_by", req.order_by);

    return true;
}

/// 获取基础统计
bool DeSheng::ProcessGetBasicStatsResult(GetBasicStatsResponse &responseData,
                                         QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "基础统计 应答信息 JSON文档为空";
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
            // 兼容双解析：服务器可能返回 string 或 number 类型
            auto parseCount = [](const QJsonValue &val) -> int64_t {
                if (val.isDouble())
                    return static_cast<int64_t>(val.toDouble());
                if (val.isString())
                    return static_cast<int64_t>(val.toString().toDouble());
                return 0;
            };
            responseData.data.total_users = parseCount(tmp_dataObj.value("total_users"));
            responseData.data.total_devices = parseCount(tmp_dataObj.value("total_devices"));
            responseData.data.daily_active = parseCount(tmp_dataObj.value("daily_active"));
            responseData.data.monthly_active = parseCount(tmp_dataObj.value("monthly_active"));
            responseData.data.daily_register = parseCount(tmp_dataObj.value("daily_register"));
        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "基础统计 应答信息解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::GetBasicStatsRequestToJson(const GetBasicStatsRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildGetBasicStatsQuery(const GetBasicStatsRequest &req,
                                       QUrlQuery &query,
                                       QString &error)
{
    Q_UNUSED(req);
    Q_UNUSED(query);
    Q_UNUSED(error);
    return true;
}

/// 获取城市活跃统计
bool DeSheng::ProcessGetCityActiveResult(GetCityActiveResponse &responseData,
                                         QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "城市活跃统计 应答信息 JSON文档为空";
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

        // data（直接是数组）
        if (tmp_jsonObj.contains("data") && tmp_jsonObj["data"].isArray()) {
            QJsonArray dataArr = tmp_jsonObj["data"].toArray();
            for (const QJsonValue &val : dataArr) {
                if (!val.isObject())
                    continue;
                QJsonObject itemObj = val.toObject();
                CityActiveItem item;
                item.city = itemObj.value("city").toString();
                item.count = static_cast<int64_t>(itemObj.value("count").toDouble());
                responseData.data.append(item);
            }
        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "城市活跃统计 应答信息解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::GetCityActiveRequestToJson(const GetCityActiveRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildGetCityActiveQuery(const GetCityActiveRequest &req,
                                       QUrlQuery &query,
                                       QString &error)
{
    Q_UNUSED(error);

    if (!req.start_time.isEmpty())
        query.addQueryItem("start_time", req.start_time);
    if (!req.end_time.isEmpty())
        query.addQueryItem("end_time", req.end_time);

    return true;
}

/// 获取国家活跃统计
bool DeSheng::ProcessGetCountryActiveResult(GetCountryActiveResponse &responseData,
                                            QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "国家活跃统计 应答信息 JSON文档为空";
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

        // data（直接是数组）
        if (tmp_jsonObj.contains("data") && tmp_jsonObj["data"].isArray()) {
            QJsonArray dataArr = tmp_jsonObj["data"].toArray();
            for (const QJsonValue &val : dataArr) {
                if (!val.isObject())
                    continue;
                QJsonObject itemObj = val.toObject();
                CountryActiveItem item;
                item.country = itemObj.value("country").toString();
                item.country_name = itemObj.value("country_name").toString();
                item.count = static_cast<int64_t>(itemObj.value("count").toDouble());
                responseData.data.append(item);
            }
        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "国家活跃统计 应答信息解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::GetCountryActiveRequestToJson(const GetCountryActiveRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildGetCountryActiveQuery(const GetCountryActiveRequest &req,
                                          QUrlQuery &query,
                                          QString &error)
{
    Q_UNUSED(error);

    if (!req.start_time.isEmpty())
        query.addQueryItem("start_time", req.start_time);
    if (!req.end_time.isEmpty())
        query.addQueryItem("end_time", req.end_time);

    return true;
}

/// 获取设备活跃统计
bool DeSheng::ProcessGetDeviceActiveResult(GetDeviceActiveResponse &responseData,
                                           QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "设备活跃统计 应答信息 JSON文档为空";
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

        // data（直接是数组）
        if (tmp_jsonObj.contains("data") && tmp_jsonObj["data"].isArray()) {
            QJsonArray dataArr = tmp_jsonObj["data"].toArray();
            for (const QJsonValue &val : dataArr) {
                if (!val.isObject())
                    continue;
                QJsonObject itemObj = val.toObject();
                DeviceActiveItem item;
                item.date = itemObj.value("date").toString();
                item.device_name = itemObj.value("device_name").toString();
                item.device_type = itemObj.value("device_type").toString();
                item.count = static_cast<int64_t>(itemObj.value("count").toDouble());
                responseData.data.append(item);
            }
        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "设备活跃统计 应答信息解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::GetDeviceActiveRequestToJson(const GetDeviceActiveRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildGetDeviceActiveQuery(const GetDeviceActiveRequest &req,
                                         QUrlQuery &query,
                                         QString &error)
{
    Q_UNUSED(error);

    if (!req.group_by.isEmpty())
        query.addQueryItem("group_by", req.group_by);
    if (!req.device_type.isEmpty())
        query.addQueryItem("device_type", req.device_type);
    if (!req.device_id.isEmpty())
        query.addQueryItem("device_id", req.device_id);
    if (!req.device_name.isEmpty())
        query.addQueryItem("device_name", req.device_name);
    if (!req.start_date.isEmpty())
        query.addQueryItem("start_date", req.start_date);
    if (!req.end_date.isEmpty())
        query.addQueryItem("end_date", req.end_date);

    return true;
}

/// 获取用户活跃统计
bool DeSheng::ProcessGetUserActiveResult(GetUserActiveResponse &responseData,
                                         QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "用户活跃统计 应答信息 JSON文档为空";
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

        // data（直接是数组）
        if (tmp_jsonObj.contains("data") && tmp_jsonObj["data"].isArray()) {
            QJsonArray dataArr = tmp_jsonObj["data"].toArray();
            for (const QJsonValue &val : dataArr) {
                if (!val.isObject())
                    continue;
                QJsonObject itemObj = val.toObject();
                UserActiveItem item;
                item.date = itemObj.value("date").toString();
                item.count = static_cast<int64_t>(itemObj.value("count").toDouble());
                responseData.data.append(item);
            }
        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "用户活跃统计 应答信息解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::GetUserActiveRequestToJson(const GetUserActiveRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildGetUserActiveQuery(const GetUserActiveRequest &req,
                                       QUrlQuery &query,
                                       QString &error)
{
    Q_UNUSED(error);

    if (!req.group_by.isEmpty())
        query.addQueryItem("group_by", req.group_by);
    if (!req.device_type.isEmpty())
        query.addQueryItem("device_type", req.device_type);
    if (!req.device_id.isEmpty())
        query.addQueryItem("device_id", req.device_id);
    if (!req.device_name.isEmpty())
        query.addQueryItem("device_name", req.device_name);
    if (!req.start_date.isEmpty())
        query.addQueryItem("start_date", req.start_date);
    if (!req.end_date.isEmpty())
        query.addQueryItem("end_date", req.end_date);

    return true;
}
