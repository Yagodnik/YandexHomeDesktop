#pragma once

#include "yh/yandexoauth_export.h"
#include "IAuthorizationFlow.h"
#include <QObject>
#include <QPointer>

class QNetworkAccessManager;
class YANDEXOAUTH_EXPORT DeviceAuthorizationFlow final : public QObject, public IAuthorizationFlow {
public:
  explicit DeviceAuthorizationFlow(QObject* parent = nullptr,
      QString configPath = ":/auth/secrets.json", QNetworkAccessManager* network = nullptr);
  ~DeviceAuthorizationFlow() override;
  void Start(QObject* context, AuthResultHandler<QString> handler) override;
  void Cancel() override;
  void SetPromptHandler(PromptHandler handler) override { prompt_ = std::move(handler); }
private:
  struct Attempt;
  void RequestCode(Attempt* attempt);
  void Poll(Attempt* attempt);
  void Finish(Attempt* attempt, AuthResult<QString> result);
  QString config_path_;
  QNetworkAccessManager* network_;
  PromptHandler prompt_;
  Attempt* attempt_ = nullptr;
};
