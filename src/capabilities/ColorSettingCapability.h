#pragma once

#include "ICapability.h"

class ColorSettingCapability : public ICapability {
  Q_OBJECT
public:
  explicit ColorSettingCapability(QObject *parent = nullptr);

  void SetValue(const QVariant& value) override;
  QVariant GetValue() const override;

  Q_INVOKABLE [[nodiscard]] QVariantMap Create(const QColor& value);
  Q_INVOKABLE [[nodiscard]] QVariantMap Create(int value);
  Q_INVOKABLE [[nodiscard]] QVariantMap Create(const QString& value);

  Q_INVOKABLE [[nodiscard]] int GetTemperatureMin() const;
  Q_INVOKABLE [[nodiscard]] int GetTemperatureMax() const;
  Q_INVOKABLE [[nodiscard]] QVariantList GetAvailableScenes() const;
};

