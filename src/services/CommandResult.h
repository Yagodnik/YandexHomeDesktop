#pragma once

#include "yh/appservices_export.h"

#include <variant>

#include "api/ApiResult.h"

// Application rejections stay independent of HTTP status and wire error codes.
enum class CommandRejectionReason {
  InvalidValue,
  InvalidDeviceResponse,
  CapabilityNotFound,
  AmbiguousCapability,
  ScenarioNotFound,
  InactiveScenario
};

struct CommandRejection {
  CommandRejectionReason reason;
  QString message;
};

using CommandError = std::variant<ApiError, CommandRejection>;
using CommandResult = std::expected<void, CommandError>;
using CommandResultHandler = std::function<void(CommandResult)>;
