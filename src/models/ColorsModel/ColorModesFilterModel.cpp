#include "ColorModesFilterModel.h"
#include "ColorModesModel.h"

ColorModesFilterModel::ColorModesFilterModel(QObject *parent) : QSortFilterProxyModel(parent ) {}

QVariantList ColorModesFilterModel::GetAllowedScenes() const {
  return allowed_scenes_;
}

void ColorModesFilterModel::SetAllowedScenes(const QVariantList &allowed_scenes) {
  allowed_scenes_ = allowed_scenes;
  emit allowedScenesChanged();
}

bool ColorModesFilterModel::filterAcceptsRow(int row, const QModelIndex &parent) const {
  const auto index = sourceModel()->index(row, 0, parent);

  if (!index.isValid()) {
    return false;
  }

  const auto mode_id = sourceModel()->data(
    index, ColorModesModel::IdRole
  ).toString();

  return std::ranges::any_of(allowed_scenes_, [&mode_id](const auto &scene) {
    const auto& data = scene.toMap();
    return data["id"].toString() == mode_id;
  });
}
