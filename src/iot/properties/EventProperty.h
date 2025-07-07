#pragma once

#include "../common/IotObject.h"
#include "utils/TitlesProvider.h"

class EventProperty : public IotObject {
  Q_OBJECT
  Q_PROPERTY(
    TitlesProvider* valuesTitles
    READ GetValuesProvider
    WRITE SetValuesProvider
    NOTIFY valuesProviderChanged
  )
public:
  explicit EventProperty(QObject* parent = nullptr);

  void SetValue(const QVariant &value) override;

  QVariant GetValue() const override;
  QString GetFormattedValue() const override;

  TitlesProvider* GetValuesProvider() const;

signals:
  void valuesProviderChanged();

public slots:
  void SetValuesProvider(TitlesProvider* values_provider);

private:
  TitlesProvider* values_provider_{nullptr};
};
