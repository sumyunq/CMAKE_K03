#include "network/server_router.h"

#include <QDebug>
#include <QMutexLocker>

#include "modules/CommunityModule/infrastructure/logger/logger.h"

ServerRouter ServerRouter::s_instance_;

ServerRouter::ServerRouter() {
  cl_servers_.insert("domestic", QString::fromUtf8("https://hubsys.xiberia.net/api/v1"));
  cl_servers_.insert("domestic-t", QString::fromUtf8("https://hubsystest.xiberia.net/api/v1"));
  cl_default_key_ = "domestic";
}

ServerRouter& ServerRouter::instance() { return s_instance_; }

// ── 注册 ──

void ServerRouter::registerServer(const QString& key, const QString& baseUrl) {
  {
    QMutexLocker lk(&cl_mutex_);
    cl_servers_.insert(key, baseUrl);
  }
  emit serversUpdated();
}

void ServerRouter::removeServer(const QString& key) {
  {
    QMutexLocker lk(&cl_mutex_);
    cl_servers_.remove(key);
  }
  emit serversUpdated();
}

QStringList ServerRouter::servers() const {
  QMutexLocker lk(&cl_mutex_);
  return cl_servers_.keys();
}

void ServerRouter::setDefaultServer(const QString& key) {
  QMutexLocker lk(&cl_mutex_);
  if (!cl_servers_.contains(key)) {
    LOG_WARN("[ServerRouter] setDefaultServer: unknown key {}", key.toStdString());
    return;
  }
  if (cl_default_key_ == key) return;
  cl_default_key_ = key;
  lk.unlock();
  emit defaultServerChanged(key);
}

QString ServerRouter::defaultServer() const {
  QMutexLocker lk(&cl_mutex_);
  return cl_default_key_;
}

// ── Tag 路由 ──

void ServerRouter::setTagDefault(const QString& tag, const QString& serverKey) {
  QMutexLocker lk(&cl_mutex_);
  if (!cl_servers_.contains(serverKey)) return;
  cl_tag_defaults_.insert(tag, serverKey);
}

void ServerRouter::removeTagDefault(const QString& tag) {
  QMutexLocker lk(&cl_mutex_);
  cl_tag_defaults_.remove(tag);
}

// ── 前缀路由 ──

void ServerRouter::setPrefixDefault(const QString& prefix, const QString& serverKey) {
  QMutexLocker lk(&cl_mutex_);
  if (!cl_servers_.contains(serverKey)) return;
  cl_prefix_defaults_.insert(prefix, serverKey);
}

void ServerRouter::removePrefixDefault(const QString& prefix) {
  QMutexLocker lk(&cl_mutex_);
  cl_prefix_defaults_.remove(prefix);
}

// ── 解析 ──

QString ServerRouter::resolveUrl(const QString& path) const {
  return resolveInternal(path, {}, {});
}

QString ServerRouter::resolveUrl(const QString& path, const QString& serverKey) const {
  return resolveInternal(path, serverKey, {});
}

QString ServerRouter::resolveUrl(const QString& path, const QString& serverKey,
                                 const QString& tag) const {
  return resolveInternal(path, serverKey, tag);
}

QString ServerRouter::resolveInternal(const QString& path, const QString& serverKey,
                                      const QString& tag) const {
  QMutexLocker lk(&cl_mutex_);

  // ① 请求级 serverKey — 最高优先级
  if (!serverKey.isEmpty() && cl_servers_.contains(serverKey))
    return cl_servers_.value(serverKey) + path;

  // ② Tag 路由（业务维度）
  if (!tag.isEmpty() && cl_tag_defaults_.contains(tag))
    return cl_servers_.value(cl_tag_defaults_.value(tag)) + path;

  // ③ 前缀路由（QMap 升序，从末尾开始 = 最长前缀优先）
  auto it = cl_prefix_defaults_.constEnd();
  while (it != cl_prefix_defaults_.constBegin()) {
    --it;
    if (path.startsWith(it.key()) && cl_servers_.contains(it.value()))
      return cl_servers_.value(it.value()) + path;
  }

  // ④ 全局默认（若已被移除，回退到第一个可用服务器）
  if (cl_servers_.contains(cl_default_key_)) return cl_servers_.value(cl_default_key_) + path;
  if (!cl_servers_.isEmpty()) return cl_servers_.constBegin().value() + path;
  return path;  // 无服务器注册 → 返回原始路径
}
