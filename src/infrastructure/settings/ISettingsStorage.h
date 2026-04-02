#pragma once

#include <QObject>

class ISettingsStorage : public QObject {
  Q_OBJECT
public:
  explicit ISettingsStorage(QObject* parent = nullptr) : QObject(parent) {}

  Q_INVOKABLE virtual void SaveList(const QString& name, const QStringList& values) = 0;
  Q_INVOKABLE virtual QStringList GetList(const QString& name) = 0;
};
