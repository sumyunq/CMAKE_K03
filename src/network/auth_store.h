#ifndef AUTH_STORE_H
#define AUTH_STORE_H

#include <QMutex>
#include <QObject>
#include <QString>

/// \brief 鉴权存储（线程安全单例）
///
/// 与 ServerRouter 分离 — token 管理独立于 URL 路由
///
/// \code
/// AuthStore::instance().setToken(loginResp.accessToken);
/// AuthStore::instance().clear();  // 退出登录
/// \endcode
class AuthStore : public QObject {
  Q_OBJECT

public:
  /// \brief 获取单例实例 / Get the singleton instance
  static AuthStore& instance();

  /// \brief 获取当前 token / Get the current access token
  QString token() const;
  /// \brief 设置 token / Set the access token
  /// \param token 鉴权令牌 / Access token string
  void setToken(const QString& token);
  /// \brief 清除 token / Clear the stored token
  void clear();
  /// \brief 是否已存储 token / Whether a token is currently stored
  bool hasToken() const;

signals:
  /// \brief 登录或退出时触发 / Emitted when login or logout occurs
  void tokenChanged();
  /// \brief 供 401 拦截器发射 / Emitted by the 401 interceptor when token expires
  void tokenExpired();

private:
  AuthStore() = default;
  AuthStore(const AuthStore&) = delete;
  AuthStore& operator=(const AuthStore&) = delete;

  QString cl_token_;
  mutable QMutex cl_mutex_;

  static AuthStore s_instance_;
};

#endif  // AUTH_STORE_H
