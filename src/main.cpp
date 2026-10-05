#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <exception>
#include <iostream>

#include "app/CliApp.h"
#include "app/GuiApp.h"
#include "app/StartupOptions.h"

#include "utils/LogManager.h"

static LogManager log_manager(LoggingMode::Console);

static bool UseRussianUi() {
  for (const QString& language : QLocale::system().uiLanguages()) {
    switch (QLocale(language).language()) {
      case QLocale::Russian: return true;
      case QLocale::English: return false;
      default: break;
    }
  }
  return false;
}

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  qInstallMessageHandler(LOGGING_CALLBACK(log_manager));

  QTranslator english_translator;
  if (!UseRussianUi()) {
    if (english_translator.load(":/i18n/YandexHomeDesktop_en.qm")) {
      app.installTranslator(&english_translator);
    } else {
      qWarning() << "English translation catalog could not be loaded";
    }
  }

#ifdef YH_DEBUG_FAKE_API
  constexpr bool allow_fake_api = true;
#else
  constexpr bool allow_fake_api = false;
#endif
  const auto options = ParseStartupOptions(app.arguments(), allow_fake_api);
  if (!options) {
    qCritical().noquote() << options.error();
    return 2;
  }
  if (options->cli_arguments.size() > 1) {
    // Disable garbage in console during CLI run
    // Duplicate of condition to disable logging before initializing context
    log_manager.DisableConsole();
  }

  try {
    AppContext app_context(&app, *options);
    if (options->cli_arguments.size() > 1) {
      CliApp cli_app(app_context, &app);
      return cli_app.Start();
    }
    GuiApp gui_app(app_context, &app);
    return gui_app.Start();
  } catch (const std::exception& error) {
    std::cerr << error.what() << std::endl;
    return 2;
  }
}
