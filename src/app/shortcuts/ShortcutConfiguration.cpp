#include "ShortcutConfiguration.h"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

WorkerLaunchConfiguration ShortcutConfiguration(const StartupOptions& options) {
  QString worker = "YandexHomeShortcuts";
#ifdef Q_OS_WIN
  worker += ".exe";
#endif
  const auto directory = QCoreApplication::applicationDirPath();
  WorkerLaunchConfiguration result;
  result.executable = QDir(directory).filePath(worker);
#ifdef Q_OS_MACOS
  if (!QFileInfo::exists(result.executable) && directory.endsWith(".app/Contents/MacOS"))
    result.executable = QDir(directory).filePath("../../../" + worker);
#endif
  auto identity = QDir::homePath() + "/live";
  if (options.use_fake_api) {
    const auto path = options.fixture_path.startsWith(":")
                          ? options.fixture_path
                          : QFileInfo(options.fixture_path).absoluteFilePath();
    identity = QDir::homePath() + "/fixture/" + path;
    result.arguments = {"--fake-api", "--fake-api-data", path};
  }
  const auto name =
      "yh-shortcuts-" +
      QString::fromLatin1(
          QCryptographicHash::hash(identity.toUtf8(), QCryptographicHash::Sha256).toHex().left(32));
#ifdef Q_OS_UNIX
  result.control_name = QDir(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation))
                            .filePath("YandexHome/run/" + name);
#else
  result.control_name = name;
#endif
  return result;
}
