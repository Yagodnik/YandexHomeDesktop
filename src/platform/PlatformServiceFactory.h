#pragma once

#include <QObject>
#include <memory>
#include "PlatformServiceImpl.h"

#ifdef Q_OS_MAC
#include "PlatformServiceMac.h"
#elif defined(Q_OS_WIN)
#include "PlatformServiceWin.h"
#else
#include "PlatformServiceLinux.h"
#endif

struct PlatformServiceFactory {
  static std::unique_ptr<PlatformServiceImpl> Create() {
#ifdef Q_OS_MAC
    return std::make_unique<PlatformServiceMac>();
#elif defined(Q_OS_WIN)
    return std::make_unique<PlatformServiceWin>(window);
#else
    return std::make_unique<PlatformServiceLinux>(window);
#endif
  }
};