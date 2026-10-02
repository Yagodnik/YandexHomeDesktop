#include "OnOffExecutor.h"

#include <QDebug>
#include <QGuiApplication>

#include "iot/capabilities/OnOffCapability.h"

OnOffExecutor::OnOffExecutor(IHomeApi *api) : IExecutor(api) {}

void OnOffExecutor::Execute(const QString& value) {
  qDebug() << "OnOffExecutor::Execute(" << value << ")";
  qDebug() << api_;

  if (value == "on") {
    value_ = true;
  } else if (value == "off") {
    value_ = false;
  } else {
    qDebug() << "Incorrect value for OnOff:" << value;
    QGuiApplication::quit();
  }

  api_->GetUserInfo(this, [this](ApiResult<UserInfo> result) {
    if (result) {
      OnUserInfoReceived(*result);
    }
  });
}

void OnOffExecutor::PrintInfo() {
  qDebug() << "On Off Capability Info";
  qDebug() << "Sets on off value";
}

void OnOffExecutor::OnUserInfoReceived(const UserInfo &userInfo) {
  qDebug() << "On User Info Received";

  // Look for right device
  // Call capability with value
}
