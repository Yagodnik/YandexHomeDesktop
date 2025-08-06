#include <QGuiApplication>

#include "app/CliApp.h"
#include "app/GuiApp.h"

#include "utils/LogManager.h"

static LogManager log_manager(LoggingMode::Console);

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);
  qInstallMessageHandler(LOGGING_CALLBACK(log_manager));

  AppContext app_context(&app);

  if (QGuiApplication::arguments().size() > 1) {
    CliApp cli_app(app_context, &app);
    return cli_app.Start();
  }

  GuiApp gui_app(app_context, &app);
  return gui_app.Start();
}
