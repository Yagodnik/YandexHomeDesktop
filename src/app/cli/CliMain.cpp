#include "app/common/AppTranslations.h"
#include "CliApp.h"
#include "CliApplication.h"
#include "cli/CliRunner.h"
#include <cstdio>
#include <exception>

int main(int argc, char* argv[]) {
  PrepareCliApplication();
  QCoreApplication app(argc, argv);
  app.setApplicationName("YandexHomeCli");
  // CLI output is owned by the runner; diagnostic logs must not enter JSON.
  qInstallMessageHandler([](QtMsgType, const QMessageLogContext&, const QString&) {});
  QTranslator translator;
  InstallEnglishTranslation(app, translator);
#ifdef YH_DEBUG_FAKE_API
  constexpr bool allow_fake_api = true;
#else
  constexpr bool allow_fake_api = false;
#endif
  const bool json = app.arguments().contains("--json") || app.arguments().contains("-j");
  const auto options = ParseStartupOptions(app.arguments(), allow_fake_api);
  if (!options) {
    const auto error = CliRunner::FormatError(json, "usage", options.error());
    std::fwrite(error.constData(), 1, error.size(), stderr);
    return CliRunner::Usage;
  }
  try {
    return RunCli(app, *options);
  } catch (const std::exception& exception) {
    const auto error =
        CliRunner::FormatError(json, "startup_error", QString::fromUtf8(exception.what()));
    std::fwrite(error.constData(), 1, error.size(), stderr);
    return CliRunner::RequestFailed;
  }
}
