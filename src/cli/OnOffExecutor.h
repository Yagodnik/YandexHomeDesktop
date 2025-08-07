#pragma once
#include "IExecutor.h"

class OnOffExecutor final : public IExecutor {
  Q_OBJECT
public:
  explicit OnOffExecutor(YandexHomeApi *api, QObject *parent);

  void Execute(const QString& name, const QString& value) override;
  void PrintInfo() override;

private slots:
  void OnUserInfoReceived(const UserInfo &userInfo);
  void OnUserInfoReceivingFailed(const QString& message);

  void OnActionExecutionFinishedSuccessfully(const QVariant& user_data);
  void OnActionExecutionFailed(const QString& message, const QVariant& user_data);

private:
  QString target_device_name_;
  QString target_device_id_;

  bool value_;
};
