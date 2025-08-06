#pragma once

#include <QObject>

#include "AppContext.h"


class CliApp : public QObject {
public:
  explicit CliApp(AppContext& app_context, QObject *parent = nullptr);

  int Start();

private slots:
  void onAuthorized();
  void onUnauthorized();
  void onAuthorizationFailed();

private:

  AppContext& app_context_;
};
