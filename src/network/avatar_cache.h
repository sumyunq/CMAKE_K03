#ifndef AVATAR_CACHE_H
#define AVATAR_CACHE_H

#include <QObject>
#include <QPixmap>
#include <QString>

class AvatarCache : public QObject {
  Q_OBJECT

 public:
  explicit AvatarCache(QObject* parent = nullptr);

  void fetchAvatar(int userId, const QString& avatarUrl);

 signals:
  void avatarReady(int userId, const QPixmap& pixmap);
};

#endif  // AVATAR_CACHE_H
