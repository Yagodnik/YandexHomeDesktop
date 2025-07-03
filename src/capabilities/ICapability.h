#pragma once

#include <QVariant>
#include "utils/TitlesList.h"

class ICapability : public QObject {
  Q_OBJECT
  Q_PROPERTY(QVariantMap state READ GetState WRITE SetState NOTIFY stateChanged)
  Q_PROPERTY(QVariantMap parameters READ GetParameters WRITE SetParameters NOTIFY parametersChanged)
  Q_PROPERTY(QVariant value READ GetValue WRITE SetValue NOTIFY valueChanged)
  Q_PROPERTY(QString title READ GetCapabilityTitle NOTIFY capabilityTitleChanged)
  Q_PROPERTY(TitlesList* titlesList MEMBER titles_list_)
public:
  explicit ICapability(const QString& capability_name, QObject *parent = nullptr);

  virtual QVariantMap GetState() const;
  virtual QVariantMap GetParameters() const;

  virtual void SetValue(const QVariant& value) = 0;

  [[nodiscard]] QString GetCapabilityTitle() const;

public slots:
  virtual void SetState(const QVariantMap& state);
  virtual void SetParameters(const QVariantMap& parameters);
  virtual QVariant GetValue() const = 0;

signals:
  void stateChanged();
  void parametersChanged();
  void valueChanged();
  void capabilityTitleChanged();

protected:
  QString capability_name_;
  QVariantMap parameters_;
  QVariantMap state_;

  TitlesList *titles_list_{nullptr};
};
