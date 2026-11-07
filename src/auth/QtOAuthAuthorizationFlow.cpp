#include "QtOAuthAuthorizationFlow.h"

#include <QDesktopServices>
#include <QFile>
#include <QPointer>
#include <QTimer>
#include <QtNetworkAuth/QOAuth2AuthorizationCodeFlow>
#include <QtNetworkAuth/QOAuthHttpServerReplyHandler>
#include "OAuthConfiguration.h"

struct QtOAuthAuthorizationFlow::Attempt : QObject {
  using QObject::QObject;
  QPointer<QObject> context;
  AuthResultHandler<QString> handler;
  QOAuth2AuthorizationCodeFlow* oauth = nullptr;
  QOAuthHttpServerReplyHandler* reply = nullptr;
};

QtOAuthAuthorizationFlow::QtOAuthAuthorizationFlow(QObject* parent, QString config_path,
    QString callback_path, BrowserOpener open_browser)
  : QObject(parent), config_path_(std::move(config_path)), callback_path_(std::move(callback_path)),
    open_browser_(open_browser ? std::move(open_browser) : QDesktopServices::openUrl) {}

QtOAuthAuthorizationFlow::~QtOAuthAuthorizationFlow() { Cancel(); }

void QtOAuthAuthorizationFlow::Start(QObject* context, AuthResultHandler<QString> handler) {
  Cancel();
  auto* attempt = new Attempt(this);
  attempt_ = attempt;
  attempt->context = context;
  attempt->handler = std::move(handler);
  connect(context, &QObject::destroyed, attempt, [this, attempt] {
    if (attempt_ == attempt) { Cancel(); }
  });
  // Report setup failures after consumers have connected to the service/model.
  // Saved-token reads and constructing a CLI service never load these files.
  QTimer::singleShot(0, attempt, [this, attempt] {
    if (attempt_ != attempt) { return; }
    QFile config_file(config_path_);
    QFile callback_file(callback_path_);
    if (!config_file.open(QIODevice::ReadOnly) || !callback_file.open(QIODevice::ReadOnly)) {
      Finish(attempt, std::unexpected(AuthError{AuthErrorKind::Initialization, 0x80010002u, {}}));
      return;
    }
    const auto config = OAuthConfiguration::Parse(config_file.readAll());
    if (!config) { Finish(attempt, std::unexpected(config.error())); return; }
    const auto address = config->redirect_base.host() == "::1"
      ? QHostAddress::LocalHostIPv6 : QHostAddress::LocalHost;
    attempt->reply = new QOAuthHttpServerReplyHandler(address, config->redirect_port, attempt);
    if (!attempt->reply->isListening()) {
      Finish(attempt, std::unexpected(AuthError{AuthErrorKind::Initialization, 0x80010003u, {}}));
      return;
    }
    attempt->reply->setCallbackHost(config->redirect_base.host());
    attempt->reply->setCallbackPath(config->redirect_base.path().isEmpty() ? "/" : config->redirect_base.path());
    attempt->reply->setCallbackText(QString::fromUtf8(callback_file.readAll()));
    attempt->oauth = new QOAuth2AuthorizationCodeFlow(attempt);
    auto* oauth = attempt->oauth;
    oauth->setReplyHandler(attempt->reply);
    oauth->setAuthorizationUrl(config->authorization_url);
    oauth->setTokenUrl(config->token_url);
    oauth->setClientIdentifier(config->client_id);
    oauth->setClientIdentifierSharedKey(config->client_secret);
    oauth->setRequestedScopeTokens(config->scopes);
    connect(oauth, &QAbstractOAuth::granted, attempt, [this, attempt] {
      Finish(attempt, attempt->oauth->token());
    });
    connect(oauth, &QAbstractOAuth::requestFailed, attempt, [this, attempt](QAbstractOAuth::Error error) {
      Finish(attempt, std::unexpected(AuthError{AuthErrorKind::Authorization,
        0x80000100u | static_cast<quint32>(error), {}}));
    });
    connect(oauth, &QAbstractOAuth2::serverReportedErrorOccurred, attempt,
      [this, attempt](const QString& error, const QString&, const QUrl&) {
        if (error == "access_denied") {
          Finish(attempt, std::unexpected(AuthError{AuthErrorKind::Canceled, 0x80000106u, {}}));
        }
      });
    connect(oauth, &QAbstractOAuth::authorizeWithBrowser, attempt, [this, attempt](const QUrl& url) {
      if (!open_browser_(url)) {
        Finish(attempt, std::unexpected(AuthError{AuthErrorKind::Authorization, 0x80010004u, {}}));
      }
    });
    oauth->grant();
  });
}

void QtOAuthAuthorizationFlow::Finish(Attempt* attempt, AuthResult<QString> result) {
  if (attempt_ != attempt) { return; }
  auto handler = std::move(attempt->handler);
  const auto context = attempt->context;
  Cancel();
  if (context) { handler(std::move(result)); }
}

void QtOAuthAuthorizationFlow::Cancel() {
  if (!attempt_) { return; }
  auto* attempt = attempt_;
  attempt_ = nullptr;
  if (attempt->reply) { attempt->reply->close(); }
  if (attempt->oauth) { attempt->oauth->disconnect(attempt); }
  // A result can cancel its own attempt while Qt is emitting a flow signal.
  attempt->deleteLater();
}
