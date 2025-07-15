#pragma once
#include "IExecutor.h"

class OnOffExecutor final : public IExecutor {
  Q_OBJECT
public:
  explicit OnOffExecutor(YandexHomeApi *api);

  void Execute(const QString& value) override;
  void PrintInfo() override;

private slots:
  void OnUserInfoReceived(const UserInfo &userInfo);

private:
  bool value_;
};
