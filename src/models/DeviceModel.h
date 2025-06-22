#pragma once

#include <QAbstractListModel>
#include "api/YandexHomeApi.h"

class DeviceModel : public QAbstractListModel {
  Q_OBJECT
public:
  explicit DeviceModel(YandexHomeApi *api, QObject *parent = nullptr);

  enum Roles {
    IdRole = Qt::UserRole + 1,
    NameRole
  };

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  Q_INVOKABLE void UseCapability(const int index, const QVariant& data) {
    /*
     * 1) Get Capability info (state property)
     * 2) Parse input data
     * 3) Use API
     */
  }

private:
  YandexHomeApi *api_;
  QList<CapabilityObject> capabilities_;
};
