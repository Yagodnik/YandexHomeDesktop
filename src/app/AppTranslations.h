#pragma once

#include <QCoreApplication>
#include <QLocale>
#include <QTranslator>

inline bool UseRussianUi() {
  for (const auto& language : QLocale::system().uiLanguages()) {
    if (QLocale(language).language() == QLocale::Russian) { return true; }
    if (QLocale(language).language() == QLocale::English) { return false; }
  }
  return false;
}
inline void InstallEnglishTranslation(QCoreApplication& app, QTranslator& translator) {
  if (!UseRussianUi() && translator.load(":/i18n/YandexHomeDesktop_en.qm")) { app.installTranslator(&translator); }
}
