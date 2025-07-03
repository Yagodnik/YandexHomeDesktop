#pragma once

#include "ICapability.h"

class ColorSettingCapability : public ICapability {
  Q_OBJECT
  Q_PROPERTY(
    int temperatureMin
    READ GetTemperatureMin
    WRITE SetTemperatureMin
    NOTIFY temperatureMinChanged
  )
  Q_PROPERTY(
    int temperatureMax
    READ GetTemperatureMax
    WRITE SetTemperatureMax
    NOTIFY temperatureMaxChanged
  )
  Q_PROPERTY(
    QVariantList availableScenes
    READ GetAvailableScenes
    WRITE SetAvailableScenes
    NOTIFY availableScenesChanged
  )
  Q_PROPERTY(bool supportsColors READ GetSupportsColors NOTIFY supportsColorsChanged)
  Q_PROPERTY(bool supportsTemperatures READ GetSupportsTemperature NOTIFY supportsTemperatureChanged)
public:
  explicit ColorSettingCapability(QObject *parent = nullptr);

  void SetValue(const QVariant& value) override;
  QVariant GetValue() const override;

  Q_INVOKABLE [[nodiscard]] QVariantMap Create(const QColor& value);
  Q_INVOKABLE [[nodiscard]] QVariantMap Create(int value);
  Q_INVOKABLE [[nodiscard]] QVariantMap Create(const QString& value);

  [[nodiscard]] int GetTemperatureMin() const;
  [[nodiscard]] int GetTemperatureMax() const;
  [[nodiscard]] QVariantList GetAvailableScenes() const;
  [[nodiscard]] bool GetSupportsColors() const;
  [[nodiscard]] bool GetSupportsTemperature() const;

  void SetParameters(const QVariantMap& parameters) override;

signals:
  void temperatureMinChanged();
  void temperatureMaxChanged();
  void availableScenesChanged();

  void supportsColorsChanged();
  void supportsTemperatureChanged();

public slots:
  void SetTemperatureMin(int value);
  void SetTemperatureMax(int value);
  void SetAvailableScenes(const QVariantList& value);
};

