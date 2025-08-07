#pragma once

#include <QObject>

#include "ICommand.h"
#include "api/YandexHomeApi.h"

class ListDevicesCommand : public ICommand {
  Q_OBJECT
public:
  explicit ListDevicesCommand(QObject *parent);

  void Execute(AppContext &app_ctx, const CommandContext &command_ctx) override;

private slots:
  static void OnUserInfoReceived(const UserInfo& info);
  static void OnUserInfoReceivingFailed(const QString& error);
};

