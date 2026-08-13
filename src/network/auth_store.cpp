#include "network/auth_store.h"

#include <QMutexLocker>

AuthStore AuthStore::s_instance_;

AuthStore& AuthStore::instance() { return s_instance_; }

QString AuthStore::token() const {
  QMutexLocker lk(&cl_mutex_);
  return cl_token_;
}

void AuthStore::setToken(const QString& token) {
  {
    QMutexLocker lk(&cl_mutex_);
    cl_token_ = token;
  }
  emit tokenChanged();
}

void AuthStore::clear() {
  {
    QMutexLocker lk(&cl_mutex_);
    cl_token_.clear();
  }
  emit tokenChanged();
}

bool AuthStore::hasToken() const {
  QMutexLocker lk(&cl_mutex_);
  return !cl_token_.isEmpty();
}
