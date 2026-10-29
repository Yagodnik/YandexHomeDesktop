#include <QApplication>
#include <QTranslator>
#include <exception>
#include <iostream>
#include "app/common/AppTranslations.h"
#include "GuiApp.h"
#include "TrayRecovery.h"
#include "app/common/StartupOptions.h"
#include "utils/LogManager.h"

int main(int argc, char* argv[]) {
#ifdef YH_DEBUG_FAKE_API
  constexpr bool allow_fake_api = true;
#else
  constexpr bool allow_fake_api = false;
#endif
  QStringList arguments;
  for (int i = 0; i < argc; ++i)
    arguments.append(QString::fromLocal8Bit(argv[i]));
  if (IsTrayRecoveryRequest(arguments)) {
    QCoreApplication app(argc, argv);
    QSettings settings("ArtemYagodnik", "YandexHomeDesktop");
    if (!DisableTrayForRecovery(settings)) {
      std::cerr << "Could not save the tray setting.\n";
      return 1;
    }
    return 0;
  }
  const auto options = ParseStartupOptions(arguments, allow_fake_api, false, true);
  if (!options || options->cli_arguments.size() > 1 ||
      options->rest_command != StartupOptions::RestCommand::None ||
      options->mcp_command != StartupOptions::McpCommand::None) {
    QCoreApplication app(argc, argv);
    QTranslator translator;
    InstallEnglishTranslation(app, translator);
    std::cerr
        << (options ? QCoreApplication::translate(
                          "StartupMessages",
                          "Use YandexHomeCli for commands and YandexHomeRest for the REST server.")
                    : options.error())
               .toStdString()
        << '\n';
    return 2;
  }
  QApplication app(argc, argv);
  static LogManager log_manager(LoggingMode::Console);
  qInstallMessageHandler(LOGGING_CALLBACK(log_manager));
  QTranslator translator;
  InstallEnglishTranslation(app, translator);
  try {
    AppContext app_context(&app, *options);
    GuiApp gui_app(app_context, &app);
    return gui_app.Start();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 2;
  }
}
