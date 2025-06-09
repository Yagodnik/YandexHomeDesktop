#include "AuthorizationService.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include <QFile>
#include <QDesktopServices>

AuthorizationService::AuthorizationService(QObject *parent) :
  QObject{parent},
  write_job_(kAppName),
  read_job_(kAppName),
  delete_job_(kAppName),
  reply_handler_(kDefaultPort)
{
  read_job_.setAutoDelete(false);
  read_job_.setKey(kSecureKey);

  connect(&read_job_, &QKeychain::ReadPasswordJob::finished, [&]() {
    if (read_job_.error()) {
      qDebug() << "AuthorizationService: Token read error -" << read_job_.errorString();
      return;
    }

    qDebug() << "AuthorizationService: Token -" <<  read_job_.textData();
  });

  write_job_.setAutoDelete(false);
  write_job_.setKey(kSecureKey);

  connect(&write_job_, &QKeychain::WritePasswordJob::finished, [&]() {
    if (write_job_.error()) {
      qWarning() << "AuthorizationService: Token write error -" << write_job_.errorString();
    } else {
      qDebug() << "AuthorizationService: Token stored successfully!";
    }
  });

  delete_job_.setAutoDelete(false);
  delete_job_.setKey(kSecureKey);

  connect(&delete_job_, &QKeychain::DeletePasswordJob::finished, [&]() {
    if (delete_job_.error()) {
      qWarning() << "AuthorizationService: Token delete error -" << delete_job_.errorString();
    } else {
      qDebug() << "AuthorizationService: Token deleted successfully!";
    }
  });

  const auto auth_secrets_object = GetAuthSecrets();

  if (!auth_secrets_object.has_value() || !PrepareCallbackPage()) {
    emit initializationFailed();
    return;
  }

  auto auth_secrets = Serialization::From<AuthSecrets>(
    auth_secrets_object.value());

  oauth2_.setReplyHandler(&reply_handler_);
  oauth2_.setAuthorizationUrl({auth_secrets.auth_url});
  oauth2_.setTokenUrl({auth_secrets.access_token_url});
  oauth2_.setClientIdentifier(auth_secrets.client_id);
  oauth2_.setClientIdentifierSharedKey(auth_secrets.client_secret);
  oauth2_.setRequestedScopeTokens(GetScopes(auth_secrets.scopes));

  connect(&oauth2_,
    &QOAuth2AuthorizationCodeFlow::statusChanged,
    this,
    &AuthorizationService::HandleAuthorizationStatus);

  connect(&oauth2_,
    &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
    this,
    &AuthorizationService::AuthorizeWithBrowser);
}

bool AuthorizationService::IsAuthorized() const {
  return token_.has_value();
}

void AuthorizationService::AttemptAuthorization() {
  oauth2_.grant();
}

void AuthorizationService::Logout() {
  token_.reset();
}

void AuthorizationService::TestWrite() {
  write_job_.setTextData("Hello world write!");
  write_job_.start();
}

void AuthorizationService::TestRead() {
  read_job_.start();
}

void AuthorizationService::TestDelete() {
  delete_job_.start();
}

std::optional<QJsonObject> AuthorizationService::GetAuthSecrets() const {
  QFile auth_secrets_file(kAuthSecretsPath);
  if (!auth_secrets_file.open(QIODevice::ReadOnly)) {
    return std::nullopt;
  }

  const QByteArray data = auth_secrets_file.readAll();
  auth_secrets_file.close();

  QJsonDocument document = QJsonDocument::fromJson(data);
  return document.object();
}

QSet<QByteArray> AuthorizationService::GetScopes(const QStringList &list) {
  QSet<QByteArray> result;

  for (const QString &scope : list) {
    result.insert(scope.toUtf8());
  }

  return result;
}

bool AuthorizationService::PrepareCallbackPage() {
  QFile callback_index(kCallbackPath);
  if (!callback_index.open(QIODevice::ReadOnly)) {
    return false;
  }

  reply_handler_.setCallbackText(callback_index.readAll());
  callback_index.close();

  return true;
}

void AuthorizationService::HandleAuthorizationStatus(const QAbstractOAuth::Status status) {
  switch (status) {
    case QAbstractOAuth::Status::Granted:
      token_ = oauth2_.token();

      write_job_.setTextData(oauth2_.token());
      write_job_.start();

      emit authorized();
    break;
    case QAbstractOAuth::Status::NotAuthenticated:
      emit authorizationFailed();
    break;
    case QAbstractOAuth::Status::RefreshingToken:
      qDebug() << "Auth: Refreshing token";
    break;
    default:
      qDebug() << "Auth: Unknown status";
    break;
  }
}

void AuthorizationService::AuthorizeWithBrowser(QUrl url) {
  QUrlQuery query(url);
  query.addQueryItem("response_type", "code");
  url.setQuery(query);

  QDesktopServices::openUrl(url);
}
