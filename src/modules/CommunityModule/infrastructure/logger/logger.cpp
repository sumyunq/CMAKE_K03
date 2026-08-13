#include "modules/CommunityModule/infrastructure/logger/logger.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QtGlobal>

#include <cstdlib>
#include <memory>

std::shared_ptr<spdlog::logger> Logger::s_logger_;
int Logger::s_keep_days_ = 30;

// ── Qt 日志转发：qWarning/qCritical → spdlog（Release 不再丢失，格式带文件/行号/函数名）──
namespace {
void qtMessageHandler(QtMsgType t_type, const QMessageLogContext &t_ctx, const QString &t_msg)
{
    auto t_logger = Logger::get();
    if (!t_logger)
        return;
    spdlog::source_loc t_loc{t_ctx.file ? t_ctx.file : "", t_ctx.line,
                             t_ctx.function ? t_ctx.function : ""};
    switch (t_type) {
    case QtDebugMsg:
        t_logger->log(t_loc, spdlog::level::debug, t_msg.toStdString());
        break;
    case QtInfoMsg:
        t_logger->log(t_loc, spdlog::level::info, t_msg.toStdString());
        break;
    case QtWarningMsg:
        t_logger->log(t_loc, spdlog::level::warn, t_msg.toStdString());
        break;
    case QtCriticalMsg:
        t_logger->log(t_loc, spdlog::level::err, t_msg.toStdString());
        break;
    case QtFatalMsg:
        t_logger->log(t_loc, spdlog::level::critical, t_msg.toStdString());
        std::abort();
    }
}
} // namespace

// ── 默认日志目录：exe 所在目录/logs ──
QString Logger::defaultLogDir() {
  return QCoreApplication::applicationDirPath() + "/logs";
}

// ── 清理旧日志 ──
void Logger::cleanupOldLogs(const QString& dir, int keepDays) {
  QDir logDir(dir);
  if (!logDir.exists()) return;

  const QDateTime cutoff = QDateTime::currentDateTime().addDays(-keepDays);
  const auto files = logDir.entryInfoList({"*.log", "*.txt"}, QDir::Files);
  for (const auto& fi : files) {
    if (fi.lastModified() < cutoff) {
      QFile::remove(fi.absoluteFilePath());
    }
  }
}

// ── 初始化 ──
void Logger::init() { init(QString(), 30); }

void Logger::init(const QString& logDir, int keepDays) {
  s_keep_days_ = keepDays;

  const QString dir = logDir.isEmpty() ? defaultLogDir() : logDir;
  QDir().mkpath(dir);

  // 清理过期日志
  cleanupOldLogs(dir, keepDays);

  // 日志文件名区分构建版本：<app>_debug.log / <app>_release.log（NDEBUG 由 Release 构建定义）
#ifdef NDEBUG
  const QString logName = QCoreApplication::applicationName() + "_release.log";
#else
  const QString logName = QCoreApplication::applicationName() + "_debug.log";
#endif

  // 每日滚动文件：<app>_debug_2026-07-24.log
  const auto logPath = (dir + "/" + logName).toStdString();
  auto fileSink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
      logPath, 0, 0, false, keepDays);  // rotation at 00:00, keep N files

  // 控制台彩色 sink
  auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

  // 统一格式
  const auto pattern = "%Y-%m-%d %H:%M:%S.%e [%^%l%$] [%s:%# %!] %v";
  fileSink->set_pattern(pattern);
  consoleSink->set_pattern(pattern);

  // 异步线程池
  spdlog::init_thread_pool(8192, 1);

  spdlog::sinks_init_list sinks = {std::move(fileSink), std::move(consoleSink)};
  s_logger_ = std::make_shared<spdlog::async_logger>("app", sinks.begin(), sinks.end(),
                                                     spdlog::thread_pool(),
                                                     spdlog::async_overflow_policy::block);
  // 级别分级：Release 只记 info 及以上（埋点/警告/错误，滤掉 qDebug 与 LOG_DEBUG 噪音）；
  // Debug 全量记录
#ifdef NDEBUG
  s_logger_->set_level(spdlog::level::info);
#else
  s_logger_->set_level(spdlog::level::debug);
#endif
  spdlog::set_default_logger(s_logger_);

  const QString logFile = dir + "/" + logName;
  LOG_INFO("Logger init — file: {} keepDays: {}", logFile.toStdString(), keepDays);

  // qWarning/qCritical 等 Qt 日志统一转发到 spdlog（Release 不再丢失）
  qInstallMessageHandler(qtMessageHandler);
}

// ── 关闭 ──
void Logger::shutdown() {
  if (s_logger_) {
    s_logger_->flush();
    spdlog::shutdown();
    s_logger_.reset();
  }
}

std::shared_ptr<spdlog::logger> Logger::get() { return s_logger_; }
int Logger::keepDays() { return s_keep_days_; }
