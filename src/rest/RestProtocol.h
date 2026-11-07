#pragma once

#include "yh/apprest_export.h"

#include <QtGlobal>

namespace RestProtocol {

inline constexpr int Version = 1;
inline constexpr int DefaultTimeoutMs = 30'000;
inline constexpr qsizetype MaxBodyBytes = 64 * 1024;

} // namespace RestProtocol
