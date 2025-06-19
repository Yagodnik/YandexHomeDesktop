#include "Themes.h"

#include <QColor>
#include <QFile>

Themes::Themes(QObject *parent)
  : QObject(parent)
{
  // QFile theme_file(":/themes/light.json");
  QFile theme_file(":/themes/dark.json");

  if (!theme_file.open(QIODevice::ReadOnly)) {
    qDebug() << "Themes: unable to load theme";
    return;
  }

  auto bytes = theme_file.readAll();

  theme_file.close();

  QJsonDocument json = QJsonDocument::fromJson(bytes);
  QJsonObject json_theme = json.object();

  themes_.insert("light", Serialization::From<Theme>(json_theme));
  current_theme_ = "light";
}

QString Themes::GetMainText() const {
  return themes_[current_theme_].main_text;
}

QString Themes::GetBackground() const {
  return themes_[current_theme_].background;
}

QString Themes::GetHeaderBackground() const {
  return themes_[current_theme_].header_background;
}

QString Themes::GetAccent() const {
  return themes_[current_theme_].accent;
}

QString Themes::GetAccent2() const {
  return themes_[current_theme_].accent2;
}

QString Themes::GetControlText() const {
  return themes_[current_theme_].control_text;
}

QString Themes::GetInactive() const {
  return themes_[current_theme_].inactive;
}

QString Themes::GetSwitchInactive() const {
  return themes_[current_theme_].switch_inactive;
}

QString Themes::GetSwitchActive() const {
  return themes_[current_theme_].switch_active;
}

QColor Themes::GetTrackColor() const {
  const auto track_color = themes_[current_theme_].track_color;

  QColor color(track_color.hex);
  color.setAlphaF(track_color.alpha);

  return color;
}
