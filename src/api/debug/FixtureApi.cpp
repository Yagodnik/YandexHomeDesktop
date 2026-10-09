#include "FixtureApi.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSet>
#include <QTimer>

namespace {
ApiError Invalid(const QString& message) {
  return {ApiErrorKind::InvalidResponse, "Fixture API: " + message};
}

bool ValidAttributes(const QJsonObject& device) {
  for (const auto* key : {"capabilities", "properties"}) {
    const auto value = device.value(key);
    if (value.isUndefined()) { continue; }
    if (!value.isArray()) { return false; }
    for (const auto& entry : value.toArray()) {
      const auto attribute = entry.toObject();
      if (!entry.isObject() || !attribute.value("type").isString() ||
          !attribute.value("retrievable").isBool() || !attribute.value("last_updated").isDouble()) {
        return false;
      }
    }
  }
  return true;
}

template<typename T>
void Deliver(QObject* context, ApiResultHandler<T> handler, ApiResult<T> result,
             const ApiResult<QJsonObject>& fixture) {
  if (!context) {
    return;
  }
  const int delay = fixture ? fixture->value("latency_ms").toInt(150) : 0;
  QTimer::singleShot(delay, context, [handler = std::move(handler), result = std::move(result)]() mutable {
    handler(std::move(result));
  });
}
}

FixtureApi::FixtureApi(QString path, QObject* parent) : QObject(parent), path_(std::move(path)) {}

ApiResult<QJsonObject> FixtureApi::ReadFixture() const {
  QFile file(path_);
  if (!file.open(QIODevice::ReadOnly)) {
    return std::unexpected(Invalid("Cannot open " + path_ + ": " + file.errorString()));
  }
  QJsonParseError error;
  const auto document = QJsonDocument::fromJson(file.readAll(), &error);
  if (error.error != QJsonParseError::NoError) {
    return std::unexpected(Invalid("Invalid JSON in " + path_ + ": " + error.errorString()));
  }
  if (!document.isObject()) {
    return std::unexpected(Invalid("Expected a JSON object in " + path_));
  }
  const auto fixture = document.object();
  const auto latency = fixture.value("latency_ms");
  if (!latency.isUndefined() && (!latency.isDouble() || latency.toDouble() < 0 ||
      latency.toDouble() > 60000 || latency.toDouble() != latency.toInt())) {
    return std::unexpected(Invalid("latency_ms must be an integer between 0 and 60000"));
  }
  const auto home = fixture.value("user_info");
  if (!home.isObject() || !home.toObject().value("status").isString()) {
    return std::unexpected(Invalid("user_info must contain an API response with a status"));
  }
  if (home.toObject().value("status") == "ok") {
    for (const auto* key : {"devices", "rooms", "households", "scenarios"}) {
      if (!home.toObject().value(key).isArray()) {
        return std::unexpected(Invalid(QString("user_info.%1 must be an array").arg(key)));
      }
      QSet<QString> ids;
      for (const auto& value : home.toObject().value(key).toArray()) {
        const auto item = value.toObject();
        const auto id = item.value("id").toString();
        if (!value.isObject() || id.isEmpty() || ids.contains(id) || !item.value("name").isString()) {
          return std::unexpected(Invalid(QString("user_info.%1 entries need unique IDs and names").arg(key)));
        }
        if (QString(key) == "scenarios" && !item.value("is_active").isBool()) {
          return std::unexpected(Invalid("Scenarios need a boolean is_active"));
        }
        if (QString(key) == "devices" && !ValidAttributes(item)) {
          return std::unexpected(Invalid("Device attributes need type, retrievable, and last_updated"));
        }
        ids.insert(id);
      }
    }
    const auto groups = home.toObject().value("groups");
    if (!groups.isUndefined() && !groups.isArray()) {
      return std::unexpected(Invalid("user_info.groups must be an array"));
    }
    for (const auto& group : groups.toArray()) {
      if (!group.isObject() || !ValidAttributes(group.toObject())) {
        return std::unexpected(Invalid("Invalid group attributes"));
      }
    }
  }
  return fixture;
}

ApiResult<void> FixtureApi::Validate() const {
  const auto fixture = ReadFixture();
  if (!fixture) {
    return std::unexpected(fixture.error());
  }
  return {};
}

ApiResult<UserInfo> FixtureApi::DecodeHome(const QJsonObject& fixture) {
  const auto response = fixture.value("user_info").toObject();
  if (response.value("status") != "ok") {
    return std::unexpected(ApiError{ApiErrorKind::Service, response.value("message").toString("Fixture load failed")});
  }
  return Serialization::From<UserInfo>(response);
}

void FixtureApi::GetUserInfo(QObject* context, ApiResultHandler<UserInfo> handler) {
  const auto fixture = ReadFixture();
  auto result = fixture ? DecodeHome(*fixture) : ApiResult<UserInfo>(std::unexpected(fixture.error()));
  Deliver(context, std::move(handler), std::move(result), fixture);
}

void FixtureApi::GetScenarios(QObject* context, ApiResultHandler<QList<ScenarioObject>> handler) {
  const auto fixture = ReadFixture();
  auto home = fixture ? DecodeHome(*fixture) : ApiResult<UserInfo>(std::unexpected(fixture.error()));
  ApiResult<QList<ScenarioObject>> result = home ? ApiResult<QList<ScenarioObject>>(home->scenarios)
                                               : std::unexpected(home.error());
  Deliver(context, std::move(handler), std::move(result), fixture);
}

void FixtureApi::GetDeviceInfo(const QString& id, QObject* context, ApiResultHandler<DeviceInfo> handler) {
  const auto fixture = ReadFixture();
  ApiResult<DeviceInfo> result = std::unexpected(fixture ? Invalid("No device_info entry for " + id) : fixture.error());
  if (fixture) {
    const auto entry = fixture->value("device_info").toObject().value(id);
    if (entry.isObject()) {
      const auto object = entry.toObject();
      if (object.value("id") != id || (object.value("status") != "ok" && object.value("status") != "error") ||
          (object.value("status") == "ok" && object.value("state") != "online" && object.value("state") != "offline") ||
          !ValidAttributes(object)) {
        result = std::unexpected(Invalid("Invalid device_info entry for " + id));
      } else if (object.value("status") != "ok") {
        result = std::unexpected(ApiError{ApiErrorKind::Service, object.value("message").toString()});
      } else {
        result = Serialization::From<DeviceInfo>(object);
      }
    }
  }
  Deliver(context, std::move(handler), std::move(result), fixture);
}

void FixtureApi::ExecuteScenario(const QString& id, QObject* context, ApiResultHandler<void> handler) {
  const auto fixture = ReadFixture();
  const auto home = fixture ? DecodeHome(*fixture) : ApiResult<UserInfo>(std::unexpected(fixture.error()));
  ApiResult<void> result = std::unexpected(home ? Invalid("Unknown or inactive scenario: " + id) : home.error());
  if (home) {
    for (const auto& scenario : home->scenarios) {
      if (scenario.id == id && scenario.is_active) {
        const auto message = fixture->value("scenario_errors").toObject().value(id).toString();
        result = message.isEmpty() ? ApiResult<void>{}
          : std::unexpected(ApiError{ApiErrorKind::Service, message});
        break;
      }
    }
  }
  Deliver(context, std::move(handler), std::move(result), fixture);
}

void FixtureApi::PerformActions(const QList<DeviceActionsObject>&, QObject* context, ApiResultHandler<void> handler) {
  const auto fixture = ReadFixture();
  ApiResult<void> result = std::unexpected(fixture
    ? ApiError{ApiErrorKind::Service, "Fixture API does not simulate device actions"} : fixture.error());
  Deliver(context, std::move(handler), std::move(result), fixture);
}

void FixtureApi::LoadData(QObject* context, ApiResultHandler<AccountInfo> handler) {
  const auto fixture = ReadFixture();
  ApiResult<AccountInfo> result = std::unexpected(fixture ? Invalid("Missing account_info") : fixture.error());
  if (fixture && fixture->value("account_info").isObject()) {
    const auto account = fixture->value("account_info").toObject();
    result = AccountInfo{account.value("display_name").toString(), {},
      account.value("default_email").toString(), account.value("id").toString()};
  }
  Deliver(context, std::move(handler), std::move(result), fixture);
}
