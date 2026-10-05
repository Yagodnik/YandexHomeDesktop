#pragma once

#include <QObject>
#include <QVariant>

class UnitsList : public QObject {
  Q_OBJECT
public:
  explicit UnitsList(QObject *parent = nullptr);

  [[nodiscard]] QString GetUnit(const QString &unit_name) const;

private:
  QVariantMap units_;
};
