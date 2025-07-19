#include "Router.h"
#include <QDebug>
#include <QUrl>
#include <QVariant>

Router::Router(QObject *parent)
  : QObject(parent), view_(nullptr) {
}

void Router::setView(QObject *view) {
  view_ = view;
}

void Router::addRoute(const QString &name, const QString &path) {
  routes_.insert(name, path);
}

void Router::navigateTo(const QString &name) const {
  if (!view_) {
    qCritical() << "Router: No view!";
    return;
  }

  if (!routes_.contains(name)) {
    qCritical() << "No route for name: " << name;
    return;
  }

  QVariant arg = QVariant::fromValue(routes_[name]);
  const bool invoked = QMetaObject::invokeMethod(
    view_, "push", Q_ARG(QVariant, arg)
  );

  if (!invoked) {
    qCritical() << "Failed to invoke push on StackView";
  }
}

void Router::goBack() const {
  if (!view_) {
    qCritical() << "Router: No view!";
    return;
  }

  QMetaObject::invokeMethod(view_, "pop");
}
