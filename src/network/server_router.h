#ifndef SERVER_ROUTER_H
#define SERVER_ROUTER_H

#include <QHash>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QString>

/// \brief 服务器 URL 路由器（饿汉单例）
///
/// 三级优先级：请求级 serverKey > Tag 路由 > 前缀路由 > 全局默认
///
/// \code
/// auto &r = ServerRouter::instance();
/// r.registerServer("domestic",   "https://api.example.com/v1");
/// r.registerServer("domestic-t", "https://api-test.example.com/v1");
/// r.registerServer("overseas",   "https://api-global.example.com/v1");
/// r.setDefaultServer("domestic");
///
/// // Tag 路由（三个方案 Tab 独立控制）
/// r.setTagDefault("schemes_expert",   "domestic");
/// r.setTagDefault("schemes_official", "domestic-t");
///
/// // 前缀路由（firmware/drive 走海外）
/// r.setPrefixDefault("/firmware/", "overseas");
/// \endcode
class ServerRouter : public QObject {
  Q_OBJECT

public:
  /// \brief 获取单例实例 / Get the singleton instance
  static ServerRouter& instance();

  /// \brief 注册服务器 / Register a server by key
  /// \param key 服务器标识 key / Identifier key for the server
  /// \param baseUrl 服务器基础 URL / Base URL of the server
  void registerServer(const QString& key, const QString& baseUrl);
  /// \brief 移除已注册的服务器 / Remove a registered server
  /// \param key 服务器标识 key / Identifier key of the server to remove
  void removeServer(const QString& key);
  /// \brief 获取所有已注册的服务器 key 列表 / Get list of all registered server keys
  QStringList servers() const;

  /// \brief 设置默认服务器 / Set the default server
  /// \param key 服务器标识 key / Identifier key of the server
  void setDefaultServer(const QString& key);
  /// \brief 获取当前默认服务器 key / Get the current default server key
  QString defaultServer() const;

  /// \brief 设置指定业务 Tag 的默认服务器 / Set the default server for a business tag
  /// \param tag 业务 Tag（如 "schemes_expert"） / Business tag (e.g. "schemes_expert")
  /// \param serverKey 服务器标识 key / Identifier key of the server
  void setTagDefault(const QString& tag, const QString& serverKey);
  /// \brief 移除业务 Tag 的默认服务器绑定 / Remove the default server binding for a tag
  /// \param tag 业务 Tag / Business tag
  void removeTagDefault(const QString& tag);

  /// \brief 设置路径前缀的默认服务器 / Set the default server for a URL path prefix
  /// \param prefix 路径前缀（如 "/firmware/"） / Path prefix (e.g. "/firmware/")
  /// \param serverKey 服务器标识 key / Identifier key of the server
  void setPrefixDefault(const QString& prefix, const QString& serverKey);
  /// \brief 移除路径前缀的默认服务器绑定 / Remove the default server binding for a path prefix
  /// \param prefix 路径前缀 / Path prefix
  void removePrefixDefault(const QString& prefix);

  /// \brief 解析 URL（默认路由） / Resolve a URL using default routing
  /// \param path API 路径 / API path
  /// \return 完整的请求 URL / Full request URL
  QString resolveUrl(const QString& path) const;
  /// \brief 解析 URL（指定服务器） / Resolve a URL with explicit server key
  /// \param path API 路径 / API path
  /// \param serverKey 服务器标识 key / Server key (empty = use routing)
  /// \return 完整的请求 URL / Full request URL
  QString resolveUrl(const QString& path, const QString& serverKey) const;
  /// \brief 解析 URL（指定服务器和 Tag） / Resolve a URL with server key and tag
  /// \param path API 路径 / API path
  /// \param serverKey 服务器标识 key / Server key (empty = use routing)
  /// \param tag 业务 Tag / Business tag (empty = use prefix routing)
  /// \return 完整的请求 URL / Full request URL
  QString resolveUrl(const QString& path, const QString& serverKey, const QString& tag) const;

signals:
  /// \brief 默认服务器变更时触发 / Emitted when the default server changes
  /// \param newKey 新的默认服务器 key / New default server key
  void defaultServerChanged(const QString& newKey);
  /// \brief 服务器注册或移除时触发 / Emitted when servers are registered or removed
  void serversUpdated();

private:
  ServerRouter();
  ServerRouter(const ServerRouter&) = delete;
  ServerRouter& operator=(const ServerRouter&) = delete;

  QString resolveInternal(const QString& path, const QString& serverKey, const QString& tag) const;

  QString cl_default_key_;
  mutable QMutex cl_mutex_;
  QHash<QString, QString> cl_servers_;         // key → baseUrl
  QMap<QString, QString> cl_prefix_defaults_;  // path prefix → key
  QHash<QString, QString> cl_tag_defaults_;    // tag → key

  static ServerRouter s_instance_;
};

#endif  // SERVER_ROUTER_H
