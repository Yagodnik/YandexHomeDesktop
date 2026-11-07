#pragma once
#include "services/ShortcutControlService.h"

class ShortcutViewModel final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool enabled READ IsEnabled NOTIFY changed)
  Q_PROPERTY(bool background READ IsBackground NOTIFY changed)
  Q_PROPERTY(bool busy READ IsBusy NOTIFY changed)
  Q_PROPERTY(QString status READ GetStatus NOTIFY changed)
  Q_PROPERTY(QString error READ GetError NOTIFY changed)
public:
  explicit ShortcutViewModel(ShortcutControlService* service, QObject* parent = nullptr);
  bool IsEnabled() const;
  bool IsBackground() const;
  bool IsBusy() const;
  QString GetStatus() const;
  QString GetError() const;
  Q_INVOKABLE void SetEnabled(bool enabled);
  Q_INVOKABLE void SetBackground(bool enabled);

private:
  ShortcutControlService* service_;
signals:
  void changed();
};
