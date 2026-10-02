#include "ListDevicesCommand.h"

#include <iostream>
#include <unistd.h>

ListDevicesCommand::ListDevicesCommand(QObject *parent) : ICommand("list-devices", parent) {}

void ListDevicesCommand::Execute(AppContext &app_ctx, const CommandContext &command_ctx) {
  app_ctx.yandex_api->GetUserInfo(this, [](ApiResult<UserInfo> result) {
    if (result) {
      OnUserInfoReceived(*result);
    } else {
      OnUserInfoReceivingFailed(result.error().message);
    }
  });
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
