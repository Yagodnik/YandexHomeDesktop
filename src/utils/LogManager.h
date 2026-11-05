#pragma once

#include "yh/appruntime_export.h"

#include <QFile>
#include <QMutex>
#include <QTextStream>

constexpr int kLogFileSizeLimit = 5 * 1024 * 1024; // 5MB

enum class LoggingMode { File, Console, Both, NoLogging };

inline bool UsesFile(LoggingMode mode) {
  return mode == LoggingMode::File || mode == LoggingMode::Both;
}

class APPRUNTIME_EXPORT LogManager : public QObject {
  Q_OBJECT
public:
  explicit LogManager(LoggingMode mode, QObject* parent = nullptr);

  explicit LogManager(const QString& path);
  ~LogManager() override;

  void InstallMessageHandler();

  void DisableConsole();

  void Log(QtMsgType type, const QMessageLogContext& context, const QString& message);

private:
  static void HandleMessage(QtMsgType type, const QMessageLogContext& context,
                            const QString& message);
  static void LogToStream(QTextStream& stream, QtMsgType type, const QMessageLogContext& context,
                          const QString& message);

  void CreateLogFile();

  static QString TypeToString(QtMsgType type);

  QtMessageHandler previous_handler_ = nullptr;
  QMutex mutex_;
  bool rotate_ = true;
  LoggingMode mode_;
  QTextStream console_;
  QFile log_file_;
  QTextStream file_;
};
