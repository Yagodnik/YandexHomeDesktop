#pragma once

#include <QSortFilterProxyModel>

class DeviceFilterModel : public QSortFilterProxyModel {
  Q_OBJECT
  Q_PROPERTY(QString targetType
             READ GetTargetType
             WRITE SetTargetType
             NOTIFY targetTypeChanged)
public:
  explicit DeviceFilterModel(QObject *parent = nullptr);

  [[nodiscard]] QString GetTargetType() const;

public slots:
  void SetTargetType(const QString& type);

signals:
  void targetTypeChanged();

protected:
  [[nodiscard]] bool filterAcceptsRow(int row, const QModelIndex &parent) const override;

private:
  QString target_type_;
};
