#include "ListDevicesCommand.h"

#include <iostream>
#include <QGuiApplication>

ListDevicesCommand::ListDevicesCommand(YandexHomeApi *api, QObject *parent)
  : QObject(parent), api_(api)
{
  connect(api_, &YandexHomeApi::userInfoReceived, this, &ListDevicesCommand::OnUserInfoReceived);
  connect(api_, &YandexHomeApi::userInfoReceivingFailed, this, &ListDevicesCommand::OnUserInfoReceivingFailed);

  api_->GetUserInfo();
}

void ListDevicesCommand::OnUserInfoReceived(const UserInfo &info) {
  QTextStream out(stdout);
  out << "List of devices:" << Qt::endl;

  int index = 1;
  for (const auto& device : info.devices) {
    out << index << ") " << device.name << " " << device.type << Qt::endl;
    index++;
  }

  QGuiApplication::quit();
}

void ListDevicesCommand::OnUserInfoReceivingFailed(const QString &error) {
  QTextStream out(stdout);
  out << "Something went wrong..." << Qt::endl;
  out << error << Qt::endl;

  QGuiApplication::quit();
}
