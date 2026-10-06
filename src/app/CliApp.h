#pragma once

#include <QCoreApplication>
#include "StartupOptions.h"

// Headless bootstrap. Help/validation run before auth; commands use services and
// saved credentials without creating AppContext, QML, widgets, or tray objects.
int RunCli(QCoreApplication& app, const StartupOptions& options);
