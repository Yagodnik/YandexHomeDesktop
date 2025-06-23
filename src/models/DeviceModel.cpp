#include "DeviceModel.h"

#include <sys/fcntl.h>

DeviceModel::DeviceModel(YandexHomeApi *api, QObject *parent)
  : QAbstractListModel(parent), api_(api)
{
  connect(api_,
    &YandexHomeApi::deviceInfoReceived,
    this,
    &DeviceModel::OnDeviceInfoReceived);

  connect(api_,
    &YandexHomeApi::deviceInfoReceivingFailed,
    this,
    &DeviceModel::OnDeviceInfoReceivingFailed);

  connect(api_,
    &YandexHomeApi::actionExecutingFinishedSuccessfully,
    this,
    &DeviceModel::OnActionExecutionFinishedSuccessfully);

  connect(api_,
    &YandexHomeApi::actionExecutingFailed,
    this,
    &DeviceModel::OnActionExecutionFailed);

  timer_.setInterval(3000);
  timer_.setSingleShot(false);

  connect(&timer_, &QTimer::timeout, [this]() {
    qDebug() << "Requesting update:" << QString::number(
      static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000,
      'f', 3
    );

    RequestData(device_id_);
  });
}

int DeviceModel::rowCount(const QModelIndex &parent) const {
  return capabilities_.size();
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= capabilities_.size()) {
    return {};
  }

  const auto& capability = capabilities_.at(index.row());

  switch (role) {
    case IdRole:
      return device_id_;
    case NameRole:
      return CapabilityType::operator[](capability.type);
    case LastUpdateTimeRole:
      return capability.last_updated;
    case DelegateSourceRole: {
      if (kDelegates.contains(capability.type)) {
        return kDelegates[capability.type];
      }

      return kUnsupportedDelegate;
    }
    case BusyRole:
      return pending_[index.row()].is_pending;
    default:
      return {};
  }
}

QHash<int, QByteArray> DeviceModel::roleNames() const {
  return {
    { IdRole, "deviceId" },
    { NameRole, "name"},
    { DelegateSourceRole, "delegateSource" },
    { BusyRole, "busy"}
  };
}

void DeviceModel::RequestData(const QString& device_id) {
  device_id_ = device_id;

  // beginResetModel();
  //
  // capabilities_.clear();
  //
  // endResetModel();

  // capabilities_.clear();

  last_update_start_time_ = static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000;

  qDebug() << "Update requested:" << QString::number(
    last_update_start_time_, 'f', 3
  );

  api_->GetDeviceInfo(device_id_);
}

CapabilityObject DeviceModel::GetCapabilityInfo(const int index) const {
  return capabilities_.at(index);
}

QVariantMap DeviceModel::GetState(const int index) const {
  const auto& capability = capabilities_.at(index);

  return capability.state;
}

QVariantMap DeviceModel::GetParameters(const int index) const {
  const auto& capability = capabilities_.at(index);

  return capability.parameters;
}

void DeviceModel::UseCapability(const int index, const QVariantMap &state) {
  pending_[index].is_pending = true;
  pending_[index].action_start_time = static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000;

  const auto model_index = createIndex(index, 0);
  emit dataChanged(model_index, model_index);

  auto& capability = capabilities_[index];

  qDebug() << "Previously updated:" << QString::number(capability.last_updated, 'f', 3);

  const CapabilityObject action = {
    .type = capability.type,
    .state = state
  };

  const DeviceActionsObject action_object = {
    .id = device_id_,
    .actions = { action }
  };

  api_->PerformActions({
    action_object
  }, index);

  capability.last_updated = static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000.0;

  qDebug() << "Action request time:" << QString::number(capability.last_updated, 'f', 3);
}


void DeviceModel::OnDeviceInfoReceived(const DeviceObject &info) {
  const double receive_time = static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000;

  qDebug() << "Update received:" << QString::number(
    receive_time, 'f', 3
  );

  bool empty = capabilities_.empty();

  if (empty) {
    beginResetModel();
  }

  if (empty) {
    pending_.resize(info.capabilities.size());
    pending_.fill({});
  }

  if (empty) {
    capabilities_.resize(info.capabilities.size());
  }

  const double ignore_delta = 0.8;

  for (int i = 0; i < capabilities_.size(); ++i) {
    const auto& incoming = info.capabilities[i];
    auto& pending = pending_[i];

    if (pending.is_pending) {
      qDebug() << "Blocking update because it is processing";
      continue;
    }

    if (receive_time >= pending.action_start_time - ignore_delta
      && receive_time <= pending.action_finish_time + ignore_delta) {
      qDebug() << "Blocking update because it was received during action processing";
      continue;
    }

    if (last_update_start_time_ <= pending.action_finish_time + ignore_delta
      && last_update_start_time_ >= pending.action_start_time - ignore_delta) {
      qDebug() << "Blocking update because it was requested during action processing";
      continue;
    }

    capabilities_[i] = incoming;
    emit dataUpdated(i);
  }

  if (empty) {
    endResetModel();
  }

  qDebug() << "DeviceModel::OnDeviceInfoReceived" << capabilities_.size();

  const auto top_left = createIndex(0, 0);
  const auto bottom_right = createIndex(capabilities_.size() - 1, 0);

  emit dataChanged(top_left, bottom_right);

  emit dataLoaded();

  timer_.start();
}

void DeviceModel::OnDeviceInfoReceivingFailed(const QString &message) {
  qDebug() << "Devices Model: Error receiving devices:" << message;

  emit dataLoadingFailed();
}

void DeviceModel::OnActionExecutionFinishedSuccessfully(const QVariant &user_data) {
  const int index = user_data.toInt();

  pending_[index].is_pending = false;
  pending_[index].action_finish_time = static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000;

  qDebug() << "Action finished:" << QString::number(index) << "at" << QString::number(
    pending_[index].action_finish_time, 'f', 3
  );

  const auto model_index = createIndex(index, 0);
  emit dataChanged(model_index, model_index);
}

void DeviceModel::OnActionExecutionFailed(const QString &message, const QVariant &user_data) {
  qDebug() << "Devices Model: Error executing action:" << message;

  const int index = user_data.toInt();

  pending_[index].is_pending = false;
  pending_[index].action_finish_time = static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000;

  qDebug() << "Action !finished! :" << QString::number(index) << "at" << QString::number(
    pending_[index].action_finish_time, 'f', 3
  );

  const auto model_index = createIndex(index, 0);
  emit dataChanged(model_index, model_index);
}
