#include "McpConfiguration.h"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

McpConfiguration MakeMcpConfiguration(const StartupOptions& options) {
  QString identity = QDir::homePath() + "/live";
  if (options.use_fake_api) {
    const auto path = options.fixture_path.startsWith(":") ? options.fixture_path : QFileInfo(options.fixture_path).absoluteFilePath();
    identity = QDir::homePath() + "/fixture/" + path;
  }
  const auto digest = QString::fromLatin1(QCryptographicHash::hash(identity.toUtf8(), QCryptographicHash::Sha256).toHex().left(32));
  const auto endpoint = [&digest](const QString& prefix) {
    const auto name = prefix + digest;
#ifdef Q_OS_UNIX
    return QDir(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)).filePath("YandexHome/run/" + name);
#else
    return name;
#endif
  };
  McpConfiguration result;
  result.gui_endpoint = endpoint("yh-mcp-gui-");
  result.worker.control_name = endpoint("yh-mcp-worker-");
  QString executable = "YandexHomeMcp";
#ifdef Q_OS_WIN
  executable += ".exe";
#endif
  const auto dir = QCoreApplication::applicationDirPath();
  result.worker.executable = QDir(dir).filePath(executable);
#ifdef Q_OS_MACOS
  if (!QFileInfo::exists(result.worker.executable) && dir.endsWith(".app/Contents/MacOS"))
    result.worker.executable = QDir(dir).filePath("../../../" + executable);
#endif
  if (options.use_fake_api) {
    const auto path = options.fixture_path.startsWith(":") ? options.fixture_path : QFileInfo(options.fixture_path).absoluteFilePath();
    result.worker.arguments = {"--fake-api", "--fake-api-data", path};
  }
  return result;
}
