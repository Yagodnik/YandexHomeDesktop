#pragma once

#include <QSortFilterProxyModel>

class ColorsFilterModel : public QSortFilterProxyModel {
  Q_OBJECT
  Q_PROPERTY(int min READ GetMin WRITE SetMin NOTIFY minChanged)
  Q_PROPERTY(int max READ GetMax WRITE SetMax NOTIFY maxChanged)
public:
  explicit ColorsFilterModel(QObject *parent = nullptr);

  void SetMin(int min);
  void SetMax(int max);

public slots:
  [[nodiscard]] int GetMin() const;
  [[nodiscard]] int GetMax() const;

signals:
  void minChanged();
  void maxChanged();

protected:
  [[nodiscard]] bool filterAcceptsRow(int row, const QModelIndex &parent) const override;
  [[nodiscard]] bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

  int min_;
  int max_;
};

