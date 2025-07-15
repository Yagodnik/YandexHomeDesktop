#pragma once

#include <QObject>

#include "api/YandexHomeApi.h"

class ListDevicesCommand : public QObject {
  Q_OBJECT
public:
  explicit ListDevicesCommand(YandexHomeApi* api, QObject *parent = nullptr);

private slots:
  static void OnUserInfoReceived(const UserInfo& info);
  static void OnUserInfoReceivingFailed(const QString& error);

private:
  YandexHomeApi* api_;
};

