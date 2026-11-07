#include "RestErrors.h"

#include <QCoreApplication>

namespace {
using Code = RestReply::StatusCode;
}

void FailApi(RestReply* reply, const ApiError& error) {
  const auto status = static_cast<Code>(error.http_status);
  if (error.kind == ApiErrorKind::Timeout) {
    reply->Fail(Code::GatewayTimeout, "timeout", error.message);
  } else if (status == Code::Unauthorized || status == Code::Forbidden) {
    reply->Fail(status, "authorization_required", error.message);
  } else if (status == Code::NotFound) {
    reply->Fail(Code::NotFound, "not_found", error.message);
  } else {
    reply->Fail(Code::BadGateway, "api_error", error.message);
  }
}

void FailCommand(RestReply* reply, const CommandError& error) {
  if (const auto* api = std::get_if<ApiError>(&error)) {
    FailApi(reply, *api);
    return;
  }

  const auto& rejection = std::get<CommandRejection>(error);
  switch (rejection.reason) {
  case CommandRejectionReason::InvalidValue:
    reply->Fail(Code::BadRequest, "invalid_value", rejection.message);
    break;
  case CommandRejectionReason::InvalidDeviceResponse:
    reply->Fail(Code::BadGateway, "api_error",
      QCoreApplication::translate("RestServer", "Некорректный ответ устройства."));
    break;
  case CommandRejectionReason::CapabilityNotFound:
    reply->Fail(Code::NotFound, "capability_not_found",
      QCoreApplication::translate("RestServer", "Умение не поддерживается устройством."));
    break;
  case CommandRejectionReason::AmbiguousCapability:
    reply->Fail(Code::Conflict, "ambiguous_capability",
      QCoreApplication::translate("RestServer", "Найдено несколько одинаковых умений."));
    break;
  case CommandRejectionReason::ScenarioNotFound:
    reply->Fail(Code::NotFound, "scenario_not_found",
      QCoreApplication::translate("RestServer", "Сценарий не найден."));
    break;
  case CommandRejectionReason::InactiveScenario:
    reply->Fail(Code::Conflict, "inactive_scenario",
      QCoreApplication::translate("RestServer", "Сценарий отключён."));
    break;
  }
}
