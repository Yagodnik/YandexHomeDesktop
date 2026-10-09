#include "app/common/AppTranslations.h"
#include "app/cli/CliApplication.h"
#include "RestApp.h"
#include "RestCommand.h"
#include <exception>

int main(int argc, char* argv[]) {
  PrepareCliApplication();
  QCoreApplication app(argc, argv);
  app.setApplicationName("YandexHomeRest");
  qInstallMessageHandler([](QtMsgType, const QMessageLogContext&, const QString&) {});
  QTranslator translator;
  InstallEnglishTranslation(app, translator);
#ifdef YH_DEBUG_FAKE_API
  constexpr bool allow_fake_api = true;
#else
  constexpr bool allow_fake_api = false;
#endif
  const bool json = app.arguments().contains("--json") || app.arguments().contains("-j");
  auto options = ParseStartupOptions(app.arguments(), allow_fake_api, true);
  if (!options) {
    RestPrint(RestError(json, "usage", options.error()), true);
    return RestConsole::Usage;
  }
  if (options->rest_command == StartupOptions::RestCommand::None)
    options->rest_command = StartupOptions::RestCommand::Serve;
  try {
    return RunRest(app, *options);
  } catch (const std::exception& error) {
    RestPrint(RestError(json, "startup_error", QString::fromUtf8(error.what())), true);
    return RestConsole::RequestFailed;
  }
}
