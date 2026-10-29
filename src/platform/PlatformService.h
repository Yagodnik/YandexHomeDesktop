#pragma once

#include <QObject>
#include <QTimer>
#include <QPointer>
#include "PlatformServiceImpl.h"

class PlatformService : public QObject {
  Q_OBJECT
public:
  explicit PlatformService(QObject *parent = nullptr);

  Q_INVOKABLE void SetWindow(QQuickWindow *window);
  Q_INVOKABLE void SetNotchEnabled(bool enabled);
  Q_PROPERTY(bool notchSupported READ NotchSupported NOTIFY notchSupportedChanged)
  bool NotchSupported() const;
  Q_INVOKABLE void ShowWindow(const QRect& icon) const;
  Q_INVOKABLE void ShowAsApp() const;
  Q_INVOKABLE void ShowOnlyInTray() const;

signals:
  void notchSupportedChanged();

private:
  std::unique_ptr<PlatformServiceImpl> impl_;
  QPointer<QQuickWindow> window_;
  QTimer notch_timer_;
  bool inside_notch_ = false;
};
