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
  return capabilities_data_.size();
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= capabilities_data_.size()) {
    return {};
  }

  const auto& capability = capabilities_data_.at(index.row()).data;

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
      return capabilities_data_[index.row()].is_pending;
      // return pending_[index.row()].is_pending;
    case StateRole: {
      const auto& c = capabilities_data_.at(index.row()).data;
      qDebug() << "Value for " << index.row() << "=" << c.state["value"];
      return c.state;
    } default:
      return {};
  }
}

QHash<int, QByteArray> DeviceModel::roleNames() const {
  return {
    { IdRole, "deviceId" },
    { NameRole, "name"},
    { DelegateSourceRole, "delegateSource" },
    { BusyRole, "busy"},
    { StateRole, "deviceState" }
  };
}

void DeviceModel::RequestData(const QString& device_id) {
  device_id_ = device_id;

  last_update_start_time_ = static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000;

  qDebug() << "Update requested:" << QString::number(
    last_update_start_time_, 'f', 3
  );

  api_->GetDeviceInfo(device_id_);
}

CapabilityObject DeviceModel::GetCapabilityInfo(const int index) const {
  return capabilities_data_.at(index).data;
}

QVariantMap DeviceModel::GetState(const int index) const {
  const auto& capability = capabilities_data_.at(index).data;

  return capability.state;
}

QVariantMap DeviceModel::GetParameters(const int index) const {
  const auto& capability = capabilities_data_.at(index).data;

  return capability.parameters;
}

void DeviceModel::UseCapability(const int index, const QVariantMap &state) {
  capabilities_data_[index].SetPending(true);
  capabilities_data_[index].SetStartTime();

  auto& capability = capabilities_data_[index].data;
  capability.state = state;

  const auto model_index = createIndex(index, 0);
  emit dataChanged(model_index, model_index);

  qDebug() << "Previously updated:" << QString::number(capability.last_updated, 'f', 3);
  qDebug() << "Requesting update value to" << state["value"];

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

  bool empty = capabilities_data_.empty();

  if (empty) {
    beginResetModel();
  }

  capabilities_data_.resize(info.capabilities.size());

  const double ignore_delta = 0.8;

  for (int i = 0; i < info.capabilities.size(); ++i) {
    auto& capability = capabilities_data_[i];
    const auto& incoming = info.capabilities[i];

    qDebug() << "Updating value for" << i << "=" << incoming.state["value"];

    if (capability.is_pending) {
      qDebug() << "Blocking update because it is processing";
      continue;
    }

    if (capability.IsInside(receive_time, ignore_delta)) {
      qDebug() << "Blocking update because it was received during action processing";
      continue;
    }

    if (capability.IsInside(last_update_start_time_, ignore_delta)) {
      qDebug() << "Blocking update because it was requested during action processing";
      continue;
    }

    capability.data = incoming;

    emit dataUpdated(i);
  }

  if (empty) {
    endResetModel();
  }

  qDebug() << "DeviceModel::OnDeviceInfoReceived" << capabilities_data_.size();

  const auto top_left = createIndex(0, 0);
  const auto bottom_right = createIndex(capabilities_data_.size() - 1, 0);

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

  capabilities_data_[index].SetPending(false);
  capabilities_data_[index].SetFinishTime();

  qDebug() << "Device Model: Action finished at" << QString::number(
    capabilities_data_[index].action_finish_time, 'f', 3);

  const auto model_index = createIndex(index, 0);
  emit dataChanged(model_index, model_index);
}

void DeviceModel::OnActionExecutionFailed(const QString &message, const QVariant &user_data) {
  qDebug() << "Device Model: Error executing action:" << message;

  const int index = user_data.toInt();

  capabilities_data_[index].SetPending(false);
  capabilities_data_[index].SetFinishTime();

  const auto model_index = createIndex(index, 0);
  emit dataChanged(model_index, model_index);

  emit errorOccured(message);
}
