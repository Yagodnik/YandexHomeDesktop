#pragma once

#include <QAbstractListModel>

#include "api/YandexHomeApi.h"
#include "api/model/UserInfo.h"

class PropertiesModel : public QAbstractListModel {
  Q_OBJECT
public:
  explicit PropertiesModel(YandexHomeApi* api, QObject* parent = nullptr);

  enum Roles {
    IdRole = Qt::UserRole + 1,
    NameRole,
    StateRole,
    ParametersRole
  };

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

signals:
  void dataLoaded();
  void dataLoadingFailed();

private:
  YandexHomeApi *api_;
  QList<PropertyObject> properties_;
  bool is_initialized_ = false;

private slots:
  void OnDeviceInfoReceived(const DeviceObject& info);
  void OnDeviceInfoReceivingFailed(const QString& message);
};

