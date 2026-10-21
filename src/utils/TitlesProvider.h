#pragma once

#include <QByteArray>
#include <QObject>
#include <QJsonObject>

class TitlesProvider : public QObject {
  Q_OBJECT
public:
  explicit TitlesProvider(const QString& path, const char *translationContext, QObject *parent = nullptr);

  [[nodiscard]] QString GetTitle(const QString& name, const QString& instance) const;

private:
  QJsonObject raw_data_;
  QByteArray translation_context_;
};
