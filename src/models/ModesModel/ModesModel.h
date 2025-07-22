#pragma once

#include <QAbstractListModel>

#include "serialization/Serialization.h"

class ModesModel : public QAbstractListModel {
  Q_OBJECT
public:
  explicit ModesModel(QObject *parent = nullptr);

  enum Roles {
    IdRole = Qt::UserRole + 1,
    NameRole
  };

  [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
  Q_INVOKABLE [[nodiscard]] QVariant getItem(int index) const;

private:
  JSON_STRUCT(Mode,
    (QString, id),
    (QString, name)
  );

  JSON_STRUCT(ModesList,
    (QList<Mode>, modes)
  );

  QList<Mode> modes_;

  const QString kModesListFile = ":/data/modes.json";
};

