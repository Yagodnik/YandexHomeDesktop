#include "LogManager.h"
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>

LogManager::LogManager(LoggingMode mode, QObject *parent)
  : QObject(parent), mode_(mode), console_(stdout, QIODevice::WriteOnly)
{
  if (UsesFile(mode_)) {
    CreateLogFile();
  }
}

void LogManager::Log(QtMsgType type, const QMessageLogContext &context, const QString &message) {
  if (UsesFile(mode_) && log_file_.size() > kLogFileSizeLimit) {
    qInfo() << "LogManager: Exceeded limit for log file! Creating new one";
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
    default:
      qWarning() << "LogManager: Unknown LoggingMode";
      break;
  };
}

void LogManager::LogToStream(
  QTextStream &stream,
  QtMsgType type,
  const QMessageLogContext &context,
  const QString &message
) {
  stream << QDateTime::currentDateTimeUtc().toString() << " ";
  stream << "[" << TypeToString(type) << "] " << message;

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

  const QString file_name = "YHD " + QDateTime::currentDateTimeUtc().toString() + ".log";

  const QString base_path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  const QDir dir(base_path);
  const QString path = dir.filePath(file_name);

  log_file_.setFileName(path);
  if (!log_file_.open(QIODevice::WriteOnly)) {
    qDebug() << path;
    qCritical() << "LogManager: Failed to open log file";
    exit(0);
    return;
  }

  file_.setDevice(&log_file_);

  qInfo() << "LogManager: Created log file at: " << path;
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
