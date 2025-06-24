#pragma once

#include <QObject>

#include "model/Capabilites.h"

class CapabilityFactory : public QObject {
  Q_OBJECT
public:
  explicit CapabilityFactory(QObject *parent = nullptr);

  Q_INVOKABLE static QVariantMap CreateOnOff(const CapabilityObject& info, bool value);
  Q_INVOKABLE static QVariantMap CreateRange(const CapabilityObject& info, int value);
  Q_INVOKABLE static QVariantMap CreateColorSetting(const CapabilityObject& info, const QColor& value);
  Q_INVOKABLE static QVariantMap CreateColorSetting(const CapabilityObject& info, int value);
  Q_INVOKABLE static QVariantMap CreateColorSetting(const CapabilityObject& info, const QString& value);
};
