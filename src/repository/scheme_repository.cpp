#include "repository/scheme_repository.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>

#include "data/schemes/schemes_api.h"
#include "network/http_client.h"
#include "network/request_options.h"

/// \brief Static helpers

static DeSheng::CreateShareCodeResponse::ReturnData parseSchemeInfo(const QJsonObject& data) {
  DeSheng::CreateShareCodeResponse::ReturnData info;
  info.id = static_cast<int64_t>(data.value("id").toDouble());
  info.share_code = data.value("share_code").toString();
  info.url = data.value("url").toString();
  info.title = data.value("title").toString();
  info.description = data.value("description").toString();
  info.device_name = data.value("device_name").toString();
  info.device_type = data.value("device_type").toString();
  info.status = data.value("status").toString();
  // Step 2 适配点：新 DTO 无 user_id / updated_at 字段（解析逻辑改造时再处理）
  info.created_at = data.value("created_at").toString();
  return info;
}

/// \brief SchemeRepository

SchemeRepository::SchemeRepository(QObject* parent) : PaginatedRepository(parent) {}

void SchemeRepository::createShareCode(const DeSheng::CreateShareCodeRequest& req) {
  QJsonObject body;
  body["url"] = req.url;
  body["device_name"] = req.device_name;
  body["device_type"] = req.device_type;
  body["title"] = req.title;
  body["description"] = req.description;

  auto* reply = HttpClient::instance().post("/schemes/share-code",
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
    emit shareCodeCreated(parseSchemeInfo(obj.value("data").toObject()));
  });
}

void SchemeRepository::resolveShareCode(const QString& code) {
  auto* reply = HttpClient::instance().get("/schemes/resolve/" + code);

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
    emit shareCodeResolved(parseSchemeInfo(obj.value("data").toObject()));
  });
}

void SchemeRepository::updateScheme(const QString& id, const QJsonObject& req) {
  auto* reply = HttpClient::instance().put("/schemes/" + id,
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
    emit schemeUpdated();
  });
}

void SchemeRepository::adminGetSchemes(int page, int pageSize,
                                       const QMap<QString, QString>& filters) {
  const QUrlQuery query = buildPageQuery(page, pageSize, filters);

  auto* reply = HttpClient::instance().get("/admin/schemes", RequestOptions{}.withQuery(query));

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
    QList<DeSheng::CreateShareCodeResponse::ReturnData> list;
    for (const QJsonValue& v : pageResult.items) {
      list.append(parseSchemeInfo(v.toObject()));
    }
    emit adminSchemesReady(list, pageResult);
  });
}

void SchemeRepository::adminUpdateScheme(const QString& id, const QJsonObject& req) {
  auto* reply = HttpClient::instance().put("/admin/schemes/" + id,
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
    emit adminSchemeUpdated();
  });
}

void SchemeRepository::adminDeleteScheme(const QString& id) {
  auto* reply = HttpClient::instance().del("/admin/schemes/" + id);

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
    emit adminSchemeDeleted();
  });
}
