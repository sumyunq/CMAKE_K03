#include "network/http_client.h"

#include <QHttpMultiPart>
#include <QMutexLocker>
#include <QNetworkDiskCache>
#include <QStandardPaths>

#include "network/auth_store.h"
#include "modules/CommunityModule/infrastructure/logger/logger.h"
#include "network/server_router.h"

// ── 单例 ──

HttpClient HttpClient::s_instance_;

HttpClient& HttpClient::instance() { return s_instance_; }

HttpClient::~HttpClient() {
  delete clp_manager_;
  clp_manager_ = nullptr;
}

QNetworkAccessManager* HttpClient::manager() {
  QMutexLocker lk(&cl_mutex_);
  if (!clp_manager_) {
    clp_manager_ = new QNetworkAccessManager();
    auto* diskCache = new QNetworkDiskCache(clp_manager_);
    diskCache->setCacheDirectory(
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/http");
    clp_manager_->setCache(diskCache);
  }
  return clp_manager_;
}

// ── 调试日志 ──

static void logRequest(const char* method, const QNetworkRequest& req,
                       const QByteArray& body = {}) {
  const QString token = req.rawHeader("Authorization");
  QString cmd;
  cmd += QStringLiteral("curl -X ") + method;
  if (!token.isEmpty()) {
    // Release 脱敏 / Debug 全量（统一走 LOG_REDACT）
    cmd += QStringLiteral(" -H \"Authorization: Bearer ") + LOG_REDACT(token) + QStringLiteral("\"");
  }
  if (!body.isEmpty()) {
    cmd += QStringLiteral(" -H \"Content-Type: application/json\" -d '");
#ifdef NDEBUG
    cmd += QStringLiteral("<redacted>");  // Release 不输出请求体（登录/注册等含密码/手机号）
#else
    cmd += QString::fromUtf8(body);
#endif
    cmd += QChar('\'');
  }
  cmd += QChar(' ') + req.url().toString();
  LOG_DEBUG("[API] {}", cmd.toStdString());
}

// ── buildRequest ──

void HttpClient::applyCommonOptions(QNetworkRequest& req, const RequestOptions& opts) {
  int timeout = opts.timeoutMs > 0 ? opts.timeoutMs : 60000;
  req.setTransferTimeout(timeout);

  if (opts.auth) {
    const QString token = AuthStore::instance().token();
    if (!token.isEmpty()) req.setRawHeader("Authorization", "Bearer " + token.toUtf8());
  }
}

QNetworkRequest HttpClient::buildRequest(const QString& path, const RequestOptions& opts) {
  QUrl url(ServerRouter::instance().resolveUrl(path, opts.serverKey, opts.tag));
  if (!opts.query.isEmpty()) url.setQuery(opts.query);

  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
  applyCommonOptions(req, opts);
  return req;
}

// ── 5 个方法 ──

QNetworkReply* HttpClient::get(const QString& path, const RequestOptions& opts) {
  auto req = buildRequest(path, opts);
  logRequest("GET", req);
  return manager()->get(req);
}

QNetworkReply* HttpClient::post(const QString& path, const RequestOptions& opts) {
  auto req = buildRequest(path, opts);
  logRequest("POST", req, opts.body);
  return manager()->post(req, opts.body);
}

QNetworkReply* HttpClient::put(const QString& path, const RequestOptions& opts) {
  auto req = buildRequest(path, opts);
  logRequest("PUT", req, opts.body);
  return manager()->put(req, opts.body);
}

QNetworkReply* HttpClient::del(const QString& path, const RequestOptions& opts) {
  auto req = buildRequest(path, opts);
  logRequest("DELETE", req, opts.body.isEmpty() ? QByteArray() : opts.body);
  return manager()->sendCustomRequest(req, "DELETE", opts.body);
}

QNetworkReply* HttpClient::upload(const QString& path, QHttpMultiPart* mp,
                                 const RequestOptions& opts) {
  QUrl url(ServerRouter::instance().resolveUrl(path, opts.serverKey, opts.tag));
  QNetworkRequest req(url);
  applyCommonOptions(req, opts);  // I2: 复用公共逻辑，不重复超时/token

  mp->setParent(nullptr);
  QNetworkReply* reply = manager()->post(req, mp);
  mp->setParent(reply);
  return reply;
}
