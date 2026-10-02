#pragma once

#include <expected>
#include <functional>
#include <QString>

enum class ApiErrorKind {
  Network,
  Timeout,
  Http,
  InvalidResponse,
  Service
};

struct ApiError {
  ApiErrorKind kind;
  QString message;
  int http_status = 0;
};

template<typename T>
using ApiResult = std::expected<T, ApiError>;

// A request can report more than one result to retain the current timeout and
// per-capability action behavior. Each result belongs only to its request.
template<typename T>
using ApiResultHandler = std::function<void(ApiResult<T>)>;
