#pragma once

#include <QObject>

#include "api/IAccountApi.h"

class AccountModel final : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString accountId READ GetAccountId NOTIFY dataLoaded)
public:
  explicit AccountModel(IAccountApi* api, QObject* parent = nullptr);

  Q_INVOKABLE void LoadData();
  void EnsureLoaded();
  [[nodiscard]] QString GetAccountId() const;
  Q_INVOKABLE [[nodiscard]] QString GetName() const;
  Q_INVOKABLE [[nodiscard]] QString GetAvatarUrl() const;
  Q_INVOKABLE [[nodiscard]] QString GetEmail() const;
  void Reset();

signals:
  void dataLoaded();
  void dataLoadingFailed();

private:
  IAccountApi* api_;
  QString name_;
  QString avatar_id_;
  QString email_;
  QString account_id_;
  bool loading_ = false;
  quint64 generation_ = 0;
  const QString kAvatarUrl = "https://avatars.yandex.net/get-yapic/%1/";
};
