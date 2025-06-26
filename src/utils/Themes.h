#pragma once

#include <QObject>
#include <QColor>

#include "../serialization/Serialization.h"

class Themes : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString mainText READ GetMainText WRITE SetMainText NOTIFY mainTextChanged)
  Q_PROPERTY(QString background READ GetBackground WRITE SetBackground NOTIFY backgroundChanged)
  Q_PROPERTY(QString headerBackground READ GetHeaderBackground WRITE SetHeaderBackground NOTIFY headerBackgroundChanged)
  Q_PROPERTY(QString accent READ GetAccent WRITE SetAccent NOTIFY accentChanged)
  Q_PROPERTY(QString accent2 READ GetAccent2 WRITE SetAccent2 NOTIFY accent2Changed)
  Q_PROPERTY(QString controlText READ GetControlText WRITE SetControlText NOTIFY controlTextChanged)
  Q_PROPERTY(QString inactive READ GetInactive WRITE SetInactive NOTIFY inactiveChanged)
  Q_PROPERTY(QString switchInactive READ GetSwitchInactive WRITE SetSwitchInactive NOTIFY switchInactiveChanged)
  Q_PROPERTY(QString switchActive READ GetSwitchActive WRITE SetSwitchActive NOTIFY switchActiveChanged)
  Q_PROPERTY(QString switchThumb READ GetSwitchThumb WRITE SetSwitchThumb NOTIFY switchThumbChanged)
  Q_PROPERTY(QColor trackColor READ GetTrackColor WRITE SetTrackColor NOTIFY trackColorChanged)

public:
  explicit Themes(QObject* parent = nullptr);

  // Getters
  [[nodiscard]] QString GetMainText() const;
  [[nodiscard]] QString GetBackground() const;
  [[nodiscard]] QString GetHeaderBackground() const;
  [[nodiscard]] QString GetAccent() const;
  [[nodiscard]] QString GetAccent2() const;
  [[nodiscard]] QString GetControlText() const;
  [[nodiscard]] QString GetInactive() const;
  [[nodiscard]] QString GetSwitchInactive() const;
  [[nodiscard]] QString GetSwitchActive() const;
  [[nodiscard]] QString GetSwitchThumb() const;
  [[nodiscard]] QColor GetTrackColor() const;

  Q_INVOKABLE void SetTheme(const QString& theme);

signals:
  void mainTextChanged();
  void backgroundChanged();
  void headerBackgroundChanged();
  void accentChanged();
  void accent2Changed();
  void controlTextChanged();
  void inactiveChanged();
  void switchInactiveChanged();
  void switchActiveChanged();
  void switchThumbChanged();
  void trackColorChanged();

public slots:
  void SetMainText(const QString& text);
  void SetBackground(const QString& background);
  void SetHeaderBackground(const QString& background);
  void SetAccent(const QString& accent);
  void SetAccent2(const QString& accent2);
  void SetControlText(const QString& text);
  void SetInactive(const QString& text);
  void SetSwitchInactive(const QString& text);
  void SetSwitchActive(const QString& text);
  void SetSwitchThumb(const QString& text);
  void SetTrackColor(QColor color);

private:
  JSON_STRUCT(Color,
    (QString, hex),
    (float, alpha)
  );

  JSON_STRUCT(Theme,
    (QString, main_text),
    (QString, background),
    (QString, header_background),
    (QString, accent),
    (QString, accent2),
    (QString, control_text),
    (QString, inactive),
    (QString, switch_inactive),
    (QString, switch_active),
    (QString, switch_thumb),
    (Color, track_color)
  );

  QString current_theme_;
  QHash<QString, Theme> themes_;

  Theme active_theme_;
};
