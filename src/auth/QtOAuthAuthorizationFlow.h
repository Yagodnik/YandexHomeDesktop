#pragma once

#include "yh/yandexoauth_export.h"

#include <QObject>
#include <QUrl>
#include "IAuthorizationFlow.h"

class YANDEXOAUTH_EXPORT QtOAuthAuthorizationFlow final : public QObject, public IAuthorizationFlow {
public:
  using BrowserOpener = std::function<bool(const QUrl&)>;
  explicit QtOAuthAuthorizationFlow(QObject* parent = nullptr,
      QString config_path = ":/auth/secrets.json", QString callback_path = ":/callback/index.html",
      BrowserOpener open_browser = {});
  ~QtOAuthAuthorizationFlow() override;
  void Start(QObject* context, AuthResultHandler<QString> handler) override;
  void Cancel() override;

private:
  struct Attempt;
  void Finish(Attempt* attempt, AuthResult<QString> result);
  QString config_path_;
  QString callback_path_;
  BrowserOpener open_browser_;
  Attempt* attempt_ = nullptr;
};
