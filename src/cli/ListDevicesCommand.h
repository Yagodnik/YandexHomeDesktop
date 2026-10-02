#pragma once

#include <QObject>

#include "api/IHomeApi.h"

class ListDevicesCommand : public QObject {
  Q_OBJECT
public:
  explicit ListDevicesCommand(IHomeApi* api, QObject *parent = nullptr);

private slots:
  static void OnUserInfoReceived(const UserInfo& info);
  static void OnUserInfoReceivingFailed(const QString& error);

private:
  IHomeApi* api_;
};
