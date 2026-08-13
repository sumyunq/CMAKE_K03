#ifndef REQUEST_OPTIONS_H
#define REQUEST_OPTIONS_H

#include <QByteArray>
#include <QString>
#include <QUrlQuery>

/// \brief 请求级配置 — 替代 15 个重载的 Builder 模式（C++17 链式 API）
///
/// \code
/// // 链式调用
/// cli.get("/path", RequestOptions{}.withQuery(q).withTag("tag"));
/// cli.post("/path", RequestOptions{}.withBody(data).noAuth());
///
/// // 或逐字段设值
/// RequestOptions opts;
/// opts.query = {{"is_official_tag", "true"}};
/// cli.get("/user-configs", opts);
/// \endcode
struct RequestOptions {
  QString
      serverKey;  ///< 请求级服务器 key（空 = 走路由）/ Server key override (empty = use routing)
  QString tag;  ///< 业务 Tag（如 "schemes_expert"，空 = 走前缀路由）/ Business tag (empty = use
                ///< prefix routing)
  QUrlQuery query;    ///< Query 参数 / Query parameters
  QByteArray body;    ///< 请求体（POST/PUT）/ Request body for POST/PUT
  int timeoutMs = 0;  ///< 超时覆盖 / Timeout override in ms (0 = global default)
  bool auth = true;   ///< 是否携带 Authorization header / Whether to send Authorization header

  /// \brief 设置请求体 / Set the request body
  RequestOptions& withBody(const QByteArray& b) {
    body = b;
    return *this;
  }
  /// \brief 设置 Query 参数 / Set query parameters
  RequestOptions& withQuery(const QUrlQuery& q) {
    query = q;
    return *this;
  }
  /// \brief 设置业务 Tag / Set the business tag
  RequestOptions& withTag(const QString& t) {
    tag = t;
    return *this;
  }
  /// \brief 设置服务器 key / Set the server key
  RequestOptions& withServer(const QString& k) {
    serverKey = k;
    return *this;
  }
  /// \brief 设置超时时间（ms） / Set the timeout in milliseconds
  RequestOptions& withTimeout(int ms) {
    timeoutMs = ms;
    return *this;
  }
  /// \brief 跳过 Authorization header / Skip the Authorization header
  RequestOptions& noAuth() {
    auth = false;
    return *this;
  }
};

#endif  // REQUEST_OPTIONS_H
