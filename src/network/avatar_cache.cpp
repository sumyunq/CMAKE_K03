#include "network/avatar_cache.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPixmapCache>
#include <QUrl>

#include "network/http_client.h"

AvatarCache::AvatarCache(QObject* parent) : QObject(parent) {}

void AvatarCache::fetchAvatar(int userId, const QString& avatarUrl) {
  if (avatarUrl.isEmpty()) {
    return;
  }

  QPixmap cached;
  if (QPixmapCache::find(avatarUrl, &cached)) {
    emit avatarReady(userId, cached);
    return;
  }

  QNetworkRequest req{QUrl(avatarUrl)};
  auto* reply = HttpClient::instance().manager()->get(req);

  QObject::connect(reply, &QNetworkReply::finished, reply, &QObject::deleteLater);
  QObject::connect(reply, &QNetworkReply::finished, this,
                   [this, reply, userId, avatarUrl] {
                     if (reply->error() != QNetworkReply::NoError) {
                       return;
                     }

                     QPixmap pixmap;
                     pixmap.loadFromData(reply->readAll());
                     if (pixmap.isNull()) {
                       return;
                     }

                     QPixmapCache::insert(avatarUrl, pixmap);
                     emit avatarReady(userId, pixmap);
                   });
}
