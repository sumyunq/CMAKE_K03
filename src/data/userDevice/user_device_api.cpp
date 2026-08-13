#include "data/userDevice/user_device_api.h"
#include "data/api_global.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>



/// 获取月度统计数据
bool DeSheng::ProcessGetDeviceMonthlyStatsResult(GetDeviceMonthlyStatsResponse &responseData,
                                                  QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "月度设备统计 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject tmp_jsonObj = jsonDocument.object();

        // code — 兼容 string "success" 与 int 0 两种成功形式
        if (tmp_jsonObj.contains("code")) {
            if (tmp_jsonObj["code"].isString()) {
                responseData.code = tmp_jsonObj["code"].toString();
            } else if (tmp_jsonObj["code"].isDouble()) {
                int t_codeInt = tmp_jsonObj["code"].toInt();
                responseData.code = (t_codeInt == 0) ? QString("success") : QString::number(t_codeInt);
            } else {
                qDebug() << "code 类型异常";
                return false;
            }
        } else {
            qDebug() << "code 数据不存在";
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
            responseData.data.period = tmp_dataObj.value("period").toString();
            responseData.data.period_type = tmp_dataObj.value("period_type").toString();

            // statistics
            if (tmp_dataObj.contains("statistics") && tmp_dataObj["statistics"].isObject()) {
                QJsonObject t_statsObj = tmp_dataObj["statistics"].toObject();

                auto parseDeviceCount = [](const QJsonObject &parent, const QString &key) -> int64_t {
                    if (parent.contains(key) && parent[key].isObject()) {
                        QJsonObject t_item = parent[key].toObject();
                        QJsonValue t_val = t_item.value("count");
                        if (t_val.isDouble())
                            return static_cast<int64_t>(t_val.toDouble());
                        if (t_val.isString())
                            return static_cast<int64_t>(t_val.toString().toDouble());
                    }
                    return 0;
                };

                responseData.data.statistics.mouse.count = parseDeviceCount(t_statsObj, "mouse");
                responseData.data.statistics.keyboard.count = parseDeviceCount(t_statsObj, "keyboard");
                responseData.data.statistics.headset.count = parseDeviceCount(t_statsObj, "headset");
            }

            // total — 兼容 string / number
            {
                QJsonValue t_totalVal = tmp_dataObj.value("total");
                if (t_totalVal.isDouble())
                    responseData.data.total = static_cast<int64_t>(t_totalVal.toDouble());
                else if (t_totalVal.isString())
                    responseData.data.total = static_cast<int64_t>(t_totalVal.toString().toDouble());
            }

        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "月度设备统计 应答信息解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::GetDeviceMonthlyStatsRequestToJson(const GetDeviceMonthlyStatsRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildGetDeviceMonthlyStatsQuery(const GetDeviceMonthlyStatsRequest &req,
                                               QUrlQuery &query,
                                               QString &error)
{
    if (!XIBERIA_X_HUB_Utils::checkNotEmpty(req.month, "month", error))
        return false;

    query.addQueryItem("month", req.month);
    return true;
}

/// 获取累计统计数据
bool DeSheng::ProcessGetDeviceCumulativeStatsResult(GetDeviceCumulativeStatsResponse &responseData,
                                                     QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "累计设备统计 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject tmp_jsonObj = jsonDocument.object();

        // code — 兼容 string "success" 与 int 0 两种成功形式
        if (tmp_jsonObj.contains("code")) {
            if (tmp_jsonObj["code"].isString()) {
                responseData.code = tmp_jsonObj["code"].toString();
            } else if (tmp_jsonObj["code"].isDouble()) {
                int t_codeInt = tmp_jsonObj["code"].toInt();
                responseData.code = (t_codeInt == 0) ? QString("success") : QString::number(t_codeInt);
            } else {
                qDebug() << "code 类型异常";
                return false;
            }
        } else {
            qDebug() << "code 数据不存在";
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
            responseData.data.period = tmp_dataObj.value("period").toString();
            responseData.data.period_type = tmp_dataObj.value("period_type").toString();

            // statistics
            if (tmp_dataObj.contains("statistics") && tmp_dataObj["statistics"].isObject()) {
                QJsonObject t_statsObj = tmp_dataObj["statistics"].toObject();

                auto parseDeviceCount = [](const QJsonObject &parent, const QString &key) -> int64_t {
                    if (parent.contains(key) && parent[key].isObject()) {
                        QJsonObject t_item = parent[key].toObject();
                        QJsonValue t_val = t_item.value("count");
                        if (t_val.isDouble())
                            return static_cast<int64_t>(t_val.toDouble());
                        if (t_val.isString())
                            return static_cast<int64_t>(t_val.toString().toDouble());
                    }
                    return 0;
                };

                responseData.data.statistics.mouse.count = parseDeviceCount(t_statsObj, "mouse");
                responseData.data.statistics.keyboard.count = parseDeviceCount(t_statsObj, "keyboard");
                responseData.data.statistics.headset.count = parseDeviceCount(t_statsObj, "headset");
            }

            // total — 兼容 string / number
            {
                QJsonValue t_totalVal = tmp_dataObj.value("total");
                if (t_totalVal.isDouble())
                    responseData.data.total = static_cast<int64_t>(t_totalVal.toDouble());
                else if (t_totalVal.isString())
                    responseData.data.total = static_cast<int64_t>(t_totalVal.toString().toDouble());
            }

        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "累计设备统计 应答信息解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::GetDeviceCumulativeStatsRequestToJson(const GetDeviceCumulativeStatsRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildGetDeviceCumulativeStatsQuery(const GetDeviceCumulativeStatsRequest &req,
                                                  QUrlQuery &query,
                                                  QString &error)
{
    Q_UNUSED(req);
    Q_UNUSED(query);
    Q_UNUSED(error);
    return true;
}
