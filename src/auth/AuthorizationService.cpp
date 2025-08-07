#include "AuthorizationService.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include <QFile>
#include <QDesktopServices>

AuthorizationService::AuthorizationService(QObject *parent) :
  QObject{parent},
  reply_handler_(kDefaultPort)
{
  const auto auth_secrets_object = GetAuthSecrets();

  if (!auth_secrets_object.has_value() || !PrepareCallbackPage()) {
    qCritical() << "AuthorizationService: Initialization failed";
    emit initializationFailed();
    return;
  }

  auto auth_secrets = Serialization::From<AuthSecrets>(
    auth_secrets_object.value());

  oauth2_.setReplyHandler(&reply_handler_);
  oauth2_.setAuthorizationUrl({auth_secrets.auth_url});
  oauth2_.setTokenUrl({auth_secrets.access_token_url});
  oauth2_.setClientIdentifier(auth_secrets.client_id);
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

void AuthorizationService::AttemptLocalAuthorization() {
  TryRead();
}

bool AuthorizationService::IsAuthorized() const {
  return token_.has_value();
}

void AuthorizationService::AttemptAuthorization() {
  oauth2_.grant();
}

void AuthorizationService::Logout() {
  token_.reset();
  TryDelete();

  emit logout();
}

QString AuthorizationService::GetLastErrorCode() const {
  QString number;
  number.setNum(last_error_code_, 16);
  return number;
}

std::optional<QString> AuthorizationService::GetToken() const {
  if (!token_.has_value()) {
    qCritical() << "AuthorizationService::GetToken: no token provided";
    return std::nullopt;
  }

  return token_.value();
}

void AuthorizationService::TryWrite(const QString& key) {
  auto *job = new QKeychain::WritePasswordJob(kAppName);
  job->setKey(kSecureKey);
  job->setTextData(key);

  connect(job, &QKeychain::Job::finished, [job, this]() {
    WriteTokenHandler(job);

    job->deleteLater();
  });

  job->start();
}

void AuthorizationService::TryRead() {
  auto *job = new QKeychain::ReadPasswordJob(kAppName);
  job->setKey(kSecureKey);

  connect(job, &QKeychain::Job::finished, [job, this]() {
    ReadTokenHandler(job);

    job->deleteLater();
  });

  job->start();
}

void AuthorizationService::TryDelete() {
  auto *job = new QKeychain::DeletePasswordJob(kAppName);
  job->setKey(kSecureKey);

  connect(job, &QKeychain::Job::finished, [job, this]() {
    DeleteTokenHandler(job);

    job->deleteLater();
  });

  job->start();
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
  last_error_code_ = (1 << 31) | static_cast<int>(status);

  switch (status) {
    case QAbstractOAuth::Status::Granted:
      qInfo() << "AuthorizationService: Access granted!";
      token_ = oauth2_.token();

      TryWrite(oauth2_.token());

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

void AuthorizationService::AuthorizeWithBrowser(QUrl url) {
  QUrlQuery query(url);
  query.addQueryItem("response_type", "code");
  url.setQuery(query);

  QDesktopServices::openUrl(url);
}

void AuthorizationService::ReadTokenHandler(QKeychain::ReadPasswordJob *job) {
  last_error_code_ = static_cast<int>(job->error());

  switch (job->error()) {
    case QKeychain::NoError:
      break;
    case QKeychain::EntryNotFound:
      qCritical() << "AuthorizationService: Key does NOT exist.";

      emit unauthorized();
      return;
    case QKeychain::AccessDeniedByUser:
      qWarning() << "AuthorizationService: User canceled operation";

      emit authorizationCanceled();
      return;
    default:
      qCritical() << "AuthorizationService: Token read error -" << job->errorString();

      emit authorizationFailed();
      return;
  }

  token_ = job->textData();

  qDebug() << "AuthorizationService: Token:" << token_.value();

  emit authorized();
}

void AuthorizationService::WriteTokenHandler(QKeychain::WritePasswordJob *job) {
  if (job->error()) {
    qWarning() << "AuthorizationService: Token write error -" << job->errorString();
  } else {
    qInfo() << "AuthorizationService: Token stored successfully!";
  }
}

void AuthorizationService::DeleteTokenHandler(QKeychain::DeletePasswordJob *job) {
  if (job->error()) {
    qWarning() << "AuthorizationService: Token delete error -" << job->errorString();
    emit logoutFailed(job->errorString());
  } else {
    qInfo() << "AuthorizationService: Token deleted successfully!";
    emit logoutFinished();
  }
}
