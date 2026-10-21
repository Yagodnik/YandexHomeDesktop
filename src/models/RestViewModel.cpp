#include "RestViewModel.h"

namespace {
constexpr int StatusRefreshIntervalMs = 2'000;
}

RestViewModel::RestViewModel(RestControlService* service, QObject* parent)
  : QObject(parent), service_(service) {
  connect(service_, &RestControlService::finished, this, [this](const QJsonObject& result) {
    if (result["ok"].toBool()) {
      enabled_ = result["enabled"].toBool();
      running_ = result["running"].toBool();
      url_ = result["url"].toString();
    } else {
      error_ = result["error"].toObject()["message"].toString();
    }
    emit changed();
  });
  poll_.setInterval(StatusRefreshIntervalMs);
  connect(&poll_, &QTimer::timeout, this, &RestViewModel::Refresh);
  poll_.start();
  QTimer::singleShot(0, this, &RestViewModel::Refresh);
}

bool RestViewModel::IsEnabled() const {
  return enabled_;
}

bool RestViewModel::IsRunning() const {
  return running_;
}

bool RestViewModel::IsBusy() const {
  return service_->IsBusy();
}

QString RestViewModel::GetUrl() const {
  return url_;
}

QString RestViewModel::GetError() const {
  return error_;
}

void RestViewModel::SetEnabled(bool enabled) {
  if (IsBusy() && enabled) {
    return;
  }
  error_.clear();
  service_->Request(
    enabled ? RestControlService::Command::Enable : RestControlService::Command::Disable);
  emit changed();
}

void RestViewModel::Refresh() {
  if (IsBusy()) {
    return;
  }
  service_->Request(RestControlService::Command::Status);
}
