#pragma once

#include <QAbstractListModel>

#include "DeviceController.h"
#include "api/YandexHomeApi.h"
#include "api/model/UserInfo.h"

// #define ALLOW_FAKE_PROPETIES
#if defined(QT_NO_DEBUG) && defined(ALLOW_FAKE_PROPETIES)
#error "Propeties Model: Building with fake data in release mode!"
#else
#warning "Propeties Model: Building with fake data in release mode!"
#endif

class PropertiesModel : public QAbstractListModel {
  Q_OBJECT
public:
  explicit PropertiesModel(DeviceController* controller, QObject* parent = nullptr);

  enum Roles {
    IdRole = Qt::UserRole + 1,
    DelegateSourceRole,
    NameRole,
    StateRole,
    ParametersRole
  };

  void ResetModel();

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

signals:
  void dataLoaded();
  void dataLoadingFailed();
  void initialized();

private:
  DeviceController* controller_;
  QList<PropertyObject> properties_;
  bool is_initialized_ = false;

private slots:
  void OnPropertiesUpdateReady(const QVariantList& properties);
};

