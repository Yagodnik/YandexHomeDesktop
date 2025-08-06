#pragma once

#include <QCommandLineParser>
#include <QGuiApplication>
#include <QMap>

#include "ICommand.h"
#include "OnOffExecutor.h"
#include "ResetCommand.h"

class CLI : public QObject {
  Q_OBJECT
public:
  explicit CLI(AppContext& app_ctx, QObject *parent = nullptr);

private:
  AppContext& app_ctx_;

  QMap<QString, ICommand*> commands_list_;

  const QMap<QString, ExecutorFactoryFunction> kCapabilityExecutors = {
    { "on_off", EXECUTOR_FACTORY(OnOffExecutor) }
  };

  [[nodiscard]] bool HandleCapabilities();

  QCommandLineParser parser_;
};
