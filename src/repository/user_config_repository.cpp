#include "repository/user_config_repository.h"

#include <QFile>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "modules/CommunityModule/infrastructure/logger/logger.h"

namespace {
/// \brief 统一 API 错误处理 — 打日志 + emit
bool logApiError(QNetworkReply* reply, const QString& action, UserConfigRepository* repo) {
    if (reply->error() != QNetworkReply::NoError) {
        LOG_DEBUG("[Repo] {} network error: {}", action.toStdString(),
                  reply->errorString().toStdString());
        emit repo->errorOccurred(reply->errorString());
        return true;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    const QString code = obj.value("code").toString();
    if (code != "success") {
        const QString msg = obj.value("message").toString();
        LOG_DEBUG("[Repo] {} api error — code:{} msg:{}", action.toStdString(),
                  code.toStdString(), msg.toStdString());
        emit repo->errorOccurred(msg);
        return true;
    }
    return false;
}
}  // namespace
#include <QStringList>

#include "network/http_client.h"
#include "network/request_options.h"
#include "network/download_repository.h"
#include "network/avatar_cache.h"
#include "data/userConfig/user_config_api.h"

/// \brief Static helpers

/// \brief 读取 JSON 字符串数组字段（原 common_fields.h json_util 辅助函数，内联保留）
static QStringList jsonStringList(const QJsonObject& obj, const QString& key) {
  QStringList list;
  const QJsonArray arr = obj.value(key).toArray();
  for (const QJsonValue& v : arr) list.append(v.toString());
  return list;
}

static DeSheng::GetPublicConfigurationListResponse::ListItem parseUserConfigInfo(
    const QJsonObject& data) {
  DeSheng::GetPublicConfigurationListResponse::ListItem info;

  // EntityMeta — id 在 API 中是 int
  if (data.value("id").isDouble())
    info.id = static_cast<int>(data.value("id").toDouble());
  else
    info.id = data.value("id").toString().toInt();
  info.created_at = data.value("created_at").toString();
  info.updated_at = data.value("updated_at").toString();

  // AuthorBrief — 注意：在 author 子对象内
  const QJsonObject a = data.value("author").toObject();
  info.author.user_id = a.value("user_id").toInt();
  info.author.username = a.value("username").toString();
  info.author.avatar = a.value("avatar").toString();
  info.author.nickname = a.value("nickname").toString();
  info.author.level = a.value("level").toInt();
  info.author.roles = jsonStringList(a, "roles");
  info.author.titles = jsonStringList(a, "titles");

  // DeviceBinding
  info.device_id = data.value("device_id").toString();
  info.device_name = data.value("device_name").toString();
  info.device_type = data.value("device_type").toString();

  // InteractionCounts
  info.like_count = data.value("like_count").toInt();
  info.collect_count = data.value("collect_count").toInt();
  info.share_count = data.value("share_count").toInt();
  info.download_count = data.value("download_count").toInt();
  info.dislike_count = data.value("dislike_count").toInt();
  info.hot_score = data.value("hot_score").toInt();
  info.like_dislike_score = data.value("like_dislike_score").toInt();

  // Version info
  info.drive_version = data.value("drive_version").toString();
  info.firmware_version = data.value("firmware_version").toString();

  // Config-specific
  info.title = data.value("title").toString();
  info.description = data.value("description").toString();
  info.language = data.value("language").toString();

  const QJsonArray tagArr = data.value("user_tags").toArray();
  for (const QJsonValue& tv : tagArr) {
    info.user_tags.append(tv.toString());
  }

  // Step 2 适配点：ListItem 无 config_url / share_code / is_pinned / pinned_at /
  // published_at / modified_at 字段，对应字段的解析需在解析逻辑改造时处理
  info.status = data.value("status").toString();
  info.visibility = data.value("visibility").toString();
  info.is_official_tag = data.value("is_official_tag").toBool();
  info.is_expert_tag = data.value("is_expert_tag").toBool();
  info.is_collected = data.value("is_collected").toBool();
  info.is_liked = data.value("is_liked").toBool();
  info.is_disliked = data.value("is_disliked").toBool();
  // 置顶标志 — my / user 列表接口返回，公开列表接口不返回（默认 false）
  info.is_pinned = data.value("is_pinned").toBool();

  // Comments
  const QJsonArray commentArr = data.value("comments").toArray();
  for (const QJsonValue& cv : commentArr) {
    const QJsonObject co = cv.toObject();
    DeSheng::GetPublicConfigurationListResponse::Comment item;
    item.id = co.value("id").toInt();
    item.comment_text = co.value("comment_text").toString();
    item.comment_text_en = co.value("comment_text_en").toString();
    item.count = co.value("count").toInt();
    item.is_clicked = co.value("is_clicked").toBool();
    info.comments.append(item);
  }

  return info;
}

static QJsonObject userConfigCreateRequestToJson(const DeSheng::UserConfigsCreateRequest& req) {
  QJsonObject body;
  body["device_id"] = req.device_id;
  body["drive_version"] = req.drive_version;
  body["firmware_version"] = req.firmware_version;
  body["device_name"] = req.device_name;
  body["device_type"] = req.device_type;
  body["title"] = req.title;
  body["description"] = req.description;
  body["language"] = req.language;

  QJsonArray tagArr;
  for (const QString& tag : req.user_tags) {
    tagArr.append(tag);
  }
  body["user_tags"] = tagArr;

  body["config_url"] = req.config_url;
  body["visibility"] = req.visibility;
  return body;
}

static DeSheng::GetPublicConfigurationListResponse::Comment parseCommentItem(const QJsonObject& data) {
  DeSheng::GetPublicConfigurationListResponse::Comment item;
  item.id = data.value("id").toInt();
  item.comment_text = data.value("comment_text").toString();
  item.comment_text_en = data.value("comment_text_en").toString();
  item.count = data.value("count").toInt();
  item.is_clicked = data.value("is_clicked").toBool();
  return item;
}

/// \brief UserConfigRepository

UserConfigRepository::UserConfigRepository(QObject* parent) : PaginatedRepository(parent) {
  clp_avatar_cache_ = new AvatarCache(this);
  clp_download_repo_ = new DownloadRepository(this);

  connect(clp_avatar_cache_, &AvatarCache::avatarReady, this,
          &UserConfigRepository::avatarReady);
  connect(clp_download_repo_, &DownloadRepository::downloadProgress, this,
          &UserConfigRepository::downloadProgress);
  connect(clp_download_repo_, &DownloadRepository::fileSaved, this,
          &UserConfigRepository::downloadFileSaved);
  connect(clp_download_repo_, &DownloadRepository::errorOccurred, this,
          &UserConfigRepository::errorOccurred);
}

/// \brief User-facing: browsing（回调版 — 每个请求独立回调，多 Tab 不互串）
void UserConfigRepository::getPublicConfigs(int page, int pageSize, const QString& sort,
                                            const QMap<QString, QString>& filters,
                                            std::function<void(const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>&,
                                                               const PaginatedResult&)> cb,
                                            std::function<void(const QString&)> errCb) {
  QMap<QString, QString> extra = filters;
  extra["sort"] = sort;
  const QUrlQuery query = buildPageQuery(page, pageSize, extra);
  auto* reply = HttpClient::instance().get("/user-configs", RequestOptions{}.withQuery(query));
  connect(reply, &QNetworkReply::finished, this, [this, reply, cb, errCb] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      // 网络错误：有 errCb 走请求级错误通道（fetch 专用），否则 fallback 全局 errorOccurred
      if (errCb)
        errCb(reply->errorString());
      else
        emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      if (errCb)
        errCb(obj.value("message").toString());
      else
        emit errorOccurred(obj.value("message").toString());
      return;
    }
    const PaginatedResult pg = parsePaginated(obj);
    LOG_DEBUG("[Repo] getPublicConfigs total:{} items:{}", pg.total, pg.items.size());
    QList<DeSheng::GetPublicConfigurationListResponse::ListItem> list;
    for (const QJsonValue& v : pg.items) list.append(parseUserConfigInfo(v.toObject()));
    cb(list, pg);
  });
}

/// \brief User-facing: browsing（信号版 — 已弃用，保留兼容）
void UserConfigRepository::getPublicConfigs(int page, int pageSize, const QString& sort,
                                            const QMap<QString, QString>& filters) {
  QMap<QString, QString> extra = filters;
  extra["sort"] = sort;
  const QUrlQuery query = buildPageQuery(page, pageSize, extra);

  auto* reply = HttpClient::instance().get("/user-configs", RequestOptions{}.withQuery(query));

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    const PaginatedResult pageResult = parsePaginated(obj);
    LOG_DEBUG("[Repo] getPublicConfigs total:{} items:{}", pageResult.total,
              pageResult.items.size());
    for (int i = 0; i < std::min(2, pageResult.items.size()); ++i) {
      const auto& it = pageResult.items[i].toObject();
      LOG_DEBUG("[Repo]   [{}] id:{} title:{} isExpert:{} isOfficial:{}", i,
                it.value("id").toInt(), it.value("title").toString().toStdString(),
                it.value("is_expert_tag").toBool(), it.value("is_official_tag").toBool());
    }
    QList<DeSheng::GetPublicConfigurationListResponse::ListItem> list;
    for (const QJsonValue& v : pageResult.items) {
      list.append(parseUserConfigInfo(v.toObject()));
    }
    emit publicConfigsReady(list, pageResult);
  });
}

void UserConfigRepository::getConfigDetail(const QString& id) {
  auto* reply = HttpClient::instance().get("/user-configs/" + id);

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    emit configDetailReady(parseUserConfigInfo(obj.value("data").toObject()));
  });
}

void UserConfigRepository::getConfigDetail(const QString& id,
                                           std::function<void(const DeSheng::GetPublicConfigurationListResponse::ListItem&)> cb) {
  auto* reply = HttpClient::instance().get("/user-configs/" + id);

  connect(reply, &QNetworkReply::finished, this, [this, reply, cb] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    if (cb) cb(parseUserConfigInfo(obj.value("data").toObject()));
  });
}

void UserConfigRepository::downloadConfig(const QString& id) {
  auto* reply = HttpClient::instance().get("/user-configs/" + id + "/download");

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    const QString url = obj.value("data").toObject().value("config_url").toString();
    emit configDownloadUrlReady(url);
  });
}

void UserConfigRepository::downloadByShareCode(const QString& code) {
  auto* reply = HttpClient::instance().get("/user-configs/share/" + code + "/download");

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    const QString url = obj.value("data").toObject().value("config_url").toString();
    emit configDownloadUrlReady(url);
  });
}

/// \brief User-facing: CRUD

void UserConfigRepository::createConfig(const DeSheng::UserConfigsCreateRequest& req) {
  const QJsonObject body = userConfigCreateRequestToJson(req);

  auto* reply = HttpClient::instance().post("/user-configs",
                                           RequestOptions{}.withBody(QJsonDocument(body).toJson()));

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    emit configCreated(parseUserConfigInfo(obj.value("data").toObject()));
  });
}

void UserConfigRepository::updateConfig(const QString& id, const QJsonObject& req) {
  auto* reply = HttpClient::instance().put("/user-configs/" + id,
                                          RequestOptions{}.withBody(QJsonDocument(req).toJson()));

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    emit configUpdated(parseUserConfigInfo(obj.value("data").toObject()));
  });
}

void UserConfigRepository::deleteConfig(const QString& id) {
  auto* reply = HttpClient::instance().del("/user-configs/" + id);

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    emit configDeleted();
  });
}

/// \brief User-facing: personal library

void UserConfigRepository::getMyConfigs(int page, int pageSize,
                                        const QMap<QString, QString>& filters) {
  const QUrlQuery query = buildPageQuery(page, pageSize, filters);
  const QString requestKey = query.toString();
  if (cl_inflight_my_configs_.contains(requestKey))
    return;
  cl_inflight_my_configs_.insert(requestKey);

  auto* reply = HttpClient::instance().get("/user-configs/my", RequestOptions{}.withQuery(query));

  connect(reply, &QNetworkReply::finished, this, [this, reply, requestKey] {
    cl_inflight_my_configs_.remove(requestKey);
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    const PaginatedResult pageResult = parsePaginated(obj);
    QList<DeSheng::GetPublicConfigurationListResponse::ListItem> list;
    for (const QJsonValue& v : pageResult.items) {
      list.append(parseUserConfigInfo(v.toObject()));
    }
    emit myConfigsReady(list, pageResult);
  });
}

void UserConfigRepository::getUserConfigs(const QString& userId, int page, int pageSize,
                                          const QMap<QString, QString>& filters) {
  // 对齐 API 文档：GET /user-configs/user/:user_id 仅路径参数传用户 ID，无 user_id query 参数
  const QUrlQuery query = buildPageQuery(page, pageSize, filters);

  auto* reply =
      HttpClient::instance().get("/user-configs/user/" + userId, RequestOptions{}.withQuery(query));

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    const PaginatedResult pageResult = parsePaginated(obj);
    QList<DeSheng::GetPublicConfigurationListResponse::ListItem> list;
    for (const QJsonValue& v : pageResult.items) {
      list.append(parseUserConfigInfo(v.toObject()));
    }
    emit userConfigsReady(list, pageResult);
  });
}

/// \brief User-facing: interactions

void UserConfigRepository::shareConfig(const QString& id) {
  auto* reply = HttpClient::instance().post("/user-configs/" + id + "/share");

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    const QJsonObject data = obj.value("data").toObject();
    const QString shareCode = data.value("share_code").toString();
    const int shareCount = data.value("share_count").toInt();
    emit configShared(shareCode, shareCount);
  });
}

void UserConfigRepository::like(const QString& id) {
  auto* reply = HttpClient::instance().post("/user-configs/" + id + "/like");
  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (logApiError(reply, QStringLiteral("like"), this)) return;
    emit configLiked();
  });
}

void UserConfigRepository::unlike(const QString& id) {
  auto* reply = HttpClient::instance().del("/user-configs/" + id + "/like");

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    emit configUnliked();
  });
}

void UserConfigRepository::dislike(const QString& id) {
  auto* reply = HttpClient::instance().post("/user-configs/" + id + "/dislike");

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    emit configDisliked();
  });
}

void UserConfigRepository::undislike(const QString& id) {
  auto* reply = HttpClient::instance().del("/user-configs/" + id + "/dislike");

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    emit configUndisliked();
  });
}

void UserConfigRepository::collect(const QString& id) {
  auto* reply = HttpClient::instance().post("/user-configs/" + id + "/collect");

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    emit configCollected();
  });
}

void UserConfigRepository::uncollect(const QString& id) {
  auto* reply = HttpClient::instance().del("/user-configs/" + id + "/collect");

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    emit configUncollected();
  });
}

void UserConfigRepository::pin(const QString& id) {
  auto* reply = HttpClient::instance().post("/user-configs/" + id + "/pin");

  connect(reply, &QNetworkReply::finished, this, [this, reply, id] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    emit configPinned(id);
  });
}

void UserConfigRepository::unpin(const QString& id) {
  auto* reply = HttpClient::instance().del("/user-configs/" + id + "/pin");

  connect(reply, &QNetworkReply::finished, this, [this, reply, id] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    emit configUnpinned(id);
  });
}

/// \brief User-facing: collections & counts

void UserConfigRepository::getMyCollects(int page, int pageSize,
                                         const QMap<QString, QString>& filters) {
  const QUrlQuery query = buildPageQuery(page, pageSize, filters);

  auto* reply =
      HttpClient::instance().get("/user-configs/collects/my", RequestOptions{}.withQuery(query));

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    const PaginatedResult pageResult = parsePaginated(obj);
    QList<DeSheng::GetPublicConfigurationListResponse::ListItem> list;
    for (const QJsonValue& v : pageResult.items) {
      list.append(parseUserConfigInfo(v.toObject()));
    }
    emit myCollectsReady(list, pageResult);
  });
}

void UserConfigRepository::getMyLikes(int page, int pageSize,
                                      const QMap<QString, QString>& filters) {
  const QUrlQuery query = buildPageQuery(page, pageSize, filters);

  auto* reply =
      HttpClient::instance().get("/user-configs/likes/my", RequestOptions{}.withQuery(query));

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    const PaginatedResult pageResult = parsePaginated(obj);
    QList<DeSheng::GetPublicConfigurationListResponse::ListItem> list;
    for (const QJsonValue& v : pageResult.items) {
      list.append(parseUserConfigInfo(v.toObject()));
    }
    emit myLikesReady(list, pageResult);
  });
}

void UserConfigRepository::getTodayCount(const QString& deviceName) {
  QUrlQuery query;
  query.addQueryItem("device_name", deviceName);

  auto* reply =
      HttpClient::instance().get("/user-configs/today-count", RequestOptions{}.withQuery(query));

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    const int count = obj.value("data").toObject().value("today_count").toInt();
    emit todayCountReady(count);
  });
}

void UserConfigRepository::getPinnedCount(const QString& deviceType) {
  QUrlQuery query;
  query.addQueryItem("device_type", deviceType);

  auto* reply =
      HttpClient::instance().get("/user-configs/pinned-count", RequestOptions{}.withQuery(query));

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    const QJsonObject data = obj.value("data").toObject();
    const int count = data.value("count").toInt();
    const int limit = data.value("limit").toInt();
    emit pinnedCountReady(count, limit);
  });
}

/// \brief User-facing: comments

void UserConfigRepository::getComments(const QString& configId) {
  auto* reply = HttpClient::instance().get("/user-configs/" + configId + "/comments");

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    const QJsonArray arr = obj.value("data").toArray();
    QList<DeSheng::GetPublicConfigurationListResponse::Comment> list;
    for (const QJsonValue& v : arr) {
      list.append(parseCommentItem(v.toObject()));
    }
    emit commentsReady(list);
  });
}

void UserConfigRepository::clickComment(const QString& configId, const QString& commentId) {
  QJsonObject body;
  body["comment_id"] = commentId.toLongLong();
  auto* reply = HttpClient::instance().post(
      "/user-configs/" + configId + "/comments",
      RequestOptions{}.withBody(QJsonDocument(body).toJson(QJsonDocument::Compact)));

  connect(reply, &QNetworkReply::finished, this,
          [this, reply, configId, commentId] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      emit commentClickFailed(configId.toInt(), commentId.toInt());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      emit commentClickFailed(configId.toInt(), commentId.toInt());
      return;
    }
    emit commentClicked();
  });
}

void UserConfigRepository::unclickComment(const QString& configId, const QString& commentId) {
  QJsonObject body;
  body["comment_id"] = commentId.toLongLong();
  auto* reply = HttpClient::instance().del(
      "/user-configs/" + configId + "/comments",
      RequestOptions{}.withBody(QJsonDocument(body).toJson(QJsonDocument::Compact)));

  connect(reply, &QNetworkReply::finished, this,
          [this, reply, configId, commentId] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      emit commentClickFailed(configId.toInt(), commentId.toInt());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      emit commentClickFailed(configId.toInt(), commentId.toInt());
      return;
    }
    emit commentUnclicked();
  });
}

void UserConfigRepository::fetchAvatar(int userId, const QString& avatarUrl) {
  clp_avatar_cache_->fetchAvatar(userId, avatarUrl);
}

void UserConfigRepository::downloadConfigFile(const QString& url, int configId) {
  clp_download_repo_->downloadToTempFile(url, configId);
}

// ── File upload ──

void UserConfigRepository::uploadUserFile(const QString& filePath) {
  QFileInfo fi(filePath);
  auto* mp = new QHttpMultiPart(QHttpMultiPart::FormDataType);

  QHttpPart filePart;
  filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                     QStringLiteral("form-data; name=\"file\"; filename=\"%1\"").arg(fi.fileName()));
  auto* file = new QFile(filePath);
  file->open(QIODevice::ReadOnly);
  filePart.setBodyDevice(file);
  file->setParent(mp);
  mp->append(filePart);

  auto* reply = HttpClient::instance().upload("/user/uploads", mp);

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    emit fileUploaded(obj.value("data").toObject().value("url").toString());
  });
}

/// \brief Admin

void UserConfigRepository::adminGetConfigs(int page, int pageSize,
                                           const QMap<QString, QString>& filters) {
  const QUrlQuery query = buildPageQuery(page, pageSize, filters);

  auto* reply = HttpClient::instance().get("/admin/user-configs", RequestOptions{}.withQuery(query));

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    const PaginatedResult pageResult = parsePaginated(obj);
    QList<DeSheng::GetPublicConfigurationListResponse::ListItem> list;
    for (const QJsonValue& v : pageResult.items) {
      list.append(parseUserConfigInfo(v.toObject()));
    }
    emit adminConfigsReady(list, pageResult);
  });
}

void UserConfigRepository::adminGetConfigDetail(const QString& id) {
  auto* reply = HttpClient::instance().get("/admin/user-configs/" + id);

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    emit adminConfigDetailReady(parseUserConfigInfo(obj.value("data").toObject()));
  });
}

void UserConfigRepository::adminUpdateStatus(const QString& id, const QString& status) {
  QJsonObject body;
  body["status"] = status;

  auto* reply = HttpClient::instance().put("/admin/user-configs/" + id + "/status",
                                          RequestOptions{}.withBody(QJsonDocument(body).toJson()));

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    emit adminConfigStatusUpdated();
  });
}

void UserConfigRepository::adminDeleteConfig(const QString& id) {
  auto* reply = HttpClient::instance().del("/admin/user-configs/" + id);

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    emit adminConfigDeleted();
  });
}

void UserConfigRepository::adminSetTags(const QString& id, bool isOfficial, bool isExpert) {
  QJsonObject body;
  body["is_official_tag"] = isOfficial;
  body["is_expert_tag"] = isExpert;

  auto* reply = HttpClient::instance().put("/admin/user-configs/" + id + "/tags",
                                          RequestOptions{}.withBody(QJsonDocument(body).toJson()));

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    emit adminTagsSet();
  });
}

/// \brief Admin: comments

void UserConfigRepository::adminGetComments(int page, int pageSize,
                                            const QMap<QString, QString>& filters) {
  const QUrlQuery query = buildPageQuery(page, pageSize, filters);

  auto* reply =
      HttpClient::instance().get("/admin/comments", RequestOptions{}.withQuery(query));

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    const PaginatedResult pageResult = parsePaginated(obj);
    QList<DeSheng::GetPublicConfigurationListResponse::Comment> list;
    for (const QJsonValue& v : pageResult.items) {
      list.append(parseCommentItem(v.toObject()));
    }
    emit adminCommentsReady(list, pageResult);
  });
}

void UserConfigRepository::adminCreateComment(const QJsonObject& req) {
  auto* reply = HttpClient::instance().post("/admin/comments",
                                           RequestOptions{}.withBody(QJsonDocument(req).toJson()));

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    emit adminCommentCreated(parseCommentItem(obj.value("data").toObject()));
  });
}

void UserConfigRepository::adminUpdateComment(const QString& id, const QString& text) {
  QJsonObject body;
  body["comment_text"] = text;

  auto* reply = HttpClient::instance().put("/admin/comments/" + id,
                                          RequestOptions{}.withBody(QJsonDocument(body).toJson()));

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    emit adminCommentUpdated();
  });
}

void UserConfigRepository::adminDeleteComment(const QString& id) {
  auto* reply = HttpClient::instance().del("/admin/comments/" + id);

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    emit adminCommentDeleted();
  });
}

void UserConfigRepository::adminUpdateCommentStatus(const QString& id, const QString& status) {
  QJsonObject body;
  body["status"] = status;

  auto* reply = HttpClient::instance().put("/admin/comments/" + id + "/status",
                                          RequestOptions{}.withBody(QJsonDocument(body).toJson()));

  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
      return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    if (obj.value("code").toString() != "success") {
      emit errorOccurred(obj.value("message").toString());
      return;
    }
    emit adminCommentStatusUpdated();
  });
}
