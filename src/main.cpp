#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include "app/CliApp.h"
#include "app/GuiApp.h"

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

  if (QGuiApplication::arguments().size() > 1) {
    // Disable garbage in console during CLI run
    // Duplicate of condition to disable logging before initializing context
    log_manager.DisableConsole();
  }

  AppContext app_context(&app);

  if (QGuiApplication::arguments().size() > 1) {
    CliApp cli_app(app_context, &app);
    return cli_app.Start();
  }

  GuiApp gui_app(app_context, &app);
  return gui_app.Start();
}
