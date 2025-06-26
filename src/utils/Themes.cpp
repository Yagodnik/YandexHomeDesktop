#include "Themes.h"

#include <QColor>
#include <QDir>

#include "JsonLoader.h"

Themes::Themes(QObject *parent)
  : QObject(parent)
{
  QDir themes_dir(":/themes/");

  for (const auto file : themes_dir.entryInfoList()) {
    const auto theme_opt = JsonLoader::Load<Theme>(file.absoluteFilePath());

    if (!theme_opt.has_value()) {
      qWarning() << "Failed to load theme: " << file.absoluteFilePath();
      return;
    }

    const auto& theme = theme_opt.value();
    themes_.insert(file.baseName(), theme);
  }

  SetTheme("light");
}

void Themes::SetTheme(const QString& theme)
{
  if (!themes_.contains(theme)) {
    qWarning() << "Theme not found:" << theme;
    return;
  }

  current_theme_ = theme;
  active_theme_ = themes_[theme];

  emit mainTextChanged();
  emit backgroundChanged();
  emit headerBackgroundChanged();
  emit accentChanged();
  emit accent2Changed();
  emit controlTextChanged();
  emit inactiveChanged();
  emit switchInactiveChanged();
  emit switchActiveChanged();
  emit trackColorChanged();
}

QString Themes::GetMainText() const {
  return active_theme_.main_text;
}

QString Themes::GetBackground() const {
  return active_theme_.background;
}

QString Themes::GetHeaderBackground() const {
  return active_theme_.header_background;
}

QString Themes::GetAccent() const {
  return active_theme_.accent;
}

QString Themes::GetAccent2() const {
  return active_theme_.accent2;
}

QString Themes::GetControlText() const {
  return active_theme_.control_text;
}

QString Themes::GetInactive() const {
  return active_theme_.inactive;
}

QString Themes::GetSwitchInactive() const {
  return active_theme_.switch_inactive;
}

QString Themes::GetSwitchActive() const {
  return active_theme_.switch_active;
}

QString Themes::GetSwitchThumb() const {
  return active_theme_.switch_thumb;
}

QString Themes::GetControlText2() const {
  return active_theme_.control_text2;
}

QColor Themes::GetTrackColor() const {
  QColor base(active_theme_.track_color.hex);
  base.setAlphaF(active_theme_.track_color.alpha);
  return base;
}

// Setters (if needed for QML write access)
void Themes::SetMainText(const QString& text) {
  if (active_theme_.main_text != text) {
    active_theme_.main_text = text;
    emit mainTextChanged();
  }
}

void Themes::SetBackground(const QString& background) {
  if (active_theme_.background != background) {
    active_theme_.background = background;
    emit backgroundChanged();
  }
}

void Themes::SetHeaderBackground(const QString& background) {
  if (active_theme_.header_background != background) {
    active_theme_.header_background = background;
    emit headerBackgroundChanged();
  }
}

void Themes::SetAccent(const QString& accent) {
  if (active_theme_.accent != accent) {
    active_theme_.accent = accent;
    emit accentChanged();
  }
}

void Themes::SetAccent2(const QString& accent2) {
  if (active_theme_.accent2 != accent2) {
    active_theme_.accent2 = accent2;
    emit accent2Changed();
  }
}

void Themes::SetControlText(const QString& text) {
  if (active_theme_.control_text != text) {
    active_theme_.control_text = text;
    emit controlTextChanged();
  }
}

void Themes::SetInactive(const QString& text) {
  if (active_theme_.inactive != text) {
    active_theme_.inactive = text;
    emit inactiveChanged();
  }
}

void Themes::SetSwitchInactive(const QString& text) {
  if (active_theme_.switch_inactive != text) {
    active_theme_.switch_inactive = text;
    emit switchInactiveChanged();
  }
}

void Themes::SetSwitchActive(const QString& text) {
  if (active_theme_.switch_active != text) {
    active_theme_.switch_active = text;
    emit switchActiveChanged();
  }
}

void Themes::SetSwitchThumb(const QString &text) {
  if (active_theme_.switch_thumb != text) {
    active_theme_.switch_thumb = text;
    emit switchThumbChanged();
  }
}

void Themes::SetControlText2(const QString &text) {
  if (active_theme_.control_text2 != text) {
    active_theme_.control_text2 = text;
    emit controlText2Changed();
  }
}

void Themes::SetTrackColor(QColor color) {
  const QString hex = color.name(QColor::HexRgb);
  float alpha = color.alphaF();

  if (active_theme_.track_color.hex != hex || active_theme_.track_color.alpha != alpha) {
    active_theme_.track_color.hex = hex;
    active_theme_.track_color.alpha = alpha;
    emit trackColorChanged();
  }
}

