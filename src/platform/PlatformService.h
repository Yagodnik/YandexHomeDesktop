#pragma once

#include <QObject>
#include "PlatformServiceImpl.h"

class PlatformService : public QObject {
  Q_OBJECT
public:
  PlatformService(QObject *parent = nullptr);

  Q_INVOKABLE void ShowAsApp() const;
  Q_INVOKABLE void ShowOnlyInTray() const;

private:
  std::unique_ptr<PlatformServiceImpl> impl_;
};
