#include "ModesModel.h"
#include "utils/JsonLoader.h"

ModesModel::ModesModel(QObject *parent) : QAbstractListModel(parent) {
  const auto modes_list_opt = JsonLoader::Load<ModesList>(kModesListFile);
  if (!modes_list_opt.has_value()) {
    qDebug() << "Failed to load modes list";
    return;
  }

  modes_ = modes_list_opt.value().modes;
}

int ModesModel::rowCount(const QModelIndex &parent) const {
  return modes_.size();
}

QVariant ModesModel::data(const QModelIndex &index, int role) const {
  if (index.row() < 0 || index.row() >= modes_.size()) {
    return {};
  }

  const Mode &mode = modes_[index.row()];

  switch (role) {
    case IdRole:
      return mode.id;
    case NameRole:
      return mode.name;
    default:
      return {};
  }
}

QHash<int, QByteArray> ModesModel::roleNames() const {
  return {
    { IdRole, "modeId" },
    { NameRole, "displayText" }
  };
}

QVariant ModesModel::getItem(int index) const {
  if (index < 0 || index >= rowCount()) {
    return {};
  }

  return QVariant::fromValue(modes_.at(index));
}
