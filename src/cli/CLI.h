#pragma once

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QMap>

#include "OnOffExecutor.h"

class CLI : public QObject {
  Q_OBJECT
public:
  explicit CLI(QCoreApplication* app, QObject *parent = nullptr);

private:
  const QMap<QString, ExecutorFactoryFunction> kCapabilityExecutors = {
    { "on_off", EXECUTOR_FACTORY(OnOffExecutor) }
  };

  [[nodiscard]] bool HandleCapabilities() const;

  QCoreApplication* app_;
  QCommandLineParser parser_;
};
