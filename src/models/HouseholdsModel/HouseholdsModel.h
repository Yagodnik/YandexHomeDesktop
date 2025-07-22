#pragma once

#include <QAbstractListModel>
#include "api/YandexHomeApi.h"

class HouseholdsModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(int count READ GetCount NOTIFY countChanged)
  Q_PROPERTY(
    QString currentHousehold
    READ GetCurrentHousehold
    WRITE SetCurrentHousehold
    NOTIFY currentHouseholdChanged
  )
  Q_PROPERTY(
    QString currentHouseholdName
    READ GetCurrentHouseholdName
    NOTIFY currentHouseholdChanged
  )
public:
  explicit HouseholdsModel(YandexHomeApi *api, QObject *parent = nullptr);

  enum Roles {
    IdRole = Qt::UserRole + 1,
    NameRole
  };

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  [[nodiscard]] int GetCount() const;
  [[nodiscard]] QString GetCurrentHousehold() const;
  [[nodiscard]] QString GetCurrentHouseholdName() const;

  Q_INVOKABLE void RequestData();

public slots:
  void SetCurrentHousehold(const QString& value);

signals:
  void countChanged();
  void currentHouseholdChanged();
  void dataLoaded();
  void dataLoadingFailed();

private:
  std::optional<QString> current_;
  QList<HouseholdObject> households_;
  YandexHomeApi *api_;

private slots:
  void OnUserInfoReceived(const UserInfo& info);
  void OnUserInfoReceivingFailed(const QString& message);
};
