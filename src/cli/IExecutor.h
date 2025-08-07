#pragma once

#include <functional>
#include <QString>

#include "api/YandexHomeApi.h"

class IExecutor : public QObject {
  Q_OBJECT
public:
  explicit IExecutor(YandexHomeApi* api, QObject *parent) : QObject(parent), api_(api) {};

  template <typename ExecutorType, typename... Args>
  static std::unique_ptr<IExecutor> Create(Args&&... args) {
    return std::make_unique<ExecutorType>(std::forward<Args>(args)...);
  }

  virtual ~IExecutor() = default;

  virtual void Execute(const QString& name, const QString& value) = 0;
  virtual void PrintInfo() = 0;

protected:
  YandexHomeApi* api_{nullptr};
};

#define EXECUTOR_FACTORY(name) [](YandexHomeApi* api) -> std::unique_ptr<IExecutor> { return IExecutor::Create<name>(api); }

using ExecutorFactoryFunction = std::function<std::unique_ptr<IExecutor>(YandexHomeApi*)>;

