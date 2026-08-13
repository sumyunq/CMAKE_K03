#ifndef USER_REPOSITORY_H
#define USER_REPOSITORY_H

#include <QObject>

#include "data/user/user_api.h"

/// \brief 用户仓库 / Repository for user endpoints.
///
/// Covers other users' public info queries (avatar popup etc.).
class UserRepository : public QObject {
  Q_OBJECT
public:
  explicit UserRepository(QObject* parent = nullptr);

  /// \brief 获取其他用户公开信息 / Get another user's public info
  /// \param userId 目标用户 ID / Target user ID
  void getPublicUserInfo(const QString& userId);

signals:
  /// \brief 公开用户信息就绪 / Emitted when public user info is ready
  void publicUserInfoReady(const DeSheng::GetPublicUserInfoResponse& info);
  /// \brief 请求出错 / Emitted when an error occurs
  void errorOccurred(const QString& error);
};

#endif  // USER_REPOSITORY_H
