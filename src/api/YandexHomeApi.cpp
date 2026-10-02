#include "YandexHomeApi.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "RequestFactory.h"
#include "model/Response.h"

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
  transport_->Get(RequestFactory::CreateBearer(kInfoEndpoint, token_provider_()), context,
                  [handler = std::move(handler)](ApiResult<HttpResponse> response) {
    handler(DecodeResult<UserInfo>(std::move(response)));
  });
}

void YandexHomeApi::GetScenarios(QObject* context,
                                  ApiResultHandler<QList<ScenarioObject>> handler) {
  transport_->Get(RequestFactory::CreateBearer(kInfoEndpoint, token_provider_()), context,
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
  transport_->Get(RequestFactory::CreateBearer(kDeviceInfoEndpoint.arg(id), token_provider_()),
                  context, [handler = std::move(handler)](ApiResult<HttpResponse> response) {
    handler(DecodeResult<DeviceInfo>(std::move(response)));
  });
}

void YandexHomeApi::ExecuteScenario(const QString& id, QObject* context,
                                     ApiResultHandler<void> handler) {
  transport_->Post(RequestFactory::CreateBearer(kExecuteScenarioEndpoint.arg(id), token_provider_()),
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
  QJsonArray json_actions;
  for (const auto& action : actions) {
    json_actions.push_back(Serialization::To(action));
  }
  QJsonObject payload;
  payload["devices"] = json_actions;

  transport_->Post(RequestFactory::CreateBearer(kDevicesActionsEndpoint, token_provider_()),
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
        if (action_result.status == "DONE") {
          handler({});
        } else {
          handler(std::unexpected(ApiError{ApiErrorKind::Service,
                                           action_result.error_code}));
        }
      }
    }
  });
}
