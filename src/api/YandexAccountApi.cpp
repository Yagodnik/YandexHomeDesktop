#include "YandexAccountApi.h"

#include <QJsonDocument>

#include "RequestFactory.h"
#include "serialization/Serialization.h"

namespace {
JSON_STRUCT(AccountResponse,
  (QString, login),
  (QString, display_name),
  (QString, default_avatar_id),
  (QString, default_email)
);
}

YandexAccountApi::YandexAccountApi(TokenProvider token_provider, IHttpTransport* transport,
                                   QObject* parent)
  : QObject(parent), token_provider_(std::move(token_provider)), transport_(transport) {}

void YandexAccountApi::LoadData(QObject* context, ApiResultHandler<AccountInfo> handler) {
  transport_->Get(RequestFactory::CreateBearer(kAccountInfoEndpoint, token_provider_()), context,
                  [handler = std::move(handler)](ApiResult<HttpResponse> response) {
    if (!response) {
      handler(std::unexpected(response.error()));
      return;
    }

    QJsonParseError parse_error;
    const auto document = QJsonDocument::fromJson(response->body, &parse_error);
    if (parse_error.error != QJsonParseError::NoError || !document.isObject()) {
      const QString message = parse_error.error == QJsonParseError::NoError
        ? "Expected a JSON object" : parse_error.errorString();
      handler(std::unexpected(ApiError{ApiErrorKind::InvalidResponse, message}));
      return;
    }

    const auto data = Serialization::From<AccountResponse>(document.object());
    handler(AccountInfo{data.display_name, data.default_avatar_id, data.default_email});
  });
}
