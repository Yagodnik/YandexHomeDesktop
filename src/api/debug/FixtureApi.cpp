#include "FixtureApi.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSet>
#include <QTimer>
#include <QUrl>

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
  if (snapshot_ != fixture) {
    snapshot_ = fixture;
    states_.clear();
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
  if (result) {
    for (auto& device : result->devices)
      ApplyStates(device.id, device.capabilities);
  }
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
        ApplyStates(id, result->capabilities);
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

void FixtureApi::ApplyStates(const QString& id, QList<CapabilityObject>& capabilities) const {
  const auto states = states_.value(id);
  for (auto it = states.cbegin(); it != states.cend(); ++it) {
    if (it.key() < capabilities.size())
      capabilities[it.key()].state = it.value();
  }
}

void FixtureApi::PerformActions(const QList<DeviceActionsObject>& actions, QObject* context,
                                ApiResultHandler<void> handler) {
  const auto fixture = ReadFixture();
  ApiResult<void> result = fixture ? ApiResult<void>{} : std::unexpected(fixture.error());
  auto next = states_;
  if (fixture) {
    for (const auto& device : actions) {
      const auto info = fixture->value("device_info").toObject().value(device.id).toObject();
      if (info.value("status") != "ok" || info.value("state") != "online") {
        result = std::unexpected(Invalid("Unknown or unavailable device: " + device.id));
        break;
      }
      const auto error = fixture->value("action_errors").toObject().value(device.id).toString();
      if (!error.isEmpty()) {
        result = std::unexpected(ApiError{ApiErrorKind::Service, error});
        break;
      }
      auto capabilities = Serialization::From<DeviceInfo>(info).capabilities;
      ApplyStates(device.id, capabilities);
      for (const auto& action : device.actions) {
        int match = -1;
        for (qsizetype i = 0; i < capabilities.size(); ++i) {
          const auto& capability = capabilities[i];
          const auto instance =
              capability.parameters.value("instance", capability.state.value("instance"));
          if (capability.type == action.type && (capability.type == CapabilityType::ColorSetting ||
                                                 instance == action.state.value("instance"))) {
            match = i;
            break;
          }
        }
        if (match < 0 || action.type == CapabilityType::VideoStream) {
          result = std::unexpected(Invalid("Unknown fixture capability"));
          break;
        }
        auto state = action.state;
        if (action.type == CapabilityType::Range && state.take("relative").toBool()) {
          const auto range = capabilities[match].parameters.value("range").toMap();
          double value =
              capabilities[match].state.value("value").toDouble() + state.value("value").toDouble();
          if (range.contains("min"))
            value = qMax(value, range.value("min").toDouble());
          if (range.contains("max"))
            value = qMin(value, range.value("max").toDouble());
          state["value"] = value;
        }
        capabilities[match].state = state;
        next[device.id][match] = state;
      }
      if (!result)
        break;
    }
    if (result)
      states_ = std::move(next);
  }
  Deliver(context, std::move(handler), std::move(result), fixture);
}

void FixtureApi::GetVideoStream(const QString& device_id, QObject* context,
                                ApiResultHandler<VideoStream> handler) {
  const auto fixture = ReadFixture();
  ApiResult<VideoStream> result =
      std::unexpected(fixture ? Invalid("No fixture video stream") : fixture.error());
  if (fixture) {
    const auto value = fixture->value("video_streams").toObject().value(device_id).toObject();
    const auto info = fixture->value("device_info").toObject().value(device_id).toObject();
    if (info.value("status") == "ok" && info.value("state") == "online" &&
        value.value("stream_url").isString() && !value.value("stream_url").toString().isEmpty() &&
        value.value("protocol") == "hls") {
      result = VideoStream{.stream_url = value.value("stream_url").toString(), .protocol = "hls"};
      // HLS demuxers resolve segment URLs themselves, outside Qt's resource I/O.
      // Copy the bundled playlist and segment together for entirely local playback.
      if (result->stream_url == "qrc:/debug/camera.m3u8") {
        if (!video_directory_) {
          video_directory_ = std::make_unique<QTemporaryDir>();
          for (const auto* name : {"camera.m3u8", "camera.ts"}) {
            if (!QFile::copy(QString(":/debug/") + name, video_directory_->filePath(name))) {
              video_directory_.reset();
              break;
            }
          }
        }
        if (video_directory_)
          result->stream_url =
              QUrl::fromLocalFile(video_directory_->filePath("camera.m3u8")).toString();
        else
          result = std::unexpected(Invalid("Cannot prepare fixture video"));
      }
    }
  }
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
