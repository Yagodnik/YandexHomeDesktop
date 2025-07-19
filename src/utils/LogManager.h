#pragma once

#include <QFile>
#include <QTextStream>

constexpr int kLogFileSizeLimit = 5 * 1024 * 1024; // 5MB

enum class LoggingMode {
  File, Console, Both, NoLogging
};

inline bool UsesFile(LoggingMode mode) {
  return mode == LoggingMode::File || mode == LoggingMode::Both;
}

class LogManager : public QObject {
  Q_OBJECT
public:
  explicit LogManager(LoggingMode mode, QObject *parent = nullptr);

  void Log(QtMsgType type, const QMessageLogContext& context,  const QString& message);

private:
  static void LogToStream(
    QTextStream& stream,
    QtMsgType type,
    const QMessageLogContext& context,
    const QString& message
  );

  void CreateLogFile();

  static QString TypeToString(QtMsgType type);

  LoggingMode mode_;
  QTextStream console_;
  QTextStream file_;
  QFile log_file_;
};

#define LOGGING_CALLBACK(logger)          \
  [](                                     \
    QtMsgType type,                       \
    const QMessageLogContext& context,    \
    const QString& message                \
  ) {                                     \
    logger.Log(type, context, message);   \
  }                                       \
