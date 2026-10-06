#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <exception>
#include <iostream>

#include "app/CliApp.h"
#include "app/GuiApp.h"
#include "app/StartupOptions.h"
#include "app/AppTranslations.h"
#include "cli/CliRunner.h"

#include "utils/LogManager.h"

static LogManager log_manager(LoggingMode::Console);

int main(int argc, char *argv[]) {
#ifdef YH_DEBUG_FAKE_API
  constexpr bool allow_fake_api = true;
#else
  constexpr bool allow_fake_api = false;
#endif
  QStringList arguments;
  for (int i = 0; i < argc; ++i) { arguments.append(QString::fromLocal8Bit(argv[i])); }
  const auto options = ParseStartupOptions(arguments, allow_fake_api);
  if (!options || options->cli_arguments.size() > 1) {
    QCoreApplication app(argc, argv);
    log_manager.DisableConsole();
    qInstallMessageHandler(LOGGING_CALLBACK(log_manager));
    QTranslator translator;
    InstallEnglishTranslation(app, translator);
    const bool json = arguments.contains("--json") || arguments.contains("-j");
    try {
      if (options) { return RunCli(app, *options); }
      std::cerr << CliRunner::FormatError(json, "usage", options.error()).constData();
      return CliRunner::Usage;
    } catch (const std::exception& error) {
      std::cerr << CliRunner::FormatError(json, "startup_error", QString::fromUtf8(error.what())).constData();
      return CliRunner::RequestFailed;
    }
  }
  QApplication app(argc, argv);
  qInstallMessageHandler(LOGGING_CALLBACK(log_manager));
  QTranslator english_translator;
  InstallEnglishTranslation(app, english_translator);

  try {
    AppContext app_context(&app, *options);
    GuiApp gui_app(app_context, &app);
    return gui_app.Start();
  } catch (const std::exception& error) {
    std::cerr << error.what() << std::endl;
    return 2;
  }
}
