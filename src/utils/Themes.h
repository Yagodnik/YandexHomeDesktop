#pragma once

#include <QObject>
#include <QColor>

#include "../serialization/Serialization.h"

class Themes : public QObject {
  Q_OBJECT
public:
  explicit Themes(QObject* parent = nullptr);

  Q_INVOKABLE [[nodiscard]] QString GetMainText() const;
  Q_INVOKABLE [[nodiscard]] QString GetBackground() const;
  Q_INVOKABLE [[nodiscard]] QString GetHeaderBackground() const;
  Q_INVOKABLE [[nodiscard]] QString GetAccent() const;
  Q_INVOKABLE [[nodiscard]] QString GetAccent2() const;
  Q_INVOKABLE [[nodiscard]] QString GetControlText() const;
  Q_INVOKABLE [[nodiscard]] QString GetInactive() const;
  Q_INVOKABLE [[nodiscard]] QString GetSwitchInactive() const;
  Q_INVOKABLE [[nodiscard]] QString GetSwitchActive() const;
  Q_INVOKABLE [[nodiscard]] QColor GetTrackColor() const;

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
    (Color, track_color)
  );

  QString current_theme_;
  QHash<QString, Theme> themes_;
};
