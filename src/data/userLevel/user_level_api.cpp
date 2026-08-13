#include "data/userLevel/user_level_api.h"
#include "data/api_global.h"


#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>


bool DeSheng::ProcessOnlineReportResult(OnlineReportResponse &responseData,
                                         const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.status.clear();
    responseData.data.gained_exp = 0;

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "在线时长上报 应答 JSON文档为空";
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
            if(responseData.message != "success")
            {
                qDebug() << responseData.message;
                return false;
            }
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

        // data
        if (tmp_jsonObj.contains("data") && tmp_jsonObj["data"].isObject()) {
            QJsonObject tmp_dataObj = tmp_jsonObj["data"].toObject();
            responseData.data.status = tmp_dataObj.value("status").toString();
            responseData.data.gained_exp = tmp_dataObj.value("gained_exp").toInt();
            // 冷却中/已达上限为文档定义的合法状态，仅未知状态视为失败
            if (responseData.data.status != "success"
                && responseData.data.status != "cooling"
                && responseData.data.status != "limit_reached")
            {
                qDebug() << responseData.message;
                return false;
            }
        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "在线时长上报 应答解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::OnlineReportRequestToJson(const OnlineReportRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildOnlineReportQuery(const OnlineReportRequest &req,
                                      QUrlQuery &query,
                                      QString &error)
{
    Q_UNUSED(req);
    Q_UNUSED(query);
    Q_UNUSED(error);
    return true;
}

bool DeSheng::ProcessGetUserLevelResult(GetUserLevelResponse &responseData,
                                         const QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "用户等级信息 应答 JSON文档为空";
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
            responseData.data.level = tmp_dataObj.value("level").toInt();
            responseData.data.total_experience = tmp_dataObj.value("total_experience").toInt();
            responseData.data.current_experience = tmp_dataObj.value("current_experience").toInt();
            responseData.data.exp_cap = tmp_dataObj.value("exp_cap").toInt();
        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "用户等级信息 应答解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::GetUserLevelRequestToJson(const GetUserLevelRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildGetUserLevelQuery(const GetUserLevelRequest &req,
                                      QUrlQuery &query,
                                      QString &error)
{
    Q_UNUSED(req);
    Q_UNUSED(query);
    Q_UNUSED(error);
    return true;
}
