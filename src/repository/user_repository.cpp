#include "repository/user_repository.h"

#include <QJsonDocument>
#include <QNetworkReply>

#include "modules/CommunityModule/infrastructure/logger/logger.h"
#include "network/http_client.h"
#include "network/request_options.h"

/// \brief UserRepository

UserRepository::UserRepository(QObject* parent) : QObject(parent) {}

void UserRepository::getPublicUserInfo(const QString& userId) {
  auto* t_reply = HttpClient::instance().get(
      QString(DeSheng::kPublicUserInfo).arg(userId),
      RequestOptions{}.withTag("user"));

  connect(t_reply, &QNetworkReply::finished, this, [this, t_reply] {
    t_reply->deleteLater();
    if (t_reply->error() != QNetworkReply::NoError) {
      LOG_DEBUG("[Repo] getPublicUserInfo network error: {}",
                t_reply->errorString().toStdString());
      emit errorOccurred(t_reply->errorString());
      return;
    }
    DeSheng::GetPublicUserInfoResponse t_resp;
    const QJsonDocument t_doc = QJsonDocument::fromJson(t_reply->readAll());
    if (!DeSheng::ProcessGetPublicUserInfoResult(t_resp, t_doc)) {
      LOG_DEBUG("[Repo] getPublicUserInfo parse error — code:{} msg:{}",
                t_resp.code.toStdString(), t_resp.message.toStdString());
      emit errorOccurred("公开用户信息响应解析失败");
      return;
    }
    if (t_resp.code != "success") {
      LOG_DEBUG("[Repo] getPublicUserInfo api error — code:{} msg:{}",
                t_resp.code.toStdString(), t_resp.message.toStdString());
      emit errorOccurred(t_resp.message);
      return;
    }
    emit publicUserInfoReady(t_resp);
  });
}
