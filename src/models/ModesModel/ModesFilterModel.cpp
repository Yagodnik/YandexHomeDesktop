#include "ModesFilterModel.h"

#include "ModesModel.h"

ModesFilterModel::ModesFilterModel(QObject *parent) : QSortFilterProxyModel(parent) {}

QVariantList ModesFilterModel::GetAllowedModes() const {
  return allowed_modes_;
}

QVariantMap ModesFilterModel::get(int index) const {
  if (index < 0 || index >= rowCount()) {
    return {};
  }

  QModelIndex proxyIndex = this->index(index, 0);
  QModelIndex sourceIndex = mapToSource(proxyIndex);
  const auto& source = sourceModel();

  QVariantMap map;
  map["id"] = source->data(sourceIndex, ModesModel::IdRole);
  map["name"] = source->data(sourceIndex, ModesModel::NameRole);

  return map;
}

void ModesFilterModel::SetAllowedModes(const QVariantList &allowed_modes) {
  allowed_modes_.clear();

  for (const auto &mode : allowed_modes) {
    const auto item = mode.toMap();
    allowed_modes_.append(item.value("value").toString());
  }

  emit allowedModesChanged();
}

bool ModesFilterModel::filterAcceptsRow(const int row, const QModelIndex &parent) const {
  const auto index = sourceModel()->index(row, 0, parent);

  if (!index.isValid()) {
    return false;
  }

  const auto mode_value = sourceModel()->data(index, ModesModel::IdRole).toString();

  return allowed_modes_.contains(mode_value);
}
