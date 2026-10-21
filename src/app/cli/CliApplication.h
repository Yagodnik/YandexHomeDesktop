#pragma once

#include <QByteArray>
#include <QtGlobal>

// Call before constructing QCoreApplication in either console entry point.
inline void PrepareCliApplication() {
#ifdef Q_OS_MACOS
  // QtKeychain completes jobs on the native main queue. The default Unix
  // dispatcher does not drain it, even after the Keychain prompt succeeds.
  qputenv("QT_EVENT_DISPATCHER_CORE_FOUNDATION", QByteArrayLiteral("1"));
#endif
}
