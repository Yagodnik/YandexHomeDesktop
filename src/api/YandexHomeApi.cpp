#include "YandexHomeApi.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

#include "RequestFactory.h"
#include "model/Response.h"
#include "serialization/Serialization.h"

namespace {
template<typename T>
ApiResult<T> DecodeResponse(const HttpResponse& response) {
  QJsonParseError parse_error;
  const auto document = QJsonDocument::fromJson(response.body, &parse_error);
  if (parse_error.error != QJsonParseError::NoError || !document.isObject()) {
    const QString message = parse_error.error == QJsonParseError::NoError
      ? "Expected a JSON object" : parse_error.errorString();
    return std::unexpected(ApiError{ApiErrorKind::InvalidResponse, message});
  }

  const T result = Serialization::From<T>(document.object());
  if (result.status != Status::Ok) {
    return std::unexpected(ApiError{ApiErrorKind::Service, result.message});
  }
  return result;
}

template<typename T>
ApiResult<T> DecodeResult(ApiResult<HttpResponse> response) {
  if (!response) {
    return std::unexpected(response.error());
  }
  return DecodeResponse<T>(*response);
}
}

YandexHomeApi::YandexHomeApi(TokenProvider token_provider, IHttpTransport* transport,
                             QObject* parent)
  : QObject(parent), token_provider_(std::move(token_provider)), transport_(transport) {}

void YandexHomeApi::GetUserInfo(QObject* context, ApiResultHandler<UserInfo> handler) {
  const auto request = RequestFactory::CreateAuthorizedBearer(kInfoEndpoint, token_provider_());
  if (!request) { handler(std::unexpected(request.error())); return; }
  transport_->Get(*request, context,
                  [handler = std::move(handler)](ApiResult<HttpResponse> response) {
    handler(DecodeResult<UserInfo>(std::move(response)));
  });
}

void YandexHomeApi::GetScenarios(QObject* context,
                                  ApiResultHandler<QList<ScenarioObject>> handler) {
  const auto request = RequestFactory::CreateAuthorizedBearer(kInfoEndpoint, token_provider_());
  if (!request) { handler(std::unexpected(request.error())); return; }
  transport_->Get(*request, context,
                  [handler = std::move(handler)](ApiResult<HttpResponse> response) {
    auto result = DecodeResult<UserInfo>(std::move(response));
    if (!result) {
      handler(std::unexpected(result.error()));
    } else {
      handler(result->scenarios);
    }
  });
}

void YandexHomeApi::GetDeviceInfo(const QString& id, QObject* context,
                                   ApiResultHandler<DeviceInfo> handler) {
  const auto request = RequestFactory::CreateAuthorizedBearer(kDeviceInfoEndpoint.arg(id), token_provider_());
  if (!request) { handler(std::unexpected(request.error())); return; }
  transport_->Get(*request,
                  context, [handler = std::move(handler)](ApiResult<HttpResponse> response) {
    handler(DecodeResult<DeviceInfo>(std::move(response)));
  });
}

void YandexHomeApi::ExecuteScenario(const QString& id, QObject* context,
                                     ApiResultHandler<void> handler) {
  const auto request = RequestFactory::CreateAuthorizedBearer(kExecuteScenarioEndpoint.arg(id), token_provider_());
  if (!request) { handler(std::unexpected(request.error())); return; }
  transport_->Post(*request,
                   {}, context, [handler = std::move(handler)](ApiResult<HttpResponse> response) {
    auto result = DecodeResult<Response>(std::move(response));
    if (!result) {
      handler(std::unexpected(result.error()));
    } else {
      handler({});
    }
  });
}

void YandexHomeApi::PerformActions(const QList<DeviceActionsObject>& actions, QObject* context,
                                    ApiResultHandler<void> handler) {
  RequestActions(actions, context,
                 [handler = std::move(handler)](ApiResult<DeviceActionResponse> result) {
                   handler(result ? ApiResult<void>{} : std::unexpected(result.error()));
                 });
}

void YandexHomeApi::GetVideoStream(const QString& device_id, QObject* context,
                                   ApiResultHandler<VideoStream> handler) {
  const CapabilityObject action{
      .type = CapabilityType::VideoStream,
      .state = {{"instance", "get_stream"},
                {"value", QVariantMap{{"protocols", QStringList{"hls"}}}}}};
  RequestActions({DeviceActionsObject{.id = device_id, .actions = {action}}}, context,
                 [device_id, handler = std::move(handler)](ApiResult<DeviceActionResponse> result) {
                   if (!result) {
                     handler(std::unexpected(result.error()));
                     return;
                   }
                   std::optional<VideoStream> stream;
                   for (const auto& device : result->devices) {
                     if (device.id != device_id)
                       continue;
                     for (const auto& capability : device.capabilities) {
                       if (capability.type != "devices.capabilities.video_stream" ||
                           capability.state.instance != "get_stream")
                         continue;
                       const auto value = Serialization::TryFrom<VideoStream>(
                           QJsonObject::fromVariantMap(capability.state.value));
                       const QUrl url(value ? value->stream_url : QString{}, QUrl::StrictMode);
                       if (stream || !value || value->protocol != "hls" || !url.isValid() ||
                           url.host().isEmpty() ||
                           (url.scheme() != "https" && url.scheme() != "http")) {
                         handler(std::unexpected(ApiError{ApiErrorKind::InvalidResponse,
                                                          "Invalid video stream response"}));
                         return;
                       }
                       stream = *value;
                     }
                   }
                   if (!stream) {
                     handler(std::unexpected(
                         ApiError{ApiErrorKind::InvalidResponse, "Missing video stream response"}));
                     return;
                   }
                   handler(*stream);
                 });
}

void YandexHomeApi::RequestActions(const QList<DeviceActionsObject>& actions, QObject* context,
                                   ApiResultHandler<DeviceActionResponse> handler) {
  const auto request = RequestFactory::CreateAuthorizedBearer(kDevicesActionsEndpoint, token_provider_());
  if (!request) { handler(std::unexpected(request.error())); return; }
  QJsonArray json_actions;
  for (const auto& action : actions) {
    json_actions.push_back(Serialization::To(action));
  }
  QJsonObject payload;
  payload["devices"] = json_actions;

  transport_->Post(*request,
                   QJsonDocument(payload).toJson(), context,
                   [handler = std::move(handler)](ApiResult<HttpResponse> response) {
    auto result = DecodeResult<DeviceActionResponse>(std::move(response));
    if (!result) {
      handler(std::unexpected(result.error()));
      return;
    }

    for (const auto& device : result->devices) {
      for (const auto& capability : device.capabilities) {
        const auto& action_result = capability.state.action_result;
        if (action_result.status != "DONE") {
          handler(std::unexpected(ApiError{ApiErrorKind::Service,
                                           action_result.error_code}));
          return;
        }
      }
    }
    handler(std::move(result));
  });
}
