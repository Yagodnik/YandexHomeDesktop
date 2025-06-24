#pragma once

#include <QObject>
#include <QMap>

class Router : public QObject {
  Q_OBJECT
public:
  explicit Router(QObject *parent = nullptr);

  Q_INVOKABLE void setView(QObject *view);
  Q_INVOKABLE void addRoute(const QString& name, const QString& path);
  Q_INVOKABLE void navigateTo(const QString& name) const;
  Q_INVOKABLE void goBack() const;

private:
  QObject *view_;
  QMap<QString, QString> routes_;

signals:
  void currentRouteChanged();
};
