#include "LogManager.h"
#include <QDateTime>
#include <QDir>
#include <QCoreApplication>
#include <QStandardPaths>

namespace {
QMutex handler_mutex;
LogManager* active_logger = nullptr;
} // namespace

LogManager::LogManager(LoggingMode mode, QObject* parent)
    : QObject(parent), mode_(mode), console_(stdout, QIODevice::WriteOnly) {
  if (UsesFile(mode_)) {
    CreateLogFile();
  }
}

LogManager::LogManager(const QString& path)
    : rotate_(false), mode_(LoggingMode::NoLogging), log_file_(path) {
  if (!path.isEmpty() && log_file_.open(QIODevice::WriteOnly | QIODevice::Append)) {
    file_.setDevice(&log_file_);
    mode_ = LoggingMode::File;
  }
}

LogManager::~LogManager() {
  const QMutexLocker lock(&handler_mutex);
  if (active_logger == this) {
    qInstallMessageHandler(previous_handler_);
    active_logger = nullptr;
  }
}

void LogManager::InstallMessageHandler() {
  const QMutexLocker lock(&handler_mutex);
  if (!active_logger) {
    previous_handler_ = qInstallMessageHandler(HandleMessage);
    active_logger = this;
  }
}

void LogManager::HandleMessage(QtMsgType type, const QMessageLogContext& context,
                               const QString& message) {
  const QMutexLocker lock(&handler_mutex);
  if (active_logger) {
    active_logger->Log(type, context, message);
  }
}

void LogManager::DisableConsole() {
  const QMutexLocker lock(&mutex_);
  if (mode_ == LoggingMode::Console) {
    mode_ = LoggingMode::NoLogging;
  }

  if (mode_ == LoggingMode::Both) {
    mode_ = LoggingMode::File;
  }
}

void LogManager::Log(QtMsgType type, const QMessageLogContext& context, const QString& message) {
  const QMutexLocker lock(&mutex_);
  if (rotate_ && UsesFile(mode_) && log_file_.size() > kLogFileSizeLimit) {
    CreateLogFile();
  }

  switch (mode_) {
  case LoggingMode::Console:
    LogToStream(console_, type, context, message);
    break;
  case LoggingMode::File:
    LogToStream(file_, type, context, message);
    break;
  case LoggingMode::Both:
    LogToStream(file_, type, context, message);
    LogToStream(console_, type, context, message);
    break;
  case LoggingMode::NoLogging:
    break;
  }
}

void LogManager::LogToStream(QTextStream& stream, QtMsgType type, const QMessageLogContext& context,
                             const QString& message) {
  stream << QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs) << " ["
         << TypeToString(type) << "] [pid " << QCoreApplication::applicationPid() << "] ["
         << (context.category ? context.category : "default") << "] " << message;

  if (context.file) {
    stream << " (" << context.file << ":" << context.line << ")";
  }

  stream << Qt::endl;
}

void LogManager::CreateLogFile() {
  if (log_file_.isOpen()) {
    log_file_.close();
    file_.reset();
  }

  const QString file_name =
      "YHD-" + QDateTime::currentDateTimeUtc().toString("yyyyMMdd-HHmmss-zzz") + ".log";
  const QString base_path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  log_file_.setFileName(QDir(base_path).filePath(file_name));
  if (!QDir().mkpath(base_path) || !log_file_.open(QIODevice::WriteOnly)) {
    mode_ = mode_ == LoggingMode::Both ? LoggingMode::Console : LoggingMode::NoLogging;
    return;
  }
  file_.setDevice(&log_file_);
}

QString LogManager::TypeToString(QtMsgType type) {
  switch (type) {
  case QtDebugMsg:
    return "DEBUG";
  case QtInfoMsg:
    return "INFO";
  case QtWarningMsg:
    return "WARNING";
  case QtCriticalMsg:
    return "CRITICAL";
  case QtFatalMsg:
    return "FATAL";
  default:
    return "UNKNOWN";
  }
}
