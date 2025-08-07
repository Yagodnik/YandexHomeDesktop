#pragma once

#include <QSettings>

class Settings : public QObject {
  Q_OBJECT
  Q_PROPERTY(int currentTheme READ GetCurrentTheme WRITE SetCurrentTheme NOTIFY currentThemeChanged)
  Q_PROPERTY(bool trayModeEnabled READ GetTrayModeEnabled WRITE SetTrayModeEnabled NOTIFY trayModeEnabledChanged)
public:
  explicit Settings(QObject *parent = nullptr);

  [[nodiscard]] bool GetTrayModeEnabled() const;
  [[nodiscard]] int GetCurrentTheme() const;
  void Reset();

signals:
  void trayModeEnabledChanged();
  void currentThemeChanged();

public slots:
  void SetCurrentTheme(int theme);
  void SetTrayModeEnabled(bool enabled);

private:
  QSettings settings_;
};
