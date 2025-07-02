#pragma once

#include <QSortFilterProxyModel>

class ModesFilterModel : public QSortFilterProxyModel {
  Q_OBJECT
  Q_PROPERTY(QVariantList allowedModes READ GetAllowedModes WRITE SetAllowedModes NOTIFY allowedModesChanged)
public:
  explicit ModesFilterModel(QObject *parent = nullptr);

  [[nodiscard]] QVariantList GetAllowedModes() const;
  Q_INVOKABLE QVariantMap get(int index) const;

public slots:
  void SetAllowedModes(const QVariantList &allowed_modes);

signals:
  void allowedModesChanged();

protected:
  [[nodiscard]] bool filterAcceptsRow(int row, const QModelIndex &parent) const override;

private:
  QVariantList allowed_modes_;
};
