#include "ColorModesModel.h"

#include "utils/JsonLoader.h"

ColorModesModel::ColorModesModel(QObject *parent) : QAbstractListModel(parent) {
  const auto& modes_file = JsonLoader::Load<ColorModesFile>(kModesFile);

  if (!modes_file.has_value()) {
    qWarning() << "ColorModesModel: Failed to load colors file";
    return;
  }

  color_modes_ = modes_file.value().color_modes;
}

int ColorModesModel::rowCount(const QModelIndex &parent) const {
  return color_modes_.size();
}

QVariant ColorModesModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= color_modes_.size()) {
    return {};
  }

  const auto& mode = color_modes_.at(index.row());

  switch (role) {
    case IdRole:
      return mode.id;
    case NameRole:
      return mode.name;
    case ImageRole:
      return kImagePrefix + mode.id + ".svg";
    default:
      return {};
  }
}

QHash<int, QByteArray> ColorModesModel::roleNames() const {
  return {
    { IdRole, "modeId" },
    { NameRole, "name" },
    { ImageRole, "image" }
  };
}
