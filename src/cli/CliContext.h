#pragma once

#include "yh/appcli_export.h"

#include "services/AccountService.h"
#include "services/DeviceService.h"
#include "services/HomeService.h"
#include "services/ScenarioService.h"
#include <QJsonObject>
#include <functional>

struct CliServices {
  HomeService* home;
  DeviceService* devices;
  ScenarioService* scenarios;
  AccountService* account;
  std::function<void(QObject*, ApiResultHandler<void>)> reset;
};

class CliContext {
public:
  enum ExitCode {
    Success = 0,
    RequestFailed = 1,
    Usage = 2,
    Unauthorized = 3,
    NotFound = 4,
    Timeout = 5
  };
  virtual ~CliContext() = default;
  virtual QObject* Owner() const = 0;
  virtual const CliServices& Services() const = 0;
  virtual void Complete(const QJsonObject& output, const QString& text) = 0;
  virtual void FailApi(const ApiError& error) = 0;
  virtual void Fail(int exit_code, const QString& code, const QString& message,
                    const QJsonObject& details = {}) = 0;
};
