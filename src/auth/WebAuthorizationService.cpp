#include "WebAuthorizationService.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include <QFile>
#include <QDesktopServices>

#include "QtKeyChainSecretsStorage.h"

WebAuthorizationService::WebAuthorizationService(QObject *parent) :
  IAuthorizationService(parent),
  reply_handler_(kDefaultPort)
{
  secrets_storage_ = std::make_unique<QtKeyChainSecretsStorage>(kAppName, kSecureKey);
  auth_secrets_ = std::make_unique<YandexOAuthSecrets>();

  const auto auth_secrets = auth_secrets_->GetAuthSecrets();

  if (!auth_secrets.has_value() || !PrepareCallbackPage()) {
    qCritical() << "AuthorizationService: Initialization failed";
    emit initializationFailed();
    return;
  }

  oauth2_.setReplyHandler(&reply_handler_);
  oauth2_.setAuthorizationUrl({auth_secrets.value().auth_url});
  oauth2_.setTokenUrl({auth_secrets.value().access_token_url});
  oauth2_.setClientIdentifier(auth_secrets.value().client_id);
  oauth2_.setRequestedScopeTokens(GetScopes(auth_secrets.value().scopes));

  connect(&oauth2_,
    &QOAuth2AuthorizationCodeFlow::statusChanged,
    this,
    &WebAuthorizationService::HandleAuthorizationStatus);

  connect(&oauth2_,
    &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
    this,
    &WebAuthorizationService::AuthorizeWithBrowser);
}

void WebAuthorizationService::TryLoadTokenFromStorage() {
  secrets_storage_->TryRead([this](std::expected<QString, ISecretsStorage::Error> result) {
    if (!result.has_value()) {
      const auto [errorCode, errorText] = result.error();
      last_error_code_ = static_cast<int>(errorCode);

      switch (errorCode) {
        case QKeychain::EntryNotFound:
          qCritical() << "AuthorizationService: Key does NOT exist.";

          emit unauthorized();
          break;
        case QKeychain::AccessDeniedByUser:
          qWarning() << "AuthorizationService: User canceled operation";

          emit authorizationCanceled();
          break;
        default:
          qCritical() << "AuthorizationService: Token read error -" << errorText;

          emit authorizationFailed();
          break;
      }
    } else {
      token_ = result.value();

      qDebug() << "AuthorizationService: Token:" << token_.value();

      emit authorized();
    }
  });
}

bool WebAuthorizationService::IsAuthorized() const {
  return token_.has_value();
}

void WebAuthorizationService::AttemptAuthorization(const QVariant& user_data) {
  oauth2_.grant();
}

void WebAuthorizationService::SaveAuthToken(const QString &token) {
  secrets_storage_->TryWrite(token, [](std::optional<ISecretsStorage::Error> error) {
    if (error.has_value()) {
      const auto [error_code, error_text] = error.value();

      qWarning() << "AuthorizationService: Token write error -" << error_code << " " << error_text;
    } else {
      qInfo() << "AuthorizationService: Token stored successfully!";
    }
  });
}

void WebAuthorizationService::Logout() {
  token_.reset();

  secrets_storage_->TryDelete([this](std::optional<ISecretsStorage::Error> error) {
    if (error.has_value()) {
      const auto [error_code, error_text] = error.value();

      qWarning() << "AuthorizationService: Token delete error -" << error_code;
      emit logoutFailed(error_text);
    } else {
      qInfo() << "AuthorizationService: Token deleted successfully!";
      emit logoutFinished();
    }
  });

  emit logout();
}

QString WebAuthorizationService::GetLastErrorCode() const {
  QString number;
  number.setNum(last_error_code_, 16);
  return number;
}

std::optional<QString> WebAuthorizationService::GetToken() const {
  if (!token_.has_value()) {
    qCritical() << "AuthorizationService::GetToken: no token provided";
    return std::nullopt;
  }

  return token_.value();
}

QSet<QByteArray> WebAuthorizationService::GetScopes(const QStringList &list) {
  QSet<QByteArray> result;

  for (const QString &scope : list) {
    result.insert(scope.toUtf8());
  }

  return result;
}

bool WebAuthorizationService::PrepareCallbackPage() {
  QFile callback_index(kCallbackPath);
  if (!callback_index.open(QIODevice::ReadOnly)) {
    return false;
  }

  reply_handler_.setCallbackText(callback_index.readAll());
  callback_index.close();

  return true;
}

void WebAuthorizationService::HandleAuthorizationStatus(const QAbstractOAuth::Status status) {
  last_error_code_ = (1 << 31) | static_cast<int>(status);

  switch (status) {
    case QAbstractOAuth::Status::Granted:
      qInfo() << "AuthorizationService: Access granted!";
      token_ = oauth2_.token();

      SaveAuthToken(oauth2_.token());

      emit authorized();
      break;
    case QAbstractOAuth::Status::NotAuthenticated:
      qInfo() << "AuthorizationService: NotAuthenticated";
      emit authorizationFailed();
      break;
    case QAbstractOAuth::Status::RefreshingToken:
      qInfo() << "AuthorizationService: Refreshing token";
      break;
    case QAbstractOAuth::Status::TemporaryCredentialsReceived:
      qInfo() << "AuthorizationService: TemporaryCredentialsReceived";
      break;
    default:
      qWarning() << "AuthorizationService: Unknown status!";
      emit authorizationFailed();
      break;
  }
}

void WebAuthorizationService::AuthorizeWithBrowser(QUrl url) {
  QUrlQuery query(url);
  query.addQueryItem("response_type", "code");
  url.setQuery(query);

  QDesktopServices::openUrl(url);
}
