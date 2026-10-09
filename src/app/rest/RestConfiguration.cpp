#include "RestConfiguration.h"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
namespace {
constexpr qsizetype ControlIdentityDigestCharacters = 32;
}
RestLaunchConfiguration RestConfiguration(const StartupOptions& options) {
  RestLaunchConfiguration configuration;
  configuration.fixture = options.use_fake_api;
  QString worker = "YandexHomeRest";
#ifdef Q_OS_WIN
  worker += ".exe";
#endif
  auto directory = QCoreApplication::applicationDirPath();
  configuration.executable = QDir(directory).filePath(worker);
#ifdef Q_OS_MACOS
  // Build-tree GUI bundles sit three levels below the console executables.
  if (!QFileInfo::exists(configuration.executable) && directory.endsWith(".app/Contents/MacOS")) {
    configuration.executable = QDir(directory).filePath("../../../" + worker);
  }
#endif
  QString identity = QDir::homePath() + "/live";
  if (options.use_fake_api) {
    const auto path = options.fixture_path.startsWith(":")
                          ? options.fixture_path
                          : QFileInfo(options.fixture_path).absoluteFilePath();
    identity = QDir::homePath() + "/fixture/" + path;
    configuration.arguments = {"--fake-api", "--fake-api-data", path};
  }
  const auto name =
      "yh-rest-" +
      QString::fromLatin1(QCryptographicHash::hash(identity.toUtf8(), QCryptographicHash::Sha256)
                              .toHex()
                              .left(ControlIdentityDigestCharacters));
#ifdef Q_OS_UNIX
  configuration.control_name =
      QDir(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation))
          .filePath("YandexHome/run/" + name);
#else
  configuration.control_name = name;
#endif
  return configuration;
}
