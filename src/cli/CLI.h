#pragma once

#include <QCommandLineParser>
#include <QGuiApplication>
#include <QMap>

#include "OnOffExecutor.h"

class CLI : public QObject {
  Q_OBJECT
public:
  explicit CLI(QGuiApplication* app, IHomeApi* api, QObject *parent = nullptr);

private:
  const QMap<QString, ExecutorFactoryFunction> kCapabilityExecutors = {
    { "on_off", EXECUTOR_FACTORY(OnOffExecutor) }
  };

  void HandleReset();
  [[nodiscard]] bool HandleCapabilities();

  QGuiApplication* app_;
  IHomeApi* api_;
  std::unique_ptr<IExecutor> current_executor_;
  QCommandLineParser parser_;
};
