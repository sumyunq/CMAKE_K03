#ifndef COMMUNITY_LOGGER_H
#define COMMUNITY_LOGGER_H

#include <QString>

#include <spdlog/async.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

/// \brief 异步日志工具类（来自 WidgetCMake，spdlog header-only）
///
/// 后台线程池 + 无锁队列，同时输出到控制台和每日滚动文件。
/// 启动时自动清理超过保留天数的旧日志。
///
/// \code
/// Logger::init();                     // 使用默认路径和保留 30 天
/// Logger::init("/custom/path", 60);   // 或自定义路径+保留天数
/// LOG_INFO("Server: {}", url);
/// Logger::shutdown();                // 退出前
/// \endcode
class Logger {
public:
  /// \brief 初始化（使用默认路径和保留 30 天）
  static void init();

  /// \brief 初始化并指定日志目录和保留天数
  /// \param logDir   日志目录路径，空 = 使用 AppData/Logs
  /// \param keepDays 日志保留天数，默认 30
  static void init(const QString& logDir, int keepDays = 30);

  /// \brief 刷新队列并关闭
  static void shutdown();

  /// \brief 获取内部 logger 指针（供宏使用）
  static std::shared_ptr<spdlog::logger> get();

  /// \brief 当前保留天数
  static int keepDays();

private:
  static void cleanupOldLogs(const QString& dir, int keepDays);
  static QString defaultLogDir();

  static std::shared_ptr<spdlog::logger> s_logger_;
  static int s_keep_days_;
};

// ── 便捷宏 — 自动携带文件、行号、函数名 ──
#define LOG_TRACE(...)                                             \
  if (auto _l = Logger::get())                                     \
    _l->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, \
            spdlog::level::trace, __VA_ARGS__)
#define LOG_DEBUG(...)                                             \
  if (auto _l = Logger::get())                                     \
    _l->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, \
            spdlog::level::debug, __VA_ARGS__)
#define LOG_INFO(...)                                              \
  if (auto _l = Logger::get())                                     \
    _l->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, \
            spdlog::level::info, __VA_ARGS__)
#define LOG_WARN(...)                                              \
  if (auto _l = Logger::get())                                     \
    _l->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, \
            spdlog::level::warn, __VA_ARGS__)
#define LOG_ERROR(...)                                             \
  if (auto _l = Logger::get())                                     \
    _l->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, \
            spdlog::level::err, __VA_ARGS__)

// ── 敏感信息脱敏 ──
// 统一约定：Release 构建（NDEBUG）将敏感值打码为 "***"，Debug 构建原样输出（调试期全量）。
// 所有可能含敏感数据（token/密码/激活码/分享码等）的日志参数必须走本宏，禁止裸打。
// 用法：LOG_INFO("token={}", LOG_REDACT(token).toStdString());
#ifdef NDEBUG
#define LOG_REDACT(x) QStringLiteral("***")
#else
#define LOG_REDACT(x) (x)
#endif

#endif  // COMMUNITY_LOGGER_H
