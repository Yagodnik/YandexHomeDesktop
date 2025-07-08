#include "CapabilitiesModel.h"

CapabilitiesModel::CapabilitiesModel(YandexHomeApi *api, QObject *parent)
  : QAbstractListModel(parent), api_(api)
{
  connect(api_,
    &YandexHomeApi::deviceInfoReceived,
    this,
    &CapabilitiesModel::OnDeviceInfoReceived);

  connect(api_,
    &YandexHomeApi::deviceInfoReceivingFailed,
    this,
    &CapabilitiesModel::OnDeviceInfoReceivingFailed);

  connect(api_,
    &YandexHomeApi::actionExecutingFinishedSuccessfully,
    this,
    &CapabilitiesModel::OnActionExecutionFinishedSuccessfully);

  connect(api_,
    &YandexHomeApi::actionExecutingFailed,
    this,
    &CapabilitiesModel::OnActionExecutionFailed);

  timer_.setInterval(3000);
  timer_.setSingleShot(false);

  connect(&timer_, &QTimer::timeout, [this]() {
    RequestUpdate();
  });
}

void CapabilitiesModel::ResetModel(const QString& device_id) {
  beginResetModel();

  device_id_ = device_id;
  capabilities_.clear();
  is_initialized_ = false;

  endResetModel();
}

int CapabilitiesModel::rowCount(const QModelIndex &parent) const {
  return capabilities_.size();
}

QVariant CapabilitiesModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= capabilities_.size()) {
    return {};
  }

  const auto& capability = capabilities_.at(index.row());

  switch (role) {
    case IdRole:
      return device_id_;
    case NameRole:
      return capability.name;
    case DelegateSourceRole: {
      if (kDelegates2.contains(capability.name)) {
        return kDelegates2[capability.name];
      }

      return kUnsupportedDelegate;
    }
    case AttributeTypeRole:
      switch (capability.type) {
        case DeviceAttribute::Capability:
          qDebug() << "capability";
          return "capability";
        case DeviceAttribute::Property:
          qDebug() << "property";
          return "property";
        default:
          return {};
      }
    case BusyRole:
      return capability.is_pending;
    case StateRole:
      return capability.state;
    case ParametersRole:
      return capability.parameters;
    default:
      return {};
  }
}

QHash<int, QByteArray> CapabilitiesModel::roleNames() const {
  return {
    { IdRole, "deviceId" },
    { NameRole, "name"},
    { DelegateSourceRole, "delegateSource" },
    { AttributeTypeRole, "attributeType" },
    { BusyRole, "busy"},
    { StateRole, "attributeState" },
    { ParametersRole, "attributeParameters" }
  };
}

void CapabilitiesModel::ResetModel() {
  capabilities_.clear();
  is_initialized_ = false;
}

void CapabilitiesModel::RequestData(const QString& device_id) {
  device_id_ = device_id;

  ResetModel();

  RequestUpdate();
}

void CapabilitiesModel::RequestUpdate() {
  last_update_start_time_ = static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000;
  api_->GetDeviceInfo(device_id_);
}

QVariantMap CapabilitiesModel::GetState(const int index) const {
  const auto& capability = capabilities_.at(index);

  return capability.state;
}

QVariantMap CapabilitiesModel::GetParameters(const int index) const {
  const auto& capability = capabilities_.at(index);

  return capability.parameters;
}

void CapabilitiesModel::UseCapability(const int index, const QVariantMap &state) {
  auto& capability = capabilities_[index];

  if (capability.type != DeviceAttribute::Capability) {
    qDebug() << "DeviceModel::UseCapability: Invalid type";
    return;
  }

  capabilities_[index].PausePolling();
  // controller_->PausePolling(index);

  capability.state = state;

  const auto model_index = createIndex(index, 0);
  emit dataChanged(model_index, model_index);

  // controller->UseCapability(index, capability, state);

  const CapabilityObject action = {
    .type = CapabilityType::operator[](capability.name),
    .state = state
  };

  const DeviceActionsObject action_object = {
    .id = device_id_,
    .actions = { action }
  };

  api_->PerformActions({
    action_object
  }, index);
}

void CapabilitiesModel::OnDeviceInfoReceived(const DeviceInfo &info) {
  const double receive_time = static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000;

  if (!is_initialized_) {
    beginResetModel();
    capabilities_.resize(info.capabilities.size());
  }

  constexpr double ignore_delta = 0.8;

  const auto blocking_condition = [this, receive_time](const auto& attr) -> bool {
    return attr.is_pending ||
           attr.IsInsideInterval(receive_time, ignore_delta) ||
           attr.IsInsideInterval(last_update_start_time_, ignore_delta);
  };

  for (auto [attr, incoming] : std::ranges::views::zip(capabilities_, info.capabilities)) {
    if (blocking_condition(attr)) {
      continue;
    }

    attr.UpdateFrom(incoming);
  }

  if (!is_initialized_) {
    endResetModel();
    emit initialized();
  }

  is_initialized_ = true;

  const auto top_left = createIndex(0, 0);
  const auto bottom_right = createIndex(capabilities_.size() - 1, 0);

  emit dataChanged(top_left, bottom_right);
  emit dataLoaded();

  timer_.start();
}

void CapabilitiesModel::OnDeviceInfoReceivingFailed(const QString &message) {
  qDebug() << "Devices Model: Error receiving devices:" << message;

  emit dataLoadingFailed();
}

void CapabilitiesModel::OnActionExecutionFinishedSuccessfully(const QVariant &user_data) {
  const int index = user_data.toInt();

  capabilities_[index].ResumePolling();

  qDebug() << "Device Model: Action finished at" << QString::number(
    capabilities_[index].polling_finish_time, 'f', 3);

  const auto model_index = createIndex(index, 0);
  emit dataChanged(model_index, model_index);
}

void CapabilitiesModel::OnActionExecutionFailed(const QString &message, const QVariant &user_data) {
  qDebug() << "Device Model: Error executing action:" << message;

  const int index = user_data.toInt();

  capabilities_[index].ResumePolling();

  const auto model_index = createIndex(index, 0);
  emit dataChanged(model_index, model_index);

  emit errorOccurred(message);
}
