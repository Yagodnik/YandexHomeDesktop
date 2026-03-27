#include "YandexTokenAuthorizationService.h"

#include "QtKeyChainSecretsStorage.h"
#include <QDebug>
#include <QDesktopServices>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QUrl>

#include "YandexOAuthSecrets.h"

YandexTokenAuthorizationService::YandexTokenAuthorizationService(
  QNetworkAccessManager* network_manager,
  QObject *parent
) :
  IAuthorizationService(parent),
  network_manager_(network_manager)
{
  secrets_storage_ = std::make_unique<QtKeyChainSecretsStorage>(kAppName, kSecureKey);
  auth_secrets_ = std::make_unique<YandexOAuthSecrets>();

  const auto auth_secrets = auth_secrets_->GetAuthSecrets();

  if (!auth_secrets.has_value()) {
    qCritical() << "AuthorizationService: Initialization failed";
    emit initializationFailed();
    return;
  }

  code_req_url_ = GetAuthCodeUrl("https://oauth.yandex.ru/authorize", auth_secrets.value());
}

void YandexTokenAuthorizationService::TryLoadTokenFromStorage() {
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

bool YandexTokenAuthorizationService::IsAuthorized() const {
  return token_.has_value();
}

void YandexTokenAuthorizationService::AttemptAuthorization(const QVariant& user_data) {
  QDesktopServices::openUrl(code_req_url_);
}

void YandexTokenAuthorizationService::SaveAuthToken(const QString &token) {
  const auto auth_secrets = auth_secrets_->GetAuthSecrets().value();

  QNetworkRequest request(QUrl("https://oauth.yandex.ru/token"));

  // TODO: Load client secret
  const QByteArray basic = QString("%1:%2")
    .arg(auth_secrets.client_id)
    .arg("")
    .toUtf8()
    .toBase64();

  const QByteArray auth_header = QString("Basic %1")
    .arg(basic)
    .toUtf8();

  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
  request.setRawHeader("Authorization", auth_header);

  QUrlQuery query;
  query.addQueryItem("grant_type", "authorization_code");
  query.addQueryItem("code", token);

  const QByteArray data = query.toString(QUrl::FullyEncoded).toUtf8();
  QNetworkReply* reply = network_manager_->post(request, data);

  connect(reply, &QNetworkReply::finished, this, [reply, this]() {
    if (reply->error() == QNetworkReply::NoError) {
      const QByteArray response = reply->readAll();
      qDebug() << "Success:" << response;

      // TODO: Move to special parser for response
      const QJsonDocument json = QJsonDocument::fromJson(response);
      const QString auth_token = json.object().value("access_token").toString();

      // TODO: Write all tokens
      secrets_storage_->TryWrite(auth_token, [this, auth_token](std::optional<ISecretsStorage::Error> error) {
        if (error.has_value()) {
          const auto [error_code, error_text] = error.value();

          qWarning() << "AuthorizationService: Token write error -" << error_code << " " << error_text;
          emit authorizationFailed();
        } else {
          qInfo() << "AuthorizationService: Token stored successfully " << auth_token;

          token_.reset();
          token_ = auth_token;

          emit authorized();
        }
      });
    } else {
      qDebug() << "Error:" << reply->errorString();
    }
    reply->deleteLater();
  });
}

void YandexTokenAuthorizationService::Logout() {
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

QString YandexTokenAuthorizationService::GetLastErrorCode() const {
  QString number;
  number.setNum(last_error_code_, 16);
  return number;
}

std::optional<QString> YandexTokenAuthorizationService::GetToken() const {
  return token_;
}

QUrl YandexTokenAuthorizationService::GetAuthCodeUrl(
  const QString& base_url,
  const YandexOAuthSecrets::AuthSecrets& secrets
) {
  QUrl url(base_url);

  QUrlQuery query(url);

  // Allows switching account during authorization
  query.addQueryItem("force_confirm", "yes");
  query.addQueryItem("response_type", "code");
  query.addQueryItem("client_id", secrets.client_id);

  url.setQuery(query);

  return url;
}

QUrl YandexTokenAuthorizationService::GetAuthTokenUrl(
  const QString& base_url,
  const QString& code
) {
  QUrl url(base_url);

  QUrlQuery query(url);
  query.addQueryItem("grant_type", "authorization_code");
  query.addQueryItem("code", code);

  url.setQuery(query);

  return url;
}
