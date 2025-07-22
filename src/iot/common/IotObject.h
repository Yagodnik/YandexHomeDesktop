#pragma once

#include <QVariant>
#include "utils/TitlesProvider.h"
#include "utils/UnitsList.h"

class IotObject : public QObject {
  Q_OBJECT
  Q_PROPERTY(QVariantMap state READ GetState WRITE SetState NOTIFY stateChanged)
  Q_PROPERTY(QVariantMap parameters READ GetParameters WRITE SetParameters NOTIFY parametersChanged)
  Q_PROPERTY(QVariant value READ GetValue WRITE SetValue NOTIFY valueChanged)
  Q_PROPERTY(QString title READ GetTitle NOTIFY titleChanged)
  Q_PROPERTY(QString instance READ GetInstance NOTIFY instanceChanged)
  Q_PROPERTY(TitlesProvider* titlesList READ GetTitlesProvider WRITE SetTitlesProvider NOTIFY titlesProviderChanged)
  Q_PROPERTY(QString formattedValue READ GetFormattedValue NOTIFY formattedValueChanged)
public:
  explicit IotObject(const QString& name, QObject *parent = nullptr);

  virtual QVariantMap GetState() const;
  virtual QVariantMap GetParameters() const;

  virtual void SetValue(const QVariant& value) = 0;

  [[nodiscard]] QString GetTitle() const;
  [[nodiscard]] QString GetInstance() const;

  TitlesProvider* GetTitlesProvider() const;
  void SetTitlesProvider(TitlesProvider* provider);

public slots:
  virtual void SetState(const QVariantMap& state);
  virtual void SetParameters(const QVariantMap& parameters);
  virtual QVariant GetValue() const = 0;
  virtual QString GetFormattedValue() const;

signals:
  void stateChanged();
  void parametersChanged();
  void valueChanged();
  void titleChanged();
  void instanceChanged();
  void formattedValueChanged();
  void titlesProviderChanged();

protected:
  QString name_;
  QVariantMap parameters_;
  QVariantMap state_;

  TitlesProvider *titles_provider_{nullptr};

  void SetStateValue(const QVariant& value);
};
