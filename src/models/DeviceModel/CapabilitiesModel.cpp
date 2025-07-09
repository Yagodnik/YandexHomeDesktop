#include "CapabilitiesModel.h"

CapabilitiesModel::CapabilitiesModel(DeviceController *controller, QObject *parent)
  : QAbstractListModel(parent), controller_(controller)
{
  connect(controller_,
    &DeviceController::loadRequestMade,
    this,
    &CapabilitiesModel::ResetModel
  );

  connect(controller_,
    &DeviceController::capabilitiesUpdateReady,
    this,
    &CapabilitiesModel::OnCapabilitiesUpdated
  );
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
      return CapabilityType::operator[](capability.type);
    case DelegateSourceRole: {
      if (kDelegates.contains(capability.type)) {
        return kDelegates[capability.type];
      }

      return kUnsupportedDelegate;
    }
    case AttributeTypeRole:
      qDebug() << "capability";
      return "capability";
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
  qDebug() << "Capabilities Model: Reset due to controller request";

  capabilities_.clear();
  is_initialized_ = false;
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

  qDebug() << "Capabilities Model: UseCapability" << index << "New state:" << state;

  capability.state = state;

  const auto model_index = createIndex(index, 0);
  emit dataChanged(model_index, model_index);

  controller_->UseCapability(index, capability, state);
}

void CapabilitiesModel::OnCapabilitiesUpdated(const DeviceController::CapabilitiesList& capabilities) {
  qDebug() << "Capabilities Model: Updates received from controller";
  qDebug() << "Capabilities count:" << capabilities.size();

  if (!is_initialized_) {
    beginResetModel();
    capabilities_.resize(capabilities.size());
  }

  int empty_count = 0;

  for (auto [capability_var, capability_instance] : std::views::zip(capabilities, capabilities_) ) {
    if (!capability_var.has_value()) {
      empty_count++;
      continue;
    }

    capability_instance = capability_var.value();
  }

  qDebug() << "Capabilities Model: Update applied to" << capabilities_.size() << "capabilities";
  qDebug() << "Capabilities Model: Update contained valid:" << (capabilities.size() - empty_count) << "Empty count:" << empty_count;

  if (!is_initialized_) {
    endResetModel();
    emit initialized();
  }

  is_initialized_ = true;

  const auto top_left = createIndex(0, 0);
  const auto bottom_right = createIndex(capabilities_.size() - 1, 0);

  emit dataChanged(top_left, bottom_right);
  emit dataLoaded();
}
