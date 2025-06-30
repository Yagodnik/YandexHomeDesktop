#include "DeviceModel.h"

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
    RequestUpdate();
  });
}

int DeviceModel::rowCount(const QModelIndex &parent) const {
  return attributes_.size();
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= attributes_.size()) {
    return {};
  }

  const auto& capability = attributes_.at(index.row());

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
          return "capability";
        case DeviceAttribute::Property:
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

QHash<int, QByteArray> DeviceModel::roleNames() const {
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

void DeviceModel::RequestData(const QString& device_id) {
  device_id_ = device_id;

  attributes_.clear();
  is_initialized_ = false;

  RequestUpdate();
}

void DeviceModel::RequestUpdate() {
  last_update_start_time_ = static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000;
  api_->GetDeviceInfo(device_id_);
}

QVariantMap DeviceModel::GetState(const int index) const {
  const auto& capability = attributes_.at(index);

  return capability.state;
}

QVariantMap DeviceModel::GetParameters(const int index) const {
  const auto& capability = attributes_.at(index);

  return capability.parameters;
}

void DeviceModel::UseCapability(const int index, const QVariantMap &state) {
  auto& capability = attributes_[index];

  if (capability.type != DeviceAttribute::Capability) {
    qDebug() << "DeviceModel::UseCapability: Invalid type";
    return;
  }

  attributes_[index].PausePolling();

  capability.state = state;

  const auto model_index = createIndex(index, 0);
  emit dataChanged(model_index, model_index);

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

void DeviceModel::OnDeviceInfoReceived(const DeviceObject &info2) {
  const double receive_time = static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000;

  DeviceObject info = info2;

  // TODO: Add property manually

  QString pseudo_property_data_str = R"(
    {
        "type": "devices.properties.float",
        "retrievable": true,
        "parameters": {
            "instance": "humidity",
            "unit": "unit.percent"
        }
        "state": {
            "instance": "humidity",
            "value": 55
        }
    }
  )";

  const QJsonObject test_object = QJsonDocument::fromJson(pseudo_property_data_str.toUtf8()).object();
  info.properties.push_back(Serialization::From<PropertyObject>(test_object));

  if (!is_initialized_) {
    beginResetModel();
    attributes_.resize(info.capabilities.size() + info.properties.size());
  }

  constexpr double ignore_delta = 0.8;

  const auto blocking_condition = [this, receive_time](const auto& attr) -> bool {
    return attr.is_pending ||
           attr.IsInsideInterval(receive_time, ignore_delta) ||
           attr.IsInsideInterval(last_update_start_time_, ignore_delta);
  };

  for (auto [attr, incoming] : std::ranges::views::zip(attributes_, info.capabilities)) {
    if (blocking_condition(attr)) {
      continue;
    }

    attr.UpdateFrom(incoming);
  }

  if (!is_initialized_) {
    endResetModel();
  }

  is_initialized_ = true;

  const auto top_left = createIndex(0, 0);
  const auto bottom_right = createIndex(attributes_.size() - 1, 0);

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

  attributes_[index].ResumePolling();

  qDebug() << "Device Model: Action finished at" << QString::number(
    attributes_[index].polling_finish_time, 'f', 3);

  const auto model_index = createIndex(index, 0);
  emit dataChanged(model_index, model_index);
}

void DeviceModel::OnActionExecutionFailed(const QString &message, const QVariant &user_data) {
  qDebug() << "Device Model: Error executing action:" << message;

  const int index = user_data.toInt();

  attributes_[index].ResumePolling();

  const auto model_index = createIndex(index, 0);
  emit dataChanged(model_index, model_index);

  emit errorOccurred(message);
}
