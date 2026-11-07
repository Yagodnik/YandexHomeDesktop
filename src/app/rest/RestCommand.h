#pragma once
#include "RestConfiguration.h"
#include <QCoreApplication>
#include <QJsonObject>
#include <expected>
namespace RestConsole {
enum ExitCode { Success = 0, RequestFailed = 1, Usage = 2, Unauthorized = 3, Timeout = 5 };
}
class RestAppMessages {
  Q_DECLARE_TR_FUNCTIONS(RestAppMessages)
};
struct RestConsoleOptions {
  bool json;
  int timeout_ms;
};
QByteArray RestError(bool json, const QString& code, const QString& message);
void RestPrint(const QByteArray& bytes, bool error = false);
int RestOutput(const QJsonObject& result, bool json);
std::expected<RestConsoleOptions, int> ParseRestConsole(const StartupOptions& options);
int RunRestControl(QCoreApplication& app, const StartupOptions& options);
