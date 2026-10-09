#include "RestRouter.h"

#include <QCoreApplication>

#include "RestProtocol.h"
#include "RestReply.h"

std::optional<RestRouter::Parameters> RestRouter::Route::Match(const QStringList& path) const {
  if (segments.size() != path.size()) {
    return std::nullopt;
  }

  Parameters parameters;
  for (qsizetype index = 0; index < segments.size(); ++index) {
    const auto& segment = segments[index];
    if (!segment.parameter.isEmpty()) {
      parameters.insert(segment.parameter, path[index]);
    } else if (segment.literal != path[index]) {
      return std::nullopt;
    }
  }
  return parameters;
}

void RestRouter::Add(QHttpServerRequest::Method method, const QString& pattern, Handler handler) {
  Route route{method, {}, std::move(handler)};
  for (const auto& part : pattern.split('/', Qt::SkipEmptyParts)) {
    if (part.startsWith('{') && part.endsWith('}')) {
      route.segments.append({{}, part.sliced(1).chopped(1)});
    } else {
      route.segments.append({part, {}});
    }
  }
  routes_.append(std::move(route));
}

void RestRouter::Handle(const QHttpServerRequest& request, RestReply* reply) const {
  // Split before decoding so an encoded slash remains part of an entity ID.
  auto path = request.url().path(QUrl::FullyEncoded).split('/', Qt::SkipEmptyParts);
  for (auto& part : path) {
    part = QUrl::fromPercentEncoding(part.toUtf8());
  }

  bool path_found = false;
  for (const auto& route : routes_) {
    const auto parameters = route.Match(path);
    if (!parameters) {
      continue;
    }
    path_found = true;
    if (request.method() != route.method) {
      continue;
    }
    if (request.body().size() > RestProtocol::MaxBodyBytes) {
      reply->Fail(RestReply::StatusCode::PayloadTooLarge, "body_too_large",
        QCoreApplication::translate("RestServer", "Тело запроса слишком большое."));
      return;
    }
    route.handler(request, *parameters, reply);
    return;
  }

  if (path_found) {
    reply->Fail(RestReply::StatusCode::MethodNotAllowed, "method_not_allowed",
      QCoreApplication::translate("RestServer", "Метод не разрешён."));
  } else {
    reply->Fail(RestReply::StatusCode::NotFound, "not_found",
      QCoreApplication::translate("RestServer", "Маршрут не найден."));
  }
}
