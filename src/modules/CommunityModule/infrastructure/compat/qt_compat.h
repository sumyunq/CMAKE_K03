#ifndef QT_COMPAT_H
#define QT_COMPAT_H

#include <QObject>

#include <memory>

/// \brief Qt 5.15 兼容的单次信号连接
///
/// Qt 6.0 引入了 Qt::SingleShotConnection，Qt 5.15 不支持。
/// 使用此函数自动在信号触发后断开连接。
///
/// \code
/// connectOnce(repo, &Repo::configLiked, this, [this, configId] {
///     // 只执行一次
/// });
/// \endcode
template <typename Sender, typename Signal, typename Receiver, typename Slot>
inline void connectOnce(Sender* sender, Signal signal, Receiver* receiver, Slot&& slot) {
  auto conn = std::make_shared<QMetaObject::Connection>();
  *conn = QObject::connect(sender, signal, receiver,
                           [conn, fn = std::forward<Slot>(slot)](auto&&... args) mutable {
                             QObject::disconnect(*conn);
                             fn(std::forward<decltype(args)>(args)...);
                           });
}

#endif  // QT_COMPAT_H
