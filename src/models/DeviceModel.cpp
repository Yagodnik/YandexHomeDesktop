#include "DeviceModel.h"

int DeviceModel::rowCount(const QModelIndex &parent) const {
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const {
}

QHash<int, QByteArray> DeviceModel::roleNames() const {
  return QAbstractListModel::roleNames();
}
