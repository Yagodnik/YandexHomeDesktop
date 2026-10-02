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

// Each request reports one result to its owning context.
template<typename T>
using ApiResultHandler = std::function<void(ApiResult<T>)>;
