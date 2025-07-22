#pragma once

#include <QObject>
#include "PlatformServiceImpl.h"

class PlatformService : public QObject {
  Q_OBJECT
public:
  explicit PlatformService(QObject *parent = nullptr);

  Q_INVOKABLE void SetWindow(QQuickWindow *window) const;
  Q_INVOKABLE void ShowWindow(const QRect& icon) const;
  Q_INVOKABLE void ShowAsApp() const;
  Q_INVOKABLE void ShowOnlyInTray() const;

private:
  std::unique_ptr<PlatformServiceImpl> impl_;
};
