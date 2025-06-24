#include "Themes.h"

#include <QColor>

#include "JsonLoader.h"

Themes::Themes(QObject *parent)
  : QObject(parent)
{
  // const QString theme_path = ":/themes/light.json";
  const QString theme_path = ":/themes/dark.json";

  const auto theme_opt = JsonLoader::Load<Theme>(theme_path);

  if (!theme_opt.has_value()) {
    qWarning() << "Failed to load theme: " << theme_path;
    return;
  }

  const auto& theme = theme_opt.value();

  themes_.insert("light", theme);
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
