#include "ListDevicesCommand.h"

#include <iostream>
#include <unistd.h>

ListDevicesCommand::ListDevicesCommand(QObject *parent) : ICommand("list-devices", parent) {}

void ListDevicesCommand::Execute(AppContext &app_ctx, const CommandContext &command_ctx) {
  connect(
    app_ctx.yandex_api,
    &YandexHomeApi::userInfoReceived,
    &ListDevicesCommand::OnUserInfoReceived);

  connect(
    app_ctx.yandex_api,
    &YandexHomeApi::userInfoReceivingFailed,
    &ListDevicesCommand::OnUserInfoReceivingFailed);

  app_ctx.yandex_api->GetUserInfo();
}

void ListDevicesCommand::OnUserInfoReceived(const UserInfo &info) {
  std::cout << "List of devices:" << std::endl;

  int index = 1;
  for (const auto& device : info.devices) {
    std::cout << index << ") " << device.name.toStdString() << " "
              << device.type.toStdString() << std::endl;
    index++;
  }

  QGuiApplication::quit();
}

void ListDevicesCommand::OnUserInfoReceivingFailed(const QString &error) {
  std::cout << "Something went wrong..." << std::endl;
  std::cout << error.toStdString() << std::endl;

  QGuiApplication::quit();
}
