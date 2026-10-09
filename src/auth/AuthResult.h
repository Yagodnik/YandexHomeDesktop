#pragma once

#include <QString>
#include <expected>
#include <functional>

enum class AuthErrorKind { NotFound, Canceled, Storage, Initialization, Authorization };

struct AuthError {
  AuthErrorKind kind;
  quint32 code = 0;
  QString message;
};

template<typename T>
using AuthResult = std::expected<T, AuthError>;
template<typename T>
using AuthResultHandler = std::function<void(AuthResult<T>)>;
