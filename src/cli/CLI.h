#pragma once

#include <QCommandLineParser>
#include <QGuiApplication>
#include <QMap>

#include "OnOffExecutor.h"

class CLI : public QObject {
  Q_OBJECT
public:
  explicit CLI(QGuiApplication* app, YandexHomeApi* api, QObject *parent = nullptr);

private:
  const QMap<QString, ExecutorFactoryFunction> kCapabilityExecutors = {
    { "on_off", EXECUTOR_FACTORY(OnOffExecutor) }
  };

  void HandleReset();
  [[nodiscard]] bool HandleCapabilities();

  QGuiApplication* app_;
  YandexHomeApi* api_;
  QCommandLineParser parser_;
};
