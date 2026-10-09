#pragma once

#include "yh/apprest_export.h"

#include <functional>
#include <optional>

#include <QHash>
#include <QHttpServerRequest>
#include <QList>

class RestReply;

class APPREST_EXPORT RestRouter final {
public:
  using Parameters = QHash<QString, QString>;
  using Handler = std::function<void(const QHttpServerRequest&, const Parameters&, RestReply*)>;

  void Add(QHttpServerRequest::Method method, const QString& pattern, Handler handler);
  void Handle(const QHttpServerRequest& request, RestReply* reply) const;

private:
  struct Segment {
    QString literal;
    QString parameter;
  };

  struct Route {
    QHttpServerRequest::Method method;
    QList<Segment> segments;
    Handler handler;

    std::optional<Parameters> Match(const QStringList& path) const;
  };

  QList<Route> routes_;
};
