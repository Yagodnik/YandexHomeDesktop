#pragma once

#include <QQmlApplicationEngine>

#include "AppContext.h"

class GuiApp : public QObject {
public:
  explicit GuiApp(AppContext& app_context, QObject *parent = nullptr);

  int Start();

private:
  void RegisterFonts() const;
  void RegisterModels();
  void RegisterCapabilities();
  void RegisterProperties();
  void RegisterViewModels();

  AppContext& app_context_;
  QQmlApplicationEngine engine;
};
