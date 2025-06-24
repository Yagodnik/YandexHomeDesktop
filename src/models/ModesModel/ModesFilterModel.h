#pragma once

#include <QSortFilterProxyModel>
#include <QVariantList>

class ModesFilterModel : public QSortFilterProxyModel {
  Q_OBJECT
  Q_PROPERTY(QVariantList allowedScenes READ GetAllowedScenes WRITE SetAllowedScenes NOTIFY allowedScenesChanged)
public:
  explicit ModesFilterModel(QObject *parent = nullptr);

  [[nodiscard]] QVariantList GetAllowedScenes() const;

public slots:
  void SetAllowedScenes(const QVariantList &allowed_scenes);

signals:
  void allowedScenesChanged();

protected:
  [[nodiscard]] bool filterAcceptsRow(int row, const QModelIndex &parent) const override;

  QVariantList allowed_scenes_;
};
