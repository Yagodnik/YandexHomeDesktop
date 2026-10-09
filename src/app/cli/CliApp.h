#pragma once

#include "app/common/StartupOptions.h"
#include <QCoreApplication>

// Headless bootstrap. Help/validation run before auth; commands use services and
// saved credentials without creating AppContext, QML, widgets, or tray objects.
int RunCli(QCoreApplication& app, const StartupOptions& options);
