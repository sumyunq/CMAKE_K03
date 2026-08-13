#include "data/userConfig/user_config_api.h"
#include "data/api_global.h"


#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>


//创建配置接口，服务器回应解析
bool DeSheng::ProcessUserConfigsCreateResult(UserConfigsCreateResponse &responseData,
                                             QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {}; // 重置结构体

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "用户配置创建 应答信息 JSON文档为空";
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

            responseData.data.id = tmp_dataObj.value("id").toInt();

            // author
            if (tmp_dataObj.contains("author") && tmp_dataObj["author"].isObject()) {
                QJsonObject authorObj = tmp_dataObj["author"].toObject();
                responseData.data.author.user_id = authorObj.value("user_id").toInt();
                responseData.data.author.username = authorObj.value("username").toString();
                responseData.data.author.avatar = authorObj.value("avatar").toString();
                responseData.data.author.nickname = authorObj.value("nickname").toString();
                responseData.data.author.level = authorObj.value("level").toInt();

                // roles
                if (authorObj.contains("roles") && authorObj["roles"].isArray()) {
                    QJsonArray rolesArr = authorObj["roles"].toArray();
                    for (const QJsonValue &v : rolesArr)
                        responseData.data.author.roles.append(v.toString());
                }
                // titles
                if (authorObj.contains("titles") && authorObj["titles"].isArray()) {
                    QJsonArray titlesArr = authorObj["titles"].toArray();
                    for (const QJsonValue &v : titlesArr)
                        responseData.data.author.titles.append(v.toString());
                }
            }

            responseData.data.device_id = tmp_dataObj.value("device_id").toString();
            responseData.data.drive_version = tmp_dataObj.value("drive_version").toString();
            responseData.data.firmware_version = tmp_dataObj.value("firmware_version").toString();
            responseData.data.device_name = tmp_dataObj.value("device_name").toString();
            responseData.data.device_type = tmp_dataObj.value("device_type").toString();
            responseData.data.title = tmp_dataObj.value("title").toString();
            responseData.data.description = tmp_dataObj.value("description").toString();
            responseData.data.language = tmp_dataObj.value("language").toString();
            responseData.data.visibility = tmp_dataObj.value("visibility").toString();

            // user_tags 数组
            if (tmp_dataObj.contains("user_tags") && tmp_dataObj["user_tags"].isArray()) {
                QJsonArray tagsArr = tmp_dataObj["user_tags"].toArray();
                for (const QJsonValue &val : tagsArr) {
                    responseData.data.user_tags.append(val.toString());
                }
            }

            responseData.data.download_count = tmp_dataObj.value("download_count").toInt();
            responseData.data.collect_count = tmp_dataObj.value("collect_count").toInt();
            responseData.data.like_count = tmp_dataObj.value("like_count").toInt();
            responseData.data.share_count = tmp_dataObj.value("share_count").toInt();
            responseData.data.dislike_count = tmp_dataObj.value("dislike_count").toInt();
            responseData.data.like_dislike_score = tmp_dataObj.value("like_dislike_score").toInt();
            responseData.data.hot_score = tmp_dataObj.value("hot_score").toInt();
            responseData.data.share_code = tmp_dataObj.value("share_code").toString();
            responseData.data.status = tmp_dataObj.value("status").toString();
            responseData.data.created_at = tmp_dataObj.value("created_at").toString();
            responseData.data.updated_at = tmp_dataObj.value("updated_at").toString();
            responseData.data.published_at = tmp_dataObj.value("published_at").toString();
            responseData.data.modified_at = tmp_dataObj.value("modified_at").toString();
            responseData.data.is_official_tag = tmp_dataObj.value("is_official_tag").toBool();
            responseData.data.is_expert_tag = tmp_dataObj.value("is_expert_tag").toBool();

        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "用户配置创建 应答信息解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}
//结构体传递得到QJsonObject
QJsonObject DeSheng::UserConfigsCreateRequestToJson(const UserConfigsCreateRequest &req)
{
    QJsonObject obj;

    // 根据实际请求结构填充，
    //设备ID (必填)
    if (!req.device_id.isEmpty())
        obj["device_id"] = req.device_id;

    //驱动版本号 (必填)
    if (!req.drive_version.isEmpty())
        obj["drive_version"] = req.drive_version;

    //固件版本号 (必填)
    if (!req.firmware_version.isEmpty())
        obj["firmware_version"] = req.firmware_version;

    // 设备名称 (必填)
    if (!req.device_name.isEmpty())
        obj["device_name"] = req.device_name;

    // 设备类型：mouse/keyboard/headset (必填)
    if (!req.device_type.isEmpty())
        obj["device_type"] = req.device_type;

    // 方案名称 (必填)
    if (!req.title.isEmpty())
        obj["title"] = req.title;

    // 方案描述，最多1000字符 (可选)
    if (!req.description.isEmpty())
        obj["description"] = req.description;

    // 用户标签数组，最多10个，单标签最多50字符 (可选)
    if (!req.user_tags.isEmpty()) {
        QJsonArray tagsArr;
        for (const QString &tag : req.user_tags) {
            tagsArr.append(tag);
        }
        obj["user_tags"] = tagsArr;
    }

    // 配置文件下载URL (必填)
    if (!req.config_url.isEmpty())
        obj["config_url"] = req.config_url;

    // 配置语言 (可选，默认zh)
    if (!req.language.isEmpty())
        obj["language"] = req.language;

    // 可见性 (可选，默认public)
    if (!req.visibility.isEmpty())
        obj["visibility"] = req.visibility;

    return obj;
}


//获取今日创建数量接口，服务器回应解析
bool DeSheng::ProcessGetTodayCountListResult(GetTodayCountListResponse &responseData,
                                    QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {}; // 重置结构体

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "用户配置创建 应答信息 JSON文档为空";
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

            responseData.data.device_name = tmp_dataObj.value("device_name").toString();
            responseData.data.today_count = tmp_dataObj.value("today_count").toInt();

        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "用户配置创建 应答信息解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}




bool DeSheng::ProcessGetPublicConfigurationListResult(
    GetPublicConfigurationListResponse &responseData, QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.list.clear();
    responseData.data.total = 0;
    responseData.data.page = 0;
    responseData.data.page_size = 0;

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "公开配置列表 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        // code
        if (root.contains("code") && root["code"].isString()) {
            responseData.code = root["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        // message
        if (root.contains("message") && root["message"].isString()) {
            responseData.message = root["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

        // data
        if (root.contains("data") && root["data"].isObject()) {
            QJsonObject dataObj = root["data"].toObject();

            // list 数组
            if (dataObj.contains("list") && dataObj["list"].isArray()) {
                QJsonArray listArr = dataObj["list"].toArray();
                for (const QJsonValue &val : listArr) {
                    if (!val.isObject())
                        continue;
                    QJsonObject itemObj = val.toObject();

                    GetPublicConfigurationListResponse::ListItem item;

                    item.id = itemObj.value("id").toInt();

                    // author
                    if (itemObj.contains("author") && itemObj["author"].isObject()) {
                        QJsonObject authorObj = itemObj["author"].toObject();
                        item.author.user_id = authorObj.value("user_id").toInt();
                        item.author.username = authorObj.value("username").toString();
                        item.author.avatar = authorObj.value("avatar").toString();
                        item.author.nickname = authorObj.value("nickname").toString();
                        item.author.level = authorObj.value("level").toInt();

                        if (authorObj.contains("roles") && authorObj["roles"].isArray()) {
                            QJsonArray rolesArr = authorObj["roles"].toArray();
                            for (const QJsonValue &v : rolesArr)
                                item.author.roles.append(v.toString());
                        }
                        if (authorObj.contains("titles") && authorObj["titles"].isArray()) {
                            QJsonArray titlesArr = authorObj["titles"].toArray();
                            for (const QJsonValue &v : titlesArr)
                                item.author.titles.append(v.toString());
                        }
                    }

                    item.device_id = itemObj.value("device_id").toString();
                    item.drive_version = itemObj.value("drive_version").toString();
                    item.firmware_version = itemObj.value("firmware_version").toString();
                    item.device_name = itemObj.value("device_name").toString();
                    item.device_type = itemObj.value("device_type").toString();
                    item.title = itemObj.value("title").toString();
                    item.description = itemObj.value("description").toString();
                    item.language = itemObj.value("language").toString();
                    item.visibility = itemObj.value("visibility").toString();

                    // user_tags
                    if (itemObj.contains("user_tags") && itemObj["user_tags"].isArray()) {
                        QJsonArray tagsArr = itemObj["user_tags"].toArray();
                        for (const QJsonValue &tag : tagsArr) {
                            item.user_tags.append(tag.toString());
                        }
                    }

                    item.download_count = itemObj.value("download_count").toInt();
                    item.collect_count = itemObj.value("collect_count").toInt();
                    item.like_count = itemObj.value("like_count").toInt();
                    item.share_count = itemObj.value("share_count").toInt();
                    item.dislike_count = itemObj.value("dislike_count").toInt();
                    item.like_dislike_score = itemObj.value("like_dislike_score").toInt();
                    item.hot_score = itemObj.value("hot_score").toInt();
                    item.status = itemObj.value("status").toString();
                    item.created_at = itemObj.value("created_at").toString();
                    item.updated_at = itemObj.value("updated_at").toString();
                    item.is_official_tag = itemObj.value("is_official_tag").toBool();
                    item.is_expert_tag = itemObj.value("is_expert_tag").toBool();
                    item.is_collected = itemObj.value("is_collected").toBool();
                    item.is_liked = itemObj.value("is_liked").toBool();
                    item.is_disliked = itemObj.value("is_disliked").toBool();
                    item.is_pinned = itemObj.value("is_pinned").toBool();  // 公开列表接口不返回，恒 false

                    // comments
                    if (itemObj.contains("comments") && itemObj["comments"].isArray()) {
                        QJsonArray commentsArr = itemObj["comments"].toArray();
                        for (const QJsonValue &cVal : commentsArr) {
                            if (!cVal.isObject())
                                continue;
                            QJsonObject cObj = cVal.toObject();
                            GetPublicConfigurationListResponse::Comment comment;
                            comment.id = cObj.value("id").toInt();
                            comment.comment_text = cObj.value("comment_text").toString();
                            comment.comment_text_en = cObj.value("comment_text_en").toString();
                            comment.count = cObj.value("count").toInt();
                            comment.is_clicked = cObj.value("is_clicked").toBool();
                            item.comments.append(comment);
                        }
                    }

                    responseData.data.list.append(item);
                }
            }

            responseData.data.total = dataObj.value("total").toInt();
            responseData.data.page = dataObj.value("page").toInt();
            responseData.data.page_size = dataObj.value("page_size").toInt();

        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "公开配置列表 应答信息解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::GetPublicConfigurationListRequestToJson(
    const GetPublicConfigurationListRequest &req)
{
    QJsonObject obj;

    /// 全部非必填，只填有值的字段
    if (!req.keyword.isEmpty())
        obj["keyword"] = req.keyword;
    if (!req.username.isEmpty())
        obj["username"] = req.username;
    if (!req.device_name.isEmpty())
        obj["device_name"] = req.device_name;
    if (!req.device_type.isEmpty())
        obj["device_type"] = req.device_type;
    if (!req.language.isEmpty())
        obj["language"] = req.language;
    if (!req.user_tag.isEmpty())
        obj["user_tag"] = req.user_tag;
    if (!req.sort.isEmpty())
        obj["sort"] = req.sort;
    if (!req.start_time.isEmpty())
        obj["start_time"] = req.start_time;
    if (!req.end_time.isEmpty())
        obj["end_time"] = req.end_time;
    if (req.is_official_tag)
        obj["is_official_tag"] = req.is_official_tag;
    if (req.is_expert_tag)
        obj["is_expert_tag"] = req.is_expert_tag;
    if (req.page != 1)
        obj["page"] = req.page;
    if (req.page_size != 20)
        obj["page_size"] = req.page_size;

    return obj;
}

bool DeSheng::buildGetPublicConfigurationListQuery(const GetPublicConfigurationListRequest &req,
                                                   QUrlQuery &query,
                                                   QString &error)
{
    Q_UNUSED(error);

    /// 全部非必填
    if (!req.keyword.isEmpty())
        query.addQueryItem("keyword", req.keyword);
    if (!req.username.isEmpty())
        query.addQueryItem("username", req.username);
    if (!req.device_name.isEmpty())
        query.addQueryItem("device_name", req.device_name);
    if (!req.device_type.isEmpty())
        query.addQueryItem("device_type", req.device_type);
    if (!req.language.isEmpty())
        query.addQueryItem("language", req.language);
    if (!req.user_tag.isEmpty())
        query.addQueryItem("user_tag", req.user_tag);
    if (!req.sort.isEmpty())
        query.addQueryItem("sort", req.sort);
    if (!req.start_time.isEmpty())
        query.addQueryItem("start_time", req.start_time);
    if (!req.end_time.isEmpty())
        query.addQueryItem("end_time", req.end_time);
    if (req.is_official_tag)
        query.addQueryItem("is_official_tag", "true");
    if (req.is_expert_tag)
        query.addQueryItem("is_expert_tag", "true");
    if (req.page != 1)
        query.addQueryItem("page", QString::number(req.page));
    if (req.page_size != 20)
        query.addQueryItem("page_size", QString::number(req.page_size));

    return true;
}

bool DeSheng::ProcessGetConfigurationDetailsResult(GetConfigurationDetailsResponse &responseData,
                                                   QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "配置详情 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        // code
        if (root.contains("code") && root["code"].isString()) {
            responseData.code = root["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        // message
        if (root.contains("message") && root["message"].isString()) {
            responseData.message = root["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

        // data
        if (root.contains("data") && root["data"].isObject()) {
            QJsonObject dataObj = root["data"].toObject();

            responseData.data.id = dataObj.value("id").toInt();

            // author
            if (dataObj.contains("author") && dataObj["author"].isObject()) {
                QJsonObject authorObj = dataObj["author"].toObject();
                responseData.data.author.user_id = authorObj.value("user_id").toInt();
                responseData.data.author.username = authorObj.value("username").toString();
                responseData.data.author.avatar = authorObj.value("avatar").toString();
                responseData.data.author.nickname = authorObj.value("nickname").toString();
                responseData.data.author.level = authorObj.value("level").toInt();

                if (authorObj.contains("roles") && authorObj["roles"].isArray()) {
                    QJsonArray rolesArr = authorObj["roles"].toArray();
                    for (const QJsonValue &v : rolesArr)
                        responseData.data.author.roles.append(v.toString());
                }
                if (authorObj.contains("titles") && authorObj["titles"].isArray()) {
                    QJsonArray titlesArr = authorObj["titles"].toArray();
                    for (const QJsonValue &v : titlesArr)
                        responseData.data.author.titles.append(v.toString());
                }
            }

            responseData.data.device_id = dataObj.value("device_id").toString();
            responseData.data.drive_version = dataObj.value("drive_version").toString();
            responseData.data.firmware_version = dataObj.value("firmware_version").toString();
            responseData.data.device_name = dataObj.value("device_name").toString();
            responseData.data.device_type = dataObj.value("device_type").toString();
            responseData.data.title = dataObj.value("title").toString();
            responseData.data.description = dataObj.value("description").toString();
            responseData.data.language = dataObj.value("language").toString();
            responseData.data.visibility = dataObj.value("visibility").toString();

            // user_tags
            if (dataObj.contains("user_tags") && dataObj["user_tags"].isArray()) {
                QJsonArray tagsArr = dataObj["user_tags"].toArray();
                for (const QJsonValue &tag : tagsArr) {
                    responseData.data.user_tags.append(tag.toString());
                }
            }

            responseData.data.download_count = dataObj.value("download_count").toInt();
            responseData.data.collect_count = dataObj.value("collect_count").toInt();
            responseData.data.like_count = dataObj.value("like_count").toInt();
            responseData.data.dislike_count = dataObj.value("dislike_count").toInt();
            responseData.data.like_dislike_score = dataObj.value("like_dislike_score").toInt();
            responseData.data.share_count = dataObj.value("share_count").toInt();
            responseData.data.hot_score = dataObj.value("hot_score").toInt();
            responseData.data.config_url = dataObj.value("config_url").toString();
            responseData.data.status = dataObj.value("status").toString();
            responseData.data.created_at = dataObj.value("created_at").toString();
            responseData.data.updated_at = dataObj.value("updated_at").toString();
            responseData.data.published_at = dataObj.value("published_at").toString();
            responseData.data.modified_at = dataObj.value("modified_at").toString();
            responseData.data.is_official_tag = dataObj.value("is_official_tag").toBool();
            responseData.data.is_expert_tag = dataObj.value("is_expert_tag").toBool();
            responseData.data.is_collected = dataObj.value("is_collected").toBool();
            responseData.data.is_liked = dataObj.value("is_liked").toBool();
            responseData.data.is_disliked = dataObj.value("is_disliked").toBool();

            // comments
            if (dataObj.contains("comments") && dataObj["comments"].isArray()) {
                QJsonArray commentsArr = dataObj["comments"].toArray();
                for (const QJsonValue &cVal : commentsArr) {
                    if (!cVal.isObject())
                        continue;
                    QJsonObject cObj = cVal.toObject();
                    GetConfigurationDetailsResponse::Comment comment;
                    comment.id = cObj.value("id").toInt();
                    comment.comment_text = cObj.value("comment_text").toString();
                    comment.comment_text_en = cObj.value("comment_text_en").toString();
                    comment.count = cObj.value("count").toInt();
                    comment.is_clicked = cObj.value("is_clicked").toBool();
                    responseData.data.comments.append(comment);
                }
            }

        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "配置详情 应答信息解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

bool DeSheng::ProcessGetMyConfigurationListResult(GetMyConfigurationListResponse &responseData,
                                                  QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.list.clear();
    responseData.data.total = 0;
    responseData.data.page = 0;
    responseData.data.page_size = 0;

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "我的配置列表 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        // code
        if (root.contains("code") && root["code"].isString()) {
            responseData.code = root["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        // message
        if (root.contains("message") && root["message"].isString()) {
            responseData.message = root["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

        // data
        if (root.contains("data") && root["data"].isObject()) {
            QJsonObject dataObj = root["data"].toObject();

            // list
            if (dataObj.contains("list") && dataObj["list"].isArray()) {
                QJsonArray listArr = dataObj["list"].toArray();
                for (const QJsonValue &val : listArr) {
                    if (!val.isObject())
                        continue;
                    QJsonObject itemObj = val.toObject();

                    GetMyConfigurationListResponse::ListItem item;
                    item.id = itemObj.value("id").toInt();
                    item.device_id = itemObj.value("device_id").toString();
                    item.device_name = itemObj.value("device_name").toString();
                    item.device_type = itemObj.value("device_type").toString();
                    item.title = itemObj.value("title").toString();
                    item.language = itemObj.value("language").toString();
                    item.visibility = itemObj.value("visibility").toString();

                    // user_tags
                    if (itemObj.contains("user_tags") && itemObj["user_tags"].isArray()) {
                        QJsonArray tagsArr = itemObj["user_tags"].toArray();
                        for (const QJsonValue &tag : tagsArr) {
                            item.user_tags.append(tag.toString());
                        }
                    }

                    item.download_count = itemObj.value("download_count").toInt();
                    item.collect_count = itemObj.value("collect_count").toInt();
                    item.like_count = itemObj.value("like_count").toInt();
                    item.share_count = itemObj.value("share_count").toInt();
                    item.dislike_count = itemObj.value("dislike_count").toInt();
                    item.like_dislike_score = itemObj.value("like_dislike_score").toInt();
                    item.hot_score = itemObj.value("hot_score").toInt();
                    item.config_url = itemObj.value("config_url").toString();
                    item.status = itemObj.value("status").toString();
                    item.created_at = itemObj.value("created_at").toString();
                    item.updated_at = itemObj.value("updated_at").toString();
                    item.is_official_tag = itemObj.value("is_official_tag").toBool();
                    item.is_expert_tag = itemObj.value("is_expert_tag").toBool();
                    item.is_pinned = itemObj.value("is_pinned").toBool();
                    item.pinned_at = itemObj.value("pinned_at").toString();
                    item.is_collected = itemObj.value("is_collected").toBool();
                    item.is_liked = itemObj.value("is_liked").toBool();
                    item.is_disliked = itemObj.value("is_disliked").toBool();

                    // author
                    if (itemObj.contains("author") && itemObj["author"].isObject()) {
                        QJsonObject t_author = itemObj["author"].toObject();
                        item.author.user_id = t_author.value("user_id").toInt();
                        item.author.username = t_author.value("username").toString();
                        item.author.avatar = t_author.value("avatar").toString();
                        item.author.nickname = t_author.value("nickname").toString();
                        item.author.level = t_author.value("level").toInt();

                        if (t_author.contains("roles") && t_author["roles"].isArray()) {
                            QJsonArray rolesArr = t_author["roles"].toArray();
                            for (const QJsonValue &v : rolesArr)
                                item.author.roles.append(v.toString());
                        }
                        if (t_author.contains("titles") && t_author["titles"].isArray()) {
                            QJsonArray titlesArr = t_author["titles"].toArray();
                            for (const QJsonValue &v : titlesArr)
                                item.author.titles.append(v.toString());
                        }
                    }

                    // drive_version / firmware_version / description
                    item.drive_version = itemObj.value("drive_version").toString();
                    item.firmware_version = itemObj.value("firmware_version").toString();
                    item.description = itemObj.value("description").toString();

                    // comments
                    if (itemObj.contains("comments") && itemObj["comments"].isArray()) {
                        QJsonArray t_comments = itemObj["comments"].toArray();
                        for (const QJsonValue &t_cv : t_comments) {
                            if (!t_cv.isObject()) continue;
                            QJsonObject t_co = t_cv.toObject();
                            GetMyConfigurationListResponse::ListItem::CommentItem t_ci;
                            t_ci.id = t_co.value("id").toInt();
                            t_ci.comment_text = t_co.value("comment_text").toString();
                            t_ci.comment_text_en = t_co.value("comment_text_en").toString();
                            t_ci.count = t_co.value("count").toInt();
                            t_ci.is_clicked = t_co.value("is_clicked").toBool();
                            item.comments.append(t_ci);
                        }
                    }

                    responseData.data.list.append(item);
                }
            }

            responseData.data.total = dataObj.value("total").toInt();
            responseData.data.page = dataObj.value("page").toInt();
            responseData.data.page_size = dataObj.value("page_size").toInt();

        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "我的配置列表 应答信息解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::GetMyConfigurationListRequestToJson(const GetMyConfigurationListRequest &req)
{
    QJsonObject obj;

    if (!req.status.isEmpty())
        obj["status"] = req.status;
    if (!req.language.isEmpty())
        obj["language"] = req.language;
    if (!req.device_name.isEmpty())
        obj["device_name"] = req.device_name;
    if (!req.device_type.isEmpty())
        obj["device_type"] = req.device_type;
    if (req.page != 1)
        obj["page"] = req.page;
    if (req.page_size != 20)
        obj["page_size"] = req.page_size;

    return obj;
}

bool DeSheng::buildGetMyConfigurationListQuery(const GetMyConfigurationListRequest &req,
                                               QUrlQuery &query,
                                               QString &error)
{
    Q_UNUSED(error);

    if (!req.status.isEmpty())
        query.addQueryItem("status", req.status);
    if (!req.language.isEmpty())
        query.addQueryItem("language", req.language);
    if (!req.device_name.isEmpty())
        query.addQueryItem("device_name", req.device_name);
    if (!req.device_type.isEmpty())
        query.addQueryItem("device_type", req.device_type);
    if (req.page != 1)
        query.addQueryItem("page", QString::number(req.page));
    if (req.page_size != 20)
        query.addQueryItem("page_size", QString::number(req.page_size));

    return true;
}

bool DeSheng::ProcessGetTargetUserConfigurationsResult(
    GetTargetUserConfigurationsResponse &responseData, QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.list.clear();
    responseData.data.total = 0;
    responseData.data.page = 0;
    responseData.data.page_size = 0;

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "指定用户配置列表 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        // code
        if (root.contains("code") && root["code"].isString()) {
            responseData.code = root["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        // message
        if (root.contains("message") && root["message"].isString()) {
            responseData.message = root["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

        // data
        if (root.contains("data") && root["data"].isObject()) {
            QJsonObject dataObj = root["data"].toObject();

            // list
            if (dataObj.contains("list") && dataObj["list"].isArray()) {
                QJsonArray listArr = dataObj["list"].toArray();
                for (const QJsonValue &val : listArr) {
                    if (!val.isObject())
                        continue;
                    QJsonObject itemObj = val.toObject();

                    GetTargetUserConfigurationsResponse::ListItem item;
                    item.id = itemObj.value("id").toInt();

                    // author
                    if (itemObj.contains("author") && itemObj["author"].isObject()) {
                        QJsonObject authorObj = itemObj["author"].toObject();
                        item.author.user_id = authorObj.value("user_id").toInt();
                        item.author.username = authorObj.value("username").toString();
                        item.author.avatar = authorObj.value("avatar").toString();
                        item.author.nickname = authorObj.value("nickname").toString();
                        item.author.level = authorObj.value("level").toInt();

                        if (authorObj.contains("roles") && authorObj["roles"].isArray()) {
                            QJsonArray rolesArr = authorObj["roles"].toArray();
                            for (const QJsonValue &v : rolesArr)
                                item.author.roles.append(v.toString());
                        }
                        if (authorObj.contains("titles") && authorObj["titles"].isArray()) {
                            QJsonArray titlesArr = authorObj["titles"].toArray();
                            for (const QJsonValue &v : titlesArr)
                                item.author.titles.append(v.toString());
                        }
                    }

                    item.device_id = itemObj.value("device_id").toString();
                    item.drive_version = itemObj.value("drive_version").toString();
                    item.firmware_version = itemObj.value("firmware_version").toString();
                    item.device_name = itemObj.value("device_name").toString();
                    item.device_type = itemObj.value("device_type").toString();
                    item.title = itemObj.value("title").toString();
                    item.description = itemObj.value("description").toString();
                    item.language = itemObj.value("language").toString();
                    item.visibility = itemObj.value("visibility").toString();

                    // user_tags
                    if (itemObj.contains("user_tags") && itemObj["user_tags"].isArray()) {
                        QJsonArray tagsArr = itemObj["user_tags"].toArray();
                        for (const QJsonValue &tag : tagsArr) {
                            item.user_tags.append(tag.toString());
                        }
                    }

                    item.download_count = itemObj.value("download_count").toInt();
                    item.collect_count = itemObj.value("collect_count").toInt();
                    item.like_count = itemObj.value("like_count").toInt();
                    item.share_count = itemObj.value("share_count").toInt();
                    item.dislike_count = itemObj.value("dislike_count").toInt();
                    item.like_dislike_score = itemObj.value("like_dislike_score").toInt();
                    item.hot_score = itemObj.value("hot_score").toInt();
                    item.status = itemObj.value("status").toString();
                    item.created_at = itemObj.value("created_at").toString();
                    item.updated_at = itemObj.value("updated_at").toString();
                    item.is_official_tag = itemObj.value("is_official_tag").toBool();
                    item.is_expert_tag = itemObj.value("is_expert_tag").toBool();
                    item.is_pinned = itemObj.value("is_pinned").toBool();
                    item.pinned_at = itemObj.value("pinned_at").toString();
                    item.is_collected = itemObj.value("is_collected").toBool();
                    item.is_liked = itemObj.value("is_liked").toBool();
                    item.is_disliked = itemObj.value("is_disliked").toBool();

                    // comments
                    if (itemObj.contains("comments") && itemObj["comments"].isArray()) {
                        QJsonArray t_comments = itemObj["comments"].toArray();
                        for (const QJsonValue &t_cv : t_comments) {
                            if (!t_cv.isObject()) continue;
                            QJsonObject t_co = t_cv.toObject();
                            GetTargetUserConfigurationsResponse::ListItem::Comment t_ci;
                            t_ci.id = t_co.value("id").toInt();
                            t_ci.comment_text = t_co.value("comment_text").toString();
                            t_ci.comment_text_en = t_co.value("comment_text_en").toString();
                            t_ci.count = t_co.value("count").toInt();
                            t_ci.is_clicked = t_co.value("is_clicked").toBool();
                            item.comments.append(t_ci);
                        }
                    }

                    responseData.data.list.append(item);
                }
            }

            responseData.data.total = dataObj.value("total").toInt();
            responseData.data.page = dataObj.value("page").toInt();
            responseData.data.page_size = dataObj.value("page_size").toInt();
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

QJsonObject DeSheng::GetTargetUserConfigurationsRequestToJson(
    const GetTargetUserConfigurationsRequest &req)
{
    QJsonObject obj;

    if (!req.keyword.isEmpty())
        obj["keyword"] = req.keyword;
    if (!req.device_name.isEmpty())
        obj["device_name"] = req.device_name;
    if (!req.device_type.isEmpty())
        obj["device_type"] = req.device_type;
    if (!req.language.isEmpty())
        obj["language"] = req.language;
    if (!req.sort.isEmpty())
        obj["sort"] = req.sort;
    if (req.page != 1)
        obj["page"] = req.page;
    if (req.page_size != 20)
        obj["page_size"] = req.page_size;

    return obj;
}

bool DeSheng::buildGetTargetUserConfigurationsQuery(const GetTargetUserConfigurationsRequest &req,
                                                    QUrlQuery &query,
                                                    QString &error)
{
    Q_UNUSED(error);

    if (!req.keyword.isEmpty())
        query.addQueryItem("keyword", req.keyword);
    if (!req.device_name.isEmpty())
        query.addQueryItem("device_name", req.device_name);
    if (!req.device_type.isEmpty())
        query.addQueryItem("device_type", req.device_type);
    if (!req.language.isEmpty())
        query.addQueryItem("language", req.language);
    if (!req.sort.isEmpty())
        query.addQueryItem("sort", req.sort);
    if (req.page != 1)
        query.addQueryItem("page", QString::number(req.page));
    if (req.page_size != 20)
        query.addQueryItem("page_size", QString::number(req.page_size));

    return true;
}

bool DeSheng::ProcessUpdateUserConfigResult(UpdateUserConfigResponse &responseData,
                                            QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "更新配置 应答信息 JSON文档为空";
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

            // author
            if (dataObj.contains("author") && dataObj["author"].isObject()) {
                QJsonObject authorObj = dataObj["author"].toObject();
                responseData.data.author.user_id = authorObj.value("user_id").toInt();
                responseData.data.author.username = authorObj.value("username").toString();
                responseData.data.author.avatar = authorObj.value("avatar").toString();
                responseData.data.author.nickname = authorObj.value("nickname").toString();
                responseData.data.author.level = authorObj.value("level").toInt();

                if (authorObj.contains("roles") && authorObj["roles"].isArray()) {
                    QJsonArray rolesArr = authorObj["roles"].toArray();
                    for (const QJsonValue &v : rolesArr)
                        responseData.data.author.roles.append(v.toString());
                }
                if (authorObj.contains("titles") && authorObj["titles"].isArray()) {
                    QJsonArray titlesArr = authorObj["titles"].toArray();
                    for (const QJsonValue &v : titlesArr)
                        responseData.data.author.titles.append(v.toString());
                }
            }

            responseData.data.device_id = dataObj.value("device_id").toString();
            responseData.data.drive_version = dataObj.value("drive_version").toString();
            responseData.data.firmware_version = dataObj.value("firmware_version").toString();
            responseData.data.device_name = dataObj.value("device_name").toString();
            responseData.data.device_type = dataObj.value("device_type").toString();
            responseData.data.title = dataObj.value("title").toString();
            responseData.data.description = dataObj.value("description").toString();
            responseData.data.language = dataObj.value("language").toString();
            responseData.data.visibility = dataObj.value("visibility").toString();
            responseData.data.config_url = dataObj.value("config_url").toString();

            if (dataObj.contains("user_tags") && dataObj["user_tags"].isArray()) {
                QJsonArray tagsArr = dataObj["user_tags"].toArray();
                for (const QJsonValue &val : tagsArr) {
                    responseData.data.user_tags.append(val.toString());
                }
            }

            responseData.data.download_count = dataObj.value("download_count").toInt();
            responseData.data.collect_count = dataObj.value("collect_count").toInt();
            responseData.data.like_count = dataObj.value("like_count").toInt();
            responseData.data.share_count = dataObj.value("share_count").toInt();
            responseData.data.dislike_count = dataObj.value("dislike_count").toInt();
            responseData.data.like_dislike_score = dataObj.value("like_dislike_score").toInt();
            responseData.data.hot_score = dataObj.value("hot_score").toInt();
            responseData.data.status = dataObj.value("status").toString();
            responseData.data.created_at = dataObj.value("created_at").toString();
            responseData.data.updated_at = dataObj.value("updated_at").toString();
            responseData.data.published_at = dataObj.value("published_at").toString();
            responseData.data.modified_at = dataObj.value("modified_at").toString();
            responseData.data.is_official_tag = dataObj.value("is_official_tag").toBool();
            responseData.data.is_expert_tag = dataObj.value("is_expert_tag").toBool();
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "更新配置 解析异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::UpdateUserConfigRequestToJson(const UpdateUserConfigRequest &req)
{
    QJsonObject obj;
    if (!req.title.isEmpty())
        obj["title"] = req.title;
    if (!req.description.isEmpty())
        obj["description"] = req.description;
    if (!req.language.isEmpty())
        obj["language"] = req.language;
    if (!req.visibility.isEmpty())
        obj["visibility"] = req.visibility;
    if (!req.config_url.isEmpty())
        obj["config_url"] = req.config_url;
    if (!req.user_tags.isEmpty()) {
        QJsonArray tagsArr;
        for (const QString &tag : req.user_tags)
            tagsArr.append(tag);
        obj["user_tags"] = tagsArr;
    }
    return obj;
}

bool DeSheng::buildUpdateUserConfigQuery(const UpdateUserConfigRequest &req,
                                          QUrlQuery &query,
                                          QString &error)
{
    Q_UNUSED(req);
    Q_UNUSED(query);
    Q_UNUSED(error);
    return true;
}


bool DeSheng::ProcessDeleteUserConfigResult(DeleteUserConfigResponse &responseData,
                                            QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "删除配置 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        // code
        if (root.contains("code") && root["code"].isString()) {
            responseData.code = root["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        // message
        if (root.contains("message") && root["message"].isString()) {
            responseData.message = root["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

        // data 为 null，不需要解析

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::DeleteUserConfigRequestToJson(const DeleteUserConfigRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject(); // DELETE 请求无 Body
}

bool DeSheng::buildDeleteUserConfigQuery(const DeleteUserConfigRequest &req,
                                         QUrlQuery &query,
                                         QString &error)
{
    Q_UNUSED(query);

    if (req.id <= 0) {
        error = "配置 ID 不能为空或小于等于 0";
        return false;
    }

    // id 在 URL 路径中，无 query 参数
    return true;
}

bool DeSheng::ProcessDownloadTargetConfigurationResult(
    DownloadTargetConfigurationResponse &responseData, QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.config_url.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "下载配置 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        // code
        if (root.contains("code") && root["code"].isString()) {
            responseData.code = root["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        // message
        if (root.contains("message") && root["message"].isString()) {
            responseData.message = root["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

        // data
        if (root.contains("data") && root["data"].isObject()) {
            QJsonObject dataObj = root["data"].toObject();
            responseData.data.config_url = dataObj.value("config_url").toString();
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

// 2. 请求转 JSON（GET 请求无 Body，返回空）
QJsonObject DeSheng::DownloadTargetConfigurationRequestToJson(
    const DownloadTargetConfigurationRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject(); // GET 请求无 Body
}

// 3. 构建查询参数（id 在 URL 路径中，无 query 参数）
bool DeSheng::buildDownloadTargetConfigurationQuery(const DownloadTargetConfigurationRequest &req,
                                                    QUrlQuery &query,
                                                    QString &error)
{
    Q_UNUSED(query);

    if (req.id <= 0) {
        error = "配置 ID 不能为空或小于等于 0";
        return false;
    }

    // id 在 URL 路径中，无 query 参数
    return true;
}

/**
 * @brief 处理通过分享码下载配置接口的响应结果
 * @param responseData 响应数据
 * @param jsonDocument JSON文档
 * @return true 成功, false 失败
 */
bool DeSheng::ProcessDownloadConfigByShareCodeResult(DownloadConfigByShareCodeResponse &responseData,
                                                     QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.config_url.clear();

    // 校验JSON文档是否为空
    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "通过分享码下载配置 应答信息 JSON文档为空";
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
            responseData.data.config_url = dataObj.value("config_url").toString();
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

/**
 * @brief 将通过分享码下载配置请求结构体转换为JSON对象
 * @param req 请求结构体
 * @return QJsonObject（GET请求无Body，返回空对象）
 */
QJsonObject DeSheng::DownloadConfigByShareCodeRequestToJson(
    const DownloadConfigByShareCodeRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

/**
 * @brief 构建通过分享码下载配置接口的URL查询参数
 * @param req 请求结构体
 * @param query URL查询参数
 * @param error 错误信息
 * @return true 成功, false 失败
 */
bool DeSheng::buildDownloadConfigByShareCodeQuery(const DownloadConfigByShareCodeRequest &req,
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

/**
 * @brief 处理分享配置接口的响应结果
 * @param responseData 响应数据
 * @param jsonDocument JSON文档
 * @return true 成功, false 失败
 */
bool DeSheng::ProcessShareConfigurationResult(ShareConfigurationResponse &responseData,
                                              QJsonDocument &jsonDocument)
{
    /// 清空旧数据
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.share_code.clear();
    responseData.data.share_count = 0;

    /// 校验JSON文档是否为空
    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "分享配置 应答信息 JSON文档为空";
        return false;
    }

    try {
        /// 校验根节点是否为对象
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        /// 解析 code
        if (root.contains("code") && root["code"].isString()) {
            responseData.code = root["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        /// 解析 message
        if (root.contains("message") && root["message"].isString()) {
            responseData.message = root["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

        /// 解析 data
        if (root.contains("data") && root["data"].isObject()) {
            QJsonObject dataObj = root["data"].toObject();
            responseData.data.share_code = dataObj.value("share_code").toString();
            responseData.data.share_count = dataObj.value("share_count").toInt();
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

/**
 * @brief 将分享配置请求结构体转换为JSON对象
 * @param req 请求结构体
 * @return QJsonObject（POST请求无Body，返回空对象）
 */
QJsonObject DeSheng::ShareConfigurationRequestToJson(const ShareConfigurationRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

/**
 * @brief 构建分享配置接口的URL查询参数
 * @param req 请求结构体
 * @param query URL查询参数
 * @param error 错误信息
 * @return true 成功, false 失败
 */
bool DeSheng::buildShareConfigurationQuery(const ShareConfigurationRequest &req,
                                           QUrlQuery &query,
                                           QString &error)
{
    Q_UNUSED(query);

    /// 校验配置ID
    if (req.id <= 0) {
        error = "配置 ID 不能为空或小于等于 0";
        return false;
    }

    return true;
}

/// 处理点赞配置接口的响应结果
bool DeSheng::ProcessLikeConfigurationResult(LikeConfigurationResponse &responseData,
                                             QJsonDocument &jsonDocument)
{
    // 清空旧数据
    responseData.code.clear();
    responseData.message.clear();

    // 校验JSON文档是否为空
    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "点赞配置 应答信息 JSON文档为空";
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

        // data 为 null，无需解析

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析异常:" << e.what();
        return false;
    }
    return true;
}

/// 将点赞配置请求结构体转换为JSON对象
QJsonObject DeSheng::LikeConfigurationRequestToJson(const LikeConfigurationRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

/// 构建点赞配置接口的URL查询参数
bool DeSheng::buildLikeConfigurationQuery(const LikeConfigurationRequest &req,
                                          QUrlQuery &query,
                                          QString &error)
{
    Q_UNUSED(query);

    // 校验配置ID
    if (req.id <= 0) {
        error = "配置 ID 不能为空或小于等于 0";
        return false;
    }

    return true;
}

/// 处理取消点赞配置接口的响应结果
bool DeSheng::ProcessCancelLikeConfigurationResult(CancelLikeConfigurationResponse &responseData,
                                                   QJsonDocument &jsonDocument)
{
    // 清空旧数据
    responseData.code.clear();
    responseData.message.clear();

    // 校验JSON文档是否为空
    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "取消点赞配置 应答信息 JSON文档为空";
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

        // data 为 null，无需解析

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析异常:" << e.what();
        return false;
    }
    return true;
}

/// 将取消点赞配置请求结构体转换为JSON对象
QJsonObject DeSheng::CancelLikeConfigurationRequestToJson(const CancelLikeConfigurationRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

/// 构建取消点赞配置接口的URL查询参数
bool DeSheng::buildCancelLikeConfigurationQuery(const CancelLikeConfigurationRequest &req,
                                                QUrlQuery &query,
                                                QString &error)
{
    Q_UNUSED(query);

    // 校验配置ID
    if (req.id <= 0) {
        error = "配置 ID 不能为空或小于等于 0";
        return false;
    }

    return true;
}

/// 处理踩配置接口的响应结果
bool DeSheng::ProcessDislikeConfigurationResult(DislikeConfigurationResponse &responseData,
                                                QJsonDocument &jsonDocument)
{
    // 清空旧数据
    responseData.code.clear();
    responseData.message.clear();

    // 校验JSON文档是否为空
    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "踩配置 应答信息 JSON文档为空";
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

        // data 为 null，无需解析

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析异常:" << e.what();
        return false;
    }
    return true;
}

/// 将踩配置请求结构体转换为JSON对象
QJsonObject DeSheng::DislikeConfigurationRequestToJson(const DislikeConfigurationRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

/// 构建踩配置接口的URL查询参数
bool DeSheng::buildDislikeConfigurationQuery(const DislikeConfigurationRequest &req,
                                             QUrlQuery &query,
                                             QString &error)
{
    Q_UNUSED(query);

    // 校验配置ID
    if (req.id <= 0) {
        error = "配置 ID 不能为空或小于等于 0";
        return false;
    }

    return true;
}

/// 处理取消踩配置接口的响应结果
bool DeSheng::ProcessCancelDislikeConfigurationResult(
    CancelDislikeConfigurationResponse &responseData, QJsonDocument &jsonDocument)
{
    // 清空旧数据
    responseData.code.clear();
    responseData.message.clear();

    // 校验JSON文档是否为空
    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "取消踩配置 应答信息 JSON文档为空";
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

        // data 为 null，无需解析

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析异常:" << e.what();
        return false;
    }
    return true;
}

/// 将取消踩配置请求结构体转换为JSON对象
QJsonObject DeSheng::CancelDislikeConfigurationRequestToJson(
    const CancelDislikeConfigurationRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

/// 构建取消踩配置接口的URL查询参数
bool DeSheng::buildCancelDislikeConfigurationQuery(const CancelDislikeConfigurationRequest &req,
                                                   QUrlQuery &query,
                                                   QString &error)
{
    Q_UNUSED(query);

    // 校验配置ID
    if (req.id <= 0) {
        error = "配置 ID 不能为空或小于等于 0";
        return false;
    }

    return true;
}

/// 处理收藏配置接口的响应结果
bool DeSheng::ProcessCollectConfigurationResult(CollectConfigurationResponse &responseData,
                                                QJsonDocument &jsonDocument)
{
    // 清空旧数据
    responseData.code.clear();
    responseData.message.clear();

    // 校验JSON文档是否为空
    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "收藏配置 应答信息 JSON文档为空";
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

        // data 为 null，无需解析

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析异常:" << e.what();
        return false;
    }
    return true;
}

/// 将收藏配置请求结构体转换为JSON对象
QJsonObject DeSheng::CollectConfigurationRequestToJson(const CollectConfigurationRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

/// 构建收藏配置接口的URL查询参数
bool DeSheng::buildCollectConfigurationQuery(const CollectConfigurationRequest &req,
                                             QUrlQuery &query,
                                             QString &error)
{
    Q_UNUSED(query);

    // 校验配置ID
    if (req.id <= 0) {
        error = "配置 ID 不能为空或小于等于 0";
        return false;
    }

    return true;
}

/// 处理取消收藏配置接口的响应结果
bool DeSheng::ProcessCancelCollectConfigurationResult(
    CancelCollectConfigurationResponse &responseData, QJsonDocument &jsonDocument)
{
    // 清空旧数据
    responseData.code.clear();
    responseData.message.clear();

    // 校验JSON文档是否为空
    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "取消收藏配置 应答信息 JSON文档为空";
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

        // data 为 null，无需解析

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析异常:" << e.what();
        return false;
    }
    return true;
}

/// 将取消收藏配置请求结构体转换为JSON对象
QJsonObject DeSheng::CancelCollectConfigurationRequestToJson(
    const CancelCollectConfigurationRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

/// 构建取消收藏配置接口的URL查询参数
bool DeSheng::buildCancelCollectConfigurationQuery(const CancelCollectConfigurationRequest &req,
                                                   QUrlQuery &query,
                                                   QString &error)
{
    Q_UNUSED(query);

    // 校验配置ID
    if (req.id <= 0) {
        error = "配置 ID 不能为空或小于等于 0";
        return false;
    }

    return true;
}

/// 处理获取我的收藏列表接口的响应结果
bool DeSheng::ProcessGetMyCollectionListResult(GetMyCollectionListResponse &responseData,
                                               QJsonDocument &jsonDocument)
{
    // 清空旧数据
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.list.clear();
    responseData.data.total = 0;
    responseData.data.page = 0;
    responseData.data.page_size = 0;

    // 校验JSON文档是否为空
    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "我的收藏列表 应答信息 JSON文档为空";
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

            // 解析 list
            if (dataObj.contains("list") && dataObj["list"].isArray()) {
                QJsonArray listArr = dataObj["list"].toArray();
                for (const QJsonValue &val : listArr) {
                    if (!val.isObject())
                        continue;
                    QJsonObject itemObj = val.toObject();

                    GetMyCollectionListResponse::ListItem item;
                    item.id = itemObj.value("id").toInt();

                    // 解析 author
                    if (itemObj.contains("author") && itemObj["author"].isObject()) {
                        QJsonObject authorObj = itemObj["author"].toObject();
                        item.author.user_id = authorObj.value("user_id").toInt();
                        item.author.username = authorObj.value("username").toString();
                        item.author.avatar = authorObj.value("avatar").toString();
                        item.author.nickname = authorObj.value("nickname").toString();
                        item.author.level = authorObj.value("level").toInt();

                        if (authorObj.contains("roles") && authorObj["roles"].isArray()) {
                            QJsonArray rolesArr = authorObj["roles"].toArray();
                            for (const QJsonValue &v : rolesArr)
                                item.author.roles.append(v.toString());
                        }
                        if (authorObj.contains("titles") && authorObj["titles"].isArray()) {
                            QJsonArray titlesArr = authorObj["titles"].toArray();
                            for (const QJsonValue &v : titlesArr)
                                item.author.titles.append(v.toString());
                        }
                    }

                    item.device_id = itemObj.value("device_id").toString();
                    item.drive_version = itemObj.value("drive_version").toString();
                    item.firmware_version = itemObj.value("firmware_version").toString();
                    item.device_name = itemObj.value("device_name").toString();
                    item.device_type = itemObj.value("device_type").toString();
                    item.title = itemObj.value("title").toString();
                    item.description = itemObj.value("description").toString();
                    item.language = itemObj.value("language").toString();
                    item.visibility = itemObj.value("visibility").toString();

                    // 解析 user_tags
                    if (itemObj.contains("user_tags") && itemObj["user_tags"].isArray()) {
                        QJsonArray tagsArr = itemObj["user_tags"].toArray();
                        for (const QJsonValue &tag : tagsArr) {
                            item.user_tags.append(tag.toString());
                        }
                    }

                    item.download_count = itemObj.value("download_count").toInt();
                    item.collect_count = itemObj.value("collect_count").toInt();
                    item.like_count = itemObj.value("like_count").toInt();
                    item.share_count = itemObj.value("share_count").toInt();
                    item.dislike_count = itemObj.value("dislike_count").toInt();
                    item.like_dislike_score = itemObj.value("like_dislike_score").toInt();
                    item.hot_score = itemObj.value("hot_score").toInt();
                    item.status = itemObj.value("status").toString();
                    item.created_at = itemObj.value("created_at").toString();
                    item.updated_at = itemObj.value("updated_at").toString();
                    item.is_official_tag = itemObj.value("is_official_tag").toBool();
                    item.is_expert_tag = itemObj.value("is_expert_tag").toBool();
                    item.is_liked = itemObj.value("is_liked").toBool();
                    item.is_disliked = itemObj.value("is_disliked").toBool();

                    // 解析 comments
                    if (itemObj.contains("comments") && itemObj["comments"].isArray()) {
                        QJsonArray t_comments = itemObj["comments"].toArray();
                        for (const QJsonValue &t_cv : t_comments) {
                            if (!t_cv.isObject()) continue;
                            QJsonObject t_co = t_cv.toObject();
                            GetMyCollectionListResponse::ListItem::CommentItem t_ci;
                            t_ci.id = t_co.value("id").toInt();
                            t_ci.comment_text = t_co.value("comment_text").toString();
                            t_ci.comment_text_en = t_co.value("comment_text_en").toString();
                            t_ci.count = t_co.value("count").toInt();
                            t_ci.is_clicked = t_co.value("is_clicked").toBool();
                            item.comments.append(t_ci);
                        }
                    }

                    responseData.data.list.append(item);
                }
            }

            responseData.data.total = dataObj.value("total").toInt();
            responseData.data.page = dataObj.value("page").toInt();
            responseData.data.page_size = dataObj.value("page_size").toInt();
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

/// 将获取我的收藏列表请求结构体转换为JSON对象
QJsonObject DeSheng::GetMyCollectionListRequestToJson(const GetMyCollectionListRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

/// 构建获取我的收藏列表接口的URL查询参数
bool DeSheng::buildGetMyCollectionListQuery(const GetMyCollectionListRequest &req,
                                            QUrlQuery &query,
                                            QString &error)
{
    Q_UNUSED(error);

    if (!req.device_name.isEmpty())
        query.addQueryItem("device_name", req.device_name);
    if (!req.device_type.isEmpty())
        query.addQueryItem("device_type", req.device_type);
    if (req.page != 1)
        query.addQueryItem("page", QString::number(req.page));
    if (req.page_size != 20)
        query.addQueryItem("page_size", QString::number(req.page_size));

    return true;
}

/// 处理获取配置评论列表接口的响应结果
bool DeSheng::ProcessGetConfigCommentsResult(GetConfigCommentsResponse &responseData,
                                             QJsonDocument &jsonDocument)
{
    // 清空旧数据
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.clear();

    // 校验JSON文档是否为空
    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "配置评论列表 应答信息 JSON文档为空";
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

        // 解析 data（直接是数组）
        if (root.contains("data") && root["data"].isArray()) {
            QJsonArray dataArr = root["data"].toArray();
            for (const QJsonValue &val : dataArr) {
                if (!val.isObject())
                    continue;
                QJsonObject itemObj = val.toObject();

                GetConfigCommentsResponse::CommentItem item;
                item.id = itemObj.value("id").toInt();
                item.comment_text = itemObj.value("comment_text").toString();
                item.comment_text_en = itemObj.value("comment_text_en").toString();
                item.count = itemObj.value("count").toInt();
                item.is_clicked = itemObj.value("is_clicked").toBool();

                responseData.data.append(item);
            }
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

/// 将获取配置评论列表请求结构体转换为JSON对象
QJsonObject DeSheng::GetConfigCommentsRequestToJson(const GetConfigCommentsRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

/// 构建获取配置评论列表接口的URL查询参数
bool DeSheng::buildGetConfigCommentsQuery(const GetConfigCommentsRequest &req,
                                          QUrlQuery &query,
                                          QString &error)
{
    Q_UNUSED(query);

    // 校验配置ID
    if (req.id <= 0) {
        error = "配置 ID 不能为空或小于等于 0";
        return false;
    }

    return true;
}


bool DeSheng::ProcessClickCommentResult(ClickCommentResponse &responseData,
                                        QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "点击评论 应答信息 JSON文档为空";
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

        // data 为 null，无需解析

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "点击评论 应答信息解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::ClickCommentRequestToJson(const ClickCommentRequest &req)
{
    QJsonObject t_obj;
    t_obj["comment_id"] = static_cast<qint64>(req.comment_id);
    return t_obj;
}

bool DeSheng::buildClickCommentQuery(const ClickCommentRequest &req,
                                      QUrlQuery &query,
                                      QString &error)
{
    Q_UNUSED(query);

    if (req.id <= 0) {
        error = "配置 ID 不能为空或小于等于 0";
        return false;
    }

    return true;
}

bool DeSheng::ProcessCancelClickCommentResult(CancelClickCommentResponse &responseData,
                                              QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "取消点击评论 应答信息 JSON文档为空";
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

        // data 为 null，无需解析

    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "取消点击评论 应答信息解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::CancelClickCommentRequestToJson(const CancelClickCommentRequest &req)
{
    QJsonObject t_obj;
    t_obj["comment_id"] = static_cast<qint64>(req.comment_id);
    return t_obj;
}

bool DeSheng::buildCancelClickCommentQuery(const CancelClickCommentRequest &req,
                                            QUrlQuery &query,
                                            QString &error)
{
    Q_UNUSED(query);

    if (req.id <= 0) {
        error = "配置 ID 不能为空或小于等于 0";
        return false;
    }

    return true;
}

// GetMyLikeList — 我的点赞列表

bool DeSheng::ProcessGetMyLikeListResult(GetMyLikeListResponse &responseData,
                                         QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.list.clear();
    responseData.data.total = 0;
    responseData.data.page = 0;
    responseData.data.page_size = 0;

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "我的点赞列表 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        if (root.contains("code") && root["code"].isString()) {
            responseData.code = root["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        if (root.contains("message") && root["message"].isString()) {
            responseData.message = root["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

        if (root.contains("data") && root["data"].isObject()) {
            QJsonObject dataObj = root["data"].toObject();

            if (dataObj.contains("list") && dataObj["list"].isArray()) {
                QJsonArray listArr = dataObj["list"].toArray();
                for (const QJsonValue &val : listArr) {
                    if (!val.isObject()) continue;
                    QJsonObject itemObj = val.toObject();

                    GetMyLikeListResponse::ListItem item;
                    item.id = itemObj.value("id").toInt();

                    if (itemObj.contains("author") && itemObj["author"].isObject()) {
                        QJsonObject authorObj = itemObj["author"].toObject();
                        item.author.user_id = authorObj.value("user_id").toInt();
                        item.author.username = authorObj.value("username").toString();
                        item.author.avatar = authorObj.value("avatar").toString();
                        item.author.nickname = authorObj.value("nickname").toString();
                        item.author.level = authorObj.value("level").toInt();

                        if (authorObj.contains("roles") && authorObj["roles"].isArray()) {
                            QJsonArray rolesArr = authorObj["roles"].toArray();
                            for (const QJsonValue &v : rolesArr)
                                item.author.roles.append(v.toString());
                        }
                        if (authorObj.contains("titles") && authorObj["titles"].isArray()) {
                            QJsonArray titlesArr = authorObj["titles"].toArray();
                            for (const QJsonValue &v : titlesArr)
                                item.author.titles.append(v.toString());
                        }
                    }

                    item.device_id = itemObj.value("device_id").toString();
                    item.drive_version = itemObj.value("drive_version").toString();
                    item.firmware_version = itemObj.value("firmware_version").toString();
                    item.device_name = itemObj.value("device_name").toString();
                    item.device_type = itemObj.value("device_type").toString();
                    item.title = itemObj.value("title").toString();
                    item.description = itemObj.value("description").toString();
                    item.language = itemObj.value("language").toString();
                    item.visibility = itemObj.value("visibility").toString();

                    if (itemObj.contains("user_tags") && itemObj["user_tags"].isArray()) {
                        QJsonArray tagsArr = itemObj["user_tags"].toArray();
                        for (const QJsonValue &tag : tagsArr)
                            item.user_tags.append(tag.toString());
                    }

                    item.download_count = itemObj.value("download_count").toInt();
                    item.collect_count = itemObj.value("collect_count").toInt();
                    item.like_count = itemObj.value("like_count").toInt();
                    item.share_count = itemObj.value("share_count").toInt();
                    item.dislike_count = itemObj.value("dislike_count").toInt();
                    item.like_dislike_score = itemObj.value("like_dislike_score").toInt();
                    item.hot_score = itemObj.value("hot_score").toInt();
                    item.status = itemObj.value("status").toString();
                    item.created_at = itemObj.value("created_at").toString();
                    item.updated_at = itemObj.value("updated_at").toString();
                    item.is_official_tag = itemObj.value("is_official_tag").toBool();
                    item.is_expert_tag = itemObj.value("is_expert_tag").toBool();
                    item.is_collected = itemObj.value("is_collected").toBool();
                    item.is_liked = itemObj.value("is_liked").toBool();
                    item.is_disliked = itemObj.value("is_disliked").toBool();

                    if (itemObj.contains("comments") && itemObj["comments"].isArray()) {
                        QJsonArray t_comments = itemObj["comments"].toArray();
                        for (const QJsonValue &t_cv : t_comments) {
                            if (!t_cv.isObject()) continue;
                            QJsonObject t_co = t_cv.toObject();
                            GetMyLikeListResponse::ListItem::CommentItem t_ci;
                            t_ci.id = t_co.value("id").toInt();
                            t_ci.comment_text = t_co.value("comment_text").toString();
                            t_ci.comment_text_en = t_co.value("comment_text_en").toString();
                            t_ci.count = t_co.value("count").toInt();
                            t_ci.is_clicked = t_co.value("is_clicked").toBool();
                            item.comments.append(t_ci);
                        }
                    }

                    responseData.data.list.append(item);
                }
            }

            responseData.data.total = dataObj.value("total").toInt();
            responseData.data.page = dataObj.value("page").toInt();
            responseData.data.page_size = dataObj.value("page_size").toInt();
        } else {
            qDebug() << "data 数据不存在 或 类型异常";
            return false;
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__
                 << "我的点赞列表 应答信息解析时 JSON数据发生异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::GetMyLikeListRequestToJson(const GetMyLikeListRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildGetMyLikeListQuery(const GetMyLikeListRequest &req,
                                      QUrlQuery &query,
                                      QString &error)
{
    if (!req.device_name.isEmpty())
        query.addQueryItem("device_name", req.device_name);
    if (!req.device_type.isEmpty())
        query.addQueryItem("device_type", req.device_type);
    if (req.page != 1)
        query.addQueryItem("page", QString::number(req.page));
    if (req.page_size != 20)
        query.addQueryItem("page_size", QString::number(req.page_size));

    Q_UNUSED(error);
    return true;
}

// ──────────────────────────────────────── 钉选 ────────────────────────────────────────

/// 处理获取钉选数量接口的响应结果
bool DeSheng::ProcessGetPinnedCountResult(GetPinnedCountResponse &responseData,
                                           QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.count = 0;
    responseData.data.limit = 0;

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "获取钉选数量 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        if (root.contains("code") && root["code"].isString()) {
            responseData.code = root["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        if (root.contains("message") && root["message"].isString()) {
            responseData.message = root["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

        if (root.contains("data") && root["data"].isObject()) {
            QJsonObject dataObj = root["data"].toObject();
            responseData.data.count = dataObj.value("count").toInt();
            responseData.data.limit = dataObj.value("limit").toInt();
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

QJsonObject DeSheng::GetPinnedCountRequestToJson(const GetPinnedCountRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildGetPinnedCountQuery(const GetPinnedCountRequest &req,
                                        QUrlQuery &query,
                                        QString &error)
{
    if (req.device_type.isEmpty()) {
        error = "device_type 不能为空";
        return false;
    }
    query.addQueryItem("device_type", req.device_type);
    return true;
}

/// 处理钉选配置接口的响应结果
bool DeSheng::ProcessPinConfigurationResult(PinConfigurationResponse &responseData,
                                             QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "钉选配置 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        if (root.contains("code") && root["code"].isString()) {
            responseData.code = root["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        if (root.contains("message") && root["message"].isString()) {
            responseData.message = root["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::PinConfigurationRequestToJson(const PinConfigurationRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildPinConfigurationQuery(const PinConfigurationRequest &req,
                                          QUrlQuery &query,
                                          QString &error)
{
    Q_UNUSED(query);
    if (req.id <= 0) {
        error = "配置 ID 不能为空或小于等于 0";
        return false;
    }
    return true;
}

/// 处理取消钉选配置接口的响应结果
bool DeSheng::ProcessCancelPinConfigurationResult(CancelPinConfigurationResponse &responseData,
                                                   QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "取消钉选配置 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        if (root.contains("code") && root["code"].isString()) {
            responseData.code = root["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        if (root.contains("message") && root["message"].isString()) {
            responseData.message = root["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::CancelPinConfigurationRequestToJson(const CancelPinConfigurationRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildCancelPinConfigurationQuery(const CancelPinConfigurationRequest &req,
                                                QUrlQuery &query,
                                                QString &error)
{
    Q_UNUSED(query);
    if (req.id <= 0) {
        error = "配置 ID 不能为空或小于等于 0";
        return false;
    }
    return true;
}

// ──────────────────────────────────────── 管理端配置 ────────────────────────────────────────

/// 处理管理端获取配置列表接口的响应结果
bool DeSheng::ProcessAdminGetUserConfigsResult(AdminGetUserConfigsResponse &responseData,
                                                QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.list.clear();
    responseData.data.total = 0;
    responseData.data.page = 0;
    responseData.data.page_size = 0;

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端获取配置列表 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        if (root.contains("code") && root["code"].isString()) {
            responseData.code = root["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        if (root.contains("message") && root["message"].isString()) {
            responseData.message = root["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

        if (root.contains("data") && root["data"].isObject()) {
            QJsonObject dataObj = root["data"].toObject();

            if (dataObj.contains("list") && dataObj["list"].isArray()) {
                QJsonArray listArr = dataObj["list"].toArray();
                for (const QJsonValue &val : listArr) {
                    if (!val.isObject()) continue;
                    QJsonObject itemObj = val.toObject();

                    AdminGetUserConfigsResponse::ListItem item;
                    item.id = itemObj.value("id").toInt();

                    if (itemObj.contains("author") && itemObj["author"].isObject()) {
                        QJsonObject authorObj = itemObj["author"].toObject();
                        item.author.user_id = authorObj.value("user_id").toInt();
                        item.author.username = authorObj.value("username").toString();
                        item.author.avatar = authorObj.value("avatar").toString();
                        item.author.nickname = authorObj.value("nickname").toString();
                        item.author.level = authorObj.value("level").toInt();

                        if (authorObj.contains("roles") && authorObj["roles"].isArray()) {
                            QJsonArray rolesArr = authorObj["roles"].toArray();
                            for (const QJsonValue &v : rolesArr)
                                item.author.roles.append(v.toString());
                        }
                        if (authorObj.contains("titles") && authorObj["titles"].isArray()) {
                            QJsonArray titlesArr = authorObj["titles"].toArray();
                            for (const QJsonValue &v : titlesArr)
                                item.author.titles.append(v.toString());
                        }
                    }

                    item.device_id = itemObj.value("device_id").toString();
                    item.drive_version = itemObj.value("drive_version").toString();
                    item.firmware_version = itemObj.value("firmware_version").toString();
                    item.device_name = itemObj.value("device_name").toString();
                    item.device_type = itemObj.value("device_type").toString();
                    item.title = itemObj.value("title").toString();
                    item.description = itemObj.value("description").toString();
                    item.language = itemObj.value("language").toString();
                    item.visibility = itemObj.value("visibility").toString();

                    if (itemObj.contains("user_tags") && itemObj["user_tags"].isArray()) {
                        QJsonArray tagsArr = itemObj["user_tags"].toArray();
                        for (const QJsonValue &tag : tagsArr)
                            item.user_tags.append(tag.toString());
                    }

                    item.download_count = itemObj.value("download_count").toInt();
                    item.collect_count = itemObj.value("collect_count").toInt();
                    item.like_count = itemObj.value("like_count").toInt();
                    item.share_count = itemObj.value("share_count").toInt();
                    item.hot_score = itemObj.value("hot_score").toInt();
                    item.config_url = itemObj.value("config_url").toString();
                    item.status = itemObj.value("status").toString();
                    item.created_at = itemObj.value("created_at").toString();
                    item.updated_at = itemObj.value("updated_at").toString();
                    item.is_official_tag = itemObj.value("is_official_tag").toBool();
                    item.is_expert_tag = itemObj.value("is_expert_tag").toBool();

                    responseData.data.list.append(item);
                }
            }

            responseData.data.total = dataObj.value("total").toInt();
            responseData.data.page = dataObj.value("page").toInt();
            responseData.data.page_size = dataObj.value("page_size").toInt();
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

QJsonObject DeSheng::AdminGetUserConfigsRequestToJson(const AdminGetUserConfigsRequest &req)
{
    QJsonObject obj;
    if (req.user_id != 0)
        obj["user_id"] = static_cast<qint64>(req.user_id);
    if (!req.status.isEmpty())
        obj["status"] = req.status;
    if (!req.visibility.isEmpty())
        obj["visibility"] = req.visibility;
    if (!req.language.isEmpty())
        obj["language"] = req.language;
    if (!req.device_type.isEmpty())
        obj["device_type"] = req.device_type;
    if (!req.device_name.isEmpty())
        obj["device_name"] = req.device_name;
    if (!req.keyword.isEmpty())
        obj["keyword"] = req.keyword;
    if (!req.username.isEmpty())
        obj["username"] = req.username;
    if (!req.user_tag.isEmpty())
        obj["user_tag"] = req.user_tag;
    if (!req.sort.isEmpty())
        obj["sort"] = req.sort;
    if (!req.start_time.isEmpty())
        obj["start_time"] = req.start_time;
    if (!req.end_time.isEmpty())
        obj["end_time"] = req.end_time;
    if (req.is_official_tag)
        obj["is_official_tag"] = req.is_official_tag;
    if (req.is_expert_tag)
        obj["is_expert_tag"] = req.is_expert_tag;
    if (req.page != 1)
        obj["page"] = req.page;
    if (req.page_size != 20)
        obj["page_size"] = req.page_size;
    return obj;
}

bool DeSheng::buildAdminGetUserConfigsQuery(const AdminGetUserConfigsRequest &req,
                                             QUrlQuery &query,
                                             QString &error)
{
    Q_UNUSED(error);

    if (req.user_id != 0)
        query.addQueryItem("user_id", QString::number(req.user_id));
    if (!req.status.isEmpty())
        query.addQueryItem("status", req.status);
    if (!req.visibility.isEmpty())
        query.addQueryItem("visibility", req.visibility);
    if (!req.language.isEmpty())
        query.addQueryItem("language", req.language);
    if (!req.device_type.isEmpty())
        query.addQueryItem("device_type", req.device_type);
    if (!req.device_name.isEmpty())
        query.addQueryItem("device_name", req.device_name);
    if (!req.keyword.isEmpty())
        query.addQueryItem("keyword", req.keyword);
    if (!req.username.isEmpty())
        query.addQueryItem("username", req.username);
    if (!req.user_tag.isEmpty())
        query.addQueryItem("user_tag", req.user_tag);
    if (!req.sort.isEmpty())
        query.addQueryItem("sort", req.sort);
    if (!req.start_time.isEmpty())
        query.addQueryItem("start_time", req.start_time);
    if (!req.end_time.isEmpty())
        query.addQueryItem("end_time", req.end_time);
    if (req.is_official_tag)
        query.addQueryItem("is_official_tag", "true");
    if (req.is_expert_tag)
        query.addQueryItem("is_expert_tag", "true");
    if (req.page != 1)
        query.addQueryItem("page", QString::number(req.page));
    if (req.page_size != 20)
        query.addQueryItem("page_size", QString::number(req.page_size));

    return true;
}

/// 处理管理端获取配置详情接口的响应结果
bool DeSheng::ProcessAdminGetUserConfigDetailResult(AdminGetUserConfigDetailResponse &responseData,
                                                     QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端获取配置详情 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        if (root.contains("code") && root["code"].isString()) {
            responseData.code = root["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        if (root.contains("message") && root["message"].isString()) {
            responseData.message = root["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

        if (root.contains("data") && root["data"].isObject()) {
            QJsonObject dataObj = root["data"].toObject();
            responseData.data.id = dataObj.value("id").toInt();

            if (dataObj.contains("author") && dataObj["author"].isObject()) {
                QJsonObject authorObj = dataObj["author"].toObject();
                responseData.data.author.user_id = authorObj.value("user_id").toInt();
                responseData.data.author.username = authorObj.value("username").toString();
                responseData.data.author.avatar = authorObj.value("avatar").toString();
                responseData.data.author.nickname = authorObj.value("nickname").toString();
                responseData.data.author.level = authorObj.value("level").toInt();

                if (authorObj.contains("roles") && authorObj["roles"].isArray()) {
                    QJsonArray rolesArr = authorObj["roles"].toArray();
                    for (const QJsonValue &v : rolesArr)
                        responseData.data.author.roles.append(v.toString());
                }
                if (authorObj.contains("titles") && authorObj["titles"].isArray()) {
                    QJsonArray titlesArr = authorObj["titles"].toArray();
                    for (const QJsonValue &v : titlesArr)
                        responseData.data.author.titles.append(v.toString());
                }
            }

            responseData.data.device_id = dataObj.value("device_id").toString();
            responseData.data.drive_version = dataObj.value("drive_version").toString();
            responseData.data.firmware_version = dataObj.value("firmware_version").toString();
            responseData.data.device_name = dataObj.value("device_name").toString();
            responseData.data.device_type = dataObj.value("device_type").toString();
            responseData.data.title = dataObj.value("title").toString();
            responseData.data.description = dataObj.value("description").toString();
            responseData.data.language = dataObj.value("language").toString();
            responseData.data.visibility = dataObj.value("visibility").toString();

            if (dataObj.contains("user_tags") && dataObj["user_tags"].isArray()) {
                QJsonArray tagsArr = dataObj["user_tags"].toArray();
                for (const QJsonValue &tag : tagsArr)
                    responseData.data.user_tags.append(tag.toString());
            }

            responseData.data.download_count = dataObj.value("download_count").toInt();
            responseData.data.collect_count = dataObj.value("collect_count").toInt();
            responseData.data.like_count = dataObj.value("like_count").toInt();
            responseData.data.share_count = dataObj.value("share_count").toInt();
            responseData.data.hot_score = dataObj.value("hot_score").toInt();
            responseData.data.config_url = dataObj.value("config_url").toString();
            responseData.data.status = dataObj.value("status").toString();
            responseData.data.created_at = dataObj.value("created_at").toString();
            responseData.data.updated_at = dataObj.value("updated_at").toString();
            responseData.data.published_at = dataObj.value("published_at").toString();
            responseData.data.modified_at = dataObj.value("modified_at").toString();
            responseData.data.is_official_tag = dataObj.value("is_official_tag").toBool();
            responseData.data.is_expert_tag = dataObj.value("is_expert_tag").toBool();

            if (dataObj.contains("comments") && dataObj["comments"].isArray()) {
                QJsonArray commentsArr = dataObj["comments"].toArray();
                for (const QJsonValue &cVal : commentsArr) {
                    if (!cVal.isObject()) continue;
                    QJsonObject cObj = cVal.toObject();
                    AdminGetUserConfigDetailResponse::Comment comment;
                    comment.id = cObj.value("id").toInt();
                    comment.comment_text = cObj.value("comment_text").toString();
                    comment.comment_text_en = cObj.value("comment_text_en").toString();
                    comment.count = cObj.value("count").toInt();
                    comment.is_clicked = cObj.value("is_clicked").toBool();
                    responseData.data.comments.append(comment);
                }
            }
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

QJsonObject DeSheng::AdminGetUserConfigDetailRequestToJson(const AdminGetUserConfigDetailRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildAdminGetUserConfigDetailQuery(const AdminGetUserConfigDetailRequest &req,
                                                  QUrlQuery &query,
                                                  QString &error)
{
    Q_UNUSED(query);
    if (req.id <= 0) {
        error = "配置 ID 不能为空或小于等于 0";
        return false;
    }
    return true;
}

/// 处理管理端更新配置状态接口的响应结果
bool DeSheng::ProcessAdminUpdateUserConfigStatusResult(AdminUpdateUserConfigStatusResponse &responseData,
                                                        QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端更新配置状态 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        if (root.contains("code") && root["code"].isString()) {
            responseData.code = root["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        if (root.contains("message") && root["message"].isString()) {
            responseData.message = root["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminUpdateUserConfigStatusRequestToJson(const AdminUpdateUserConfigStatusRequest &req)
{
    QJsonObject obj;
    obj["status"] = req.status;
    return obj;
}

bool DeSheng::buildAdminUpdateUserConfigStatusQuery(const AdminUpdateUserConfigStatusRequest &req,
                                                     QUrlQuery &query,
                                                     QString &error)
{
    Q_UNUSED(query);
    if (req.id <= 0) {
        error = "配置 ID 不能为空或小于等于 0";
        return false;
    }
    return true;
}

/// 处理管理端删除配置接口的响应结果
bool DeSheng::ProcessAdminDeleteUserConfigResult(AdminDeleteUserConfigResponse &responseData,
                                                  QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端删除配置 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        if (root.contains("code") && root["code"].isString()) {
            responseData.code = root["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        if (root.contains("message") && root["message"].isString()) {
            responseData.message = root["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminDeleteUserConfigRequestToJson(const AdminDeleteUserConfigRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildAdminDeleteUserConfigQuery(const AdminDeleteUserConfigRequest &req,
                                               QUrlQuery &query,
                                               QString &error)
{
    Q_UNUSED(query);
    if (req.id <= 0) {
        error = "配置 ID 不能为空或小于等于 0";
        return false;
    }
    return true;
}

/// 处理管理端设置配置标签接口的响应结果
bool DeSheng::ProcessAdminSetUserConfigTagsResult(AdminSetUserConfigTagsResponse &responseData,
                                                   QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端设置配置标签 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        if (root.contains("code") && root["code"].isString()) {
            responseData.code = root["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        if (root.contains("message") && root["message"].isString()) {
            responseData.message = root["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminSetUserConfigTagsRequestToJson(const AdminSetUserConfigTagsRequest &req)
{
    QJsonObject obj;
    obj["is_official_tag"] = req.is_official_tag;
    obj["is_expert_tag"] = req.is_expert_tag;
    return obj;
}

bool DeSheng::buildAdminSetUserConfigTagsQuery(const AdminSetUserConfigTagsRequest &req,
                                                QUrlQuery &query,
                                                QString &error)
{
    Q_UNUSED(query);
    if (req.id <= 0) {
        error = "配置 ID 不能为空或小于等于 0";
        return false;
    }
    return true;
}

// ──────────────────────────────────────── 管理端评论 ────────────────────────────────────────

/// 处理管理端获取评论列表接口的响应结果
bool DeSheng::ProcessAdminGetCommentsResult(AdminGetCommentsResponse &responseData,
                                             QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data.list.clear();
    responseData.data.total = 0;
    responseData.data.page = 0;
    responseData.data.page_size = 0;

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端获取评论列表 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        if (root.contains("code") && root["code"].isString()) {
            responseData.code = root["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        if (root.contains("message") && root["message"].isString()) {
            responseData.message = root["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

        if (root.contains("data") && root["data"].isObject()) {
            QJsonObject dataObj = root["data"].toObject();

            if (dataObj.contains("list") && dataObj["list"].isArray()) {
                QJsonArray listArr = dataObj["list"].toArray();
                for (const QJsonValue &val : listArr) {
                    if (!val.isObject()) continue;
                    QJsonObject itemObj = val.toObject();

                    AdminGetCommentsResponse::ListItem item;
                    item.id = itemObj.value("id").toInt();
                    item.device_id = itemObj.value("device_id").toInt();
                    item.device_name = itemObj.value("device_name").toString();
                    item.comment_text = itemObj.value("comment_text").toString();
                    item.comment_text_en = itemObj.value("comment_text_en").toString();
                    item.status = itemObj.value("status").toString();
                    item.use_count = itemObj.value("use_count").toInt();
                    item.created_at = itemObj.value("created_at").toString();

                    responseData.data.list.append(item);
                }
            }

            responseData.data.total = dataObj.value("total").toInt();
            responseData.data.page = dataObj.value("page").toInt();
            responseData.data.page_size = dataObj.value("page_size").toInt();
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

QJsonObject DeSheng::AdminGetCommentsRequestToJson(const AdminGetCommentsRequest &req)
{
    QJsonObject obj;
    if (req.device_id != 0)
        obj["device_id"] = static_cast<qint64>(req.device_id);
    if (req.page != 1)
        obj["page"] = req.page;
    if (req.page_size != 20)
        obj["page_size"] = req.page_size;
    return obj;
}

bool DeSheng::buildAdminGetCommentsQuery(const AdminGetCommentsRequest &req,
                                          QUrlQuery &query,
                                          QString &error)
{
    Q_UNUSED(error);

    if (req.device_id != 0)
        query.addQueryItem("device_id", QString::number(req.device_id));
    if (req.page != 1)
        query.addQueryItem("page", QString::number(req.page));
    if (req.page_size != 20)
        query.addQueryItem("page_size", QString::number(req.page_size));

    return true;
}

/// 处理管理端创建设备评论接口的响应结果
bool DeSheng::ProcessAdminCreateCommentResult(AdminCreateCommentResponse &responseData,
                                               QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();
    responseData.data = {};

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端创建设备评论 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        if (root.contains("code") && root["code"].isString()) {
            responseData.code = root["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        if (root.contains("message") && root["message"].isString()) {
            responseData.message = root["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }

        if (root.contains("data") && root["data"].isObject()) {
            QJsonObject dataObj = root["data"].toObject();
            responseData.data.id = dataObj.value("id").toInt();
            responseData.data.device_id = dataObj.value("device_id").toInt();
            responseData.data.comment_text = dataObj.value("comment_text").toString();
            responseData.data.comment_text_en = dataObj.value("comment_text_en").toString();
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

QJsonObject DeSheng::AdminCreateCommentRequestToJson(const AdminCreateCommentRequest &req)
{
    QJsonObject obj;
    obj["device_id"] = static_cast<qint64>(req.device_id);
    obj["comment_text"] = req.comment_text;
    if (!req.comment_text_en.isEmpty())
        obj["comment_text_en"] = req.comment_text_en;
    return obj;
}

bool DeSheng::buildAdminCreateCommentQuery(const AdminCreateCommentRequest &req,
                                            QUrlQuery &query,
                                            QString &error)
{
    Q_UNUSED(req);
    Q_UNUSED(query);
    Q_UNUSED(error);
    return true;
}

/// 处理管理端更新评论接口的响应结果
bool DeSheng::ProcessAdminUpdateCommentResult(AdminUpdateCommentResponse &responseData,
                                               QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端更新评论 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        if (root.contains("code") && root["code"].isString()) {
            responseData.code = root["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        if (root.contains("message") && root["message"].isString()) {
            responseData.message = root["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminUpdateCommentRequestToJson(const AdminUpdateCommentRequest &req)
{
    QJsonObject obj;
    obj["comment_text"] = req.comment_text;
    if (!req.comment_text_en.isEmpty())
        obj["comment_text_en"] = req.comment_text_en;
    return obj;
}

bool DeSheng::buildAdminUpdateCommentQuery(const AdminUpdateCommentRequest &req,
                                            QUrlQuery &query,
                                            QString &error)
{
    Q_UNUSED(query);
    if (req.id <= 0) {
        error = "评论 ID 不能为空或小于等于 0";
        return false;
    }
    return true;
}

/// 处理管理端删除评论接口的响应结果
bool DeSheng::ProcessAdminDeleteCommentResult(AdminDeleteCommentResponse &responseData,
                                               QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端删除评论 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        if (root.contains("code") && root["code"].isString()) {
            responseData.code = root["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        if (root.contains("message") && root["message"].isString()) {
            responseData.message = root["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminDeleteCommentRequestToJson(const AdminDeleteCommentRequest &req)
{
    Q_UNUSED(req);
    return QJsonObject();
}

bool DeSheng::buildAdminDeleteCommentQuery(const AdminDeleteCommentRequest &req,
                                            QUrlQuery &query,
                                            QString &error)
{
    Q_UNUSED(query);
    if (req.id <= 0) {
        error = "评论 ID 不能为空或小于等于 0";
        return false;
    }
    return true;
}

/// 处理管理端更新评论状态接口的响应结果
bool DeSheng::ProcessAdminUpdateCommentStatusResult(AdminUpdateCommentStatusResponse &responseData,
                                                     QJsonDocument &jsonDocument)
{
    responseData.code.clear();
    responseData.message.clear();

    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        qDebug() << __FILE__ << __FUNCTION__ << "管理端更新评论状态 应答信息 JSON文档为空";
        return false;
    }

    try {
        if (!jsonDocument.isObject()) {
            qDebug() << "JSON根节点不是对象";
            return false;
        }
        QJsonObject root = jsonDocument.object();

        if (root.contains("code") && root["code"].isString()) {
            responseData.code = root["code"].toString();
        } else {
            qDebug() << "code 数据不存在 或 类型异常";
            return false;
        }

        if (root.contains("message") && root["message"].isString()) {
            responseData.message = root["message"].toString();
        } else {
            qDebug() << "message 数据不存在 或 类型异常";
            return false;
        }
    } catch (const std::exception &e) {
        qDebug() << __FILE__ << __FUNCTION__ << "解析异常:" << e.what();
        return false;
    }
    return true;
}

QJsonObject DeSheng::AdminUpdateCommentStatusRequestToJson(const AdminUpdateCommentStatusRequest &req)
{
    QJsonObject obj;
    obj["status"] = req.status;
    return obj;
}

bool DeSheng::buildAdminUpdateCommentStatusQuery(const AdminUpdateCommentStatusRequest &req,
                                                  QUrlQuery &query,
                                                  QString &error)
{
    Q_UNUSED(query);
    if (req.id <= 0) {
        error = "评论 ID 不能为空或小于等于 0";
        return false;
    }
    return true;
}
