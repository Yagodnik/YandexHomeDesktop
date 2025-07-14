#pragma once

#include <functional>
#include <QString>

class IExecutor {
public:
  template <typename ExecutorType, typename... Args>
  static std::unique_ptr<IExecutor> Create(Args&&... args) {
    return std::make_unique<ExecutorType>(std::forward<Args>(args)...);
  }

  virtual ~IExecutor() = default;

  virtual void Execute(const QString& value) = 0;
  virtual void PrintInfo() = 0;
};

#define EXECUTOR_FACTORY(name, ...) []() -> std::unique_ptr<IExecutor> { return IExecutor::Create<name>(__VA_ARGS__); }

using ExecutorFactoryFunction = std::function<std::unique_ptr<IExecutor>()>;

