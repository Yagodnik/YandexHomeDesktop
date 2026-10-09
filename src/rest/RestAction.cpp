#include "RestAction.h"

#include <optional>

#include <QCoreApplication>
#include <QJsonDocument>

namespace {
std::optional<CapabilityType> ParseCapability(const QString& value) {
  const QString prefix = "devices.capabilities.";
  for (const CapabilityType type : {CapabilityType::OnOff, CapabilityType::Range,
         CapabilityType::Toggle, CapabilityType::Mode, CapabilityType::ColorSetting}) {
    const auto name = CapabilityType::AsString(type);
    if (value == name || value == name.sliced(prefix.size())) {
      return type;
    }
  }
  return std::nullopt;
}
} // namespace

std::expected<RestAction, RestActionError> ParseRestAction(const QHttpServerRequest& request) {
  using Code = RestReply::StatusCode;
  const auto content_type =
    request.headers().value("Content-Type").toByteArray().split(';').first().trimmed().toLower();
  if (content_type != "application/json") {
    return std::unexpected(RestActionError{Code::UnsupportedMediaType, "content_type",
      QCoreApplication::translate("RestServer", "Требуется application/json.")});
  }

  QJsonParseError error;
  const auto json = QJsonDocument::fromJson(request.body(), &error);
  if (error.error != QJsonParseError::NoError || !json.isObject()) {
    return std::unexpected(RestActionError{Code::BadRequest, "invalid_json",
      QCoreApplication::translate("RestServer", "Требуется JSON-объект.")});
  }
  const auto body = json.object();
  const auto type = ParseCapability(body["capability"].toString());
  bool unknown_field = false;
  for (const auto& key : body.keys()) {
    unknown_field |= key != "capability" && key != "state";
  }
  if (!type || !body["state"].isObject() || unknown_field) {
    return std::unexpected(RestActionError{Code::BadRequest, "invalid_action",
      QCoreApplication::translate("RestServer", "Укажите capability и объект state.")});
  }

  const auto state = body["state"].toObject();
  const bool text_value = *type == CapabilityType::Mode ||
                          (*type == CapabilityType::ColorSetting && state["instance"] == "scene");
  if (!state["instance"].isString() || !state.contains("value") ||
      (state.contains("relative") && !state["relative"].isBool()) ||
      (text_value && !state["value"].isString())) {
    return std::unexpected(RestActionError{Code::BadRequest, "invalid_action",
      QCoreApplication::translate("RestServer", "Некорректные типы полей state.")});
  }
  for (const auto& key : state.keys()) {
    if (key != "instance" && key != "value" && key != "relative") {
      return std::unexpected(RestActionError{Code::BadRequest, "invalid_action",
        QCoreApplication::translate("RestServer", "Неизвестное поле state.")});
    }
  }
  return RestAction{*type, state.toVariantMap()};
}
