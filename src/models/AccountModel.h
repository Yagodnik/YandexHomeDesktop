#pragma once

#include <QObject>

#include "api/IAccountApi.h"

class AccountModel final : public QObject {
  Q_OBJECT
public:
  explicit AccountModel(IAccountApi* api, QObject* parent = nullptr);

  Q_INVOKABLE void LoadData();
  Q_INVOKABLE [[nodiscard]] QString GetName() const;
  Q_INVOKABLE [[nodiscard]] QString GetAvatarUrl() const;

signals:
  void dataLoaded();
  void dataLoadingFailed();

private:
  IAccountApi* api_;
  QString name_;
  QString avatar_id_;
  const QString kAvatarUrl = "https://avatars.yandex.net/get-yapic/%1/";
};
